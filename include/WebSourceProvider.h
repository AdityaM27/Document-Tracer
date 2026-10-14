#ifndef DOCUTRACE_WEBSOURCEPROVIDER_H
#define DOCUTRACE_WEBSOURCEPROVIDER_H

#include "SourceProvider.h"
#include <unordered_map>
#include <mutex>

namespace docutrace {

/**
 * @brief Configurable Web Source Provider for general-web discovery.
 * Demonstrates Unit III: Polymorphism, respectful crawling guidelines, and offline fallback.
 */
class WebSourceProvider : public SourceProvider {
private:
    std::string apiKey_;
    std::string endpoint_;
    bool onlineMode_{false}; // Default false to prevent unconfigured external calls
    std::string cacheFilePath_;
    mutable std::mutex cacheMutex_;
    mutable std::unordered_map<std::string, std::vector<AcademicSource>> queryCache_;

    void loadCache();
    void saveCache() const;

public:
    explicit WebSourceProvider(
        const std::string& apiKey = "",
        const std::string& endpoint = "",
        const std::string& cacheFilePath = "data/cache/web_cache.json");
    ~WebSourceProvider() override;

    std::string getProviderName() const override { return "WebSourceProvider (General Web)"; }
    bool isOnline() const override { return onlineMode_; }

    std::vector<AcademicSource> search(const std::string& query, size_t maxResults = 5) const override;
    std::string fetchText(const std::string& sourceIdentifier) const override;

    void setOnlineMode(bool online) { onlineMode_ = online; }
    void setApiKey(const std::string& key) { apiKey_ = key; }
};

} // namespace docutrace

#endif // DOCUTRACE_WEBSOURCEPROVIDER_H
