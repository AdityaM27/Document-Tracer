#ifndef DOCUTRACE_ANALYSISSERVICE_H
#define DOCUTRACE_ANALYSISSERVICE_H

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <mutex>
#include "Models.h"
#include "Document.h"
#include "DocumentLoader.h"
#include "TextProcessor.h"
#include "SimilarityStrategy.h"
#include "ReferenceIndex.h"
#include "SourceDiscovery.h"
#include "AcademicSourceProvider.h"
#include "WebSourceProvider.h"
#include "WritingPatternAnalyzer.h"
#include "AIAnalysisClient.h"
#include "ThreadPool.h"
#include "ReportGenerator.h"

namespace docutrace {

struct AnalysisProgress {
    std::string submissionId;
    std::string currentStage;   // e.g. "EXTRACTION", "LOCAL_MATCHING"
    int progressPercent{0};     // 0 to 100
    std::string statusMessage;
    bool isCompleted{false};
    bool hasError{false};
    std::string errorMessage;
};

/**
 * @brief High-level Analysis Service orchestrating document processing,
 * concurrent comparison, AI writing analysis, and report generation.
 * Demonstrates Unit I - V integration.
 */
class AnalysisService {
private:
    DocumentLoader documentLoader_;
    TextProcessor textProcessor_;
    std::shared_ptr<ReferenceIndex> referenceIndex_;
    std::vector<std::unique_ptr<SimilarityStrategy>> strategies_;
    SourceDiscovery sourceDiscovery_;
    std::shared_ptr<AcademicSourceProvider> academicProvider_;
    std::shared_ptr<WebSourceProvider> webProvider_;
    WritingPatternAnalyzer writingAnalyzer_;
    AIAnalysisClient aiClient_;
    ReportGenerator reportGenerator_;
    std::unique_ptr<ThreadPool> threadPool_;

    mutable std::mutex progressMutex_;
    std::map<std::string, AnalysisProgress> progressMap_;

    mutable std::mutex resultsMutex_;
    std::map<std::string, AnalysisSummary> completedResults_;

    void updateProgress(const std::string& submissionId, const std::string& stage, int percent, const std::string& msg);
    void initializeStrategies();

public:
    explicit AnalysisService(
        std::shared_ptr<ReferenceIndex> refIndex = nullptr,
        size_t threadCount = 4,
        const std::string& outputDir = "output");
    ~AnalysisService() = default;

    /**
     * @brief Pre-registers an analysis task or updates its initial status.
     * Thread-safe.
     */
    void createTask(const std::string& submissionId, const std::string& filename = "");

    /**
     * @brief Checks if a task with the given ID exists in memory.
     */
    bool hasTask(const std::string& submissionId) const;

    /**
     * @brief Launches analysis on a file asynchronously in a background worker thread.
     * Returns the submission ID immediately.
     */
    std::string startAsyncAnalysis(
        const std::string& filepath,
        const AnalysisOptions& options = AnalysisOptions(),
        const std::string& customSubmissionId = "");

    /**
     * @brief Executes full analysis on a document file.
     */
    AnalysisSummary analyzeFile(
        const std::string& filepath,
        const AnalysisOptions& options = AnalysisOptions(),
        const std::string& customSubmissionId = "");

    /**
     * @brief Executes full analysis on raw text.
     */
    AnalysisSummary analyzeText(
        const std::string& text,
        const std::string& filename = "submission.txt",
        const AnalysisOptions& options = AnalysisOptions(),
        const std::string& customSubmissionId = "");

    /**
     * @brief Retrieves live progress of an ongoing or completed analysis.
     */
    AnalysisProgress getProgress(const std::string& submissionId) const;

    /**
     * @brief Retrieves cached results of a completed analysis.
     */
    bool getResult(const std::string& submissionId, AnalysisSummary& outSummary) const;

    // Component accessors
    ReferenceIndex& getReferenceIndex() { return *referenceIndex_; }
    const ReferenceIndex& getReferenceIndex() const { return *referenceIndex_; }
    AcademicSourceProvider& getAcademicProvider() { return *academicProvider_; }
    AIAnalysisClient& getAIClient() { return aiClient_; }
    ReportGenerator& getReportGenerator() { return reportGenerator_; }

    void setThreadPoolSize(size_t threads);
};

} // namespace docutrace

#endif // DOCUTRACE_ANALYSISSERVICE_H
