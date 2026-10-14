#ifndef DOCUTRACE_WRITINGPATTERNANALYZER_H
#define DOCUTRACE_WRITINGPATTERNANALYZER_H

#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include "Models.h"
#include "Document.h"

namespace docutrace {

/**
 * @brief Analyzes explainable statistical stylometric features of a document.
 * Demonstrates Unit I: STL algorithms (accumulate, transform), math, maps, and Unit II: OOP.
 */
class WritingPatternAnalyzer {
private:
    std::unordered_set<std::string> transitionPhrases_;

    void initializeDefaultTransitionPhrases();

public:
    WritingPatternAnalyzer();
    ~WritingPatternAnalyzer() = default;

    /**
     * @brief Computes comprehensive statistical writing pattern metrics.
     */
    WritingPatternStats analyze(const Document& doc) const;

    /**
     * @brief Calculates Type-Token Ratio (Vocabulary Diversity).
     */
    static double calculateTypeTokenRatio(size_t uniqueWords, size_t totalWords);

    /**
     * @brief Calculates Root TTR (Guiraud's Index: unique / sqrt(total)).
     */
    static double calculateRootTTR(size_t uniqueWords, size_t totalWords);

    /**
     * @brief Calculates burstiness metric between -1.0 and 1.0 based on sentence length variation.
     */
    static double calculateBurstiness(double mean, double stdDev);

    /**
     * @brief Calculates Shannon character/token entropy.
     */
    static double calculateEntropy(const std::vector<Token>& tokens);

    void addTransitionPhrase(const std::string& phrase);
};

} // namespace docutrace

#endif // DOCUTRACE_WRITINGPATTERNANALYZER_H
