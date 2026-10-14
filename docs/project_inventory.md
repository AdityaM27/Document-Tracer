# Document Tracer: Complete Project Inventory

**Project Location:** `C:\C++ CBP`  
**Standard:** C++17 / C++20, React 18 / TypeScript, Python 3.11  
**Target:** Academic & Engineering Document Similarity and AI-Text Analysis System  

---

## 1. Directory Structure Overview

```
C:\C++ CBP\
├── include\               # C++ public class declarations and header interfaces
├── src\                   # C++ component implementations and CLI entry point
├── backend\               # C++ REST API server implementation (cpp-httplib)
├── ai_service\            # Python microservice (Flask, Hugging Face transformers, statistical fallback)
├── frontend\              # React 18 + TypeScript + Vite web interface
├── tests\                 # Comprehensive unit, integration, and regression test suites
├── scripts\               # Automation, build, and execution scripts
├── docs\                  # Architectural, algorithmic, and educational documentation
├── data\                  # Input uploads, reference corpus, and persistent caches
└── output\                # Persisted JSON and HTML analysis reports
```

---

## 2. Comprehensive File Inventory

### Core C++ Components (`include/` and `src/`)

| File Name | Purpose | Key Classes & Functions | Dependencies | Inspected | Tested |
| :--- | :--- | :--- | :--- | :---: | :---: |
| `include/Document.h`<br>`src/Document.cpp` | In-memory representation of ingested documents and extracted text passages. | `Document`, `Token`, `PassageSpan`, `calculateWordFrequencies()`, `getOriginalText()` | C++ STL (`<string>`, `<vector>`, `<map>`) | Yes | Yes |
| `include/Tokenizer.h`<br>`src/Tokenizer.cpp` | Lexical analysis, word token extraction, character offset tracking, case/punctuation normalization. | `Tokenizer`, `tokenize()`, `isWhitespace()`, `isPunctuation()`, `toLower()` | C++ STL | Yes | Yes |
| `include/TextProcessor.h`<br>`src/TextProcessor.cpp` | High-level linguistic normalization, sentence segmentation, n-gram construction, and sliding passage windows. | `TextProcessor`, `normalizeWhitespace()`, `splitSentences()`, `generateNGrams()`, `buildPassages()` | `Document`, `Tokenizer` | Yes | Yes |
| `include/DocumentLoader.h`<br>`src/DocumentLoader.cpp` | Multi-format document ingestion from disk (TXT, DOCX XML, PDF streams) and raw text input. | `DocumentLoader`, `loadFromFile()`, `loadFromText()`, `extractFromTxt()`, `extractFromDocx()`, `extractFromPdf()`, `generateUniqueId()` | `Document`, `Exceptions`, `<filesystem>` | Yes | Yes |
| `include/Exceptions.h` | Comprehensive custom exception hierarchy demonstrating course Unit V concepts. | `DocuTraceException`, `DocumentExtractionException`, `InvalidFormatException`, `EmptyDocumentException`, `FileSizeLimitException`, `IndexException`, `NetworkException`, `AlgorithmException` | `<stdexcept>`, `<string>` | Yes | Yes |
| `include/SimilarityStrategy.h` | Abstract base class establishing the polymorphic similarity strategy interface (Unit II & III). | `SimilarityStrategy`, `calculateSimilarity()`, `findPassageMatches()`, `getStrategyName()` | `Document`, `Models` | Yes | Yes |
| `include/ExactMatchStrategy.h`<br>`src/ExactMatchStrategy.cpp` | Exact verbatim substring and passage match detector with rolling n-word fingerprinting. | `ExactMatchStrategy`, `calculateSimilarity()`, `findPassageMatches()` | `SimilarityStrategy`, `Document` | Yes | Yes |
| `include/NGramSimilarity.h`<br>`src/NGramSimilarity.cpp` | Multi-word n-gram overlap and containment coefficient calculator. | `NGramSimilarity`, `calculateSimilarity()`, `findPassageMatches()`, `extractNGrams()` | `SimilarityStrategy`, `Document` | Yes | Yes |
| `include/JaccardSimilarity.h`<br>`src/JaccardSimilarity.cpp` | Lexical set-theoretic token similarity measuring intersection over union: \(J(A,B) = \frac{\|A \cap B\|}{\|A \cup B\|}\). | `JaccardSimilarity`, `calculateSimilarity()`, `findPassageMatches()` | `SimilarityStrategy`, `Document` | Yes | Yes |
| `include/SemanticSimilarity.h`<br>`src/SemanticSimilarity.cpp` | Dense vector cosine similarity of document embeddings via feature hashing and microservice representations. | `SemanticSimilarity`, `calculateSimilarity()`, `findPassageMatches()`, `computeFeatureVector()`, `cosineSimilarity()` | `SimilarityStrategy`, `Document` | Yes | Yes |
| `include/ReferenceIndex.h`<br>`src/ReferenceIndex.cpp` | Local reference document corpus manager and in-memory inverted index for sub-millisecond candidate filtering. | `ReferenceIndex`, `indexDocument()`, `findCandidateDocuments()`, `buildIndex()`, `removeDocument()`, `rebuildIndex()` | `Document`, `DocumentLoader`, `<filesystem>` | Yes | Yes |
| `include/SourceProvider.h` | Abstract interface for external literature and internet source discovery providers. | `SourceProvider`, `search()`, `fetchContent()`, `getProviderName()` | `Models` | Yes | Yes |
| `include/AcademicSourceProvider.h`<br>`src/AcademicSourceProvider.cpp` | Scholarly publication metadata search querying OpenAlex and Crossref APIs with persistent disk caching. | `AcademicSourceProvider`, `search()`, `fetchFromOpenAlex()`, `fetchFromCrossref()`, `loadCache()`, `saveCache()` | `SourceProvider`, `httplib`, `nlohmann::json` | Yes | Yes |
| `include/WebSourceProvider.h`<br>`src/WebSourceProvider.cpp` | General-web search query provider with cache fallbacks and offline resilience. | `WebSourceProvider`, `search()`, `fetchContent()` | `SourceProvider`, `httplib`, `nlohmann::json` | Yes | Yes |
| `include/SourceDiscovery.h`<br>`src/SourceDiscovery.cpp` | Orchestrator managing multiple discovery providers, selecting salient query sentences, retrieving candidate source text, and computing similarity. | `SourceDiscovery`, `addProvider()`, `discoverSources()`, `compareAgainstSources()` | `SourceProvider`, `Document`, `ExactMatchStrategy` | Yes | Yes |
| `include/WritingPatternAnalyzer.h`<br>`src/WritingPatternAnalyzer.cpp` | Explainable stylometric linguistic analyzer measuring TTR, Root TTR, sentence burstiness, Shannon entropy, and discourse markers. | `WritingPatternAnalyzer`, `analyze()`, `calculateTTR()`, `calculateRootTTR()`, `calculateEntropy()`, `calculateBurstiness()` | `Document`, `Models`, `<cmath>` | Yes | Yes |
| `include/AIAnalysisClient.h`<br>`src/AIAnalysisClient.cpp` | Native C++ client communicating with local Python AI microservice over HTTP REST. | `AIAnalysisClient`, `classifyDocument()`, `classifyText()`, `checkHealth()`, `getHealthDetails()` | `httplib`, `nlohmann::json`, `Models` | Yes | Yes |
| `include/ThreadPool.h`<br>`src/ThreadPool.cpp` | Work-stealing / Producer-Consumer multithreaded worker pool (Unit IV) for concurrent document comparisons. | `ThreadPool`, `enqueue()`, `waitAll()`, `workerLoop()` | `<thread>`, `<mutex>`, `<condition_variable>`, `<future>` | Yes | Yes |
| `include/ReportGenerator.h`<br>`src/ReportGenerator.cpp` | Audit report serialization into formatted JSON and self-contained interactive HTML reports. | `ReportGenerator`, `generateJsonReport()`, `generateHtmlReport()` | `Document`, `Models`, `nlohmann::json`, `<fstream>` | Yes | Yes |
| `include/AnalysisService.h`<br>`src/AnalysisService.cpp` | Top-level coordinator orchestrating extraction, multithreaded comparisons, source retrieval, stylometrics, AI analysis, reports, and task progress tracking. | `AnalysisService`, `createTask()`, `hasTask()`, `startAsyncAnalysis()`, `analyzeFile()`, `analyzeText()`, `getProgress()`, `getResult()` | All Core Modules | Yes | Yes |
| `include/Models.h` | Shared C++ data models, configuration options, match records, and JSON serialization converters (`to_json`/`from_json`). | `AnalysisOptions`, `MatchRecord`, `AnalysisSummary`, `WritingPatternStats`, `AIClassificationResult` | `nlohmann::json` | Yes | Yes |

