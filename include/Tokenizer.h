#ifndef DOCUTRACE_TOKENIZER_H
#define DOCUTRACE_TOKENIZER_H

#include <string>
#include <vector>
#include <unordered_set>
#include "Models.h"

namespace docutrace {

/**
 * @brief Tokenizes text while preserving exact character offsets from the original document.
 * Demonstrates Unit I: C++ Strings and Vectors, STL algorithms.
 */
class Tokenizer {
private:
    bool lowercase_{true};
    bool removePunctuation_{true};
    size_t minTokenLength_{1};

public:
    Tokenizer(bool lowercase = true, bool removePunctuation = true, size_t minTokenLength = 1);
    ~Tokenizer() = default;

    /**
     * @brief Tokenizes input text into a sequence of Tokens with precise start/end offsets.
     */
    std::vector<Token> tokenize(const std::string& text) const;

    /**
     * @brief Converts a token to lowercase ASCII/UTF-8 compatible.
     */
    static std::string toLower(const std::string& str);

    /**
     * @brief Determines whether a character is a punctuation mark.
     */
    static bool isPunctuation(char c);

    /**
     * @brief Determines whether a character is whitespace.
     */
    static bool isWhitespace(char c);
};

} // namespace docutrace

#endif // DOCUTRACE_TOKENIZER_H
