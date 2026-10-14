#ifndef DOCUTRACE_DOCUMENT_H
#define DOCUTRACE_DOCUMENT_H

#include <string>
#include <vector>
#include <memory>
#include "Models.h"

namespace docutrace {

/**
 * @brief Represents an ingested document with original text, normalized text,
 * token sequence, and passage segmentation.
 * Demonstrates Unit II: Object-Oriented Programming (Encapsulation, Information Hiding).
 */
class Document {
private:
    std::string id_;
    std::string filename_;
    std::string filepath_;
    std::string fileExtension_;
    size_t fileSize_{0};
    
    std::string originalText_;
    std::string normalizedText_;
    
    std::vector<Token> tokens_;
    std::vector<Passage> sentences_;
    std::vector<Passage> paragraphs_;
    std::vector<Passage> passageWindows_; // Overlapping sliding windows for fine-grained matching

    ProcessingStatus status_{ProcessingStatus::UNPROCESSED};
    std::string errorMessage_;
    size_t wordCount_{0};
    size_t characterCount_{0};

public:
    Document();
    Document(const std::string& id, const std::string& filename, const std::string& filepath = "");
    ~Document() = default;

    // Getters (const-correctness - Unit II)
    const std::string& getId() const noexcept { return id_; }
    const std::string& getFilename() const noexcept { return filename_; }
    const std::string& getFilepath() const noexcept { return filepath_; }
    const std::string& getFileExtension() const noexcept { return fileExtension_; }
    size_t getFileSize() const noexcept { return fileSize_; }

    const std::string& getOriginalText() const noexcept { return originalText_; }
    const std::string& getNormalizedText() const noexcept { return normalizedText_; }

    const std::vector<Token>& getTokens() const noexcept { return tokens_; }
    const std::vector<Passage>& getSentences() const noexcept { return sentences_; }
    const std::vector<Passage>& getParagraphs() const noexcept { return paragraphs_; }
    const std::vector<Passage>& getPassageWindows() const noexcept { return passageWindows_; }

    ProcessingStatus getStatus() const noexcept { return status_; }
    const std::string& getErrorMessage() const noexcept { return errorMessage_; }
    size_t getWordCount() const noexcept { return wordCount_; }
    size_t getCharacterCount() const noexcept { return characterCount_; }

    // Setters
    void setId(const std::string& id) { id_ = id; }
    void setOriginalText(const std::string& text);
    void setNormalizedText(const std::string& text) { normalizedText_ = text; }
    void setFileSize(size_t size) { fileSize_ = size; }
    void setStatus(ProcessingStatus status) { status_ = status; }
    void setErrorMessage(const std::string& err) { errorMessage_ = err; status_ = ProcessingStatus::FAILED; }

    void setTokens(const std::vector<Token>& tokens);
    void setSentences(const std::vector<Passage>& sentences) { sentences_ = sentences; }
    void setParagraphs(const std::vector<Passage>& paragraphs) { paragraphs_ = paragraphs; }
    void setPassageWindows(const std::vector<Passage>& windows) { passageWindows_ = windows; }

    // Utility methods
    std::string extractSnippet(size_t startChar, size_t endChar, size_t contextPadding = 30) const;
    nlohmann::json toJson() const;
    static Document fromJson(const nlohmann::json& j);
};

} // namespace docutrace

#endif // DOCUTRACE_DOCUMENT_H
