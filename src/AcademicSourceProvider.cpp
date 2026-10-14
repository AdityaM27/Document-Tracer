#include "../include/AcademicSourceProvider.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include "../third_party/nlohmann/json.hpp"

namespace docutrace {

AcademicSourceProvider::AcademicSourceProvider(
    const std::string& cacheFilePath,
    const std::string& politeEmail)
    : cacheFilePath_(cacheFilePath),
      userAgent_("DocuTrace-Academic/1.0 (mailto:" + politeEmail + ")"),
      onlineMode_(true) {
    
    std::filesystem::create_directories(std::filesystem::path(cacheFilePath_).parent_path());
    loadCache();
}

AcademicSourceProvider::~AcademicSourceProvider() {
    saveCache();
}

void AcademicSourceProvider::loadCache() {
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
                    if (sJ.contains("authors")) s.authors = sJ["authors"].get<std::vector<std::string>>();
                    if (sJ.contains("publicationDate")) s.publicationDate = sJ["publicationDate"].get<std::string>();
                    if (sJ.contains("doi")) s.doi = sJ["doi"].get<std::string>();
                    if (sJ.contains("url")) s.url = sJ["url"].get<std::string>();
                    if (sJ.contains("abstractText")) s.abstractText = sJ["abstractText"].get<std::string>();
                    if (sJ.contains("provider")) s.provider = sJ["provider"].get<std::string>();
                    if (sJ.contains("retrievalStatus")) s.retrievalStatus = sJ["retrievalStatus"].get<std::string>();
                    if (sJ.contains("relevanceScore")) s.relevanceScore = sJ["relevanceScore"].get<double>();
                    sources.push_back(s);
                }
                queryCache_[query] = sources;
            }
        }
    } catch (...) {}
}

void AcademicSourceProvider::saveCache() const {
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

std::vector<std::string> AcademicSourceProvider::extractSalientQueries(const std::vector<Token>& tokens, size_t maxQueries) {
    std::vector<std::string> queries;
    if (tokens.size() < 4) return queries;

    // Extract salient 3-5 word sequences that contain substantive words
    size_t step = std::max<size_t>(1, tokens.size() / (maxQueries + 1));
    for (size_t i = 0; i + 4 < tokens.size() && queries.size() < maxQueries; i += step) {
        std::string query;
        for (size_t k = 0; k < 4; ++k) {
            query += tokens[i + k].text + (k + 1 < 4 ? " " : "");
        }
        queries.push_back(query);
    }

    return queries;
}

std::vector<AcademicSource> AcademicSourceProvider::queryOpenAlex(const std::string& query, size_t maxResults) const {
    std::vector<AcademicSource> results;
    if (!onlineMode_) return results;

    // Use python fetcher script for secure HTTPS + OpenAlex JSON parsing
    std::string pyScript = "scripts/fetch_academic.py";
    std::string tmpOut = "data/cache/openalex_tmp.json";

    std::string cmd = "python " + pyScript + " --provider openalex --query \"" + query + "\" --max " + std::to_string(maxResults) + " --out \"" + tmpOut + "\"";
    int ret = std::system(cmd.c_str());
    if (ret == 0 && std::filesystem::exists(tmpOut)) {
        try {
            std::ifstream f(tmpOut);
            nlohmann::json j = nlohmann::json::parse(f);
            f.close();
            std::filesystem::remove(tmpOut);

            if (j.is_array()) {
                for (const auto& item : j) {
                    AcademicSource s;
                    s.title = item.value("title", "Untitled");
                    if (item.contains("authors") && item["authors"].is_array()) {
                        s.authors = item["authors"].get<std::vector<std::string>>();
                    }
                    s.publicationDate = item.value("publication_date", "");
                    s.doi = item.value("doi", "");
                    s.url = item.value("url", "");
                    s.abstractText = item.value("abstract", "");
                    s.provider = "OpenAlex";
                    s.retrievalStatus = s.abstractText.empty() ? "METADATA_DISCOVERED" : "ABSTRACT_RETRIEVED";
                    results.push_back(s);
                }
            }
        } catch (...) {}
    }

    return results;
}

std::vector<AcademicSource> AcademicSourceProvider::queryCrossref(const std::string& query, size_t maxResults) const {
    std::vector<AcademicSource> results;
    if (!onlineMode_) return results;

    std::string pyScript = "scripts/fetch_academic.py";
    std::string tmpOut = "data/cache/crossref_tmp.json";

    std::string cmd = "python " + pyScript + " --provider crossref --query \"" + query + "\" --max " + std::to_string(maxResults) + " --out \"" + tmpOut + "\"";
    int ret = std::system(cmd.c_str());
    if (ret == 0 && std::filesystem::exists(tmpOut)) {
        try {
            std::ifstream f(tmpOut);
            nlohmann::json j = nlohmann::json::parse(f);
            f.close();
            std::filesystem::remove(tmpOut);

            if (j.is_array()) {
                for (const auto& item : j) {
                    AcademicSource s;
                    s.title = item.value("title", "Untitled");
                    if (item.contains("authors") && item["authors"].is_array()) {
                        s.authors = item["authors"].get<std::vector<std::string>>();
                    }
                    s.publicationDate = item.value("publication_date", "");
                    s.doi = item.value("doi", "");
                    s.url = item.value("url", "");
                    s.abstractText = item.value("abstract", "");
                    s.provider = "Crossref";
                    s.retrievalStatus = s.abstractText.empty() ? "METADATA_DISCOVERED" : "ABSTRACT_RETRIEVED";
                    results.push_back(s);
                }
            }
        } catch (...) {}
    }

    return results;
}

std::vector<AcademicSource> AcademicSourceProvider::search(const std::string& query, size_t maxResults) const {
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        auto it = queryCache_.find(query);
        if (it != queryCache_.end()) {
            return it->second;
        }
    }

    std::vector<AcademicSource> combined;
    if (onlineMode_) {
        auto openAlexRes = queryOpenAlex(query, (maxResults + 1) / 2);
        auto crossrefRes = queryCrossref(query, maxResults / 2);

        combined.insert(combined.end(), openAlexRes.begin(), openAlexRes.end());
        combined.insert(combined.end(), crossrefRes.begin(), crossrefRes.end());
    }

    // Cache results
    {
        std::lock_guard<std::mutex> lock(cacheMutex_);
        queryCache_[query] = combined;
    }
    saveCache();

    return combined;
}

std::string AcademicSourceProvider::fetchText(const std::string& sourceIdentifier) const {
    // If abstract already exists in cache, return it
    std::lock_guard<std::mutex> lock(cacheMutex_);
    for (const auto& [q, sources] : queryCache_) {
        for (const auto& s : sources) {
            if (s.doi == sourceIdentifier || s.url == sourceIdentifier || s.title == sourceIdentifier) {
                return s.abstractText;
            }
        }
    }
    return "";
}

} // namespace docutrace
