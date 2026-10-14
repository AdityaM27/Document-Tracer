#ifndef DOCUTRACE_MODELS_H
#define DOCUTRACE_MODELS_H

#include <string>
#include <vector>
#include <map>
#include <chrono>
#include "../third_party/nlohmann/json.hpp"

namespace docutrace {

enum class ProcessingStatus {
    UNPROCESSED,
    EXTRACTED,
    PREPROCESSED,
    INDEXED,
    ANALYZED,
    FAILED
};

enum class MatchType {
    EXACT_MATCH,
    NGRAM_MATCH,
    JACCARD_MATCH,
    SEMANTIC_MATCH
};

inline std::string matchTypeToString(MatchType type) {
    switch (type) {
        case MatchType::EXACT_MATCH: return "EXACT_MATCH";
        case MatchType::NGRAM_MATCH: return "NGRAM_MATCH";
        case MatchType::JACCARD_MATCH: return "JACCARD_MATCH";
        case MatchType::SEMANTIC_MATCH: return "SEMANTIC_MATCH";
        default: return "UNKNOWN";
    }
}

inline MatchType stringToMatchType(const std::string& str) {
    if (str == "EXACT_MATCH") return MatchType::EXACT_MATCH;
    if (str == "NGRAM_MATCH") return MatchType::NGRAM_MATCH;
    if (str == "JACCARD_MATCH") return MatchType::JACCARD_MATCH;
    if (str == "SEMANTIC_MATCH") return MatchType::SEMANTIC_MATCH;
    return MatchType::EXACT_MATCH;
}

struct Token {
    std::string text;           // Normalized lowercase text
    std::string originalText;   // As appeared in original source
    size_t startChar{0};        // Character start offset in document originalText
    size_t endChar{0};          // Character end offset in document originalText
    size_t sentenceIndex{0};
    size_t paragraphIndex{0};

