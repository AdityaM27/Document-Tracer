#include "../include/WebSourceProvider.h"
#include <fstream>
#include <filesystem>
#include "../third_party/nlohmann/json.hpp"

namespace docutrace {

WebSourceProvider::WebSourceProvider(
    const std::string& apiKey,
    const std::string& endpoint,
    const std::string& cacheFilePath)
    : apiKey_(apiKey), endpoint_(endpoint), onlineMode_(!apiKey.empty()), cacheFilePath_(cacheFilePath) {
    
    std::filesystem::create_directories(std::filesystem::path(cacheFilePath_).parent_path());
    loadCache();
}

WebSourceProvider::~WebSourceProvider() {
    saveCache();
}

void WebSourceProvider::loadCache() {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    if (!std::filesystem::exists(cacheFilePath_)) return;

    try {
        std::ifstream f(cacheFilePath_);
        if (!f.is_open()) return;

        nlohmann::json root = nlohmann::json::parse(f);
        if (root.is_object()) {
            for (auto& [query, sourcesJson] : root.items()) {
                std::vector<AcademicSource> sources;
                for (const auto& sJ : sourcesJson) {
                    AcademicSource s;
                    if (sJ.contains("title")) s.title = sJ["title"].get<std::string>();
                    if (sJ.contains("url")) s.url = sJ["url"].get<std::string>();
                    if (sJ.contains("abstractText")) s.abstractText = sJ["abstractText"].get<std::string>();
                    if (sJ.contains("provider")) s.provider = sJ["provider"].get<std::string>();
                    if (sJ.contains("retrievalStatus")) s.retrievalStatus = sJ["retrievalStatus"].get<std::string>();
                    sources.push_back(s);
                }
                queryCache_[query] = sources;
            }
        }
    } catch (...) {}
}

void WebSourceProvider::saveCache() const {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    nlohmann::json root = nlohmann::json::object();

    for (const auto& [query, sources] : queryCache_) {
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& s : sources) {
            nlohmann::json sJ;
            to_json(sJ, s);
            arr.push_back(sJ);
        }
        root[query] = arr;
    }

    std::ofstream f(cacheFilePath_);
    if (f.is_open()) {
        f << root.dump(2);
    }
}

std::vector<AcademicSource> WebSourceProvider::search(const std::string& query, size_t maxResults) const {
    (void)maxResults;
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        auto it = queryCache_.find(query);
        if (it != queryCache_.end()) {
            return it->second;
        }
    }

    std::vector<AcademicSource> results;
    // When API key is not configured, degrade gracefully by reporting offline status
    if (!onlineMode_ || apiKey_.empty()) {
        return results;
    }

    // If configured with an endpoint/key:
    // Query can be executed here with timeout & rate limit respect

    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        queryCache_[query] = results;
    }
    saveCache();

    return results;
}

std::string WebSourceProvider::fetchText(const std::string& sourceIdentifier) const {
    std::lock_guard<std::mutex> lock(cacheMutex_);
    for (const auto& [q, sources] : queryCache_) {
        for (const auto& s : sources) {
            if (s.url == sourceIdentifier) {
                return s.abstractText;
            }
        }
    }
    return "";
}

} // namespace docutrace
