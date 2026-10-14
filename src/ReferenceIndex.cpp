#include "../include/ReferenceIndex.h"
#include "../include/DocumentLoader.h"
#include <fstream>
#include <filesystem>
#include <iostream>

namespace docutrace {

ReferenceIndex::ReferenceIndex(const std::string& indexDir)
    : indexDirectory_(indexDir), metadataFilePath_(indexDir + "/index_cache.json"), textProcessor_(3) {
    std::filesystem::create_directories(indexDirectory_);
    loadFromDisk();
}

bool ReferenceIndex::addDocument(const Document& doc) {
    std::lock_guard<std::mutex> lock(indexMutex_);

    Document processedDoc = doc;
    if (processedDoc.getStatus() == ProcessingStatus::UNPROCESSED ||
        processedDoc.getStatus() == ProcessingStatus::EXTRACTED) {
        textProcessor_.processDocument(processedDoc);
    }

    std::string docId = processedDoc.getId();
    documents_[docId] = processedDoc;

    // Update inverted index
    for (const auto& token : processedDoc.getTokens()) {
        if (!textProcessor_.isStopWord(token.text)) {
            invertedIndex_[token.text].insert(docId);
        }
    }

    // Update n-gram inverted index
    for (const auto& window : processedDoc.getPassageWindows()) {
        for (const auto& ngram : window.ngrams) {
            ngramInvertedIndex_[ngram].insert(docId);
        }
    }

    saveToDisk();
    return true;
}

bool ReferenceIndex::removeDocument(const std::string& docId) {
    std::lock_guard<std::mutex> lock(indexMutex_);

    auto it = documents_.find(docId);
    if (it == documents_.end()) return false;

    // Clean up inverted indices
    for (auto& pair : invertedIndex_) {
        pair.second.erase(docId);
    }
    for (auto& pair : ngramInvertedIndex_) {
        pair.second.erase(docId);
    }

    documents_.erase(it);
    saveToDisk();
    return true;
}

bool ReferenceIndex::hasDocument(const std::string& docId) const {
    std::lock_guard<std::mutex> lock(indexMutex_);
    return documents_.find(docId) != documents_.end();
}

Document ReferenceIndex::getDocument(const std::string& docId) const {
    std::lock_guard<std::mutex> lock(indexMutex_);
    auto it = documents_.find(docId);
    if (it == documents_.end()) {
        throw IndexException("Document ID not found in index: " + docId, docId);
    }
    return it->second;
}

std::vector<Document> ReferenceIndex::getAllDocuments() const {
    std::lock_guard<std::mutex> lock(indexMutex_);
    std::vector<Document> docs;
    docs.reserve(documents_.size());
    for (const auto& pair : documents_) {
        docs.push_back(pair.second);
    }
    return docs;
}

size_t ReferenceIndex::size() const {
    std::lock_guard<std::mutex> lock(indexMutex_);
    return documents_.size();
}

std::vector<std::string> ReferenceIndex::search(const std::string& queryText, size_t limit) const {
    std::lock_guard<std::mutex> lock(indexMutex_);
    std::vector<std::string> results;

    std::unordered_map<std::string, size_t> matchCounts;
    Tokenizer tok;
    auto tokens = tok.tokenize(queryText);

    for (const auto& t : tokens) {
        auto it = invertedIndex_.find(t.text);
        if (it != invertedIndex_.end()) {
            for (const auto& docId : it->second) {
                matchCounts[docId]++;
            }
        }
    }

    // Sort by match count descending
    std::vector<std::pair<std::string, size_t>> sorted(matchCounts.begin(), matchCounts.end());
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        return a.second > b.second;
    });

    for (size_t i = 0; i < sorted.size() && i < limit; ++i) {
        results.push_back(sorted[i].first);
    }

    return results;
}

