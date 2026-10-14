#include "../include/AIAnalysisClient.h"
#include "../third_party/httplib.h"
#include "../third_party/nlohmann/json.hpp"
#include <iostream>

namespace docutrace {

AIAnalysisClient::AIAnalysisClient(const std::string& host, int port, int timeoutMs)
    : host_(host), port_(port), timeoutMs_(timeoutMs) {}

bool AIAnalysisClient::checkHealth() const {
    bool online = false;
    std::string model;
    bool hfLoaded = false;
    std::string mode;
    return getHealthDetails(online, model, hfLoaded, mode) && online;
}

bool AIAnalysisClient::getHealthDetails(bool& online, std::string& model, bool& hfLoaded, std::string& mode) const {
    online = false;
    model = "roberta-base-openai-detector";
    hfLoaded = false;
    mode = "offline";

    try {
        httplib::Client cli(host_, port_);
        cli.set_connection_timeout(0, 3000 * 1000); // 3 seconds for health check
        cli.set_read_timeout(0, 3000 * 1000);

        auto res = cli.Get("/health");
        if (res && res->status == 200) {
            online = true;
            nlohmann::json j = nlohmann::json::parse(res->body);
            model = j.value("model", "roberta-base-openai-detector");
            hfLoaded = j.value("hf_loaded", false);
            mode = j.value("mode", hfLoaded ? "huggingface" : "calibrated_statistical");
            return true;
        }
    } catch (...) {}
    return false;
}

AIClassificationResult AIAnalysisClient::classifyText(const std::string& text) const {
    AIClassificationResult result;
    result.serviceAvailable = false;
    result.modelName = "roberta-base-openai-detector";

    if (text.empty()) {
        result.label = "Uncertain";
        result.reasoning = "Document contains no text for classification.";
        return result;
    }

    try {
        httplib::Client cli(host_, port_);
        cli.set_connection_timeout(0, timeoutMs_ * 1000);
        cli.set_read_timeout(0, timeoutMs_ * 1000);

        nlohmann::json payload;
        // Support sliding window chunking across full documents (up to 200,000 characters)
        payload["text"] = text.size() > 200000 ? text.substr(0, 200000) : text;

        auto res = cli.Post("/analyze", payload.dump(), "application/json");
        if (res && res->status == 200) {
            nlohmann::json j = nlohmann::json::parse(res->body);
            
            // Check if service signaled an error or missing probabilities
            if (j.value("error", false) || !j.contains("ai_score") || j["ai_score"].is_null()) {
                result.serviceAvailable = false;
                result.isPretrainedModel = false;
                result.label = j.value("label", "Inference Error");
                result.aiProbability = 0.0;
                result.humanProbability = 0.0;
                result.confidence = 0.0;
                result.reasoning = j.value("reasoning", "AI classification inference error.");
                result.inferenceMode = "error";
                return result;
            }

            result.serviceAvailable = true;
            result.modelName = j.value("model", "roberta-base-openai-detector");
            result.label = j.value("label", "Uncertain");
            result.aiProbability = j.value("ai_score", 0.0);
            result.humanProbability = j.value("human_score", 0.0);
            result.confidence = j.value("confidence", 0.0);
            result.reasoning = j.value("reasoning", "Model classification completed.");
            result.isPretrainedModel = j.value("is_pretrained", false);
            result.inferenceMode = j.value("mode", result.isPretrainedModel ? "huggingface" : "calibrated_statistical");
            if (j.contains("disclaimer")) {
                result.disclaimer = j["disclaimer"].get<std::string>();
            }
            return result;
        } else if (res && res->status != 200) {
            result.serviceAvailable = false;
            result.isPretrainedModel = false;
            result.label = "Inference Error";
            result.aiProbability = 0.0;
            result.humanProbability = 0.0;
            try {
                nlohmann::json errJ = nlohmann::json::parse(res->body);
                result.reasoning = errJ.value("reasoning", errJ.value("error_message", "AI service returned HTTP error " + std::to_string(res->status)));
            } catch (...) {
                result.reasoning = "AI service returned HTTP error " + std::to_string(res->status);
            }
            return result;
        }
    } catch (const std::exception& e) {
        result.reasoning = "AI service connection error: " + std::string(e.what());
    } catch (...) {
        result.reasoning = "Unknown error communicating with AI classification service.";
    }

    // Graceful offline fallback
    result.serviceAvailable = false;
    result.label = "Service Offline";
    result.aiProbability = 0.0;
    result.humanProbability = 0.0;
    result.reasoning = "Local Python AI classification service (ai_service/app.py) is currently offline or unreachable. "
                       "Start the service on port 8000 using 'python ai_service/app.py' to enable pretrained Hugging Face classification.";
    return result;
}

AIClassificationResult AIAnalysisClient::classifyDocument(const Document& doc) const {
    return classifyText(doc.getOriginalText());
}

} // namespace docutrace
