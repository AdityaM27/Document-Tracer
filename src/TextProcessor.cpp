#include "../include/TextProcessor.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace docutrace {

TextProcessor::TextProcessor() : TextProcessor(3) {}

TextProcessor::TextProcessor(int ngramSize) : defaultNgramSize_(ngramSize) {
    initializeDefaultStopWords();
}

void TextProcessor::initializeDefaultStopWords() {
    // Standard academic English stop words
    const char* words[] = {
        "a", "about", "above", "after", "again", "against", "all", "am", "an", "and",
        "any", "are", "aren't", "as", "at", "be", "because", "been", "before", "being",
        "below", "between", "both", "but", "by", "can't", "cannot", "could", "couldn't",
        "did", "didn't", "do", "does", "doesn't", "doing", "don't", "down", "during",
        "each", "few", "for", "from", "further", "had", "hadn't", "has", "hasn't",
        "have", "haven't", "having", "he", "he'd", "he'll", "he's", "her", "here",
        "here's", "hers", "herself", "him", "himself", "his", "how", "how's", "i",
        "i'd", "i'll", "i'm", "i've", "if", "in", "into", "is", "isn't", "it",
        "it's", "its", "itself", "let's", "me", "more", "most", "mustn't", "my",
        "myself", "no", "nor", "not", "of", "off", "on", "once", "only", "or",
        "other", "ought", "our", "ours", "ourselves", "out", "over", "own", "same",
        "shan't", "she", "she'd", "she'll", "she's", "should", "shouldn't", "so",
        "some", "such", "than", "that", "that's", "the", "their", "theirs", "them",
        "themselves", "then", "there", "there's", "these", "they", "they'd", "they'll",
        "they're", "they've", "this", "those", "through", "to", "too", "under", "until",
        "up", "very", "was", "wasn't", "we", "we'd", "we'll", "we're", "we've",
        "were", "weren't", "what", "what's", "when", "when's", "where", "where's",
        "which", "while", "who", "who's", "whom", "why", "why's", "with", "won't",
        "would", "wouldn't", "you", "you'd", "you'll", "you're", "you've", "your",
        "yours", "yourself", "yourselves"
    };
    for (const char* w : words) {
        stopWords_.insert(w);
    }
}

std::string TextProcessor::normalizeWhitespace(const std::string& input) const {
    std::string result;
    result.reserve(input.size());

    bool inWhitespace = false;
    for (char c : input) {
        if (Tokenizer::isWhitespace(c)) {
            if (!inWhitespace) {
                result.push_back(' ');
                inWhitespace = true;
            }
        } else {
            result.push_back(c);
            inWhitespace = false;
        }
    }

    // Trim leading and trailing space
    while (!result.empty() && result.front() == ' ') result.erase(result.begin());
    while (!result.empty() && result.back() == ' ') result.pop_back();

    return result;
}

std::string TextProcessor::normalizeText(const std::string& input, bool toLower, bool removePunct) const {
    std::string res;
    res.reserve(input.size());

    for (char c : input) {
        if (removePunct && Tokenizer::isPunctuation(c)) {
            res.push_back(' ');
        } else if (toLower) {
            res.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(c))));
        } else {
            res.push_back(c);
        }
    }

    return normalizeWhitespace(res);
}

bool TextProcessor::isAbbreviation(const std::string& word) {
    static const std::unordered_set<std::string> abbrs = {
        "dr.", "mr.", "mrs.", "ms.", "prof.", "sr.", "jr.", "vs.", "etc.",
        "e.g.", "i.e.", "al.", "inc.", "ltd.", "co.", "univ.", "u.s.", "u.k."
    };
    std::string lower = Tokenizer::toLower(word);
    return abbrs.find(lower) != abbrs.end();
}

std::vector<Passage> TextProcessor::segmentSentences(const std::string& text, const std::vector<Token>& tokens) const {
    std::vector<Passage> sentences;
    if (text.empty() || tokens.empty()) return sentences;

    size_t passageIdx = 0;
    size_t startTokenIdx = 0;

    for (size_t i = 0; i < tokens.size(); ++i) {
        bool isLastToken = (i + 1 == tokens.size());
        bool sentenceEnd = false;

        // Check if token in original text ends with sentence terminators
        if (tokens[i].endChar < text.size()) {
            char nextChar = text[tokens[i].endChar];
            char tokenLastChar = tokens[i].originalText.empty() ? ' ' : tokens[i].originalText.back();

            if (tokenLastChar == '.' || tokenLastChar == '?' || tokenLastChar == '!' ||
                nextChar == '.' || nextChar == '?' || nextChar == '!') {
                
                // Ensure not abbreviation
                if (!isAbbreviation(tokens[i].originalText)) {
                    sentenceEnd = true;
                }
            }
        }

        if (sentenceEnd || isLastToken) {
            Passage p;
            p.passageIndex = passageIdx++;
            p.startChar = tokens[startTokenIdx].startChar;
            p.endChar = tokens[i].endChar;

            if (p.endChar > p.startChar && p.startChar < text.size()) {
                p.originalText = text.substr(p.startChar, p.endChar - p.startChar);
            }
            p.normalizedText = normalizeText(p.originalText);

            for (size_t k = startTokenIdx; k <= i; ++k) {
                p.tokens.push_back(tokens[k]);
            }
            p.ngrams = generateNGrams(p.tokens, defaultNgramSize_);

            sentences.push_back(p);
            startTokenIdx = i + 1;
        }
    }

    return sentences;
}

