#include "../include/DocumentLoader.h"
#include "../include/Tokenizer.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <random>
#include <chrono>
#include <regex>
#include <cstdlib>

namespace docutrace {

DocumentLoader::DocumentLoader(size_t maxFileSizeBytes)
    : maxFileSizeBytes_(maxFileSizeBytes) {}

std::string DocumentLoader::detectExtension(const std::string& filepath) {
    std::filesystem::path p(filepath);
    std::string ext = p.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext;
}

std::string DocumentLoader::generateUniqueId(const std::string& filename) {
    (void)filename;
    auto now = std::chrono::system_clock::now().time_since_epoch().count();
    static std::mt19937_64 rng(1337);
    uint64_t randVal = rng();
    std::stringstream ss;
    ss << "doc_" << (now & 0xFFFFFF) << "_" << std::hex << (randVal & 0xFFFF);
    return ss.str();
}

std::string DocumentLoader::extractFromTxt(const std::string& filepath) const {
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw DocumentExtractionException("Failed to open text file for reading", filepath);
    }

    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());

    // Strip UTF-8 BOM if present (EF BB BF)
    if (content.size() >= 3 &&
        static_cast<unsigned char>(content[0]) == 0xEF &&
        static_cast<unsigned char>(content[1]) == 0xBB &&
        static_cast<unsigned char>(content[2]) == 0xBF) {
        content = content.substr(3);
    }

    return content;
}

static std::string getPythonExecutable() {
    if (std::filesystem::exists(".venv/Scripts/python.exe")) {
        return ".\\.venv\\Scripts\\python.exe";
    }
    if (std::filesystem::exists(".venv/bin/python")) {
        return ".venv/bin/python";
    }
    return "python";
}

std::string DocumentLoader::extractFromDocx(const std::string& filepath) const {
    if (!std::filesystem::exists(filepath)) {
        throw DocumentExtractionException("File does not exist: " + filepath, filepath);
    }
    if (std::filesystem::file_size(filepath) == 0) {
        throw EmptyDocumentException(filepath);
    }

    std::string pythonExe = getPythonExecutable();
    std::string scriptPath = "scripts/extract_text.py";
    std::string tempOut = filepath + ".tmp.txt";

    std::string cmd = pythonExe + " " + scriptPath + " \"" + filepath + "\" \"" + tempOut + "\"";
    int ret = std::system(cmd.c_str());

    if (std::filesystem::exists(tempOut)) {
        std::ifstream f(tempOut, std::ios::binary);
        std::string res((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();
        std::filesystem::remove(tempOut);

        if (ret == 0 && !res.empty()) {
            bool hasText = false;
            for (char c : res) {
                if (!Tokenizer::isWhitespace(c)) {
                    hasText = true;
                    break;
                }
            }
            if (hasText) {
                return res;
            }
        }

        const std::string errPrefix = "DOCUTRACE_EXTRACTION_ERROR:";
        if (res.rfind(errPrefix, 0) == 0) {
            std::string errBody = res.substr(errPrefix.size());
            auto colonPos = errBody.find(':');
            std::string code = (colonPos != std::string::npos) ? errBody.substr(0, colonPos) : "ERROR";
            std::string msg = (colonPos != std::string::npos) ? errBody.substr(colonPos + 1) : errBody;

            if (code == "EMPTY_FILE" || code == "EMPTY_DOCX") {
                throw EmptyDocumentException(filepath);
            }
            throw DocumentExtractionException(msg, filepath);
        }
    }

    // Direct fallback: Scan binary file for XML text elements
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw DocumentExtractionException("Cannot open DOCX file for reading", filepath);
    }
    std::string raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    // Look for <w:t> tags in raw byte stream
    std::string extracted;
    std::regex wtRegex("<w:t[^>]*>([^<]+)</w:t>");
    auto words_begin = std::sregex_iterator(raw.begin(), raw.end(), wtRegex);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        extracted += match[1].str() + " ";
    }

    if (extracted.empty()) {
        throw DocumentExtractionException("Unable to extract text from DOCX archive (document may be corrupted or contain no text)", filepath);
    }
    return extracted;
}

