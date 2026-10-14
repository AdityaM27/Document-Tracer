#ifndef DOCUTRACE_ACADEMICSOURCEPROVIDER_H
#define DOCUTRACE_ACADEMICSOURCEPROVIDER_H

#include <unordered_map>
#include <mutex>
#include "SourceProvider.h"

namespace docutrace {

/**
 * @brief Searches scholarly literature via OpenAlex and Crossref APIs.
 * Demonstrates Unit III: Inheritance & Concrete Provider, Unit IV: Disk caching, Unit V: Network error recovery.
 */
class AcademicSourceProvider : public SourceProvider {
private:
    std::string cacheFilePath_;
    std::string userAgent_;
    bool onlineMode_{true};
    mutable std::mutex cacheMutex_;
    mutable std::unordered_map<std::string, std::vector<AcademicSource>> queryCache_;

    void loadCache();
    void saveCache() const;

    std::vector<AcademicSource> queryOpenAlex(const std::string& query, size_t maxResults) const;
    std::vector<AcademicSource> queryCrossref(const std::string& query, size_t maxResults) const;

public:
    explicit AcademicSourceProvider(
        const std::string& cacheFilePath = "data/cache/academic_cache.json",
        const std::string& politeEmail = "docutrace-lab@university.edu");
    ~AcademicSourceProvider() override;

    std::string getProviderName() const override { return "AcademicSourceProvider (OpenAlex & Crossref)"; }
    bool isOnline() const override { return onlineMode_; }

    std::vector<AcademicSource> search(const std::string& query, size_t maxResults = 5) const override;
    std::string fetchText(const std::string& sourceIdentifier) const override;

    void setOnlineMode(bool online) { onlineMode_ = online; }

    /**
     * @brief Extracts distinctive query phrases (salient n-grams) from a document.
     */
    static std::vector<std::string> extractSalientQueries(const std::vector<Token>& tokens, size_t maxQueries = 3);
};

} // namespace docutrace

#endif // DOCUTRACE_ACADEMICSOURCEPROVIDER_H
