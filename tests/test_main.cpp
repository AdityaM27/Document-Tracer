#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <cmath>
#include <thread>
#include <chrono>

#include "../include/Models.h"
#include "../include/Document.h"
#include "../include/Tokenizer.h"
#include "../include/TextProcessor.h"
#include "../include/ExactMatchStrategy.h"
#include "../include/NGramSimilarity.h"
#include "../include/JaccardSimilarity.h"
#include "../include/SemanticSimilarity.h"
#include "../include/DocumentLoader.h"
#include "../include/ReferenceIndex.h"
#include "../include/WritingPatternAnalyzer.h"
#include "../include/ThreadPool.h"
#include "../include/AcademicSourceProvider.h"
#include "../include/ReportGenerator.h"
#include "../include/AnalysisService.h"
#include "../include/Exceptions.h"

using namespace docutrace;

int g_passed = 0;
int g_failed = 0;

#define TEST_ASSERT(cond, msg) \
    do { \
        if (cond) { \
            g_passed++; \
        } else { \
            g_failed++; \
            std::cerr << "  [FAIL] " << msg << " (line " << __LINE__ << ")\n"; \
        } \
    } while (0)

void testTokenizerAndNormalization() {
    std::cout << "[Test Suite 1: Tokenizer & Text Preprocessing]\n";

    Tokenizer tok(true, true, 1);
    std::string sample = "Hello, World! Testing DocuTrace (2026).";
    auto tokens = tok.tokenize(sample);

    TEST_ASSERT(tokens.size() == 5, "Token count matches expected words");
    TEST_ASSERT(tokens[0].text == "hello", "Token 0 normalized to lowercase");
    TEST_ASSERT(tokens[0].originalText == "Hello", "Token 0 preserved original case");
    TEST_ASSERT(tokens[0].startChar == 0 && tokens[0].endChar == 5, "Token 0 offset matches [0, 5]");
    TEST_ASSERT(tokens[1].text == "world", "Token 1 normalized to 'world'");
    TEST_ASSERT(tokens[1].startChar == 7 && tokens[1].endChar == 12, "Token 1 offset matches [7, 12]");

    TextProcessor proc(3);
    std::string rawWithSpaces = "  Multiple    spaces \t and \n\n newlines  ";
    std::string clean = proc.normalizeWhitespace(rawWithSpaces);
    TEST_ASSERT(clean == "Multiple spaces and newlines", "Whitespace normalization collapsed spaces correctly");

    std::vector<std::string> words = {"alpha", "beta", "gamma", "delta"};
    auto ngrams = proc.generateNGramsFromStrings(words, 3);
    TEST_ASSERT(ngrams.size() == 2, "4 words yield 2 trigrams");
    TEST_ASSERT(ngrams[0] == "alpha beta gamma", "Trigram 0 matches expected");
    TEST_ASSERT(ngrams[1] == "beta gamma delta", "Trigram 1 matches expected");
}

