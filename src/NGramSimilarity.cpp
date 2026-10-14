#include "../include/NGramSimilarity.h"
#include <unordered_set>
#include <algorithm>

namespace docutrace {

NGramSimilarity::NGramSimilarity(int n) : n_(n) {}

double NGramSimilarity::computeNGramContainment(
    const std::vector<std::string>& ngramsA,
    const std::vector<std::string>& ngramsB) const {

    if (ngramsA.empty() || ngramsB.empty()) return 0.0;

    std::unordered_set<std::string> setB(ngramsB.begin(), ngramsB.end());
    size_t sharedCount = 0;

    for (const auto& gram : ngramsA) {
        if (setB.find(gram) != setB.end()) {
            sharedCount++;
        }
    }

    return static_cast<double>(sharedCount) / static_cast<double>(ngramsA.size());
}

double NGramSimilarity::calculateSimilarity(const Document& docA, const Document& docB) const {
    const auto& tokensA = docA.getTokens();
    const auto& tokensB = docB.getTokens();

    if (tokensA.size() < static_cast<size_t>(n_) || tokensB.size() < static_cast<size_t>(n_)) {
        return 0.0;
    }

    std::unordered_set<std::string> ngramsA;
    for (size_t i = 0; i <= tokensA.size() - n_; ++i) {
        std::string gram;
        for (int k = 0; k < n_; ++k) {
            gram += tokensA[i + k].text + " ";
        }
        ngramsA.insert(gram);
    }

    std::unordered_set<std::string> ngramsB;
    for (size_t j = 0; j <= tokensB.size() - n_; ++j) {
        std::string gram;
        for (int k = 0; k < n_; ++k) {
            gram += tokensB[j + k].text + " ";
        }
        ngramsB.insert(gram);
    }

    if (ngramsA.empty()) return 0.0;

    size_t intersectionCount = 0;
    for (const auto& g : ngramsA) {
        if (ngramsB.find(g) != ngramsB.end()) {
            intersectionCount++;
        }
    }

    // Containment score: proportion of document A's n-grams found in document B
    return static_cast<double>(intersectionCount) / static_cast<double>(ngramsA.size());
}

std::vector<MatchRecord> NGramSimilarity::findPassageMatches(
    const Document& queryDoc,
    const Document& targetDoc,
    double threshold) const {

    std::vector<MatchRecord> results;
    const auto& qPassages = queryDoc.getPassageWindows().empty() ? queryDoc.getSentences() : queryDoc.getPassageWindows();
    const auto& tPassages = targetDoc.getPassageWindows().empty() ? targetDoc.getSentences() : targetDoc.getPassageWindows();

    if (qPassages.empty() || tPassages.empty()) return results;

    std::vector<bool> queryCovered(queryDoc.getOriginalText().size(), false);
    size_t matchIdCounter = 1;

    for (const auto& qPassage : qPassages) {
        if (qPassage.ngrams.empty()) continue;

        size_t midPoint = qPassage.startChar + (qPassage.endChar - qPassage.startChar) / 2;
        if (midPoint < queryCovered.size() && queryCovered[midPoint]) {
            continue;
        }

        double bestScore = 0.0;
        const Passage* bestTargetPassage = nullptr;
        std::vector<std::string> matchedTokens;

        for (const auto& tPassage : tPassages) {
            if (tPassage.ngrams.empty()) continue;

            double score = computeNGramContainment(qPassage.ngrams, tPassage.ngrams);
            if (score > bestScore) {
                bestScore = score;
                bestTargetPassage = &tPassage;
            }
        }

        if (bestTargetPassage != nullptr && bestScore >= threshold) {
            std::unordered_set<std::string> setT(bestTargetPassage->ngrams.begin(), bestTargetPassage->ngrams.end());
            for (const auto& g : qPassage.ngrams) {
                if (setT.find(g) != setT.end()) {
                    matchedTokens.push_back(g);
                }
            }

            MatchRecord record;
            record.id = "ngram_" + std::to_string(matchIdCounter++);
            record.queryDocId = queryDoc.getId();
            record.targetDocId = targetDoc.getId();
            record.targetDocName = targetDoc.getFilename();
            record.targetSourceType = "LOCAL_REFERENCE";
            record.matchType = MatchType::NGRAM_MATCH;
            record.strategyName = getName();
            record.score = bestScore;
            record.queryPassage = qPassage;
            record.targetPassage = *bestTargetPassage;
            record.matchedTokens = matchedTokens;
            record.reviewStatus = (bestScore >= 0.75) ? "SUBSTANTIAL_OVERLAP" : "POTENTIAL_PARAPHRASE";

            results.push_back(record);

            for (size_t c = qPassage.startChar; c < qPassage.endChar && c < queryCovered.size(); ++c) {
                queryCovered[c] = true;
            }
        }
    }

    return results;
}

} // namespace docutrace
