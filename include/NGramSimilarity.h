#ifndef DOCUTRACE_NGRAMSIMILARITY_H
#define DOCUTRACE_NGRAMSIMILARITY_H

#include "SimilarityStrategy.h"

namespace docutrace {

/**
 * @brief Computes similarity based on overlapping n-gram word sequences.
 * Demonstrates Unit III: Polymorphism and Unit I: STL algorithms.
 */
class NGramSimilarity : public SimilarityStrategy {
private:
    int n_{3}; // Default 3-grams

public:
    explicit NGramSimilarity(int n = 3);
    ~NGramSimilarity() override = default;

    std::string getName() const override { return "N-Gram Similarity (" + std::to_string(n_) + "-gram)"; }
    MatchType getMatchType() const override { return MatchType::NGRAM_MATCH; }

    double calculateSimilarity(const Document& docA, const Document& docB) const override;

    std::vector<MatchRecord> findPassageMatches(
        const Document& queryDoc,
        const Document& targetDoc,
        double threshold) const override;

    int getNGramSize() const noexcept { return n_; }
    void setNGramSize(int n) { if (n > 0) n_ = n; }

    // Computes containment of n-grams of passageA in passageB
    double computeNGramContainment(const std::vector<std::string>& ngramsA, const std::vector<std::string>& ngramsB) const;
};

} // namespace docutrace

#endif // DOCUTRACE_NGRAMSIMILARITY_H