void testSimilarityStrategies() {
    std::cout << "[Test Suite 2: Modular Similarity Strategies (Polymorphism)]\n";

    DocumentLoader loader;
    TextProcessor proc(3);

    Document docIdentical1 = loader.loadFromFile("data/references/oop_cpp.txt");
    Document docIdentical2 = loader.loadFromFile("data/test_corpus/query_identical.txt");
    Document docUnrelated = loader.loadFromFile("data/test_corpus/query_unrelated.txt");
    Document docPunct = loader.loadFromFile("data/test_corpus/query_punctuation_case.txt");
    Document docDS = loader.loadFromFile("data/references/data_structures.txt");

    proc.processDocument(docIdentical1);
    proc.processDocument(docIdentical2);
    proc.processDocument(docUnrelated);
    proc.processDocument(docPunct);
    proc.processDocument(docDS);

    // Exact Match Strategy
    ExactMatchStrategy exact(6);
    double exactIdentical = exact.calculateSimilarity(docIdentical1, docIdentical2);
    double exactUnrelated = exact.calculateSimilarity(docIdentical1, docUnrelated);
    TEST_ASSERT(exactIdentical >= 0.99, "ExactMatch on identical documents is 1.0 (got " + std::to_string(exactIdentical) + ")");
    TEST_ASSERT(exactUnrelated <= 0.05, "ExactMatch on unrelated documents is ~0.0 (got " + std::to_string(exactUnrelated) + ")");

    // N-Gram Similarity
    NGramSimilarity ngram(3);
    double ngramIdentical = ngram.calculateSimilarity(docIdentical1, docIdentical2);
    double ngramUnrelated = ngram.calculateSimilarity(docIdentical1, docUnrelated);
    TEST_ASSERT(ngramIdentical >= 0.99, "NGramSimilarity on identical documents is 1.0");
    TEST_ASSERT(ngramUnrelated <= 0.05, "NGramSimilarity on unrelated documents is ~0.0");

    // Jaccard Similarity
    JaccardSimilarity jaccard;
    double jaccardIdentical = jaccard.calculateSimilarity(docIdentical1, docIdentical2);
    double jaccardUnrelated = jaccard.calculateSimilarity(docIdentical1, docUnrelated);
    TEST_ASSERT(jaccardIdentical >= 0.99, "Jaccard on identical documents is 1.0");
    TEST_ASSERT(jaccardUnrelated <= 0.08, "Jaccard on unrelated documents is <= 0.08 (got " + std::to_string(jaccardUnrelated) + ")");

    // Punctuation and case invariance
    double jaccardPunct = jaccard.calculateSimilarity(docDS, docPunct);
    TEST_ASSERT(jaccardPunct >= 0.70, "Jaccard handles punctuation and casing differences robustly (> 0.70)");

    // Jaccard Edge Cases: empty sets
    std::unordered_set<std::string> emptyA, emptyB;
    double jEmpty = JaccardSimilarity::computeJaccard(emptyA, emptyB);
    TEST_ASSERT(jEmpty == 0.0, "Jaccard with empty sets avoids division by zero and returns 0.0");

    // Semantic Cosine Similarity
    SemanticSimilarity semantic;
    std::vector<float> vA = {1.0f, 0.0f, 0.0f};
    std::vector<float> vB = {1.0f, 0.0f, 0.0f};
    std::vector<float> vC = {0.0f, 1.0f, 0.0f};
    std::vector<float> vZero = {0.0f, 0.0f, 0.0f};

    TEST_ASSERT(std::abs(SemanticSimilarity::cosineSimilarity(vA, vB) - 1.0) < 1e-4, "Cosine identical vectors is 1.0");
    TEST_ASSERT(std::abs(SemanticSimilarity::cosineSimilarity(vA, vC)) < 1e-4, "Cosine orthogonal vectors is 0.0");
    TEST_ASSERT(SemanticSimilarity::cosineSimilarity(vA, vZero) == 0.0, "Cosine with zero vector safely returns 0.0");
}

