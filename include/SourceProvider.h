#ifndef DOCUTRACE_SOURCEPROVIDER_H
#define DOCUTRACE_SOURCEPROVIDER_H

#include <string>
#include <vector>
#include "Models.h"

namespace docutrace {

/**
 * @brief Abstract Base Class for external source providers.
 * Demonstrates Unit III: Inheritance and Runtime Polymorphism.
 */
class SourceProvider {
public:
    virtual ~SourceProvider() = default;

    virtual std::string getProviderName() const = 0;
    virtual bool isOnline() const = 0;

    /**
     * @brief Searches candidate sources matching the query string.
     */
    virtual std::vector<AcademicSource> search(const std::string& query, size_t maxResults = 5) const = 0;

    /**
     * @brief Fetches full text or abstract from a source URL or identifier if accessible.
     */
    virtual std::string fetchText(const std::string& sourceIdentifier) const = 0;
};

} // namespace docutrace

#endif // DOCUTRACE_SOURCEPROVIDER_H