---

### Backend HTTP Server (`backend/`)

| File Name | Purpose | Key Classes & Functions | Dependencies | Inspected | Tested |
| :--- | :--- | :--- | :--- | :---: | :---: |
| `backend/Server.h`<br>`backend/Server.cpp` | C++ REST API HTTP server exposing endpoints for uploads, async analysis, status polling, library management, and reports. | `Server`, `start()`, `stop()`, `setupRoutes()`, `setCorsHeaders()`, `/api/upload`, `/api/analyze`, `/api/analysis/:id/status`, `/api/analysis/:id`, `/api/references`, `/api/reports`, `/api/selftest` | `AnalysisService`, `httplib`, `nlohmann::json` | Yes | Yes |
| `backend/server_main.cpp` | Executable entry point for the Document Tracer C++ HTTP API Server daemon. | `main()`, `signalHandler()` | `Server`, `AnalysisService` | Yes | Yes |

---

### Python AI Microservice (`ai_service/`)

| File Name | Purpose | Key Classes & Functions | Dependencies | Inspected | Tested |
| :--- | :--- | :--- | :--- | :---: | :---: |
| `ai_service/app.py` | Flask HTTP microservice running on port 8000. Serves Hugging Face transformer models or explainable statistical fallback. | `app`, `initialize_model()`, `calculate_statistical_ai_score()`, `/health`, `/analyze`, `/embed` | `flask`, `transformers`, `torch`, `requests` | Yes | Yes |
| `ai_service/requirements.txt` | Python dependency declaration file. | Declares `flask`, `transformers`, `torch`, `requests`, `numpy`. | Python Package Index | Yes | Yes |

