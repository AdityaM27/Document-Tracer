#ifndef DOCUTRACE_REFERENCEINDEX_H
#define DOCUTRACE_REFERENCEINDEX_H

#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <memory>
#include <mutex>
#include "Document.h"
#include "TextProcessor.h"
#include "Exceptions.h"

namespace docutrace {

/**
 * @brief Manages a local reference document corpus with an inverted index,
 * disk persistence, and version control.
 * Demonstrates Unit I: STL Containers (unordered_map, unordered_set), Unit II: Encapsulation,
 * Unit IV: File I/O persistence, and Unit V: Mutex protection for thread safety.
 */
class ReferenceIndex {
private:
    static constexpr int CURRENT_INDEX_VERSION = 2;

    std::string indexDirectory_;
    std::string metadataFilePath_;

    // Document storage: docId -> Document
    std::unordered_map<std::string, Document> documents_;

    // Inverted index: token -> list of docIds
    std::unordered_map<std::string, std::unordered_set<std::string>> invertedIndex_;

    // Inverted index for 3-grams: 3gram -> list of docIds
    std::unordered_map<std::string, std::unordered_set<std::string>> ngramInvertedIndex_;

    TextProcessor textProcessor_;
    mutable std::mutex indexMutex_;

public:
    explicit ReferenceIndex(const std::string& indexDir = "data/references");
    ~ReferenceIndex() = default;

    /**
     * @brief Adds a document to the reference index, updates inverted indices, and saves to disk.
     */
    bool addDocument(const Document& doc);

    /**
     * @brief Removes a document by ID and cleans up indices.
     */
    bool removeDocument(const std::string& docId);

    /**
     * @brief Checks if a document ID exists.
     */
    bool hasDocument(const std::string& docId) const;

    /**
     * @brief Retrieves a document by ID. Throws IndexException if not found.
     */
    Document getDocument(const std::string& docId) const;

    /**
     * @brief Returns list of all indexed documents.
     */
    std::vector<Document> getAllDocuments() const;

    /**
     * @brief Total count of indexed documents.
     */
    size_t size() const;

    /**
     * @brief Searches documents containing query tokens/keywords.
     */
    std::vector<std::string> search(const std::string& queryText, size_t limit = 10) const;

    /**
     * @brief Finds candidate document IDs that share at least minOverlap tokens/ngrams with query.
     */
    std::vector<std::string> findCandidateDocuments(const Document& queryDoc, size_t minOverlap = 3) const;

    /**
     * @brief Persists index metadata and indexed documents to disk.
     */
    void saveToDisk() const;

    /**
     * @brief Loads index from disk. If version mismatch or corrupt, rebuilds gracefully.
     */
    bool loadFromDisk();

    /**
     * @brief Scans the reference directory on disk and re-indexes all valid files.
     */
    size_t rebuildIndex(const std::string& rawDocsDirectory);

    /**
     * @brief Clears all indexed documents.
     */
    void clear();

    const std::string& getIndexDirectory() const noexcept { return indexDirectory_; }
};

} // namespace docutrace

#endif // DOCUTRACE_REFERENCEINDEX_H
