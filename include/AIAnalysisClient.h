#ifndef DOCUTRACE_AIANALYSISCLIENT_H
#define DOCUTRACE_AIANALYSISCLIENT_H

#include <string>
#include "Models.h"
#include "Document.h"

namespace docutrace {

/**
 * @brief Communicates with the local pretrained AI-text classification microservice.
 * Demonstrates Unit V: Network error handling and graceful degradation.
 */
class AIAnalysisClient {
private:
    std::string host_;
    int port_;
    int timeoutMs_;

public:
    explicit AIAnalysisClient(
        const std::string& host = "127.0.0.1",
        int port = 8000,
        int timeoutMs = 60000);
    ~AIAnalysisClient() = default;

    /**
     * @brief Checks if the Python AI classification service is running and healthy.
     */
    bool checkHealth() const;

    /**
     * @brief Retrieves detailed health status, including whether the model is a genuine
     * pretrained Hugging Face model or running in calibrated statistical fallback mode.
     */
    bool getHealthDetails(bool& online, std::string& model, bool& hfLoaded, std::string& mode) const;

    /**
     * @brief Sends document text to the AI classifier and returns parsed results.
     * Degrades gracefully if service is unreachable.
     */
    AIClassificationResult classifyText(const std::string& text) const;

    /**
     * @brief Classifies a full document.
     */
    AIClassificationResult classifyDocument(const Document& doc) const;

    const std::string& getHost() const noexcept { return host_; }
    int getPort() const noexcept { return port_; }
};

} // namespace docutrace

#endif // DOCUTRACE_AIANALYSISCLIENT_H