---

### Frontend Web Application (`frontend/`)

| File Name | Purpose | Key Classes & Functions | Dependencies | Inspected | Tested |
| :--- | :--- | :--- | :--- | :---: | :---: |
| `frontend/src/main.tsx` | React 18 root mounting script. | Mounts `App` into `#root` DOM element. | `react`, `react-dom` | Yes | Yes |
| `frontend/src/App.tsx` | Main application shell, route state manager, background health polling, active submission lifecycle. | `App`, tab navigation (`dashboard`, `upload`, `progress`, `results`, `references`, `reports`, `selftest`) | `api.ts`, View components | Yes | Yes |
| `frontend/src/api.ts` | Frontend HTTP client communicating with C++ backend on `/api`. | `api`, `getHealth()`, `uploadFile()`, `uploadText()`, `startAnalysis()`, `getAnalysisStatus()`, `getAnalysisResult()`, `getReferences()`, `uploadReference()`, `runSelfTest()` | Fetch API | Yes | Yes |
| `frontend/src/types.ts` | Complete TypeScript type definitions matching backend C++ data contracts. | `AnalysisOptions`, `AnalysisSummary`, `AnalysisProgress`, `MatchRecord`, `AIClassificationResult`, `SystemHealth` | TypeScript | Yes | Yes |
| `frontend/src/components/Navbar.tsx` | Top navigation bar with Document Tracer logo, tab switchers, and real-time backend/AI service status pills. | `Navbar` | `lucide-react` | Yes | Yes |
| `frontend/src/views/DashboardView.tsx` | Centered introductory dashboard displaying genuine backend stats, indexed references, completed audits, and recent runs table. | `DashboardView` | `types.ts` | Yes | Yes |
| `frontend/src/views/UploadView.tsx` | Document upload dropzone (TXT, DOCX, PDF) and text-paste editor with configurable analysis settings. | `UploadView`, `handleSubmit()`, `handleDrop()` | `api.ts`, `lucide-react` | Yes | Yes |
| `frontend/src/views/ProgressView.tsx` | Real-time analysis progress tracker polling `/api/analysis/:id/status` across all 7 pipeline stages. | `ProgressView`, `pollStatus()`, stage icons checklist | `api.ts`, `lucide-react` | Yes | Yes |
| `frontend/src/views/ResultsView.tsx` | Comprehensive audit results display: similarity metrics, matched passages side-by-side, AI classification card, and stylometrics. | `ResultsView`, matched passage highlight renderer | `types.ts`, `lucide-react` | Yes | Yes |
| `frontend/src/views/ReferenceLibraryView.tsx` | Reference document library table with search filter, document inspector, and file/text upload forms. | `ReferenceLibraryView`, reference file upload | `api.ts`, `lucide-react` | Yes | Yes |
| `frontend/src/views/ReportsView.tsx` | Audit reports catalog table listing document names, dates, similarity scores, with HTML view & JSON download actions. | `ReportsView` | `api.ts`, `lucide-react` | Yes | Yes |
| `frontend/src/views/SelfTestView.tsx` | Live validation center executing genuine tests against the C++ engine and auditing AI service mode. | `SelfTestView`, `runTest()` | `api.ts`, `lucide-react` | Yes | Yes |

