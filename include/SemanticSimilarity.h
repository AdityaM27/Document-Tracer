#ifndef DOCUTRACE_SEMANTICSIMILARITY_H
#define DOCUTRACE_SEMANTICSIMILARITY_H

#include <vector>
#include <unordered_map>
#include "SimilarityStrategy.h"

namespace docutrace {

/**
 * @brief Identifies semantic similarity and paraphrase candidates using vector representations
 * and cosine similarity: cos(u, v) = (u . v) / (||u|| * ||v||).
 * Demonstrates Unit III: Polymorphism, Unit I: Vector math & zero-division safety.
 */
class SemanticSimilarity : public SimilarityStrategy {
private:
    std::string modelEndpoint_;
    bool isEndpointAvailable_{false};

public:
    explicit SemanticSimilarity(const std::string& endpoint = "http://127.0.0.1:8000/embed");
    ~SemanticSimilarity() override = default;

    std::string getName() const override { return "Semantic Similarity (Cosine Embedding)"; }
    MatchType getMatchType() const override { return MatchType::SEMANTIC_MATCH; }

    double calculateSimilarity(const Document& docA, const Document& docB) const override;

    std::vector<MatchRecord> findPassageMatches(
        const Document& queryDoc,
        const Document& targetDoc,
        double threshold) const override;

    /**
     * @brief Computes Cosine similarity between two float vectors.
     * Guaranteed safe against zero-length or zero-norm vectors.
     */
    static double cosineSimilarity(const std::vector<float>& vecA, const std::vector<float>& vecB);

    /**
     * @brief Computes character n-gram / subword lexical embedding fallback when offline.
     */
    static std::vector<float> computeFallbackEmbedding(const std::string& text, size_t dim = 128);

    bool isEndpointAvailable() const noexcept { return isEndpointAvailable_; }
    void setEndpointAvailable(bool available) { isEndpointAvailable_ = available; }
};

} // namespace docutrace

#endif // DOCUTRACE_SEMANTICSIMILARITY_H