std::string DocumentLoader::extractFromPdf(const std::string& filepath) const {
    if (!std::filesystem::exists(filepath)) {
        throw DocumentExtractionException("File does not exist: " + filepath, filepath);
    }
    if (std::filesystem::file_size(filepath) == 0) {
        throw EmptyDocumentException(filepath);
    }

    std::string pythonExe = getPythonExecutable();
    std::string scriptPath = "scripts/extract_text.py";
    std::string tempOut = filepath + ".tmp.txt";

    std::string cmd = pythonExe + " " + scriptPath + " \"" + filepath + "\" \"" + tempOut + "\"";
    int ret = std::system(cmd.c_str());

    if (std::filesystem::exists(tempOut)) {
        std::ifstream f(tempOut, std::ios::binary);
        std::string res((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
        f.close();
        std::filesystem::remove(tempOut);

        if (ret == 0 && !res.empty()) {
            bool hasText = false;
            for (char c : res) {
                if (!Tokenizer::isWhitespace(c)) {
                    hasText = true;
                    break;
                }
            }
            if (hasText) {
                return res;
            }
        }

        const std::string errPrefix = "DOCUTRACE_EXTRACTION_ERROR:";
        if (res.rfind(errPrefix, 0) == 0) {
            std::string errBody = res.substr(errPrefix.size());
            auto colonPos = errBody.find(':');
            std::string code = (colonPos != std::string::npos) ? errBody.substr(0, colonPos) : "ERROR";
            std::string msg = (colonPos != std::string::npos) ? errBody.substr(colonPos + 1) : errBody;

            if (code == "EMPTY_FILE" || code == "EMPTY_PDF") {
                throw EmptyDocumentException(filepath);
            }
            throw DocumentExtractionException(msg, filepath);
        }
    }

    // Direct fallback: check basic PDF validity
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        throw DocumentExtractionException("Cannot open PDF file for reading", filepath);
    }
    std::string raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();

    if (raw.size() < 4 || raw.rfind("%PDF-", 0) != 0) {
        throw DocumentExtractionException("File does not have a valid PDF header (%PDF-)", filepath);
    }

    // Match text literal syntax: (Text) Tj
    std::string extracted;
    std::regex tjRegex(R"(\(([^)]+)\)\s*Tj)");
    auto words_begin = std::sregex_iterator(raw.begin(), raw.end(), tjRegex);
    auto words_end = std::sregex_iterator();
    for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
        std::smatch match = *i;
        extracted += match[1].str() + " ";
    }

    if (extracted.empty()) {
        throw DocumentExtractionException("Unable to extract text from PDF (file may be scanned/image-only, encrypted, or corrupted)", filepath);
    }
    return extracted;
}

Document DocumentLoader::loadFromFile(const std::string& filepath, const std::string& customId) const {
    if (!std::filesystem::exists(filepath)) {
        throw DocumentExtractionException("File does not exist: " + filepath, filepath);
    }

    auto fileSize = std::filesystem::file_size(filepath);
    if (fileSize > maxFileSizeBytes_) {
        throw FileSizeLimitException(fileSize, maxFileSizeBytes_, filepath);
    }

    std::string ext = detectExtension(filepath);
    std::string filename = std::filesystem::path(filepath).filename().string();
    std::string id = customId.empty() ? generateUniqueId(filename) : customId;

    std::string extractedText;
    if (ext == ".txt" || ext == ".md" || ext == ".cpp" || ext == ".h" || ext == ".c" || ext == ".py" || ext == ".json" || ext == ".csv") {
        extractedText = extractFromTxt(filepath);
    } else if (ext == ".docx") {
        extractedText = extractFromDocx(filepath);
    } else if (ext == ".pdf") {
        extractedText = extractFromPdf(filepath);
    } else {
        throw InvalidFormatException(ext, filepath);
    }

    // Check if extracted content is empty
    bool hasNonWhitespace = false;
    for (char c : extractedText) {
        if (!Tokenizer::isWhitespace(c)) {
            hasNonWhitespace = true;
            break;
        }
    }

    if (!hasNonWhitespace) {
        throw EmptyDocumentException(filepath);
    }

    Document doc(id, filename, filepath);
    doc.setFileSize(fileSize);
    doc.setOriginalText(extractedText);

    return doc;
}

Document DocumentLoader::loadFromText(const std::string& text, const std::string& filename, const std::string& customId) const {
    bool hasNonWhitespace = false;
    for (char c : text) {
        if (!Tokenizer::isWhitespace(c)) {
            hasNonWhitespace = true;
            break;
        }
    }
    if (!hasNonWhitespace) {
        throw EmptyDocumentException(filename);
    }

    std::string id = customId.empty() ? generateUniqueId(filename) : customId;
    Document doc(id, filename, "");
    doc.setFileSize(text.size());
    doc.setOriginalText(text);

    return doc;
}

} // namespace docutrace
