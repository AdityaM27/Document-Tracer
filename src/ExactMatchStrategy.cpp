#include "../include/ExactMatchStrategy.h"
#include <unordered_set>
#include <algorithm>

namespace docutrace {

ExactMatchStrategy::ExactMatchStrategy(size_t minMatchingWords)
    : minMatchingWords_(minMatchingWords) {}

double ExactMatchStrategy::calculateSimilarity(const Document& docA, const Document& docB) const {
    const auto& tokensA = docA.getTokens();
    const auto& tokensB = docB.getTokens();

    if (tokensA.empty() || tokensB.empty()) return 0.0;

    // Track matching token positions in docA to prevent duplicate counting
    std::vector<bool> matched(tokensA.size(), false);

    // Build hash index of n-word sequences from docB
    if (tokensB.size() < minMatchingWords_) return 0.0;

    std::unordered_set<std::string> phrasesB;
    for (size_t j = 0; j <= tokensB.size() - minMatchingWords_; ++j) {
        std::string phrase;
        for (size_t k = 0; k < minMatchingWords_; ++k) {
            phrase += tokensB[j + k].text + " ";
        }
        phrasesB.insert(phrase);
    }

    if (tokensA.size() < minMatchingWords_) return 0.0;

    for (size_t i = 0; i <= tokensA.size() - minMatchingWords_; ++i) {
        std::string phrase;
        for (size_t k = 0; k < minMatchingWords_; ++k) {
            phrase += tokensA[i + k].text + " ";
        }
        if (phrasesB.find(phrase) != phrasesB.end()) {
            for (size_t k = 0; k < minMatchingWords_; ++k) {
                matched[i + k] = true;
            }
        }
    }

    size_t matchedCount = 0;
    for (bool m : matched) {
        if (m) matchedCount++;
    }

    return static_cast<double>(matchedCount) / static_cast<double>(tokensA.size());
}

std::vector<MatchRecord> ExactMatchStrategy::findPassageMatches(
    const Document& queryDoc,
    const Document& targetDoc,
    double threshold) const {

    std::vector<MatchRecord> results;
    const auto& qPassages = queryDoc.getSentences().empty() ? queryDoc.getPassageWindows() : queryDoc.getSentences();
    const auto& tPassages = targetDoc.getSentences().empty() ? targetDoc.getPassageWindows() : targetDoc.getSentences();

    if (qPassages.empty() || tPassages.empty()) return results;

    // To prevent duplicate overlapping windows from being reported
    std::vector<bool> queryCovered(queryDoc.getOriginalText().size(), false);

    size_t matchIdCounter = 1;

    for (const auto& qPassage : qPassages) {
        size_t qWords = qPassage.tokens.empty() ? (qPassage.normalizedText.size() / 5) : qPassage.tokens.size();
        if (qWords < minMatchingWords_) continue;

        // Check if passage is already covered
        size_t midPoint = qPassage.startChar + (qPassage.endChar - qPassage.startChar) / 2;
        if (midPoint < queryCovered.size() && queryCovered[midPoint]) {
            continue;
        }

        double bestScore = 0.0;
        const Passage* bestTargetPassage = nullptr;
        std::vector<std::string> matchedTokens;

        for (const auto& tPassage : tPassages) {
            size_t tWords = tPassage.tokens.empty() ? (tPassage.normalizedText.size() / 5) : tPassage.tokens.size();
            if (tWords < minMatchingWords_) continue;

            // Check exact phrase match
            if (qPassage.normalizedText == tPassage.normalizedText && !qPassage.normalizedText.empty()) {
                bestScore = 1.0;
                bestTargetPassage = &tPassage;
                for (const auto& t : qPassage.tokens) {
                    matchedTokens.push_back(t.text);
                }
                break;
            }

            // Substring or sub-sequence exact match
            if (tPassage.normalizedText.find(qPassage.normalizedText) != std::string::npos && !qPassage.normalizedText.empty()) {
                double score = static_cast<double>(qPassage.normalizedText.size()) / static_cast<double>(tPassage.normalizedText.size());
                if (score > bestScore) {
                    bestScore = 1.0;
                    bestTargetPassage = &tPassage;
                    for (const auto& t : qPassage.tokens) {
                        matchedTokens.push_back(t.text);
                    }
                }
            } else if (qPassage.normalizedText.find(tPassage.normalizedText) != std::string::npos && !tPassage.normalizedText.empty()) {
                double score = static_cast<double>(tPassage.normalizedText.size()) / static_cast<double>(qPassage.normalizedText.size());
                if (score > bestScore) {
                    bestScore = score;
                    bestTargetPassage = &tPassage;
                    for (const auto& t : tPassage.tokens) {
                        matchedTokens.push_back(t.text);
                    }
                }
            }
        }

        if (bestTargetPassage != nullptr && bestScore >= threshold) {
            MatchRecord record;
            record.id = "exact_" + std::to_string(matchIdCounter++);
            record.queryDocId = queryDoc.getId();
            record.targetDocId = targetDoc.getId();
            record.targetDocName = targetDoc.getFilename();
            record.targetSourceType = "LOCAL_REFERENCE";
            record.matchType = MatchType::EXACT_MATCH;
            record.strategyName = getName();
            record.score = bestScore;
            record.queryPassage = qPassage;
            record.targetPassage = *bestTargetPassage;
            record.matchedTokens = matchedTokens;
            record.reviewStatus = (bestScore >= 0.95) ? "EXACT_COPY" : "SUBSTANTIAL_OVERLAP";

            results.push_back(record);

            // Mark span as covered
            for (size_t c = qPassage.startChar; c < qPassage.endChar && c < queryCovered.size(); ++c) {
                queryCovered[c] = true;
            }
        }
    }

    return results;
}

} // namespace docutrace
