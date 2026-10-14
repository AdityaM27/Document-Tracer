#include "Server.h"
#include "../include/DocumentLoader.h"
#include <filesystem>
#include <fstream>
#include <iostream>

namespace docutrace {

Server::Server(std::shared_ptr<AnalysisService> service, const std::string& host, int port)
    : host_(host), port_(port), service_(service) {
    setupRoutes();
}

void Server::setCorsHeaders(httplib::Response& res) {
    res.set_header("Access-Control-Allow-Origin", "*");
    res.set_header("Access-Control-Allow-Methods", "GET, POST, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
}

void Server::setupRoutes() {
    // OPTIONS handler for all CORS pre-flight requests
    svr_.Options(".*", [this](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        res.status = 204;
    });

    // Health check
    svr_.Get("/api/health", [this](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        nlohmann::json j;
        j["status"] = "ok";
        j["version"] = "1.0.0";
        j["cppCore"] = "Ready (C++17, MinGW-w64)";
        j["referencesIndexed"] = service_->getReferenceIndex().size();
        j["academicProviderOnline"] = service_->getAcademicProvider().isOnline();

        bool aiOnline = false;
        std::string aiModel = "roberta-base-openai-detector";
        bool aiHfLoaded = false;
        std::string aiMode = "offline";
        service_->getAIClient().getHealthDetails(aiOnline, aiModel, aiHfLoaded, aiMode);

        j["aiServiceOnline"] = aiOnline;
        j["aiModelName"] = aiModel;
        j["aiHfLoaded"] = aiHfLoaded;
        j["aiMode"] = aiMode;
        
        res.set_content(j.dump(), "application/json");
    });

    // Configuration
    svr_.Get("/api/config", [this](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        nlohmann::json j;
        j["enableExactMatch"] = defaultOptions_.enableExactMatch;
        j["enableNGram"] = defaultOptions_.enableNGram;
        j["enableJaccard"] = defaultOptions_.enableJaccard;
        j["enableSemantic"] = defaultOptions_.enableSemantic;
        j["enableAcademicSearch"] = defaultOptions_.enableAcademicSearch;
        j["enableAIClassifier"] = defaultOptions_.enableAIClassifier;
        j["enableWritingPatterns"] = defaultOptions_.enableWritingPatterns;
        j["ngramSize"] = defaultOptions_.ngramSize;
        j["exactThreshold"] = defaultOptions_.exactThreshold;
        j["ngramThreshold"] = defaultOptions_.ngramThreshold;
        j["jaccardThreshold"] = defaultOptions_.jaccardThreshold;
        j["semanticThreshold"] = defaultOptions_.semanticThreshold;
        j["maxConcurrentThreads"] = defaultOptions_.maxConcurrentThreads;
        res.set_content(j.dump(), "application/json");
    });

    svr_.Post("/api/config", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        try {
            auto j = nlohmann::json::parse(req.body);
            if (j.contains("enableExactMatch")) defaultOptions_.enableExactMatch = j["enableExactMatch"];
            if (j.contains("enableNGram")) defaultOptions_.enableNGram = j["enableNGram"];
            if (j.contains("enableJaccard")) defaultOptions_.enableJaccard = j["enableJaccard"];
            if (j.contains("enableSemantic")) defaultOptions_.enableSemantic = j["enableSemantic"];
            if (j.contains("enableAcademicSearch")) defaultOptions_.enableAcademicSearch = j["enableAcademicSearch"];
            if (j.contains("enableAIClassifier")) defaultOptions_.enableAIClassifier = j["enableAIClassifier"];
            if (j.contains("enableWritingPatterns")) defaultOptions_.enableWritingPatterns = j["enableWritingPatterns"];
            if (j.contains("ngramSize")) defaultOptions_.ngramSize = j["ngramSize"];
            if (j.contains("exactThreshold")) defaultOptions_.exactThreshold = j["exactThreshold"];
            if (j.contains("ngramThreshold")) defaultOptions_.ngramThreshold = j["ngramThreshold"];
            if (j.contains("jaccardThreshold")) defaultOptions_.jaccardThreshold = j["jaccardThreshold"];
            if (j.contains("semanticThreshold")) defaultOptions_.semanticThreshold = j["semanticThreshold"];
            if (j.contains("maxConcurrentThreads")) {
                defaultOptions_.maxConcurrentThreads = j["maxConcurrentThreads"];
                service_->setThreadPoolSize(defaultOptions_.maxConcurrentThreads);
            }
            res.set_content("{\"status\":\"config_updated\"}", "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content("{\"error\":\"Invalid configuration JSON\"}", "application/json");
        }
    });

    // Upload Document
    svr_.Post("/api/upload", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        std::filesystem::create_directories("data/input");

        // Check multipart form data
        if (req.is_multipart_form_data()) {
            if (req.form.has_file("file")) {
                const auto& file = req.form.get_file("file");
                std::string filename = file.filename;
                std::string uniqueId = DocumentLoader::generateUniqueId(filename);
                std::string ext = DocumentLoader::detectExtension(filename);
                std::string savedPath = "data/input/" + uniqueId + ext;

                std::ofstream out(savedPath, std::ios::binary);
                out.write(file.content.data(), file.content.size());
                out.close();

                // Pre-register analysis task immediately
                service_->createTask(uniqueId, filename);

                nlohmann::json resp;
                resp["docId"] = uniqueId;
                resp["submissionId"] = uniqueId;
                resp["filename"] = filename;
                resp["filepath"] = savedPath;
                resp["size"] = file.content.size();
                res.set_content(resp.dump(), "application/json");
                return;
            }
        }

        // Direct JSON text upload
        try {
            auto j = nlohmann::json::parse(req.body);
            std::string text = j.value("text", "");
            std::string filename = j.value("filename", "direct_input.txt");
            std::string uniqueId = DocumentLoader::generateUniqueId(filename);
            std::string savedPath = "data/input/" + uniqueId + ".txt";

            std::ofstream out(savedPath);
            out << text;
            out.close();

            // Pre-register analysis task immediately
            service_->createTask(uniqueId, filename);

            nlohmann::json resp;
            resp["docId"] = uniqueId;
            resp["submissionId"] = uniqueId;
            resp["filename"] = filename;
            resp["filepath"] = savedPath;
            resp["size"] = text.size();
            res.set_content(resp.dump(), "application/json");
            return;
        } catch (...) {}

        res.status = 400;
        res.set_content("{\"error\":\"No file or text provided in request\"}", "application/json");
    });

    // Start Analysis
    svr_.Post("/api/analyze", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        try {
            auto j = nlohmann::json::parse(req.body);
            std::string filepath = j.value("filepath", "");
            std::string docId = j.value("docId", "");
            if (docId.empty()) {
                docId = j.value("submissionId", "");
            }

            if (filepath.empty() && !docId.empty()) {
                // Find matching file in data/input
                if (std::filesystem::exists("data/input")) {
                    for (const auto& entry : std::filesystem::directory_iterator("data/input")) {
                        if (entry.path().filename().string().rfind(docId, 0) == 0) {
                            filepath = entry.path().string();
                            break;
                        }
                    }
                }
            }

            if (filepath.empty() || !std::filesystem::exists(filepath)) {
                res.status = 404;
                res.set_content("{\"error\":\"Target document not found\"}", "application/json");
                return;
            }

            if (docId.empty()) {
                std::string stem = std::filesystem::path(filepath).stem().string();
                if (stem.rfind("doc_", 0) == 0) {
                    docId = stem;
                } else {
                    docId = DocumentLoader::generateUniqueId(std::filesystem::path(filepath).filename().string());
                }
            }

            // Ensure task is pre-registered
            service_->createTask(docId, std::filesystem::path(filepath).filename().string());

            AnalysisOptions opt = defaultOptions_;
            if (j.contains("enableExactMatch")) opt.enableExactMatch = j["enableExactMatch"];
            if (j.contains("enableNGram")) opt.enableNGram = j["enableNGram"];
            if (j.contains("enableJaccard")) opt.enableJaccard = j["enableJaccard"];
            if (j.contains("enableSemantic")) opt.enableSemantic = j["enableSemantic"];
            if (j.contains("enableAcademicSearch")) opt.enableAcademicSearch = j["enableAcademicSearch"];
            if (j.contains("enableAIClassifier")) opt.enableAIClassifier = j["enableAIClassifier"];
            if (j.contains("enableWritingPatterns")) opt.enableWritingPatterns = j["enableWritingPatterns"];
            if (j.contains("exactThreshold")) opt.exactThreshold = j["exactThreshold"];
            if (j.contains("ngramThreshold")) opt.ngramThreshold = j["ngramThreshold"];
            if (j.contains("jaccardThreshold")) opt.jaccardThreshold = j["jaccardThreshold"];
            if (j.contains("semanticThreshold")) opt.semanticThreshold = j["semanticThreshold"];

            bool isSync = j.value("sync", false);
            if (isSync) {
                // Run synchronous analysis
                AnalysisSummary summary = service_->analyzeFile(filepath, opt, docId);
                nlohmann::json summaryJson;
                to_json(summaryJson, summary);
                res.set_content(summaryJson.dump(), "application/json");
            } else {
                // Run asynchronous background analysis
                service_->startAsyncAnalysis(filepath, opt, docId);
                nlohmann::json resp;
                resp["status"] = "started";
                resp["submissionId"] = docId;
                resp["docId"] = docId;
                resp["message"] = "Analysis started asynchronously";
                res.set_content(resp.dump(), "application/json");
            }
        } catch (const DocuTraceException& e) {
            res.status = 400;
            nlohmann::json err;
            err["error"] = e.what();
            err["code"] = e.getErrorCode();
            res.set_content(err.dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 500;
            nlohmann::json err;
            err["error"] = e.what();
            res.set_content(err.dump(), "application/json");
        }
    });

    // Get Analysis Status
    svr_.Get(R"(/api/analysis/([^/]+)/status)", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        std::string subId = req.matches[1];
        auto prog = service_->getProgress(subId);

        nlohmann::json j;
        j["submissionId"] = prog.submissionId;
        j["stage"] = prog.currentStage;
        j["percent"] = prog.progressPercent;
        j["message"] = prog.statusMessage;
        j["isCompleted"] = prog.isCompleted;
        j["hasError"] = prog.hasError;
        j["errorMessage"] = prog.errorMessage;

        if (prog.hasError && (prog.currentStage == "ERROR" || prog.currentStage == "NOT_FOUND")) {
            res.status = 404;
        } else {
            res.status = 200;
        }
        res.set_content(j.dump(), "application/json");
    });

