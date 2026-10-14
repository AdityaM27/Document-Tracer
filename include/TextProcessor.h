#ifndef DOCUTRACE_TEXTPROCESSOR_H
#define DOCUTRACE_TEXTPROCESSOR_H

#include <string>
#include <vector>
#include <unordered_set>
#include "Models.h"
#include "Document.h"
#include "Tokenizer.h"

namespace docutrace {

/**
 * @brief Handles text normalization, sentence segmentation, paragraph segmentation,
 * n-gram construction, and stop-word filtering.
 * Demonstrates Unit I & II: STL containers, algorithms, clean encapsulation.
 */
class TextProcessor {
private:
    Tokenizer tokenizer_;
    std::unordered_set<std::string> stopWords_;
    int defaultNgramSize_{3};

    void initializeDefaultStopWords();

public:
    TextProcessor();
    explicit TextProcessor(int ngramSize);
    ~TextProcessor() = default;

    // Normalization functions
    std::string normalizeWhitespace(const std::string& input) const;
    std::string normalizeText(const std::string& input, bool toLower = true, bool removePunct = true) const;

    // Segmentation functions
    std::vector<Passage> segmentSentences(const std::string& text, const std::vector<Token>& tokens) const;
    std::vector<Passage> segmentParagraphs(const std::string& text, const std::vector<Token>& tokens) const;
    std::vector<Passage> generateSlidingWindows(const std::vector<Token>& tokens, size_t windowSize = 40, size_t stepSize = 20) const;

    // N-gram extraction
    std::vector<std::string> generateNGrams(const std::vector<Token>& tokens, int n) const;
    std::vector<std::string> generateNGramsFromStrings(const std::vector<std::string>& words, int n) const;

    // Stop words
    bool isStopWord(const std::string& word) const;
    void addStopWord(const std::string& word);
    void removeStopWord(const std::string& word);
    std::vector<Token> filterStopWords(const std::vector<Token>& tokens) const;

    // Full document pipeline
    void processDocument(Document& doc, size_t windowSize = 40, size_t stepSize = 20) const;

    // Helper utilities
    static bool isAbbreviation(const std::string& word);
};

} // namespace docutrace

#endif // DOCUTRACE_TEXTPROCESSOR_H
