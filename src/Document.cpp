#include "../include/Document.h"
#include <algorithm>

namespace docutrace {

Document::Document()
    : id_(""), filename_(""), filepath_(""), fileExtension_(""), status_(ProcessingStatus::UNPROCESSED) {}

Document::Document(const std::string& id, const std::string& filename, const std::string& filepath)
    : id_(id), filename_(filename), filepath_(filepath), status_(ProcessingStatus::UNPROCESSED) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos != std::string::npos) {
        fileExtension_ = filename.substr(dotPos);
        std::transform(fileExtension_.begin(), fileExtension_.end(), fileExtension_.begin(), ::tolower);
    }
}

void Document::setOriginalText(const std::string& text) {
    originalText_ = text;
    characterCount_ = text.size();
    status_ = ProcessingStatus::EXTRACTED;
}

void Document::setTokens(const std::vector<Token>& tokens) {
    tokens_ = tokens;
    wordCount_ = tokens.size();
    status_ = ProcessingStatus::PREPROCESSED;
}

std::string Document::extractSnippet(size_t startChar, size_t endChar, size_t contextPadding) const {
    if (originalText_.empty()) return "";

    size_t start = (startChar > contextPadding) ? (startChar - contextPadding) : 0;
    size_t end = std::min(originalText_.size(), endChar + contextPadding);

    std::string snippet = originalText_.substr(start, end - start);
    if (start > 0) snippet = "..." + snippet;
    if (end < originalText_.size()) snippet = snippet + "...";

    return snippet;
}

nlohmann::json Document::toJson() const {
    nlohmann::json j;
    j["id"] = id_;
    j["filename"] = filename_;
    j["filepath"] = filepath_;
    j["fileExtension"] = fileExtension_;
    j["fileSize"] = fileSize_;
    j["originalText"] = originalText_;
    j["normalizedText"] = normalizedText_;
    j["wordCount"] = wordCount_;
    j["characterCount"] = characterCount_;
    j["status"] = static_cast<int>(status_);
    j["errorMessage"] = errorMessage_;

    nlohmann::json tokensJson = nlohmann::json::array();
    for (const auto& t : tokens_) {
        tokensJson.push_back(t);
    }
    j["tokens"] = tokensJson;

    nlohmann::json sentencesJson = nlohmann::json::array();
    for (const auto& s : sentences_) {
        sentencesJson.push_back(s);
    }
    j["sentences"] = sentencesJson;

    nlohmann::json paragraphsJson = nlohmann::json::array();
    for (const auto& p : paragraphs_) {
        paragraphsJson.push_back(p);
    }
    j["paragraphs"] = paragraphsJson;

    nlohmann::json windowsJson = nlohmann::json::array();
    for (const auto& w : passageWindows_) {
        windowsJson.push_back(w);
    }
    j["passageWindows"] = windowsJson;

    return j;
}

Document Document::fromJson(const nlohmann::json& j) {
    Document doc;
    if (j.contains("id")) doc.id_ = j["id"].get<std::string>();
    if (j.contains("filename")) doc.filename_ = j["filename"].get<std::string>();
    if (j.contains("filepath")) doc.filepath_ = j["filepath"].get<std::string>();
    if (j.contains("fileExtension")) doc.fileExtension_ = j["fileExtension"].get<std::string>();
    if (j.contains("fileSize")) doc.fileSize_ = j["fileSize"].get<size_t>();
    if (j.contains("originalText")) doc.originalText_ = j["originalText"].get<std::string>();
    if (j.contains("normalizedText")) doc.normalizedText_ = j["normalizedText"].get<std::string>();
    if (j.contains("wordCount")) doc.wordCount_ = j["wordCount"].get<size_t>();
    if (j.contains("characterCount")) doc.characterCount_ = j["characterCount"].get<size_t>();
    if (j.contains("status")) doc.status_ = static_cast<ProcessingStatus>(j["status"].get<int>());
    if (j.contains("errorMessage")) doc.errorMessage_ = j["errorMessage"].get<std::string>();

    if (j.contains("tokens") && j["tokens"].is_array()) {
        for (const auto& tJson : j["tokens"]) {
            Token t;
            from_json(tJson, t);
            doc.tokens_.push_back(t);
        }
    }

    if (j.contains("sentences") && j["sentences"].is_array()) {
        for (const auto& sJson : j["sentences"]) {
            Passage s;
            from_json(sJson, s);
            doc.sentences_.push_back(s);
        }
    }

    if (j.contains("paragraphs") && j["paragraphs"].is_array()) {
        for (const auto& pJson : j["paragraphs"]) {
            Passage p;
            from_json(pJson, p);
            doc.paragraphs_.push_back(p);
        }
    }

    if (j.contains("passageWindows") && j["passageWindows"].is_array()) {
        for (const auto& wJson : j["passageWindows"]) {
            Passage w;
            from_json(wJson, w);
            doc.passageWindows_.push_back(w);
        }
    }

    return doc;
}

} // namespace docutrace
