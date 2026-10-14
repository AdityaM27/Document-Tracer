#ifndef DOCUTRACE_DOCUMENTLOADER_H
#define DOCUTRACE_DOCUMENTLOADER_H

#include <string>
#include <memory>
#include "Document.h"
#include "Exceptions.h"

namespace docutrace {

/**
 * @brief Ingests documents from disk across multiple formats (TXT, DOCX, PDF).
 * Demonstrates Unit IV: File I/O streams and Unit V: Exception handling.
 */
class DocumentLoader {
private:
    size_t maxFileSizeBytes_{15 * 1024 * 1024}; // 15 MB limit

    // Extraction backends
    std::string extractFromTxt(const std::string& filepath) const;
    std::string extractFromDocx(const std::string& filepath) const;
    std::string extractFromPdf(const std::string& filepath) const;

public:
    explicit DocumentLoader(size_t maxFileSizeBytes = 15 * 1024 * 1024);
    ~DocumentLoader() = default;

    /**
     * @brief Loads a document from the given file path into a Document object.
     * Throws DocumentExtractionException, InvalidFormatException, EmptyDocumentException, FileSizeLimitException.
     */
    Document loadFromFile(const std::string& filepath, const std::string& customId = "") const;

    /**
     * @brief Ingests direct raw text (e.g. from web clipboard or API payload).
     */
    Document loadFromText(const std::string& text, const std::string& filename = "direct_input.txt", const std::string& customId = "") const;

    size_t getMaxFileSize() const noexcept { return maxFileSizeBytes_; }
    void setMaxFileSize(size_t bytes) { maxFileSizeBytes_ = bytes; }

    static std::string detectExtension(const std::string& filepath);
    static std::string generateUniqueId(const std::string& filename);
};

} // namespace docutrace

#endif // DOCUTRACE_DOCUMENTLOADER_H