---

### Tests and Benchmarks (`tests/`)

| File Name | Purpose | Key Classes & Functions | Dependencies | Inspected | Tested |
| :--- | :--- | :--- | :--- | :---: | :---: |
| `tests/test_main.cpp` | Comprehensive automated test suite comprising 8 test suites and 67 assertions. | `testTokenizerAndNormalization()`, `testSimilarityStrategies()`, `testDocumentLoaderAndExceptions()`, `testReferenceIndex()`, `testStylometricsAndWritingPatterns()`, `testThreadPoolConcurrency()`, `testEndToEndAnalysisService()`, `testAnalysisTaskLifecycleAndRegression()` | `docutrace_core` | Yes | Yes (67/67 PASS) |
| `tests/benchmark.cpp` | Performance and scalability benchmark measuring throughput, memory footprint, and thread pool scaling. | `benchmarkSimilarityStrategies()`, `benchmarkConcurrencyScaling()`, `benchmarkTextExtraction()` | `docutrace_core` | Yes | Yes |

---

### Scripts & Automation (`scripts/`)

| File Name | Purpose | Primary Commands | Inspected | Tested |
| :--- | :--- | :--- | :---: | :---: |
| `scripts/build.ps1` | Automated CMake configure and Ninja build script. | `cmake -B build -G Ninja`, `cmake --build build` | Yes | Yes |
| `scripts/run_server.ps1` | Launches the C++ HTTP backend REST server on port 8080. | `.\build\docutrace_server.exe --port 8080` | Yes | Yes |
| `scripts/run_ai_service.ps1` | Launches the Python AI classification microservice on port 8000. | `python ai_service/app.py` | Yes | Yes |
| `scripts/run_frontend.ps1` | Starts the Vite development server on port 5173. | `npm run dev` | Yes | Yes |
| `scripts/run_tests.ps1` | Runs the automated C++ unit and regression test suite. | `.\build\docutrace_tests.exe` | Yes | Yes |
| `scripts/run_benchmark.ps1` | Runs the performance and scalability benchmark. | `.\build\docutrace_benchmark.exe` | Yes | Yes |
| `scripts/extract_text.py` | Python fallback helper script for DOCX and PDF text extraction. | `docx.Document`, `pypdf.PdfReader` | Yes | Yes |
| `scripts/fetch_academic.py` | Python helper script for OpenAlex and Crossref testing. | `requests` | Yes | Yes |

---

## 3. Key Pipeline Flow & Entry Points

1. **CLI Entry Point:** `src/main.cpp` (`docutrace_cli.exe`)
2. **Backend Server Entry Point:** `backend/server_main.cpp` (`docutrace_server.exe` on port 8080)
3. **AI Microservice Entry Point:** `ai_service/app.py` (Flask on port 8000)
4. **Frontend Entry Point:** `frontend/src/main.tsx` (Vite on port 5173)
5. **Core Document Analysis Pipeline:**
   `DocumentLoader` (file/text ingestion)  
   $\rightarrow$ `TextProcessor` (normalization, sentence splitting, passage construction)  
   $\rightarrow$ `ReferenceIndex` (inverted index lookup for candidate reference documents)  
   $\rightarrow$ `ThreadPool` concurrent comparison across polymorphic `SimilarityStrategy` implementations:
     - `ExactMatchStrategy` (word n-gram exact passage alignment)
     - `NGramSimilarity` (n-gram containment)
     - `JaccardSimilarity` (token set intersection over union)
     - `SemanticSimilarity` (dense feature vector cosine similarity)  
   $\rightarrow$ `SourceDiscovery` (OpenAlex & Crossref scholarly literature discovery)  
   $\rightarrow$ `AIAnalysisClient` (local Python AI microservice query)  
   $\rightarrow$ `WritingPatternAnalyzer` (stylometric and linguistic pattern extraction)  
   $\rightarrow$ `ReportGenerator` (persisted JSON and interactive HTML report serialization)
