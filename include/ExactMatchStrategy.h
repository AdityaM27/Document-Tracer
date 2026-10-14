#ifndef DOCUTRACE_EXACTMATCHSTRATEGY_H
#define DOCUTRACE_EXACTMATCHSTRATEGY_H

#include "SimilarityStrategy.h"

namespace docutrace {

/**
 * @brief Detects exact textual matches and identical phrase sequences.
 * Demonstrates Unit III: Inheritance & Concrete Strategy implementation.
 */
class ExactMatchStrategy : public SimilarityStrategy {
private:
    size_t minMatchingWords_{6}; // Minimum consecutive words to qualify as exact phrase match

public:
    explicit ExactMatchStrategy(size_t minMatchingWords = 6);
    ~ExactMatchStrategy() override = default;

    std::string getName() const override { return "Exact Match Strategy"; }
    MatchType getMatchType() const override { return MatchType::EXACT_MATCH; }

    double calculateSimilarity(const Document& docA, const Document& docB) const override;

    std::vector<MatchRecord> findPassageMatches(
        const Document& queryDoc,
        const Document& targetDoc,
        double threshold) const override;

    size_t getMinMatchingWords() const noexcept { return minMatchingWords_; }
    void setMinMatchingWords(size_t count) { minMatchingWords_ = count; }
};

} // namespace docutrace

#endif // DOCUTRACE_EXACTMATCHSTRATEGY_H
