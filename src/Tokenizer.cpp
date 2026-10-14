#include "../include/Tokenizer.h"
#include <cctype>
#include <algorithm>

namespace docutrace {

Tokenizer::Tokenizer(bool lowercase, bool removePunctuation, size_t minTokenLength)
    : lowercase_(lowercase), removePunctuation_(removePunctuation), minTokenLength_(minTokenLength) {}

bool Tokenizer::isWhitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '\v' || c == '\f';
}

bool Tokenizer::isPunctuation(char c) {
    // Standard ASCII punctuation check
    return (c >= 33 && c <= 47) || (c >= 58 && c <= 64) || 
           (c >= 91 && c <= 96) || (c >= 123 && c <= 126);
}

std::string Tokenizer::toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

std::vector<Token> Tokenizer::tokenize(const std::string& text) const {
    std::vector<Token> tokens;
    const size_t len = text.size();
    size_t i = 0;

    size_t currentSentence = 0;
    size_t currentParagraph = 0;

    while (i < len) {
        // Track paragraphs (consecutive newlines)
        if (text[i] == '\n') {
            if (i + 1 < len && text[i + 1] == '\n') {
                currentParagraph++;
                i += 2;
                continue;
            }
        }

        // Track sentences
        if (text[i] == '.' || text[i] == '?' || text[i] == '!') {
            currentSentence++;
        }

        // Skip whitespace
        if (isWhitespace(text[i])) {
            i++;
            continue;
        }

        // Skip or capture punctuation
        if (isPunctuation(text[i])) {
            if (removePunctuation_) {
                i++;
                continue;
            }
        }

        // We have started a token
        size_t startChar = i;
        while (i < len && !isWhitespace(text[i]) && (!removePunctuation_ || !isPunctuation(text[i]))) {
            i++;
        }
        size_t endChar = i;

        if (endChar > startChar && (endChar - startChar) >= minTokenLength_) {
            std::string original = text.substr(startChar, endChar - startChar);
            std::string norm = lowercase_ ? toLower(original) : original;

            Token token;
            token.text = norm;
            token.originalText = original;
            token.startChar = startChar;
            token.endChar = endChar;
            token.sentenceIndex = currentSentence;
            token.paragraphIndex = currentParagraph;

            tokens.push_back(token);
        }
    }

    return tokens;
}

} // namespace docutrace
