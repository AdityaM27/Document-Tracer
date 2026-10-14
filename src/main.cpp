#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <filesystem>
#include <iomanip>
#include "../include/AnalysisService.h"
#include "../include/DocumentLoader.h"
#include "../include/Exceptions.h"

using namespace docutrace;

void printUsage() {
    std::cout << "===============================================================\n"
              << " DOCUMENT TRACER: Intelligent Document Similarity & AI-Text Analysis\n"
              << " B.Tech III Semester OOP through C++ Laboratory Project\n"
              << "===============================================================\n"
              << "Usage: docutrace_cli [COMMAND] [OPTIONS]\n\n"
              << "Commands:\n"
              << "  analyze <file>          Analyze a document against reference index\n"
              << "  index-add <file>        Add a document to local reference corpus\n"
              << "  index-list              List all indexed reference documents\n"
              << "  index-rebuild <dir>     Rebuild reference index from folder\n"
              << "  health                  Check system components and service status\n"
              << "\nOptions:\n"
              << "  --no-ai                 Disable AI-writing classification\n"
              << "  --no-academic           Disable online academic literature search\n"
              << "  --threads <n>           Specify worker thread count (default: 4)\n"
              << "  --help                  Show this help message\n"
              << "===============================================================\n";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printUsage();
        return 0;
    }

    std::string command = argv[1];
    if (command == "--help" || command == "-h") {
        printUsage();
        return 0;
    }

    try {
        AnalysisOptions options;
        size_t threads = 4;

        for (int i = 2; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--no-ai") options.enableAIClassifier = false;
            else if (arg == "--no-academic") options.enableAcademicSearch = false;
            else if (arg == "--threads" && i + 1 < argc) {
                threads = std::stoul(argv[++i]);
            }
        }

        AnalysisService service(nullptr, threads);

        if (command == "health") {
            std::cout << "[Document Tracer System Status]\n";
            std::cout << "  C++ Core: Ready (C++17, STL, Multithreaded)\n";
            std::cout << "  Reference Docs Indexed: " << service.getReferenceIndex().size() << "\n";
            std::cout << "  Academic Provider: " << (service.getAcademicProvider().isOnline() ? "Online" : "Offline") << "\n";
            bool aiOk = service.getAIClient().checkHealth();
            std::cout << "  AI Classification Service: " << (aiOk ? "Online (localhost:8000)" : "Offline (ai_service/app.py not running)") << "\n";
            std::cout << "  Thread Pool Workers: " << threads << "\n";
            return 0;
        }

        if (command == "index-list") {
            auto docs = service.getReferenceIndex().getAllDocuments();
            std::cout << "Indexed Reference Documents (" << docs.size() << " total):\n";
            for (size_t i = 0; i < docs.size(); ++i) {
                std::cout << " [" << (i + 1) << "] " << docs[i].getId()
                          << " | " << docs[i].getFilename()
                          << " (" << docs[i].getWordCount() << " words)\n";
            }
            return 0;
        }

        if (command == "index-add") {
            if (argc < 3) {
                std::cerr << "Error: Filepath required for index-add\n";
                return 1;
            }
            std::string path = argv[2];
            DocumentLoader loader;
            Document doc = loader.loadFromFile(path);
            service.getReferenceIndex().addDocument(doc);
            std::cout << "Successfully indexed document: " << doc.getFilename()
                      << " (ID: " << doc.getId() << ")\n";
            return 0;
        }

        if (command == "index-rebuild") {
            std::string dir = (argc >= 3) ? argv[2] : "data/references";
            size_t count = service.getReferenceIndex().rebuildIndex(dir);
            std::cout << "Rebuilt reference index from '" << dir << "'. Indexed " << count << " documents.\n";
            return 0;
        }

        if (command == "analyze") {
            if (argc < 3) {
                std::cerr << "Error: Filepath required for analyze\n";
                return 1;
            }
            std::string filepath = argv[2];
            std::cout << "Starting Document Tracer analysis on: " << filepath << "...\n";

            AnalysisSummary summary = service.analyzeFile(filepath, options);

            std::cout << "\n===============================================================\n"
                      << "                    ANALYSIS FINDINGS SUMMARY                  \n"
                      << "===============================================================\n"
                      << " Document:           " << summary.submissionFilename << "\n"
                      << " Timestamp:          " << summary.timestamp << "\n"
                      << " Execution Time:     " << std::fixed << std::setprecision(1) << summary.processingTimeMs << " ms\n"
                      << " References Checked: " << summary.referencesCheckedCount << "\n"
                      << "---------------------------------------------------------------\n"
                      << " Overall Similarity: " << std::setprecision(1) << (summary.overallSimilarityScore * 100.0) << " %\n"
                      << "   - Exact Match:    " << (summary.exactSimilarityScore * 100.0) << " %\n"
                      << "   - 3-Gram Overlap: " << (summary.ngramSimilarityScore * 100.0) << " %\n"
                      << "   - Jaccard Metric: " << (summary.jaccardSimilarityScore * 100.0) << " %\n"
                      << "   - Semantic Match: " << (summary.semanticSimilarityScore * 100.0) << " %\n"
                      << "---------------------------------------------------------------\n"
                      << " AI Classification:  " << summary.aiResult.label << "\n"
                      << "   - AI Probability: " << (summary.aiResult.aiProbability * 100.0) << " %\n"
                      << "   - Reasoning:      " << summary.aiResult.reasoning << "\n"
                      << "---------------------------------------------------------------\n"
                      << " Stylometrics (Writing Patterns):\n"
                      << "   - Vocab Diversity (TTR): " << (summary.writingStats.typeTokenRatio * 100.0) << " %\n"
                      << "   - Burstiness:            " << summary.writingStats.burstiness << "\n"
                      << "   - Token Entropy:         " << summary.writingStats.characterEntropy << " bits\n"
                      << "---------------------------------------------------------------\n"
                      << " Matches Found:      " << summary.matches.size() << " passages\n"
                      << " Academic Sources:   " << summary.discoveredSources.size() << " sources\n"
                      << " HTML Report:        output/report_" << summary.submissionId << ".html\n"
                      << " JSON Report:        output/report_" << summary.submissionId << ".json\n"
                      << "===============================================================\n";
            return 0;
        }

        std::cerr << "Unknown command: " << command << ". Use --help for usage.\n";
        return 1;

    } catch (const DocuTraceException& e) {
        std::cerr << "\n[Document Tracer Error (Code " << e.getErrorCode() << ")]: " << e.what() << "\n";
        if (!e.getContext().empty()) std::cerr << "Context: " << e.getContext() << "\n";
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "\n[Standard Error]: " << e.what() << "\n";
        return 1;
    }
}
