#include "../include/WritingPatternAnalyzer.h"
#include "../include/Tokenizer.h"
#include <cmath>
#include <numeric>
#include <unordered_map>
#include <sstream>

namespace docutrace {

WritingPatternAnalyzer::WritingPatternAnalyzer() {
    initializeDefaultTransitionPhrases();
}

void WritingPatternAnalyzer::initializeDefaultTransitionPhrases() {
    const char* phrases[] = {
        "moreover", "furthermore", "in conclusion", "consequently", "as a result",
        "nevertheless", "on the other hand", "to summarize", "in addition", "additionally",
        "therefore", "thus", "however", "in contrast", "subsequently",
        "in particular", "specifically", "for instance", "for example", "in other words"
    };
    for (const char* p : phrases) {
        transitionPhrases_.insert(p);
    }
}

void WritingPatternAnalyzer::addTransitionPhrase(const std::string& phrase) {
    transitionPhrases_.insert(Tokenizer::toLower(phrase));
}

double WritingPatternAnalyzer::calculateTypeTokenRatio(size_t uniqueWords, size_t totalWords) {
    if (totalWords == 0) return 0.0;
    return static_cast<double>(uniqueWords) / static_cast<double>(totalWords);
}

double WritingPatternAnalyzer::calculateRootTTR(size_t uniqueWords, size_t totalWords) {
    if (totalWords == 0) return 0.0;
    return static_cast<double>(uniqueWords) / std::sqrt(static_cast<double>(totalWords));
}

double WritingPatternAnalyzer::calculateBurstiness(double mean, double stdDev) {
    if (mean + stdDev <= 1e-9) return 0.0;
    return (stdDev - mean) / (stdDev + mean);
}

double WritingPatternAnalyzer::calculateEntropy(const std::vector<Token>& tokens) {
    if (tokens.empty()) return 0.0;

    std::unordered_map<std::string, size_t> freqs;
    for (const auto& t : tokens) {
        freqs[t.text]++;
    }

    double total = static_cast<double>(tokens.size());
    double entropy = 0.0;
    for (const auto& pair : freqs) {
        double p = static_cast<double>(pair.second) / total;
        if (p > 0.0) {
            entropy -= p * (std::log(p) / std::log(2.0));
        }
    }

    return entropy;
}

WritingPatternStats WritingPatternAnalyzer::analyze(const Document& doc) const {
    WritingPatternStats stats;
    const auto& tokens = doc.getTokens();
    const auto& sentences = doc.getSentences();

    stats.totalWords = tokens.size();
    if (stats.totalWords == 0) {
        stats.interpretation = "Document contains insufficient text for writing pattern analysis.";
        return stats;
    }

    // Unique words & TTR
    std::unordered_map<std::string, size_t> wordCounts;
    for (const auto& t : tokens) {
        wordCounts[t.text]++;
    }
    stats.uniqueWords = wordCounts.size();
    stats.typeTokenRatio = calculateTypeTokenRatio(stats.uniqueWords, stats.totalWords);
    stats.rootTTR = calculateRootTTR(stats.uniqueWords, stats.totalWords);

    // Sentence lengths
    stats.totalSentences = sentences.empty() ? 1 : sentences.size();
    std::vector<double> sentenceLengths;
    if (!sentences.empty()) {
        for (const auto& s : sentences) {
            sentenceLengths.push_back(static_cast<double>(s.tokens.size()));
        }
    } else {
        sentenceLengths.push_back(static_cast<double>(stats.totalWords));
    }

    double sum = std::accumulate(sentenceLengths.begin(), sentenceLengths.end(), 0.0);
    stats.avgSentenceLength = sum / sentenceLengths.size();

    double accum = 0.0;
    for (double len : sentenceLengths) {
        accum += (len - stats.avgSentenceLength) * (len - stats.avgSentenceLength);
    }
    stats.sentenceLengthVariance = accum / sentenceLengths.size();
    stats.sentenceLengthStdDev = std::sqrt(stats.sentenceLengthVariance);
    stats.burstiness = calculateBurstiness(stats.avgSentenceLength, stats.sentenceLengthStdDev);

    // Shannon Entropy
    stats.characterEntropy = calculateEntropy(tokens);

    // Transition phrase detection
    std::string lowerText = Tokenizer::toLower(doc.getOriginalText());
    for (const auto& phrase : transitionPhrases_) {
        size_t count = 0;
        size_t pos = 0;
        while ((pos = lowerText.find(phrase, pos)) != std::string::npos) {
            count++;
            pos += phrase.size();
        }
        if (count > 0) {
            stats.transitionPhraseFrequencies[phrase] = count;
        }
    }

    // Repeated 3-word phrases (occurring 3+ times)
    std::unordered_map<std::string, size_t> phraseFreq;
    if (tokens.size() >= 3) {
        for (size_t i = 0; i <= tokens.size() - 3; ++i) {
            std::string tri = tokens[i].text + " " + tokens[i + 1].text + " " + tokens[i + 2].text;
            phraseFreq[tri]++;
        }
        for (const auto& pair : phraseFreq) {
            if (pair.second >= 3) {
                stats.repeatedPhrases.push_back("\"" + pair.first + "\" (" + std::to_string(pair.second) + "x)");
            }
        }
    }

    // Objective explanation
    std::stringstream ss;
    ss << "Writing Pattern Observations: "
       << "Vocabulary diversity (TTR) is " << (stats.typeTokenRatio > 0.6 ? "high" : (stats.typeTokenRatio > 0.4 ? "moderate" : "low"))
       << " (" << std::round(stats.typeTokenRatio * 1000.0) / 10.0 << "%). "
       << "Sentence length burstiness is " << std::round(stats.burstiness * 100.0) / 100.0 << ". "
       << "Identified " << stats.transitionPhraseFrequencies.size() << " distinct transition phrase types. "
       << "NOTE: Statistical stylometry highlights structural tendencies and must not be used as isolated proof of AI or human authorship.";
    stats.interpretation = ss.str();

    return stats;
}

} // namespace docutrace