void testDocumentLoaderAndExceptions() {
    std::cout << "[Test Suite 3: DocumentLoader & Custom Exception Hierarchy]\n";

    DocumentLoader loader;

    // Test successful loading
    Document doc = loader.loadFromFile("data/references/oop_cpp.txt");
    TEST_ASSERT(!doc.getOriginalText().empty(), "Loaded document has text");
    TEST_ASSERT(doc.getFileExtension() == ".txt", "Extension correctly detected as .txt");

    // Test empty file exception
    bool caughtEmpty = false;
    try {
        loader.loadFromFile("data/test_corpus/query_empty.txt");
    } catch (const EmptyDocumentException& e) {
        caughtEmpty = true;
        TEST_ASSERT(e.getErrorCode() == 1003, "EmptyDocumentException has correct error code 1003");
    }
    TEST_ASSERT(caughtEmpty, "Empty file throws EmptyDocumentException");

    // Test non-existent file
    bool caughtNotFound = false;
    try {
        loader.loadFromFile("data/non_existent_document_12345.txt");
    } catch (const DocumentExtractionException& e) {
        caughtNotFound = true;
        TEST_ASSERT(e.getErrorCode() == 1001, "DocumentExtractionException has error code 1001");
    }
    TEST_ASSERT(caughtNotFound, "Non-existent file throws DocumentExtractionException");

    // Test unsupported format
    std::string fakeExe = "data/input/bad_file.xyz";
    std::ofstream out(fakeExe);
    out << "binary header data";
    out.close();

    bool caughtInvalid = false;
    try {
        loader.loadFromFile(fakeExe);
    } catch (const InvalidFormatException& e) {
        caughtInvalid = true;
        TEST_ASSERT(e.getErrorCode() == 1002, "InvalidFormatException has error code 1002");
    }
    std::filesystem::remove(fakeExe);
    TEST_ASSERT(caughtInvalid, "Unsupported extension throws InvalidFormatException");

    // Test PDF extraction - valid searchable PDF
    if (std::filesystem::exists("data/test_corpus/test_ref_normal.pdf")) {
        Document pdfDoc = loader.loadFromFile("data/test_corpus/test_ref_normal.pdf");
        TEST_ASSERT(!pdfDoc.getOriginalText().empty(), "Normal PDF has extracted text");
        TEST_ASSERT(pdfDoc.getFileExtension() == ".pdf", "Normal PDF extension is .pdf");
        TEST_ASSERT(pdfDoc.getOriginalText().find("Distributed") != std::string::npos, "PDF extracted content contains 'Distributed'");
    }

    // Test DOCX extraction - valid DOCX
    if (std::filesystem::exists("data/test_corpus/test_ref_normal.docx")) {
        Document docxDoc = loader.loadFromFile("data/test_corpus/test_ref_normal.docx");
        TEST_ASSERT(!docxDoc.getOriginalText().empty(), "Normal DOCX has extracted text");
        TEST_ASSERT(docxDoc.getFileExtension() == ".docx", "Normal DOCX extension is .docx");
        TEST_ASSERT(docxDoc.getOriginalText().find("modularity") != std::string::npos ||
                    docxDoc.getOriginalText().find("Software") != std::string::npos,
                    "DOCX extracted content contains 'modularity' or 'Software'");
    }

    // Test PDF extraction - corrupted PDF
    if (std::filesystem::exists("data/test_corpus/test_ref_corrupt.pdf")) {
        bool caughtCorrupt = false;
        try {
            loader.loadFromFile("data/test_corpus/test_ref_corrupt.pdf");
        } catch (const DocumentExtractionException& e) {
            caughtCorrupt = true;
            TEST_ASSERT(std::string(e.what()).find("corrupt") != std::string::npos ||
                        std::string(e.what()).find("header") != std::string::npos,
                        "Corrupted PDF reports corruption error");
        }
        TEST_ASSERT(caughtCorrupt, "Corrupted PDF throws DocumentExtractionException");
    }

    // Test PDF extraction - scanned/image-only PDF with no selectable text
    if (std::filesystem::exists("data/test_corpus/test_ref_scanned.pdf")) {
        bool caughtScanned = false;
        try {
            loader.loadFromFile("data/test_corpus/test_ref_scanned.pdf");
        } catch (const DocumentExtractionException& e) {
            caughtScanned = true;
            TEST_ASSERT(std::string(e.what()).find("scanned") != std::string::npos ||
                        std::string(e.what()).find("OCR") != std::string::npos,
                        "Scanned PDF reports scanned/OCR error");
        }
        TEST_ASSERT(caughtScanned, "Scanned PDF throws DocumentExtractionException");
    }
}

void testReferenceIndex() {
    std::cout << "[Test Suite 4: Local Reference Index & Inverted Indexing]\n";

    ReferenceIndex index("data/references");
    size_t count = index.rebuildIndex("data/references");
    TEST_ASSERT(count >= 3, "Indexed at least 3 reference files");
    TEST_ASSERT(index.size() >= 3, "Index size reflects indexed files");

    // Search query
    auto results = index.search("polymorphism encapsulation", 5);
    TEST_ASSERT(!results.empty(), "Inverted index returns matches for 'polymorphism'");

    // Verify disk persistence
    index.saveToDisk();
    ReferenceIndex indexReloaded("data/references");
    TEST_ASSERT(indexReloaded.size() == index.size(), "Reloaded index preserves document count from disk");
}