std::vector<std::string> ReferenceIndex::findCandidateDocuments(const Document& queryDoc, size_t minOverlap) const {
    std::lock_guard<std::mutex> lock(indexMutex_);
    std::unordered_map<std::string, size_t> candidateScores;

    for (const auto& t : queryDoc.getTokens()) {
        if (textProcessor_.isStopWord(t.text)) continue;

        auto it = invertedIndex_.find(t.text);
        if (it != invertedIndex_.end()) {
            for (const auto& docId : it->second) {
                candidateScores[docId]++;
            }
        }
    }

    std::vector<std::string> eligible;
    for (const auto& pair : candidateScores) {
        if (pair.second >= minOverlap) {
            eligible.push_back(pair.first);
        }
    }

    // If no document meets minOverlap but index is small (< 20 docs), return all for thoroughness
    if (eligible.empty() && documents_.size() <= 20) {
        for (const auto& pair : documents_) {
            eligible.push_back(pair.first);
        }
    }

    return eligible;
}

void ReferenceIndex::saveToDisk() const {
    nlohmann::json root;
    root["version"] = CURRENT_INDEX_VERSION;
    root["count"] = documents_.size();

    nlohmann::json docsArray = nlohmann::json::array();
    for (const auto& pair : documents_) {
        docsArray.push_back(pair.second.toJson());
    }
    root["documents"] = docsArray;

    std::ofstream file(metadataFilePath_);
    if (file.is_open()) {
        file << root.dump(2);
    }
}

bool ReferenceIndex::loadFromDisk() {
    std::lock_guard<std::mutex> lock(indexMutex_);

    if (!std::filesystem::exists(metadataFilePath_)) {
        return false;
    }

    try {
        std::ifstream file(metadataFilePath_);
        if (!file.is_open()) return false;

        nlohmann::json root = nlohmann::json::parse(file);
        int version = root.value("version", 0);
        if (version != CURRENT_INDEX_VERSION) {
            // Version mismatch, rebuild cleanly
            return false;
        }

        documents_.clear();
        invertedIndex_.clear();
        ngramInvertedIndex_.clear();

        if (root.contains("documents") && root["documents"].is_array()) {
            for (const auto& docJson : root["documents"]) {
                Document doc = Document::fromJson(docJson);
                std::string docId = doc.getId();
                documents_[docId] = doc;

                // Rebuild inverted index
                for (const auto& token : doc.getTokens()) {
                    if (!textProcessor_.isStopWord(token.text)) {
                        invertedIndex_[token.text].insert(docId);
                    }
                }
                for (const auto& window : doc.getPassageWindows()) {
                    for (const auto& ngram : window.ngrams) {
                        ngramInvertedIndex_[ngram].insert(docId);
                    }
                }
            }
        }
        return true;
    } catch (...) {
        return false;
    }
}

size_t ReferenceIndex::rebuildIndex(const std::string& rawDocsDirectory) {
    std::lock_guard<std::mutex> lock(indexMutex_);
    documents_.clear();
    invertedIndex_.clear();
    ngramInvertedIndex_.clear();

    DocumentLoader loader;
    size_t count = 0;

    if (std::filesystem::exists(rawDocsDirectory)) {
        for (const auto& entry : std::filesystem::directory_iterator(rawDocsDirectory)) {
            if (entry.is_regular_file()) {
                std::string ext = DocumentLoader::detectExtension(entry.path().string());
                if (ext == ".txt" || ext == ".docx" || ext == ".pdf" || ext == ".md") {
                    try {
                        Document doc = loader.loadFromFile(entry.path().string());
                        textProcessor_.processDocument(doc);
                        std::string docId = doc.getId();
                        documents_[docId] = doc;

                        for (const auto& token : doc.getTokens()) {
                            if (!textProcessor_.isStopWord(token.text)) {
                                invertedIndex_[token.text].insert(docId);
                            }
                        }
                        for (const auto& window : doc.getPassageWindows()) {
                            for (const auto& ngram : window.ngrams) {
                                ngramInvertedIndex_[ngram].insert(docId);
                            }
                        }
                        count++;
                    } catch (...) {
                        // Skip unreadable files gracefully
                    }
                }
            }
        }
    }

    saveToDisk();
    return count;
}

void ReferenceIndex::clear() {
    std::lock_guard<std::mutex> lock(indexMutex_);
    documents_.clear();
    invertedIndex_.clear();
    ngramInvertedIndex_.clear();
    saveToDisk();
}

} // namespace docutrace
