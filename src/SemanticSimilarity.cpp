#include "../include/SemanticSimilarity.h"
#include <cmath>
#include <numeric>
#include <unordered_map>
#include <sstream>

namespace docutrace {

SemanticSimilarity::SemanticSimilarity(const std::string& endpoint)
    : modelEndpoint_(endpoint), isEndpointAvailable_(false) {}

double SemanticSimilarity::cosineSimilarity(const std::vector<float>& vecA, const std::vector<float>& vecB) {
    if (vecA.empty() || vecB.empty() || vecA.size() != vecB.size()) {
        return 0.0;
    }

    double dotProduct = 0.0;
    double normA = 0.0;
    double normB = 0.0;

    for (size_t i = 0; i < vecA.size(); ++i) {
        dotProduct += static_cast<double>(vecA[i]) * static_cast<double>(vecB[i]);
        normA += static_cast<double>(vecA[i]) * static_cast<double>(vecA[i]);
        normB += static_cast<double>(vecB[i]) * static_cast<double>(vecB[i]);
    }

    if (normA <= 1e-9 || normB <= 1e-9) {
        return 0.0;
    }

    double cosine = dotProduct / (std::sqrt(normA) * std::sqrt(normB));
    if (cosine > 1.0) cosine = 1.0;
    if (cosine < 0.0) cosine = 0.0;

    return cosine;
}

std::vector<float> SemanticSimilarity::computeFallbackEmbedding(const std::string& text, size_t dim) {
    std::vector<float> embedding(dim, 0.0f);
    if (text.empty()) return embedding;

    // Feature hashing / character 3-gram embedding
    for (size_t i = 0; i + 2 < text.size(); ++i) {
        uint32_t hash = 2166136261u;
        for (size_t k = 0; k < 3; ++k) {
            hash ^= static_cast<unsigned char>(std::tolower(text[i + k]));
            hash *= 16777619u;
        }
        size_t idx = hash % dim;
        float sign = (hash & 0x1000) ? 1.0f : -1.0f;
        embedding[idx] += sign;
    }

    // Also hash individual words
    std::istringstream iss(text);
    std::string word;
    while (iss >> word) {
        uint32_t hash = 2166136261u;
        for (char c : word) {
            hash ^= static_cast<unsigned char>(std::tolower(c));
            hash *= 16777619u;
        }
        size_t idx = hash % dim;
        embedding[idx] += 2.0f;
    }

    // Normalize vector to unit length
    float norm = 0.0f;
    for (float val : embedding) norm += val * val;
    if (norm > 1e-9f) {
        float sqrtNorm = std::sqrt(norm);
        for (float& val : embedding) val /= sqrtNorm;
    }

    return embedding;
}

double SemanticSimilarity::calculateSimilarity(const Document& docA, const Document& docB) const {
    auto embA = computeFallbackEmbedding(docA.getNormalizedText());
    auto embB = computeFallbackEmbedding(docB.getNormalizedText());

    return cosineSimilarity(embA, embB);
}

std::vector<MatchRecord> SemanticSimilarity::findPassageMatches(
    const Document& queryDoc,
    const Document& targetDoc,
    double threshold) const {

    std::vector<MatchRecord> results;
    const auto& qPassages = queryDoc.getPassageWindows().empty() ? queryDoc.getSentences() : queryDoc.getPassageWindows();
    const auto& tPassages = targetDoc.getPassageWindows().empty() ? targetDoc.getSentences() : targetDoc.getPassageWindows();

    if (qPassages.empty() || tPassages.empty()) return results;

    std::vector<std::vector<float>> targetEmbeddings;
    targetEmbeddings.reserve(tPassages.size());
    for (const auto& tp : tPassages) {
        targetEmbeddings.push_back(computeFallbackEmbedding(tp.normalizedText));
    }

    std::vector<bool> queryCovered(queryDoc.getOriginalText().size(), false);
    size_t matchIdCounter = 1;

    for (const auto& qPassage : qPassages) {
        if (qPassage.normalizedText.size() < 15) continue;

        size_t midPoint = qPassage.startChar + (qPassage.endChar - qPassage.startChar) / 2;
        if (midPoint < queryCovered.size() && queryCovered[midPoint]) {
            continue;
        }

        auto queryEmbedding = computeFallbackEmbedding(qPassage.normalizedText);

        double bestScore = 0.0;
        size_t bestTargetIdx = 0;

        for (size_t t = 0; t < tPassages.size(); ++t) {
            if (tPassages[t].normalizedText.size() < 15) continue;

            double score = cosineSimilarity(queryEmbedding, targetEmbeddings[t]);
            if (score > bestScore) {
                bestScore = score;
                bestTargetIdx = t;
            }
        }

        if (bestScore >= threshold && bestTargetIdx < tPassages.size()) {
            MatchRecord record;
            record.id = "semantic_" + std::to_string(matchIdCounter++);
            record.queryDocId = queryDoc.getId();
            record.targetDocId = targetDoc.getId();
            record.targetDocName = targetDoc.getFilename();
            record.targetSourceType = "LOCAL_REFERENCE";
            record.matchType = MatchType::SEMANTIC_MATCH;
            record.strategyName = getName();
            record.score = bestScore;
            record.queryPassage = qPassage;
            record.targetPassage = tPassages[bestTargetIdx];
            record.matchedTokens = {"cosine_score=" + std::to_string(bestScore)};
            record.reviewStatus = (bestScore >= 0.85) ? "POTENTIAL_PARAPHRASE" : "SEMANTIC_SIMILARITY";

            results.push_back(record);

            for (size_t c = qPassage.startChar; c < qPassage.endChar && c < queryCovered.size(); ++c) {
                queryCovered[c] = true;
            }
        }
    }

    return results;
}

} // namespace docutrace