void testStylometricsAndWritingPatterns() {
    std::cout << "[Test Suite 5: Writing Pattern Analyzer (Stylometrics)]\n";

    DocumentLoader loader;
    TextProcessor proc;
    Document doc = loader.loadFromFile("data/references/oop_cpp.txt");
    proc.processDocument(doc);

    WritingPatternAnalyzer analyzer;
    auto stats = analyzer.analyze(doc);

    TEST_ASSERT(stats.totalWords > 0, "Word count is positive");
    TEST_ASSERT(stats.uniqueWords > 0 && stats.uniqueWords <= stats.totalWords, "Unique words within bounds");
    TEST_ASSERT(stats.typeTokenRatio > 0.0 && stats.typeTokenRatio <= 1.0, "TTR between 0.0 and 1.0");
    TEST_ASSERT(stats.characterEntropy > 0.0, "Shannon entropy is positive");
    TEST_ASSERT(!stats.interpretation.empty(), "Stylometric explanation generated");
}

void testThreadPoolConcurrency() {
    std::cout << "[Test Suite 6: Multithreading & Producer-Consumer Worker Pool]\n";

    constexpr size_t NUM_TASKS = 50;
    ThreadPool pool(4);
    std::atomic<int> counter{0};
    std::vector<std::future<int>> futures;

    for (size_t i = 0; i < NUM_TASKS; ++i) {
        futures.push_back(pool.enqueue([&counter, i]() {
            counter.fetch_add(1, std::memory_order_relaxed);
            return static_cast<int>(i * 2);
        }));
    }

    pool.waitAll();

    TEST_ASSERT(counter.load() == NUM_TASKS, "All 50 tasks executed concurrently");

    bool allFuturesCorrect = true;
    for (size_t i = 0; i < NUM_TASKS; ++i) {
        if (futures[i].get() != static_cast<int>(i * 2)) {
            allFuturesCorrect = false;
        }
    }
    TEST_ASSERT(allFuturesCorrect, "Futures returned expected values deterministically");
}

void testEndToEndAnalysisService() {
    std::cout << "[Test Suite 7: Full Analysis Pipeline Integration]\n";

    AnalysisService service(nullptr, 4, "output");

    AnalysisOptions opt;
    opt.enableAIClassifier = false;     // Offline unit test mode
    opt.enableAcademicSearch = false; // Offline unit test mode

    // 1. Identical document check
    auto summaryIdentical = service.analyzeFile("data/test_corpus/query_identical.txt", opt);
    TEST_ASSERT(summaryIdentical.overallSimilarityScore >= 0.85, 
        "Identical document detected with high similarity (>= 85%, got " + std::to_string(summaryIdentical.overallSimilarityScore * 100) + "%)");
    TEST_ASSERT(summaryIdentical.exactSimilarityScore >= 0.85, "Exact match score >= 85%");

    // 2. Unrelated document check
    auto summaryUnrelated = service.analyzeFile("data/test_corpus/query_unrelated.txt", opt);
    TEST_ASSERT(summaryUnrelated.overallSimilarityScore <= 0.15,
        "Unrelated document detected with low similarity (<= 15%, got " + std::to_string(summaryUnrelated.overallSimilarityScore * 100) + "%)");

    // 3. Partial overlap document check
    auto summaryPartial = service.analyzeFile("data/test_corpus/query_partial_overlap.txt", opt);
    TEST_ASSERT(summaryPartial.overallSimilarityScore > 0.15 && summaryPartial.overallSimilarityScore < 0.85,
        "Partial overlap document properly bounded between 15% and 85% (got " + std::to_string(summaryPartial.overallSimilarityScore * 100) + "%)");

    // 4. Verify generated report files
    std::string htmlReport = "output/report_" + summaryIdentical.submissionId + ".html";
    std::string jsonReport = "output/report_" + summaryIdentical.submissionId + ".json";
    TEST_ASSERT(std::filesystem::exists(htmlReport), "HTML report generated: " + htmlReport);
    TEST_ASSERT(std::filesystem::exists(jsonReport), "JSON report generated: " + jsonReport);
}

