#ifndef DOCUTRACE_EXCEPTIONS_H
#define DOCUTRACE_EXCEPTIONS_H

#include <stdexcept>
#include <string>

namespace docutrace {

/**
 * @brief Base exception class for all DocuTrace errors.
 * Demonstrates Unit V: C++ Custom Exception Hierarchies.
 */
class DocuTraceException : public std::exception {
protected:
    std::string message_;
    std::string context_;
    int errorCode_;

public:
    DocuTraceException(const std::string& message, const std::string& context = "", int errorCode = 1000)
        : message_(message), context_(context), errorCode_(errorCode) {}

    virtual ~DocuTraceException() noexcept = default;

    const char* what() const noexcept override {
        return message_.c_str();
    }

    const std::string& getContext() const noexcept {
        return context_;
    }

    int getErrorCode() const noexcept {
        return errorCode_;
    }
};

class DocumentExtractionException : public DocuTraceException {
public:
    DocumentExtractionException(const std::string& message, const std::string& filepath = "")
        : DocuTraceException("Document Extraction Failed: " + message, filepath, 1001) {}
};

class InvalidFormatException : public DocuTraceException {
public:
    InvalidFormatException(const std::string& format, const std::string& filepath = "")
        : DocuTraceException("Unsupported or invalid file format: " + format, filepath, 1002) {}
};

class EmptyDocumentException : public DocuTraceException {
public:
    EmptyDocumentException(const std::string& filepath = "")
        : DocuTraceException("Document contains no readable text", filepath, 1003) {}
};

class FileSizeLimitException : public DocuTraceException {
public:
    FileSizeLimitException(size_t size, size_t limit, const std::string& filepath = "")
        : DocuTraceException("File size (" + std::to_string(size) + " bytes) exceeds limit (" + std::to_string(limit) + " bytes)", filepath, 1004) {}
};

class IndexException : public DocuTraceException {
public:
    IndexException(const std::string& message, const std::string& context = "")
        : DocuTraceException("Reference Index Error: " + message, context, 2001) {}
};

class StrategyException : public DocuTraceException {
public:
    StrategyException(const std::string& strategyName, const std::string& detail)
        : DocuTraceException("Similarity Strategy Error [" + strategyName + "]: " + detail, strategyName, 3001) {}
};

class NetworkException : public DocuTraceException {
public:
    NetworkException(const std::string& serviceName, const std::string& detail)
        : DocuTraceException("Network / External Service Error [" + serviceName + "]: " + detail, serviceName, 4001) {}
};

class AIAnalysisException : public DocuTraceException {
public:
    AIAnalysisException(const std::string& message, const std::string& detail = "")
        : DocuTraceException("AI Analysis Service Error: " + message + (detail.empty() ? "" : " (" + detail + ")"), detail, 5001) {}
};

class ReportGenerationException : public DocuTraceException {
public:
    ReportGenerationException(const std::string& message, const std::string& targetPath = "")
        : DocuTraceException("Report Generation Error: " + message, targetPath, 6001) {}
};

} // namespace docutrace

#endif // DOCUTRACE_EXCEPTIONS_H
