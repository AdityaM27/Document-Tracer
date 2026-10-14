#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <numeric>
#include "../include/AnalysisService.h"

using namespace docutrace;

struct BenchmarkResult {
    size_t threadCount;
    double elapsedMs;
    double throughputDocsPerSec;
};

void runConcurrencyBenchmark() {
    std::cout << "\n--- MULTITHREADING CONCURRENCY BENCHMARK ---\n";
    std::vector<size_t> threadConfigs = {1, 2, 4, 8};
    std::string testFile = "data/test_corpus/query_identical.txt";

    AnalysisOptions opt;
    opt.enableAIClassifier = false;
    opt.enableAcademicSearch = false;

    constexpr int ITERATIONS = 10;

    std::vector<BenchmarkResult> results;

    for (size_t threads : threadConfigs) {
        AnalysisService service(nullptr, threads, "output");

        // Warmup
        service.analyzeFile(testFile, opt);

        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < ITERATIONS; ++i) {
            service.analyzeFile(testFile, opt);
        }
        auto end = std::chrono::high_resolution_clock::now();

        double totalMs = std::chrono::duration<double, std::milli>(end - start).count();
        double avgMs = totalMs / ITERATIONS;
        double throughput = (ITERATIONS * 1000.0) / totalMs;

        results.push_back({threads, avgMs, throughput});
    }

    std::cout << std::setw(10) << "Threads"
              << std::setw(18) << "Avg Latency (ms)"
              << std::setw(22) << "Throughput (ops/sec)"
              << std::setw(15) << "Speedup" << "\n";
    std::cout << std::string(65, '-') << "\n";

    double baseLatency = results[0].elapsedMs;
    for (const auto& r : results) {
        double speedup = baseLatency / r.elapsedMs;
        std::cout << std::setw(10) << r.threadCount
                  << std::setw(18) << std::fixed << std::setprecision(2) << r.elapsedMs
                  << std::setw(22) << std::fixed << std::setprecision(2) << r.throughputDocsPerSec
                  << std::setw(15) << std::fixed << std::setprecision(2) << speedup << "x\n";
    }
}

void runAccuracyEvaluation() {
    std::cout << "\n--- SIMILARITY DETECTION ACCURACY EVALUATION (LABELED CORPUS) ---\n";

    AnalysisService service(nullptr, 4, "output");
    AnalysisOptions opt;
    opt.enableAIClassifier = false;
    opt.enableAcademicSearch = false;

    struct LabeledCase {
        std::string filepath;
        bool groundTruthIsMatch; // True if should detect similarity >= 0.20
        std::string description;
    };

    std::vector<LabeledCase> cases = {
        {"data/test_corpus/query_identical.txt", true, "Identical Document"},
        {"data/test_corpus/query_punctuation_case.txt", true, "Case & Punctuation Altered"},
        {"data/test_corpus/query_paraphrased.txt", true, "Paraphrased Operating Systems"},
        {"data/test_corpus/query_partial_overlap.txt", true, "Partial Passage Overlap"},
        {"data/test_corpus/query_unrelated.txt", false, "Unrelated Marine Biology"}
    };

    int TP = 0, FP = 0, TN = 0, FN = 0;
    double threshold = 0.15;

    std::cout << std::left << std::setw(32) << "Test Case"
              << std::setw(15) << "Ground Truth"
              << std::setw(18) << "Detected Score"
              << std::setw(12) << "Outcome" << "\n";
    std::cout << std::string(77, '-') << "\n";

    for (const auto& c : cases) {
        auto summary = service.analyzeFile(c.filepath, opt);
        bool predictedPositive = (summary.overallSimilarityScore >= threshold);

        std::string outcome;
        if (c.groundTruthIsMatch && predictedPositive) {
            TP++; outcome = "TP (Hit)";
        } else if (!c.groundTruthIsMatch && !predictedPositive) {
            TN++; outcome = "TN (Correct Rejection)";
        } else if (!c.groundTruthIsMatch && predictedPositive) {
            FP++; outcome = "FP (False Alarm)";
        } else {
            FN++; outcome = "FN (Miss)";
        }

        std::cout << std::left << std::setw(32) << c.description
                  << std::setw(15) << (c.groundTruthIsMatch ? "POSITIVE" : "NEGATIVE")
                  << std::setw(18) << (std::to_string(static_cast<int>(summary.overallSimilarityScore * 100)) + "%")
                  << std::setw(12) << outcome << "\n";
    }

    double precision = (TP + FP > 0) ? static_cast<double>(TP) / (TP + FP) : 0.0;
    double recall = (TP + FN > 0) ? static_cast<double>(TP) / (TP + FN) : 0.0;
    double f1 = (precision + recall > 0) ? 2.0 * (precision * recall) / (precision + recall) : 0.0;
    double fpr = (FP + TN > 0) ? static_cast<double>(FP) / (FP + TN) : 0.0;

    std::cout << "\nEvaluation Metrics Summary:\n";
    std::cout << "  Precision:            " << std::fixed << std::setprecision(3) << precision << " (" << (precision * 100.0) << "%)\n";
    std::cout << "  Recall:               " << std::fixed << std::setprecision(3) << recall << " (" << (recall * 100.0) << "%)\n";
    std::cout << "  F1 Score:             " << std::fixed << std::setprecision(3) << f1 << "\n";
    std::cout << "  False Positive Rate:  " << std::fixed << std::setprecision(3) << fpr << "\n";
}

int main() {
    std::cout << "===============================================================\n"
              << "    DOCUMENT TRACER PERFORMANCE & ACCURACY BENCHMARK SUITE     \n"
              << "===============================================================\n";

    runConcurrencyBenchmark();
    runAccuracyEvaluation();

    std::cout << "\n===============================================================\n";
    return 0;
}