void testAnalysisTaskLifecycleAndRegression() {
    std::cout << "[Test Suite 8: Task Creation, Status Polling, Async Processing & Result Retrieval]\n";

    AnalysisService service(nullptr, 4, "output");

    AnalysisOptions opt;
    opt.enableAIClassifier = false;
    opt.enableAcademicSearch = false;

    // 1. Test Early Task Pre-registration
    std::string testTaskId = "task_regression_001";
    service.createTask(testTaskId, "query_identical.txt");
    TEST_ASSERT(service.hasTask(testTaskId), "Task pre-registration confirmed by hasTask()");

    auto progressInit = service.getProgress(testTaskId);
    TEST_ASSERT(progressInit.submissionId == testTaskId, "Initial progress submissionId matches exact registered ID");
    TEST_ASSERT(progressInit.currentStage == "INITIALIZING", "Initial progress stage is INITIALIZING");
    TEST_ASSERT(progressInit.progressPercent == 5, "Initial progress percent is 5%");
    TEST_ASSERT(!progressInit.hasError, "Initial progress hasError is false");
    TEST_ASSERT(!progressInit.isCompleted, "Initial progress isCompleted is false");

    // 2. Test Meaningful Error on Non-existent Task (never show NOT_FOUND as valid stage)
    std::string unknownId = "task_non_existent_999";
    auto progressUnknown = service.getProgress(unknownId);
    TEST_ASSERT(progressUnknown.hasError == true, "Non-existent task correctly flags hasError == true");
    TEST_ASSERT(progressUnknown.currentStage == "ERROR", "Non-existent task stage is ERROR (not NOT_FOUND)");
    TEST_ASSERT(!progressUnknown.errorMessage.empty(), "Meaningful error message returned for unknown task ID");

    // 3. Test Synchronous Analysis with Pre-registered ID Preservation
    auto summarySync = service.analyzeFile("data/test_corpus/query_identical.txt", opt, testTaskId);
    TEST_ASSERT(summarySync.submissionId == testTaskId, "Analysis preserves exact custom submission ID");

    auto progressDone = service.getProgress(testTaskId);
    TEST_ASSERT(progressDone.isCompleted == true, "Synchronous task marked as completed");
    TEST_ASSERT(progressDone.progressPercent == 100, "Progress reached 100%");
    TEST_ASSERT(progressDone.currentStage == "COMPLETED", "Final stage is COMPLETED");

    AnalysisSummary retrievedSync;
    bool gotSync = service.getResult(testTaskId, retrievedSync);
    TEST_ASSERT(gotSync, "Result successfully retrieved from memory cache via exact task ID");
    TEST_ASSERT(retrievedSync.overallSimilarityScore >= 0.85, "Retrieved similarity score matches expected");

    // 4. Test Asynchronous Task Execution and Polling Lifecycle
    std::string asyncTaskId = "task_regression_async_002";
    std::string spawnedId = service.startAsyncAnalysis("data/test_corpus/query_partial_overlap.txt", opt, asyncTaskId);
    TEST_ASSERT(spawnedId == asyncTaskId, "startAsyncAnalysis returned matching task ID");

    // Poll until completed or timeout (max 5 seconds)
    bool asyncCompleted = false;
    for (int retry = 0; retry < 50; ++retry) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        auto prog = service.getProgress(asyncTaskId);
        if (prog.isCompleted || prog.progressPercent >= 100) {
            asyncCompleted = true;
            break;
        }
    }
    TEST_ASSERT(asyncCompleted, "Asynchronous task completed within deadline");

    AnalysisSummary retrievedAsync;
    bool gotAsync = service.getResult(asyncTaskId, retrievedAsync);
    TEST_ASSERT(gotAsync, "Async result successfully retrieved via exact task ID");
    TEST_ASSERT(retrievedAsync.submissionId == asyncTaskId, "Async summary ID matches task ID");
    TEST_ASSERT(retrievedAsync.overallSimilarityScore > 0.15, "Async summary has valid computed similarity score");

    std::string jsonReport = "output/report_" + asyncTaskId + ".json";
    TEST_ASSERT(std::filesystem::exists(jsonReport), "Async task report serialized to disk: " + jsonReport);
}

int main() {
    std::cout << "===============================================================\n"
              << "       DOCUMENT TRACER AUTOMATED COMPREHENSIVE TEST SUITE      \n"
              << "===============================================================\n";

    testTokenizerAndNormalization();
    testSimilarityStrategies();
    testDocumentLoaderAndExceptions();
    testReferenceIndex();
    testStylometricsAndWritingPatterns();
    testThreadPoolConcurrency();
    testEndToEndAnalysisService();
    testAnalysisTaskLifecycleAndRegression();

    std::cout << "===============================================================\n"
              << " TEST RESULTS: " << g_passed << " PASSED, " << g_failed << " FAILED\n"
              << "===============================================================\n";

    return (g_failed == 0) ? 0 : 1;
}
