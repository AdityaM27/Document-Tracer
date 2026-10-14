#include "../include/AnalysisService.h"
#include "../include/ExactMatchStrategy.h"
#include "../include/NGramSimilarity.h"
#include "../include/JaccardSimilarity.h"
#include "../include/SemanticSimilarity.h"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <iostream>
#include <filesystem>
#include <thread>

namespace docutrace {

AnalysisService::AnalysisService(
    std::shared_ptr<ReferenceIndex> refIndex,
    size_t threadCount,
    const std::string& outputDir)
    : textProcessor_(3),
      referenceIndex_(refIndex ? refIndex : std::make_shared<ReferenceIndex>("data/references")),
      academicProvider_(std::make_shared<AcademicSourceProvider>("data/cache/academic_cache.json")),
      webProvider_(std::make_shared<WebSourceProvider>("", "", "data/cache/web_cache.json")),
      aiClient_("127.0.0.1", 8000, 60000),
      reportGenerator_(outputDir),
      threadPool_(std::make_unique<ThreadPool>(threadCount)) {

    sourceDiscovery_.addProvider(academicProvider_);
    sourceDiscovery_.addProvider(webProvider_);

    initializeStrategies();
}

void AnalysisService::initializeStrategies() {
    strategies_.clear();
    strategies_.push_back(std::make_unique<ExactMatchStrategy>(6));
    strategies_.push_back(std::make_unique<NGramSimilarity>(3));
    strategies_.push_back(std::make_unique<JaccardSimilarity>());
    strategies_.push_back(std::make_unique<SemanticSimilarity>());
}

void AnalysisService::setThreadPoolSize(size_t threads) {
    threadPool_ = std::make_unique<ThreadPool>(threads);
}

void AnalysisService::createTask(const std::string& submissionId, const std::string& filename) {
    if (submissionId.empty()) return;
    std::lock_guard<std::mutex> lock(progressMutex_);
    auto& p = progressMap_[submissionId];
    p.submissionId = submissionId;
    p.currentStage = "INITIALIZING";
    p.progressPercent = 5;
    p.statusMessage = filename.empty()
        ? "Document registered. Ready for analysis."
        : "Document '" + filename + "' registered. Ready for analysis.";
    p.isCompleted = false;
    p.hasError = false;
    p.errorMessage.clear();
}

bool AnalysisService::hasTask(const std::string& submissionId) const {
    if (submissionId.empty()) return false;
    std::lock_guard<std::mutex> lock(progressMutex_);
    return progressMap_.find(submissionId) != progressMap_.end();
}

void AnalysisService::updateProgress(
    const std::string& submissionId,
    const std::string& stage,
    int percent,
    const std::string& msg) {

    std::lock_guard<std::mutex> lock(progressMutex_);
    AnalysisProgress& p = progressMap_[submissionId];
    p.submissionId = submissionId;
    p.currentStage = stage;
    p.progressPercent = percent;
    p.statusMessage = msg;
    if (percent >= 100) {
        p.isCompleted = true;
    }
}

AnalysisProgress AnalysisService::getProgress(const std::string& submissionId) const {
    // 1. Check in-memory progress
    {
        std::lock_guard<std::mutex> lock(progressMutex_);
        auto it = progressMap_.find(submissionId);
        if (it != progressMap_.end()) {
            return it->second;
        }
    }

    // 2. Check in-memory completed results
    {
        std::lock_guard<std::mutex> lock(resultsMutex_);
        auto it = completedResults_.find(submissionId);
        if (it != completedResults_.end()) {
            AnalysisProgress p;
            p.submissionId = submissionId;
            p.currentStage = "COMPLETED";
            p.progressPercent = 100;
            p.statusMessage = "Analysis completed successfully.";
            p.isCompleted = true;
            p.hasError = false;
            return p;
        }
    }

    // 3. Check persisted JSON report on disk
    std::string reportJson = "output/report_" + submissionId + ".json";
    if (std::filesystem::exists(reportJson)) {
        AnalysisProgress p;
        p.submissionId = submissionId;
        p.currentStage = "COMPLETED";
        p.progressPercent = 100;
        p.statusMessage = "Analysis completed successfully (cached on disk).";
        p.isCompleted = true;
        p.hasError = false;
        return p;
    }

    // 4. Truly not found - return structured error state
    AnalysisProgress notFound;
    notFound.submissionId = submissionId;
    notFound.currentStage = "ERROR";
    notFound.progressPercent = 0;
    notFound.statusMessage = "No analysis task found for ID: " + submissionId;
    notFound.isCompleted = false;
    notFound.hasError = true;
    notFound.errorMessage = "Analysis task ID not found: " + submissionId;
    return notFound;
}

bool AnalysisService::getResult(const std::string& submissionId, AnalysisSummary& outSummary) const {
    std::lock_guard<std::mutex> lock(resultsMutex_);
    auto it = completedResults_.find(submissionId);
    if (it != completedResults_.end()) {
        outSummary = it->second;
        return true;
    }
    return false;
}

std::string AnalysisService::startAsyncAnalysis(
    const std::string& filepath,
    const AnalysisOptions& options,
    const std::string& customSubmissionId) {

    std::string subId = customSubmissionId;
    if (subId.empty()) {
        std::filesystem::path p(filepath);
        std::string stem = p.stem().string();
        if (stem.rfind("doc_", 0) == 0) {
            subId = stem;
        } else {
            subId = DocumentLoader::generateUniqueId(p.filename().string());
        }
    }

    createTask(subId, std::filesystem::path(filepath).filename().string());

    std::thread worker([this, filepath, options, subId]() {
        try {
            analyzeFile(filepath, options, subId);
        } catch (const DocuTraceException& e) {
            std::lock_guard<std::mutex> lock(progressMutex_);
            auto& p = progressMap_[subId];
            p.submissionId = subId;
            p.currentStage = "ERROR";
            p.hasError = true;
            p.errorMessage = e.what();
            p.statusMessage = "Analysis failed: " + std::string(e.what());
        } catch (const std::exception& e) {
            std::lock_guard<std::mutex> lock(progressMutex_);
            auto& p = progressMap_[subId];
            p.submissionId = subId;
            p.currentStage = "ERROR";
            p.hasError = true;
            p.errorMessage = e.what();
            p.statusMessage = "Analysis failed: " + std::string(e.what());
        } catch (...) {
            std::lock_guard<std::mutex> lock(progressMutex_);
            auto& p = progressMap_[subId];
            p.submissionId = subId;
            p.currentStage = "ERROR";
            p.hasError = true;
            p.errorMessage = "An unexpected error occurred during background analysis.";
            p.statusMessage = "Analysis failed unexpectedly.";
        }
    });
    worker.detach();

    return subId;
}

AnalysisSummary AnalysisService::analyzeFile(
    const std::string& filepath,
    const AnalysisOptions& options,
    const std::string& customSubmissionId) {

    auto startTime = std::chrono::high_resolution_clock::now();

    std::string subId = customSubmissionId;
    if (subId.empty()) {
        std::filesystem::path p(filepath);
        std::string stem = p.stem().string();
        if (stem.rfind("doc_", 0) == 0) {
            subId = stem;
        }
    }

    Document queryDoc;
    try {
        queryDoc = documentLoader_.loadFromFile(filepath, subId);
        subId = queryDoc.getId();
    } catch (const DocuTraceException& e) {
        std::string errId = subId.empty() ? e.getContext() : subId;
        if (!errId.empty()) {
            std::lock_guard<std::mutex> lock(progressMutex_);
            AnalysisProgress& p = progressMap_[errId];
            p.submissionId = errId;
            p.currentStage = "ERROR";
            p.hasError = true;
            p.errorMessage = e.what();
            p.statusMessage = "Document loading failed: " + std::string(e.what());
        }
        throw;
    } catch (const std::exception& e) {
        if (!subId.empty()) {
            std::lock_guard<std::mutex> lock(progressMutex_);
            AnalysisProgress& p = progressMap_[subId];
            p.submissionId = subId;
            p.currentStage = "ERROR";
            p.hasError = true;
            p.errorMessage = e.what();
            p.statusMessage = "Document loading failed: " + std::string(e.what());
        }
        throw;
    }

    createTask(subId, queryDoc.getFilename());
    updateProgress(subId, "EXTRACTION", 10, "Document extracted successfully.");

    try {

    // Preprocessing
    updateProgress(subId, "PREPROCESSING", 25, "Normalizing text and building passage index...");
    textProcessor_.processDocument(queryDoc);

    // Prepare Summary Object
    AnalysisSummary summary;
    summary.submissionId = subId;
    summary.submissionFilename = queryDoc.getFilename();

    auto now = std::chrono::system_clock::now();
    auto in_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream timeSs;
    timeSs << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
    summary.timestamp = timeSs.str();

    // Local Matching via ThreadPool
    updateProgress(subId, "LOCAL_MATCHING", 40, "Comparing against local reference corpus...");
    auto candidateDocIds = referenceIndex_->findCandidateDocuments(queryDoc, 2);
    summary.referencesCheckedCount = candidateDocIds.size();

    std::mutex matchCollectionMutex;
    std::vector<MatchRecord> allMatches;

    std::vector<std::future<void>> futures;

    // Concurrent comparison per reference document
    for (const auto& targetId : candidateDocIds) {
        futures.push_back(threadPool_->enqueue([this, &queryDoc, targetId, &options, &matchCollectionMutex, &allMatches]() {
            try {
                Document targetDoc = referenceIndex_->getDocument(targetId);
                std::vector<MatchRecord> localMatches;

                if (options.enableExactMatch) {
                    ExactMatchStrategy exact(6);
                    auto m = exact.findPassageMatches(queryDoc, targetDoc, options.exactThreshold);
                    localMatches.insert(localMatches.end(), m.begin(), m.end());
                }
                if (options.enableNGram) {
                    NGramSimilarity ngram(options.ngramSize);
                    auto m = ngram.findPassageMatches(queryDoc, targetDoc, options.ngramThreshold);
                    localMatches.insert(localMatches.end(), m.begin(), m.end());
                }
                if (options.enableJaccard) {
                    JaccardSimilarity jaccard;
                    auto m = jaccard.findPassageMatches(queryDoc, targetDoc, options.jaccardThreshold);
                    localMatches.insert(localMatches.end(), m.begin(), m.end());
                }
                if (options.enableSemantic) {
                    SemanticSimilarity semantic;
                    auto m = semantic.findPassageMatches(queryDoc, targetDoc, options.semanticThreshold);
                    localMatches.insert(localMatches.end(), m.begin(), m.end());
                }

                if (!localMatches.empty()) {
                    std::lock_guard<std::mutex> lock(matchCollectionMutex);
                    allMatches.insert(allMatches.end(), localMatches.begin(), localMatches.end());
                }
            } catch (...) {}
        }));
    }

    for (auto& f : futures) {
        f.get();
    }

    summary.matches = allMatches;

    // Online Academic Literature Discovery
    if (options.enableAcademicSearch) {
        updateProgress(subId, "ACADEMIC_SEARCH", 60, "Querying scholarly databases (OpenAlex & Crossref)...");
        try {
            summary.discoveredSources = sourceDiscovery_.discoverSources(queryDoc, 3);
            auto academicMatches = sourceDiscovery_.compareAgainstSources(queryDoc, summary.discoveredSources, 0.45);
            summary.matches.insert(summary.matches.end(), academicMatches.begin(), academicMatches.end());
        } catch (const std::exception& e) {
            summary.errorsAndWarnings.push_back(std::string("Academic Search Warning: ") + e.what());
        }
    }

    // AI-Writing Classification
    if (options.enableAIClassifier) {
        updateProgress(subId, "AI_ANALYSIS", 75, "Running pretrained AI-writing classifier...");
        try {
            summary.aiResult = aiClient_.classifyDocument(queryDoc);
        } catch (const std::exception& e) {
            summary.errorsAndWarnings.push_back(std::string("AI Service Warning: ") + e.what());
            summary.aiResult.serviceAvailable = false;
            summary.aiResult.label = "Service Offline";
            summary.aiResult.reasoning = e.what();
        }
    } else {
        summary.aiResult.serviceAvailable = false;
        summary.aiResult.label = "Module Disabled";
        summary.aiResult.reasoning = "AI classification was disabled in analysis options.";
    }

    // Explainable Writing Pattern Statistics
    if (options.enableWritingPatterns) {
        updateProgress(subId, "WRITING_PATTERNS", 85, "Analyzing stylometrics and writing patterns...");
        summary.writingStats = writingAnalyzer_.analyze(queryDoc);
    }

    // Calculate aggregated non-overlapping similarity coverage
    const std::string& orig = queryDoc.getOriginalText();
    std::vector<bool> covered(orig.size(), false);
    std::vector<bool> exactCovered(orig.size(), false);
    std::vector<bool> ngramCovered(orig.size(), false);
    std::vector<bool> jaccardCovered(orig.size(), false);
    std::vector<bool> semanticCovered(orig.size(), false);

    for (const auto& m : summary.matches) {
        size_t s = m.queryPassage.startChar;
        size_t e = std::min(m.queryPassage.endChar, orig.size());
        for (size_t c = s; c < e; ++c) {
            covered[c] = true;
            if (m.matchType == MatchType::EXACT_MATCH) exactCovered[c] = true;
            else if (m.matchType == MatchType::NGRAM_MATCH) ngramCovered[c] = true;
            else if (m.matchType == MatchType::JACCARD_MATCH) jaccardCovered[c] = true;
            else if (m.matchType == MatchType::SEMANTIC_MATCH) semanticCovered[c] = true;
        }
    }

    size_t totalChars = orig.size();
    if (totalChars > 0) {
        auto countCovered = [](const std::vector<bool>& vec) {
            size_t cnt = 0;
            for (bool b : vec) if (b) cnt++;
            return cnt;
        };

        summary.overallSimilarityScore = static_cast<double>(countCovered(covered)) / static_cast<double>(totalChars);
        summary.exactSimilarityScore = static_cast<double>(countCovered(exactCovered)) / static_cast<double>(totalChars);
        summary.ngramSimilarityScore = static_cast<double>(countCovered(ngramCovered)) / static_cast<double>(totalChars);
        summary.jaccardSimilarityScore = static_cast<double>(countCovered(jaccardCovered)) / static_cast<double>(totalChars);
        summary.semanticSimilarityScore = static_cast<double>(countCovered(semanticCovered)) / static_cast<double>(totalChars);
    }

    summary.totalPassagesCount = queryDoc.getPassageWindows().size() + queryDoc.getSentences().size();
    summary.matchedPassagesCount = summary.matches.size();

    // Report Generation
    updateProgress(subId, "REPORT_GENERATION", 95, "Serializing HTML and JSON audit reports...");
    reportGenerator_.generateJsonReport(summary);
    reportGenerator_.generateHtmlReport(queryDoc, summary);

    auto endTime = std::chrono::high_resolution_clock::now();
    summary.processingTimeMs = std::chrono::duration<double, std::milli>(endTime - startTime).count();

    // Cache completed result
    {
        std::lock_guard<std::mutex> lock(resultsMutex_);
        completedResults_[subId] = summary;
    }

    updateProgress(subId, "COMPLETED", 100, "Analysis completed successfully.");

    return summary;
    } catch (const DocuTraceException& e) {
        std::lock_guard<std::mutex> lock(progressMutex_);
        AnalysisProgress& p = progressMap_[subId];
        p.submissionId = subId;
        p.currentStage = "ERROR";
        p.hasError = true;
        p.errorMessage = e.what();
        p.statusMessage = "Analysis failed: " + std::string(e.what());
        throw;
    } catch (const std::exception& e) {
        std::lock_guard<std::mutex> lock(progressMutex_);
        AnalysisProgress& p = progressMap_[subId];
        p.submissionId = subId;
        p.currentStage = "ERROR";
        p.hasError = true;
        p.errorMessage = e.what();
        p.statusMessage = "Analysis failed: " + std::string(e.what());
        throw;
    } catch (...) {
        std::lock_guard<std::mutex> lock(progressMutex_);
        AnalysisProgress& p = progressMap_[subId];
        p.submissionId = subId;
        p.currentStage = "ERROR";
        p.hasError = true;
        p.errorMessage = "An unexpected error occurred during document analysis.";
        p.statusMessage = "Analysis failed unexpectedly.";
        throw;
    }
}

AnalysisSummary AnalysisService::analyzeText(
    const std::string& text,
    const std::string& filename,
    const AnalysisOptions& options,
    const std::string& customSubmissionId) {

    std::string subId = customSubmissionId;
    if (subId.empty()) {
        subId = DocumentLoader::generateUniqueId(filename);
    }

    createTask(subId, filename);

    std::filesystem::create_directories("data/input");
    std::string tempFile = "data/input/" + subId + ".txt";
    
    std::ofstream out(tempFile);
    out << text;
    out.close();

    return analyzeFile(tempFile, options, subId);
}

} // namespace docutrace