std::vector<Passage> TextProcessor::segmentParagraphs(const std::string& text, const std::vector<Token>& tokens) const {
    std::vector<Passage> paragraphs;
    if (text.empty() || tokens.empty()) return paragraphs;

    size_t passageIdx = 0;
    size_t startTokenIdx = 0;
    size_t currentPara = tokens[0].paragraphIndex;

    for (size_t i = 0; i < tokens.size(); ++i) {
        bool isLastToken = (i + 1 == tokens.size());
        bool paraChanged = (tokens[i].paragraphIndex != currentPara);

        if (paraChanged || isLastToken) {
            size_t endTokenIdx = paraChanged ? (i - 1) : i;

            if (endTokenIdx >= startTokenIdx) {
                Passage p;
                p.passageIndex = passageIdx++;
                p.startChar = tokens[startTokenIdx].startChar;
                p.endChar = tokens[endTokenIdx].endChar;

                if (p.endChar > p.startChar && p.startChar < text.size()) {
                    p.originalText = text.substr(p.startChar, p.endChar - p.startChar);
                }
                p.normalizedText = normalizeText(p.originalText);

                for (size_t k = startTokenIdx; k <= endTokenIdx; ++k) {
                    p.tokens.push_back(tokens[k]);
                }
                p.ngrams = generateNGrams(p.tokens, defaultNgramSize_);

                paragraphs.push_back(p);
            }

            startTokenIdx = i;
            currentPara = tokens[i].paragraphIndex;
        }
    }

    return paragraphs;
}

std::vector<Passage> TextProcessor::generateSlidingWindows(const std::vector<Token>& tokens, size_t windowSize, size_t stepSize) const {
    std::vector<Passage> windows;
    if (tokens.empty() || windowSize == 0 || stepSize == 0) return windows;

    size_t passageIdx = 0;
    for (size_t i = 0; i < tokens.size(); i += stepSize) {
        size_t endIdx = std::min(tokens.size(), i + windowSize);

        Passage p;
        p.passageIndex = passageIdx++;
        p.startChar = tokens[i].startChar;
        p.endChar = tokens[endIdx - 1].endChar;

        std::string originalCombined;
        for (size_t k = i; k < endIdx; ++k) {
            p.tokens.push_back(tokens[k]);
            originalCombined += tokens[k].originalText + (k + 1 < endIdx ? " " : "");
        }
        p.originalText = originalCombined;
        p.normalizedText = normalizeText(originalCombined);
        p.ngrams = generateNGrams(p.tokens, defaultNgramSize_);

        windows.push_back(p);

        if (endIdx == tokens.size()) break;
    }

    return windows;
}

std::vector<std::string> TextProcessor::generateNGrams(const std::vector<Token>& tokens, int n) const {
    std::vector<std::string> ngrams;
    if (n <= 0 || tokens.size() < static_cast<size_t>(n)) return ngrams;

    for (size_t i = 0; i <= tokens.size() - n; ++i) {
        std::string gram;
        for (int k = 0; k < n; ++k) {
            gram += tokens[i + k].text;
            if (k + 1 < n) gram += " ";
        }
        ngrams.push_back(gram);
    }

    return ngrams;
}

std::vector<std::string> TextProcessor::generateNGramsFromStrings(const std::vector<std::string>& words, int n) const {
    std::vector<std::string> ngrams;
    if (n <= 0 || words.size() < static_cast<size_t>(n)) return ngrams;

    for (size_t i = 0; i <= words.size() - n; ++i) {
        std::string gram;
        for (int k = 0; k < n; ++k) {
            gram += words[i + k];
            if (k + 1 < n) gram += " ";
        }
        ngrams.push_back(gram);
    }

    return ngrams;
}

bool TextProcessor::isStopWord(const std::string& word) const {
    return stopWords_.find(Tokenizer::toLower(word)) != stopWords_.end();
}

void TextProcessor::addStopWord(const std::string& word) {
    stopWords_.insert(Tokenizer::toLower(word));
}

void TextProcessor::removeStopWord(const std::string& word) {
    stopWords_.erase(Tokenizer::toLower(word));
}

std::vector<Token> TextProcessor::filterStopWords(const std::vector<Token>& tokens) const {
    std::vector<Token> filtered;
    for (const auto& t : tokens) {
        if (!isStopWord(t.text)) {
            filtered.push_back(t);
        }
    }
    return filtered;
}

void TextProcessor::processDocument(Document& doc, size_t windowSize, size_t stepSize) const {
    const std::string& text = doc.getOriginalText();
    if (text.empty()) {
        doc.setStatus(ProcessingStatus::FAILED);
        return;
    }

    // Normalize
    std::string norm = normalizeText(text);
    doc.setNormalizedText(norm);

    // Tokenize
    std::vector<Token> tokens = tokenizer_.tokenize(text);
    doc.setTokens(tokens);

    // Segment
    std::vector<Passage> sentences = segmentSentences(text, tokens);
    doc.setSentences(sentences);

    std::vector<Passage> paragraphs = segmentParagraphs(text, tokens);
    doc.setParagraphs(paragraphs);

    std::vector<Passage> windows = generateSlidingWindows(tokens, windowSize, stepSize);
    doc.setPassageWindows(windows);

    doc.setStatus(ProcessingStatus::PREPROCESSED);
}

} // namespace docutrace