    bool operator==(const Token& other) const {
        return text == other.text && startChar == other.startChar;
    }
};

struct Passage {
    size_t passageIndex{0};
    size_t startChar{0};
    size_t endChar{0};
    std::string originalText;
    std::string normalizedText;
    std::vector<Token> tokens;
    std::vector<std::string> ngrams;
    std::vector<float> embedding; // Optional sentence embedding vector
};

struct MatchRecord {
    std::string id;
    std::string queryDocId;
    std::string targetDocId;
    std::string targetDocName;
    std::string targetSourceType; // "LOCAL_REFERENCE", "ACADEMIC", "WEB"
    MatchType matchType{MatchType::EXACT_MATCH};
    std::string strategyName;
    double score{0.0};            // 0.0 to 1.0
    Passage queryPassage;
    Passage targetPassage;
    std::vector<std::string> matchedTokens;
    std::string reviewStatus;     // "EXACT_COPY", "SUBSTANTIAL_OVERLAP", "POTENTIAL_PARAPHRASE", "COMMON_KNOWLEDGE"
};

struct AcademicSource {
    std::string title;
    std::vector<std::string> authors;
    std::string publicationDate;
    std::string doi;
    std::string url;
    std::string abstractText;
    std::string provider;         // "OpenAlex", "Crossref"
    std::string retrievalStatus;  // "METADATA_DISCOVERED", "ABSTRACT_RETRIEVED", "FULL_TEXT_RETRIEVED", "PAGE_INACCESSIBLE", "FAILED"
    double relevanceScore{0.0};
};

struct WritingPatternStats {
    size_t totalWords{0};
    size_t uniqueWords{0};
    double typeTokenRatio{0.0};     // Unique / Total
    double rootTTR{0.0};            // Unique / sqrt(Total)
    size_t totalSentences{0};
    double avgSentenceLength{0.0};
    double sentenceLengthVariance{0.0};
    double sentenceLengthStdDev{0.0};
    double burstiness{0.0};         // (StdDev - Mean) / (StdDev + Mean)
    double characterEntropy{0.0};   // Shannon entropy
    std::map<std::string, size_t> transitionPhraseFrequencies;
    std::vector<std::string> repeatedPhrases;
    std::string interpretation;
};

struct AIClassificationResult {
    bool serviceAvailable{false};
    std::string modelName{"roberta-base-openai-detector"};
    std::string label{"Uncertain"};  // "AI-like", "Human-like", "Uncertain"
    double aiProbability{0.0};
    double humanProbability{0.0};
    double confidence{0.0};
    std::string reasoning;
    std::string disclaimer{
        "DISCLAIMER: AI-generated text classification is probabilistic and experimental. "
        "It should never be treated as definitive proof of authorship."
    };
    bool isPretrainedModel{false};
    std::string inferenceMode{"offline"}; // "huggingface", "calibrated_statistical", "offline"
};

struct AnalysisOptions {
    bool enableExactMatch{true};
    bool enableNGram{true};
    bool enableJaccard{true};
    bool enableSemantic{true};
    bool enableAcademicSearch{true};
    bool enableWebSearch{false};
    bool enableAIClassifier{true};
    bool enableWritingPatterns{true};
    int ngramSize{3};
    double exactThreshold{0.80};
    double ngramThreshold{0.40};
    double jaccardThreshold{0.35};
    double semanticThreshold{0.65};
    int maxConcurrentThreads{4};
};

struct AnalysisSummary {
    std::string submissionId;
    std::string submissionFilename;
    std::string timestamp;
    double overallSimilarityScore{0.0}; // Aggregated weighted or non-overlapping coverage
    double exactSimilarityScore{0.0};
    double ngramSimilarityScore{0.0};
    double jaccardSimilarityScore{0.0};
    double semanticSimilarityScore{0.0};
    size_t referencesCheckedCount{0};
    size_t totalPassagesCount{0};
    size_t matchedPassagesCount{0};
    std::vector<MatchRecord> matches;
    std::vector<AcademicSource> discoveredSources;
    WritingPatternStats writingStats;
    AIClassificationResult aiResult;
    std::vector<std::string> errorsAndWarnings;
    double processingTimeMs{0.0};
};

// JSON serialization helpers using nlohmann::json
inline void to_json(nlohmann::json& j, const Token& t) {
    j = nlohmann::json{
        {"text", t.text},
        {"originalText", t.originalText},
        {"startChar", t.startChar},
        {"endChar", t.endChar},
        {"sentenceIndex", t.sentenceIndex},
        {"paragraphIndex", t.paragraphIndex}
    };
}

inline void from_json(const nlohmann::json& j, Token& t) {
    j.at("text").get_to(t.text);
    if (j.contains("originalText")) j.at("originalText").get_to(t.originalText);
    if (j.contains("startChar")) j.at("startChar").get_to(t.startChar);
    if (j.contains("endChar")) j.at("endChar").get_to(t.endChar);
    if (j.contains("sentenceIndex")) j.at("sentenceIndex").get_to(t.sentenceIndex);
    if (j.contains("paragraphIndex")) j.at("paragraphIndex").get_to(t.paragraphIndex);
}

inline void to_json(nlohmann::json& j, const Passage& p) {
    j = nlohmann::json{
        {"passageIndex", p.passageIndex},
        {"startChar", p.startChar},
        {"endChar", p.endChar},
        {"originalText", p.originalText},
        {"normalizedText", p.normalizedText},
        {"ngrams", p.ngrams},
        {"tokens", p.tokens}
    };
}

inline void from_json(const nlohmann::json& j, Passage& p) {
    if (j.contains("passageIndex")) j.at("passageIndex").get_to(p.passageIndex);
    if (j.contains("startChar")) j.at("startChar").get_to(p.startChar);
    if (j.contains("endChar")) j.at("endChar").get_to(p.endChar);
    if (j.contains("originalText")) j.at("originalText").get_to(p.originalText);
    if (j.contains("normalizedText")) j.at("normalizedText").get_to(p.normalizedText);
    if (j.contains("ngrams")) j.at("ngrams").get_to(p.ngrams);
    if (j.contains("tokens") && j["tokens"].is_array()) {
        for (const auto& tj : j["tokens"]) {
            Token t;
            from_json(tj, t);
            p.tokens.push_back(t);
        }
    }
}

inline void to_json(nlohmann::json& j, const MatchRecord& m) {
    j = nlohmann::json{
        {"id", m.id},
        {"queryDocId", m.queryDocId},
        {"targetDocId", m.targetDocId},
        {"targetDocName", m.targetDocName},
        {"targetSourceType", m.targetSourceType},
        {"matchType", matchTypeToString(m.matchType)},
        {"strategyName", m.strategyName},
        {"score", m.score},
        {"queryPassage", m.queryPassage},
        {"targetPassage", m.targetPassage},
        {"matchedTokens", m.matchedTokens},
        {"reviewStatus", m.reviewStatus}
    };
}

inline void to_json(nlohmann::json& j, const AcademicSource& s) {
    j = nlohmann::json{
        {"title", s.title},
        {"authors", s.authors},
        {"publicationDate", s.publicationDate},
        {"doi", s.doi},
        {"url", s.url},
        {"abstractText", s.abstractText},
        {"provider", s.provider},
        {"retrievalStatus", s.retrievalStatus},
        {"relevanceScore", s.relevanceScore}
    };
}

inline void to_json(nlohmann::json& j, const WritingPatternStats& w) {
    j = nlohmann::json{
        {"totalWords", w.totalWords},
        {"uniqueWords", w.uniqueWords},
        {"typeTokenRatio", w.typeTokenRatio},
        {"rootTTR", w.rootTTR},
        {"totalSentences", w.totalSentences},
        {"avgSentenceLength", w.avgSentenceLength},
        {"sentenceLengthVariance", w.sentenceLengthVariance},
        {"sentenceLengthStdDev", w.sentenceLengthStdDev},
        {"burstiness", w.burstiness},
        {"characterEntropy", w.characterEntropy},
        {"transitionPhraseFrequencies", w.transitionPhraseFrequencies},
        {"repeatedPhrases", w.repeatedPhrases},
        {"interpretation", w.interpretation}
    };
}

inline void to_json(nlohmann::json& j, const AIClassificationResult& a) {
    j = nlohmann::json{
        {"serviceAvailable", a.serviceAvailable},
        {"modelName", a.modelName},
        {"label", a.label},
        {"aiProbability", a.aiProbability},
        {"humanProbability", a.humanProbability},
        {"confidence", a.confidence},
        {"reasoning", a.reasoning},
        {"disclaimer", a.disclaimer},
        {"isPretrainedModel", a.isPretrainedModel},
        {"inferenceMode", a.inferenceMode}
    };
}

inline void to_json(nlohmann::json& j, const AnalysisSummary& s) {
    j = nlohmann::json{
        {"submissionId", s.submissionId},
        {"submissionFilename", s.submissionFilename},
        {"timestamp", s.timestamp},
        {"overallSimilarityScore", s.overallSimilarityScore},
        {"exactSimilarityScore", s.exactSimilarityScore},
        {"ngramSimilarityScore", s.ngramSimilarityScore},
        {"jaccardSimilarityScore", s.jaccardSimilarityScore},
        {"semanticSimilarityScore", s.semanticSimilarityScore},
        {"referencesCheckedCount", s.referencesCheckedCount},
        {"totalPassagesCount", s.totalPassagesCount},
        {"matchedPassagesCount", s.matchedPassagesCount},
        {"matches", s.matches},
        {"discoveredSources", s.discoveredSources},
        {"writingStats", s.writingStats},
        {"aiResult", s.aiResult},
        {"errorsAndWarnings", s.errorsAndWarnings},
        {"processingTimeMs", s.processingTimeMs}
    };
}

} // namespace docutrace

#endif // DOCUTRACE_MODELS_H
