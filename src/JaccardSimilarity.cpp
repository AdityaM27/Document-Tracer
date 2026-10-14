#include "../include/JaccardSimilarity.h"
#include <unordered_set>
#include <algorithm>

namespace docutrace {

double JaccardSimilarity::computeJaccard(
    const std::unordered_set<std::string>& setA,
    const std::unordered_set<std::string>& setB) {

    if (setA.empty() && setB.empty()) return 0.0;
    if (setA.empty() || setB.empty()) return 0.0;

    // Iterate through smaller set for faster intersection check
    const auto& smaller = (setA.size() < setB.size()) ? setA : setB;
    const auto& larger  = (setA.size() < setB.size()) ? setB : setA;

    size_t intersectionCount = 0;
    for (const auto& elem : smaller) {
        if (larger.find(elem) != larger.end()) {
            intersectionCount++;
        }
    }

    size_t unionCount = setA.size() + setB.size() - intersectionCount;
    if (unionCount == 0) return 0.0;

    return static_cast<double>(intersectionCount) / static_cast<double>(unionCount);
}

double JaccardSimilarity::calculateSimilarity(const Document& docA, const Document& docB) const {
    const auto& tokensA = docA.getTokens();
    const auto& tokensB = docB.getTokens();

    if (tokensA.empty() || tokensB.empty()) return 0.0;

    std::unordered_set<std::string> setA;
    for (const auto& t : tokensA) setA.insert(t.text);

    std::unordered_set<std::string> setB;
    for (const auto& t : tokensB) setB.insert(t.text);

    return computeJaccard(setA, setB);
}

std::vector<MatchRecord> JaccardSimilarity::findPassageMatches(
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
        if (qPassage.tokens.empty()) continue;

        size_t midPoint = qPassage.startChar + (qPassage.endChar - qPassage.startChar) / 2;
        if (midPoint < queryCovered.size() && queryCovered[midPoint]) {
            continue;
        }

        std::unordered_set<std::string> setQ;
        for (const auto& t : qPassage.tokens) setQ.insert(t.text);

        double bestScore = 0.0;
        const Passage* bestTargetPassage = nullptr;

        for (const auto& tPassage : tPassages) {
            if (tPassage.tokens.empty()) continue;

            std::unordered_set<std::string> setT;
            for (const auto& t : tPassage.tokens) setT.insert(t.text);

            double score = computeJaccard(setQ, setT);
            if (score > bestScore) {
                bestScore = score;
                bestTargetPassage = &tPassage;
            }
        }

        if (bestTargetPassage != nullptr && bestScore >= threshold) {
            std::unordered_set<std::string> setT;
            for (const auto& t : bestTargetPassage->tokens) setT.insert(t.text);

            std::vector<std::string> matchedTokens;
            for (const auto& word : setQ) {
                if (setT.find(word) != setT.end()) {
                    matchedTokens.push_back(word);
                }
            }

            MatchRecord record;
            record.id = "jaccard_" + std::to_string(matchIdCounter++);
            record.queryDocId = queryDoc.getId();
            record.targetDocId = targetDoc.getId();
            record.targetDocName = targetDoc.getFilename();
            record.targetSourceType = "LOCAL_REFERENCE";
            record.matchType = MatchType::JACCARD_MATCH;
            record.strategyName = getName();
            record.score = bestScore;
            record.queryPassage = qPassage;
            record.targetPassage = *bestTargetPassage;
            record.matchedTokens = matchedTokens;
            record.reviewStatus = (bestScore >= 0.60) ? "SUBSTANTIAL_OVERLAP" : "POTENTIAL_PARAPHRASE";

            results.push_back(record);

            for (size_t c = qPassage.startChar; c < qPassage.endChar && c < queryCovered.size(); ++c) {
                queryCovered[c] = true;
            }
        }
    }

    return results;
}

} // namespace docutrace