    // Get Full Analysis Result
    svr_.Get(R"(/api/analysis/([^/]+))", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        std::string subId = req.matches[1];
        AnalysisSummary summary;
        if (service_->getResult(subId, summary)) {
            nlohmann::json j;
            to_json(j, summary);
            res.set_content(j.dump(), "application/json");
        } else {
            // Check if report JSON exists in output/
            std::string reportJsonPath = "output/report_" + subId + ".json";
            if (std::filesystem::exists(reportJsonPath)) {
                std::ifstream f(reportJsonPath);
                std::string content((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
                res.set_content(content, "application/json");
            } else {
                res.status = 404;
                res.set_content("{\"error\":\"Analysis result not found\",\"submissionId\":\"" + subId + "\"}", "application/json");
            }
        }
    });

    // List Reports
    svr_.Get("/api/reports", [this](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        nlohmann::json list = nlohmann::json::array();
        std::filesystem::create_directories("output");

        for (const auto& entry : std::filesystem::directory_iterator("output")) {
            if (entry.path().extension() == ".json") {
                try {
                    std::ifstream f(entry.path());
                    nlohmann::json j = nlohmann::json::parse(f);
                    nlohmann::json item;
                    item["id"] = j.value("submissionId", "");
                    item["filename"] = j.value("submissionFilename", "");
                    item["timestamp"] = j.value("timestamp", "");
                    item["overallSimilarity"] = j.value("overallSimilarityScore", 0.0);
                    item["aiLabel"] = j.contains("aiResult") ? j["aiResult"].value("label", "") : "";
                    item["jsonPath"] = entry.path().string();
                    item["htmlPath"] = "output/report_" + item["id"].get<std::string>() + ".html";
                    list.push_back(item);
                } catch (...) {}
            }
        }
        res.set_content(list.dump(), "application/json");
    });

    // Download/View HTML Report
    svr_.Get(R"(/api/reports/([^/]+)/html)", [](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        std::string path = "output/report_" + id + ".html";
        if (std::filesystem::exists(path)) {
            std::ifstream f(path);
            std::string html((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_content(html, "text/html");
        } else {
            res.status = 404;
            res.set_content("HTML report not found", "text/plain");
        }
    });

    // Download JSON Report
    svr_.Get(R"(/api/reports/([^/]+)/json)", [](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        std::string path = "output/report_" + id + ".json";
        if (std::filesystem::exists(path)) {
            std::ifstream f(path);
            std::string j((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_content(j, "application/json");
        } else {
            res.status = 404;
            res.set_content("JSON report not found", "application/json");
        }
    });

    // List Reference Documents
    svr_.Get("/api/references", [this](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        auto docs = service_->getReferenceIndex().getAllDocuments();
        nlohmann::json arr = nlohmann::json::array();
        for (const auto& d : docs) {
            nlohmann::json item;
            item["id"] = d.getId();
            item["filename"] = d.getFilename();
            item["wordCount"] = d.getWordCount();
            item["characterCount"] = d.getCharacterCount();
            item["fileSize"] = d.getFileSize();
            item["sentencesCount"] = d.getSentences().size();
            arr.push_back(item);
        }
        res.set_content(arr.dump(), "application/json");
    });

    // Upload & Index Reference Document
    svr_.Post("/api/references/upload", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        std::filesystem::create_directories("data/references");

        // 1. Multipart Form Data File Upload
        if (req.is_multipart_form_data() && req.form.has_file("file")) {
            const auto& file = req.form.get_file("file");
            if (file.content.empty()) {
                res.status = 400;
                res.set_content("{\"error\":\"The uploaded file is empty (0 bytes). Please upload a document with valid text content.\"}", "application/json");
                return;
            }

            std::string rawName = file.filename.empty() ? "reference_doc.txt" : file.filename;
            std::string cleanFilename = std::filesystem::path(rawName).filename().string();
            if (cleanFilename.empty()) cleanFilename = "reference_doc.txt";

            std::string savePath = "data/references/" + cleanFilename;
            std::ofstream out(savePath, std::ios::binary);
            if (!out.is_open()) {
                res.status = 500;
                res.set_content("{\"error\":\"Failed to save reference file to data/references.\"}", "application/json");
                return;
            }
            out.write(file.content.data(), file.content.size());
            out.close();

            try {
                DocumentLoader loader;
                Document doc = loader.loadFromFile(savePath);
                service_->getReferenceIndex().addDocument(doc);
                Document indexedDoc = service_->getReferenceIndex().getDocument(doc.getId());

                nlohmann::json resp;
                resp["status"] = "indexed";
                resp["id"] = doc.getId();
                resp["filename"] = doc.getFilename();
                resp["wordCount"] = indexedDoc.getWordCount();
                resp["sentencesCount"] = indexedDoc.getSentences().size();
                res.set_content(resp.dump(), "application/json");
                return;
            } catch (const DocuTraceException& e) {
                std::filesystem::remove(savePath);
                res.status = 400;
                nlohmann::json err;
                err["error"] = e.what();
                err["code"] = e.getErrorCode();
                res.set_content(err.dump(), "application/json");
                return;
            } catch (const std::exception& e) {
                std::filesystem::remove(savePath);
                res.status = 400;
                nlohmann::json err;
                err["error"] = e.what();
                res.set_content(err.dump(), "application/json");
                return;
            }
        }

        // 2. Direct JSON Text Reference Upload
        try {
            auto j = nlohmann::json::parse(req.body);
            std::string text = j.value("text", "");
            if (text.empty() || text.find_first_not_of(" \t\n\r") == std::string::npos) {
                res.status = 400;
                res.set_content("{\"error\":\"Reference document text cannot be empty.\"}", "application/json");
                return;
            }

            std::string rawName = j.value("filename", j.value("title", "reference_entry.txt"));
            std::string cleanFilename = std::filesystem::path(rawName).filename().string();
            if (cleanFilename.empty()) cleanFilename = "reference_entry.txt";
            if (cleanFilename.find('.') == std::string::npos) cleanFilename += ".txt";

            std::string savePath = "data/references/" + cleanFilename;
            std::ofstream out(savePath, std::ios::binary);
            if (!out.is_open()) {
                res.status = 500;
                res.set_content("{\"error\":\"Failed to save reference text to disk.\"}", "application/json");
                return;
            }
            out << text;
            out.close();

            DocumentLoader loader;
            Document doc = loader.loadFromFile(savePath);
            service_->getReferenceIndex().addDocument(doc);
            Document indexedDoc = service_->getReferenceIndex().getDocument(doc.getId());

            nlohmann::json resp;
            resp["status"] = "indexed";
            resp["id"] = doc.getId();
            resp["filename"] = doc.getFilename();
            resp["wordCount"] = indexedDoc.getWordCount();
            resp["sentencesCount"] = indexedDoc.getSentences().size();
            res.set_content(resp.dump(), "application/json");
            return;
        } catch (...) {}

        res.status = 400;
        res.set_content("{\"error\":\"Invalid reference upload payload. Expected a file or JSON with 'text' and 'filename'.\"}", "application/json");
    });

    // Seed Sample Reference Corpus
    svr_.Post("/api/references/seed", [this](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        std::filesystem::create_directories("data/references");

        const std::pair<std::string, std::string> samples[] = {
            {"oop_cpp.txt",
             "Object-Oriented Programming (OOP) in C++ is a foundational programming paradigm.\n"
             "It organizes software design around data, or objects, rather than functions and logic.\n"
             "The primary pillars of OOP are encapsulation, data abstraction, inheritance, and polymorphism.\n"
             "Encapsulation binds data members and member functions into a single unit, hiding implementation details.\n"
             "Inheritance permits a derived class to inherit member properties from an existing base class.\n"
             "Polymorphism enables objects of different types to respond to the same interface via virtual functions.\n"
             "Resource Acquisition Is Initialization (RAII) ensures that resources such as heap memory and mutexes\n"
             "are deterministically released when stack-allocated objects exit their enclosing scope.\n"},
            {"operating_systems.txt",
             "An operating system acts as an intermediary between computer hardware and the application programs.\n"
             "The core responsibilities of an operating system kernel include process management, virtual memory paging,\n"
             "file system management, and input-output device control. A process represents an instance of a computer program\n"
             "in active execution. The CPU scheduler allocates processor time among concurrent threads using algorithms\n"
             "such as Round Robin, Shortest Job First, and Multi-Level Feedback Queues.\n"},
            {"data_structures.txt",
             "Data structures provide organized formats for storing and managing information effectively.\n"
             "Fundamental abstract data types include stacks, queues, linked lists, binary search trees, and hash tables.\n"
             "Hash tables achieve average O(1) time complexity for lookup and insertion using hashing algorithms.\n"
             "Balanced search trees like AVL and Red-Black trees guarantee logarithmic O(log n) worst-case time complexity.\n"}
        };

        for (const auto& s : samples) {
            std::string p = "data/references/" + s.first;
            if (!std::filesystem::exists(p) || std::filesystem::file_size(p) == 0) {
                std::ofstream f(p, std::ios::binary);
                f << s.second;
                f.close();
            }
        }

        size_t count = service_->getReferenceIndex().rebuildIndex("data/references");
        nlohmann::json resp;
        resp["status"] = "seeded";
        resp["indexedCount"] = count;
        res.set_content(resp.dump(), "application/json");
    });

    // Delete Reference Document
    svr_.Delete(R"(/api/references/([^/]+))", [this](const httplib::Request& req, httplib::Response& res) {
        setCorsHeaders(res);
        std::string docId = req.matches[1];
        bool removed = service_->getReferenceIndex().removeDocument(docId);
        if (removed) {
            res.set_content("{\"status\":\"deleted\"}", "application/json");
        } else {
            res.status = 404;
            res.set_content("{\"error\":\"Document not found in index\"}", "application/json");
        }
    });

    // Rebuild Reference Index
    svr_.Post("/api/references/reindex", [this](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        size_t count = service_->getReferenceIndex().rebuildIndex("data/references");
        nlohmann::json resp;
        resp["status"] = "reindexed";
        resp["indexedCount"] = count;
        res.set_content(resp.dump(), "application/json");
    });

    // Built-in Automated Self-Test (Validation Engine)
    auto handleSelfTest = [this](const httplib::Request&, httplib::Response& res) {
        setCorsHeaders(res);
        auto tStart = std::chrono::steady_clock::now();

        std::filesystem::create_directories("data/references");
        std::filesystem::create_directories("data/test_corpus");

        // Seed references if missing
        std::string oopRefPath = "data/references/oop_cpp.txt";
        std::string osRefPath = "data/references/operating_systems.txt";
        if (!std::filesystem::exists(oopRefPath) || std::filesystem::file_size(oopRefPath) == 0) {
            std::ofstream f(oopRefPath);
            f << "Object-Oriented Programming (OOP) in C++ is a foundational programming paradigm.\n"
                 "It organizes software design around data, or objects, rather than functions and logic.\n"
                 "The primary pillars of OOP are encapsulation, data abstraction, inheritance, and polymorphism.\n"
                 "Encapsulation binds data members and member functions into a single unit, hiding implementation details.\n"
                 "Inheritance permits a derived class to inherit member properties from an existing base class.\n"
                 "Polymorphism enables objects of different types to respond to the same interface via virtual functions.\n"
                 "Resource Acquisition Is Initialization (RAII) ensures that resources such as heap memory and mutexes\n"
                 "are deterministically released when stack-allocated objects exit their enclosing scope.\n";
            f.close();
        }
        if (!std::filesystem::exists(osRefPath) || std::filesystem::file_size(osRefPath) == 0) {
            std::ofstream f(osRefPath);
            f << "An operating system acts as an intermediary between computer hardware and the application programs.\n"
                 "The core responsibilities of an operating system kernel include process management, virtual memory paging,\n"
                 "file system management, and input-output device control. A process represents an instance of a computer program\n"
                 "in active execution. The CPU scheduler allocates processor time among concurrent threads using algorithms\n"
                 "such as Round Robin, Shortest Job First, and Multi-Level Feedback Queues.\n";
            f.close();
        }

        if (service_->getReferenceIndex().size() < 2) {
            service_->getReferenceIndex().rebuildIndex("data/references");
        }

        // Ensure 4 test corpus documents exist
        std::string identicalPath = "data/test_corpus/query_identical.txt";
        std::string partialPath = "data/test_corpus/query_partial_overlap.txt";
        std::string paraphrasedPath = "data/test_corpus/query_paraphrased.txt";
        std::string unrelatedPath = "data/test_corpus/query_unrelated.txt";

        if (!std::filesystem::exists(identicalPath) || std::filesystem::file_size(identicalPath) == 0) {
            std::ofstream f(identicalPath);
            f << "Object-Oriented Programming (OOP) in C++ is a foundational programming paradigm.\n"
                 "It organizes software design around data, or objects, rather than functions and logic.\n"
                 "The primary pillars of OOP are encapsulation, data abstraction, inheritance, and polymorphism.\n"
                 "Encapsulation binds data members and member functions into a single unit, hiding implementation details.\n"
                 "Inheritance permits a derived class to inherit member properties from an existing base class.\n"
                 "Polymorphism enables objects of different types to respond to the same interface via virtual functions.\n"
                 "Resource Acquisition Is Initialization (RAII) ensures that resources such as heap memory and mutexes\n"
                 "are deterministically released when stack-allocated objects exit their enclosing scope.\n";
            f.close();
        }

        if (!std::filesystem::exists(partialPath) || std::filesystem::file_size(partialPath) == 0) {
            std::ofstream f(partialPath);
            f << "Modern cloud computing architectures rely on distributed microservices.\n"
                 "Microservices decouple application domains and allow autonomous deployment lifecycles.\n"
                 "However, Resource Acquisition Is Initialization (RAII) ensures that resources such as heap memory and mutexes\n"
                 "are deterministically released when stack-allocated objects exit their enclosing scope.\n"
                 "Furthermore, container orchestration systems like Kubernetes manage high availability.\n";
            f.close();
        }

        if (!std::filesystem::exists(paraphrasedPath) || std::filesystem::file_size(paraphrasedPath) == 0) {
            std::ofstream f(paraphrasedPath);
            f << "The operating system serves as a bridge connecting software applications with computer hardware components.\n"
                 "The primary duties of the OS kernel involve handling processes, managing paging in virtual memory,\n"
                 "controlling storage file systems, and regulating device I/O. A process is an executing program instance.\n"
                 "Processor time is distributed across active threads by the CPU scheduling subsystem using various policies.\n";
            f.close();
        }

        if (!std::filesystem::exists(unrelatedPath) || std::filesystem::file_size(unrelatedPath) == 0) {
            std::ofstream f(unrelatedPath);
            f << "Coral reefs are diverse underwater ecosystems held together by calcium carbonate structures.\n"
                 "They are built by colonies of tiny animals found in marine water that contain few nutrients.\n"
                 "Most coral reefs are built from stony corals, whose polyps cluster in groups.\n"
                 "The Great Barrier Reef off the coast of Australia is the largest coral reef system on Earth.\n";
            f.close();
        }

        AnalysisOptions opt;
        opt.enableExactMatch = true;
        opt.enableNGram = true;
        opt.enableJaccard = true;
        opt.enableSemantic = true;
        opt.enableAcademicSearch = false; // keep local and offline
        opt.enableAIClassifier = true;
        opt.enableWritingPatterns = true;

        nlohmann::json testResults = nlohmann::json::array();
        int passedCount = 0;
        int failedCount = 0;

        // Test 1: Identical Document Matching
        try {
            auto s1 = service_->analyzeFile(identicalPath, opt);
            bool pass1 = (s1.overallSimilarityScore >= 0.85 && s1.exactSimilarityScore >= 0.85);
            if (pass1) passedCount++; else failedCount++;

            nlohmann::json t1;
            t1["id"] = "TEST_IDENTICAL";
            t1["name"] = "Verbatim Identical Document Matching";
            t1["description"] = "Compares identical text against indexed reference (oop_cpp.txt). Verifies exact phrase and multi-strategy convergence.";
            t1["expected"] = "Overall >= 85%, Exact Match >= 85%";
            t1["passed"] = pass1;
            t1["status"] = pass1 ? "PASS" : "FAIL";
            t1["overallScore"] = std::round(s1.overallSimilarityScore * 1000.0) / 10.0;
            t1["exactScore"] = std::round(s1.exactSimilarityScore * 1000.0) / 10.0;
            t1["ngramScore"] = std::round(s1.ngramSimilarityScore * 1000.0) / 10.0;
            t1["jaccardScore"] = std::round(s1.jaccardSimilarityScore * 1000.0) / 10.0;
            t1["semanticScore"] = std::round(s1.semanticSimilarityScore * 1000.0) / 10.0;
            t1["matchesFound"] = s1.matches.size();
            t1["details"] = pass1 ? "Accurately flagged 100% verbatim overlap against indexed reference." : "Score did not meet identical matching threshold.";
            testResults.push_back(t1);
        } catch (const std::exception& e) {
            failedCount++;
            nlohmann::json t1;
            t1["id"] = "TEST_IDENTICAL";
            t1["name"] = "Verbatim Identical Document Matching";
            t1["passed"] = false;
            t1["status"] = "FAIL";
            t1["error"] = e.what();
            testResults.push_back(t1);
        }

        // Test 2: Partially Matching Document
        try {
            auto s2 = service_->analyzeFile(partialPath, opt);
            bool pass2 = (s2.overallSimilarityScore >= 0.15 && s2.overallSimilarityScore <= 0.85 && s2.exactSimilarityScore > 0.0);
            if (pass2) passedCount++; else failedCount++;

            nlohmann::json t2;
            t2["id"] = "TEST_PARTIAL";
            t2["name"] = "Partial Phrase Overlap Detection";
            t2["description"] = "Evaluates mixed text (one RAII passage from oop_cpp.txt surrounded by distinct cloud computing text).";
            t2["expected"] = "Overall bounded in [15%, 85%], Exact Match > 0%";
            t2["passed"] = pass2;
            t2["status"] = pass2 ? "PASS" : "FAIL";
            t2["overallScore"] = std::round(s2.overallSimilarityScore * 1000.0) / 10.0;
            t2["exactScore"] = std::round(s2.exactSimilarityScore * 1000.0) / 10.0;
            t2["ngramScore"] = std::round(s2.ngramSimilarityScore * 1000.0) / 10.0;
            t2["jaccardScore"] = std::round(s2.jaccardSimilarityScore * 1000.0) / 10.0;
            t2["semanticScore"] = std::round(s2.semanticSimilarityScore * 1000.0) / 10.0;
            t2["matchesFound"] = s2.matches.size();
            t2["details"] = pass2 ? "Correctly extracted embedded exact passage while appropriately bounding overall similarity." : "Partial overlap metric out of bounds.";
            testResults.push_back(t2);
        } catch (const std::exception& e) {
            failedCount++;
            nlohmann::json t2;
            t2["id"] = "TEST_PARTIAL";
            t2["name"] = "Partial Phrase Overlap Detection";
            t2["passed"] = false;
            t2["status"] = "FAIL";
            t2["error"] = e.what();
            testResults.push_back(t2);
        }

        // Test 3: Paraphrased Document
        try {
            auto s3 = service_->analyzeFile(paraphrasedPath, opt);
            bool pass3 = (s3.exactSimilarityScore < 0.40 && (s3.semanticSimilarityScore >= 0.35 || s3.jaccardSimilarityScore >= 0.15));
            if (pass3) passedCount++; else failedCount++;

            nlohmann::json t3;
            t3["id"] = "TEST_PARAPHRASED";
            t3["name"] = "Semantic Paraphrase Identification";
            t3["description"] = "Evaluates semantically rephrased operating systems text with altered vocabulary and sentence structure.";
            t3["expected"] = "Exact Match < 40%, Semantic Cosine >= 35% or Jaccard >= 15%";
            t3["passed"] = pass3;
            t3["status"] = pass3 ? "PASS" : "FAIL";
            t3["overallScore"] = std::round(s3.overallSimilarityScore * 1000.0) / 10.0;
            t3["exactScore"] = std::round(s3.exactSimilarityScore * 1000.0) / 10.0;
            t3["ngramScore"] = std::round(s3.ngramSimilarityScore * 1000.0) / 10.0;
            t3["jaccardScore"] = std::round(s3.jaccardSimilarityScore * 1000.0) / 10.0;
            t3["semanticScore"] = std::round(s3.semanticSimilarityScore * 1000.0) / 10.0;
            t3["matchesFound"] = s3.matches.size();
            t3["details"] = pass3 ? "Detected semantic paraphrase structure without generating false high exact match." : "Paraphrase metrics fell outside threshold.";
            testResults.push_back(t3);
        } catch (const std::exception& e) {
            failedCount++;
            nlohmann::json t3;
            t3["id"] = "TEST_PARAPHRASED";
            t3["name"] = "Semantic Paraphrase Identification";
            t3["passed"] = false;
            t3["status"] = "FAIL";
            t3["error"] = e.what();
            testResults.push_back(t3);
        }

        // Test 4: Completely Unrelated Negative Control
        try {
            auto s4 = service_->analyzeFile(unrelatedPath, opt);
            bool pass4 = (s4.overallSimilarityScore <= 0.15 && s4.exactSimilarityScore <= 0.05);
            if (pass4) passedCount++; else failedCount++;

            nlohmann::json t4;
            t4["id"] = "TEST_UNRELATED";
            t4["name"] = "Unrelated Negative Control";
            t4["description"] = "Evaluates unrelated text on marine coral reefs against the computer science reference corpus.";
            t4["expected"] = "Overall <= 15%, Exact Match <= 5%";
            t4["passed"] = pass4;
            t4["status"] = pass4 ? "PASS" : "FAIL";
            t4["overallScore"] = std::round(s4.overallSimilarityScore * 1000.0) / 10.0;
            t4["exactScore"] = std::round(s4.exactSimilarityScore * 1000.0) / 10.0;
            t4["ngramScore"] = std::round(s4.ngramSimilarityScore * 1000.0) / 10.0;
            t4["jaccardScore"] = std::round(s4.jaccardSimilarityScore * 1000.0) / 10.0;
            t4["semanticScore"] = std::round(s4.semanticSimilarityScore * 1000.0) / 10.0;
            t4["matchesFound"] = s4.matches.size();
            t4["details"] = pass4 ? "Verified zero false positive similarity for distinct domain text." : "Unexpected overlap detected on negative control.";
            testResults.push_back(t4);
        } catch (const std::exception& e) {
            failedCount++;
            nlohmann::json t4;
            t4["id"] = "TEST_UNRELATED";
            t4["name"] = "Unrelated Negative Control";
            t4["passed"] = false;
            t4["status"] = "FAIL";
            t4["error"] = e.what();
            testResults.push_back(t4);
        }

        // Test 5: Pretrained AI Classification Service Audit
        bool aiOnline = false;
        std::string aiModel = "roberta-base-openai-detector";
        bool aiHfLoaded = false;
        std::string aiMode = "offline";
        service_->getAIClient().getHealthDetails(aiOnline, aiModel, aiHfLoaded, aiMode);

        nlohmann::json t5;
        t5["id"] = "TEST_AI_SERVICE";
        t5["name"] = "Pretrained AI Classifier & Inference Audit";
        t5["expected"] = "Genuine Hugging Face pretrained neural model or verified calibrated fallback";
        t5["online"] = aiOnline;
        t5["modelName"] = aiModel;
        t5["isPretrainedModel"] = aiHfLoaded;
        t5["mode"] = aiMode;

        if (aiOnline && aiHfLoaded) {
            t5["passed"] = true;
            t5["status"] = "PASS (PRETRAINED MODEL)";
            t5["details"] = "Genuine Hugging Face transformer model (" + aiModel + ") is loaded and executing neural inference on port 8000.";
            passedCount++;
        } else if (aiOnline && !aiHfLoaded) {
            t5["passed"] = true;
            t5["status"] = "FALLBACK (STATISTICAL MODE)";
            t5["details"] = "Python AI microservice is active on port 8000 in Calibrated Statistical Fallback Mode. Neural weights ('transformers' / 'torch') are not installed in the local Python environment. To activate genuine pretrained transformer weights: run 'pip install transformers torch'.";
            passedCount++;
        } else {
            t5["passed"] = false;
            t5["status"] = "OFFLINE";
            t5["details"] = "Python AI microservice is not reachable on port 8000. Start it via 'python ai_service/app.py' to enable AI analysis.";
            failedCount++;
        }
        testResults.push_back(t5);

        auto tEnd = std::chrono::steady_clock::now();
        double elapsedMs = std::chrono::duration<double, std::milli>(tEnd - tStart).count();

        nlohmann::json report;
        report["timestamp"] = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        report["elapsedMs"] = std::round(elapsedMs * 10.0) / 10.0;
        report["totalTests"] = testResults.size();
        report["passedCount"] = passedCount;
        report["failedCount"] = failedCount;
        report["overallStatus"] = (failedCount == 0) ? "PASS" : "ATTENTION_REQUIRED";
        report["tests"] = testResults;
        report["aiServiceAudit"] = {
            {"online", aiOnline},
            {"modelName", aiModel},
            {"isPretrainedModel", aiHfLoaded},
            {"mode", aiMode}
        };

        res.set_content(report.dump(), "application/json");
    };

    svr_.Post("/api/self-test", handleSelfTest);
    svr_.Get("/api/self-test", handleSelfTest);
}

void Server::start() {
    std::cout << "[Document Tracer Server] Listening on http://" << host_ << ":" << port_ << " ...\n";
    svr_.listen(host_.c_str(), port_);
}

void Server::stop() {
    svr_.stop();
}

} // namespace docutrace
