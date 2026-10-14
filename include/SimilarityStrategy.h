#ifndef DOCUTRACE_SIMILARITYSTRATEGY_H
#define DOCUTRACE_SIMILARITYSTRATEGY_H

#include <string>
#include <vector>
#include <memory>
#include "Models.h"
#include "Document.h"

namespace docutrace {

/**
 * @brief Abstract Base Class for all document similarity comparison algorithms.
 * Demonstrates Unit III: Inheritance and Runtime Polymorphism (Pure Virtual Functions, Virtual Destructor).
 */
class SimilarityStrategy {
public:
    virtual ~SimilarityStrategy() = default;

    /**
     * @brief Gets the human-readable name of the strategy.
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Gets the match type enum associated with this strategy.
     */
    virtual MatchType getMatchType() const = 0;

    /**
     * @brief Computes whole-document similarity score between 0.0 and 1.0.
     */
    virtual double calculateSimilarity(const Document& docA, const Document& docB) const = 0;

    /**
     * @brief Identifies matching passages between queryDoc and targetDoc exceeding the given threshold.
     */
    virtual std::vector<MatchRecord> findPassageMatches(
        const Document& queryDoc,
        const Document& targetDoc,
        double threshold) const = 0;
};

} // namespace docutrace

#endif // DOCUTRACE_SIMILARITYSTRATEGY_H
