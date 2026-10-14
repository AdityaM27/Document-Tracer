#ifndef DOCUTRACE_JACCARDSIMILARITY_H
#define DOCUTRACE_JACCARDSIMILARITY_H

#include <unordered_set>
#include "SimilarityStrategy.h"

namespace docutrace {

/**
 * @brief Computes Jaccard set similarity: J(A, B) = |A ∩ B| / |A ∪ B|.
 * Demonstrates Unit III: Polymorphism, Unit I: STL set operations and edge-case handling.
 */
class JaccardSimilarity : public SimilarityStrategy {
public:
    JaccardSimilarity() = default;
    ~JaccardSimilarity() override = default;

    std::string getName() const override { return "Jaccard Similarity"; }
    MatchType getMatchType() const override { return MatchType::JACCARD_MATCH; }

    double calculateSimilarity(const Document& docA, const Document& docB) const override;

    std::vector<MatchRecord> findPassageMatches(
        const Document& queryDoc,
        const Document& targetDoc,
        double threshold) const override;

    /**
     * @brief Computes Jaccard coefficient between two token collections.
     */
    static double computeJaccard(
        const std::unordered_set<std::string>& setA,
        const std::unordered_set<std::string>& setB);
};

} // namespace docutrace

#endif // DOCUTRACE_JACCARDSIMILARITY_H
