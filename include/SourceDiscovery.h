#ifndef DOCUTRACE_SOURCEDISCOVERY_H
#define DOCUTRACE_SOURCEDISCOVERY_H

#include <vector>
#include <memory>
#include "SourceProvider.h"
#include "Document.h"

namespace docutrace {

/**
 * @brief Orchestrates external academic and web source discovery and matches retrieved text.
 * Demonstrates Unit III: Collection of polymorphic base class pointers (std::unique_ptr<SourceProvider>).
 */
class SourceDiscovery {
private:
    std::vector<std::shared_ptr<SourceProvider>> providers_;

public:
    SourceDiscovery() = default;
    ~SourceDiscovery() = default;

    void addProvider(std::shared_ptr<SourceProvider> provider);

    /**
     * @brief Discovers candidate external publications and web sources for a document.
     */
    std::vector<AcademicSource> discoverSources(const Document& queryDoc, size_t maxPerQuery = 3);

    /**
     * @brief Compares query document passages against discovered sources whose text/abstract was retrieved.
     */
    std::vector<MatchRecord> compareAgainstSources(
        const Document& queryDoc,
        const std::vector<AcademicSource>& sources,
        double threshold = 0.50);
};

} // namespace docutrace

#endif // DOCUTRACE_SOURCEDISCOVERY_H
