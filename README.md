# DOCUMENT TRACER

## Intelligent Document Similarity and AI-Text Analysis System

**Course Context**: B.Tech III Semester &bull; Object Oriented Programming Through C++ Laboratory Project  
**Implementation**: Complete C++17 Core Engine &bull; Producer-Consumer Concurrency &bull; Python AI Microservice &bull; React TypeScript Dashboard  

---

## 1. Project Overview

**Document Tracer** is a full-stack, enterprise-grade document similarity and stylometric analysis system engineered in modern C++ (C++17/C++20). It enables educators, researchers, and students to inspect submitted documents for:

1. **Exact Phrase Matching**: Detects identical sentence sequences with boundary protection and character offset tracking.
2. **N-Gram Multi-Set Containment**: Detects multi-word phrase overlap (default 3-grams) resistant to minor token alterations.
3. **Jaccard Lexical Overlap**: Evaluates vocabulary set intersection over union with division-by-zero protection.
4. **Semantic Paraphrase Candidates**: Vector cosine similarity identifying candidate paraphrased passages.
5. **Scholarly Literature Discovery**: Queries OpenAlex and Crossref using distinctive phrases extracted from the document.
6. **Explainable Stylometrics**: Calculates Type-Token Ratio (TTR), sentence burstiness, Shannon token entropy, and discourse transition frequencies.
7. **Pretrained AI-Writing Classification**: Integrates Hugging Face pretrained models (`roberta-base-openai-detector`) with graceful offline statistical inference.
8. **Producer-Consumer Multithreading**: Coordinates worker threads using `std::thread`, `std::mutex`, `std::condition_variable`, and `std::atomic` to parallelize comparisons across reference documents.
9. **Dual HTML & JSON Reporting**: Serializes comprehensive audit files with interactive side-by-side passage comparison.

---

## 2. Technology Stack

* **Core Backend**: C++17, MinGW-w64 (GCC 16.2), CMake 4.4, Ninja.
* **HTTP Framework**: `cpp-httplib` (header-only, multi-threaded REST server).
* **JSON Serialization**: `nlohmann/json` (modern C++ JSON).
* **AI Microservice**: Python 3.11, Flask, Hugging Face Transformers, PyTorch, NumPy.
* **External APIs**: OpenAlex REST API, Crossref REST API.
* **Frontend**: React 18 / 19, TypeScript, Vite, Lucide Icons.

---

## 3. Project Structure

```
DocumentTracer/
|-- CMakeLists.txt              # CMake build configuration for core, CLI, server, tests & benchmarks
|-- README.md                   # Complete system manual & documentation
|-- .gitignore                  # Git exclusion rules
|-- .env.example                # Environment variables template
|-- requirements.txt            # Python AI service requirements
|
|-- include/                    # C++ Header Declarations
|   |-- Exceptions.h            # Custom exception hierarchy (Unit V)
|   |-- Models.h                # Data structs, enums & JSON serialization
|   |-- Document.h              # Document abstraction (Unit II OOP)
|   |-- Tokenizer.h             # Tokenizer preserving source char offsets (Unit I)
|   |-- TextProcessor.h         # Normalization & segmentation (Unit I & II)
|   |-- SimilarityStrategy.h    # Abstract Base Class (Unit III Polymorphism)
|   |-- ExactMatchStrategy.h    # Exact phrase matching
|   |-- NGramSimilarity.h       # N-gram multiset containment
|   |-- JaccardSimilarity.h     # Jaccard set overlap
|   |-- SemanticSimilarity.h    # Vector cosine similarity
|   |-- DocumentLoader.h        # Ingestion for TXT, DOCX, PDF (Unit IV I/O)
|   |-- ReferenceIndex.h        # Inverted index & disk persistence
|   |-- SourceProvider.h        # Source discovery interface (Unit III)
|   |-- AcademicSourceProvider.h# OpenAlex & Crossref provider
|   |-- WebSourceProvider.h     # Web search provider
|   |-- SourceDiscovery.h       # Orchestrator for literature discovery
|   |-- WritingPatternAnalyzer.h# Explainable stylometrics
|   |-- AIAnalysisClient.h      # HTTP client to Python microservice
|   |-- ThreadPool.h            # Producer-consumer thread pool (Unit V)
|   |-- ReportGenerator.h       # HTML & JSON report generation (Unit IV)
|   |-- AnalysisService.h       # High-level pipeline coordinator
|
|-- src/                        # C++ Implementation Files
|   |-- Document.cpp
|   |-- Tokenizer.cpp
|   |-- TextProcessor.cpp
|   |-- DocumentLoader.cpp
|   |-- ExactMatchStrategy.cpp
|   |-- NGramSimilarity.cpp
|   |-- JaccardSimilarity.cpp
|   |-- SemanticSimilarity.cpp
|   |-- ReferenceIndex.cpp
|   |-- AcademicSourceProvider.cpp
|   |-- WebSourceProvider.cpp
|   |-- SourceDiscovery.cpp
|   |-- WritingPatternAnalyzer.cpp
|   |-- AIAnalysisClient.cpp
|   |-- ThreadPool.cpp
|   |-- ReportGenerator.cpp
|   |-- AnalysisService.cpp
|   |-- main.cpp                # CLI entry point (docutrace_cli)
|
|-- backend/                    # C++ REST API Server
|   |-- Server.h
|   |-- Server.cpp
|   |-- server_main.cpp         # Server entry point (docutrace_server)
|
|-- third_party/                # Vendored Header Libraries
|   |-- httplib.h               # cpp-httplib HTTP server/client
|   |-- nlohmann/json.hpp       # Modern C++ JSON
|
|-- ai_service/                 # Pretrained AI Microservice
|   |-- app.py                  # Flask microservice
|   |-- requirements.txt
|   |-- README.md
|
|-- frontend/                   # React + TypeScript Dashboard (Vite)
|   |-- src/
|   |   |-- components/Navbar.tsx
|   |   |-- views/DashboardView.tsx
|   |   |-- views/UploadView.tsx
|   |   |-- views/ProgressView.tsx
|   |   |-- views/ResultsView.tsx
|   |   |-- views/ReferenceLibraryView.tsx
|   |   |-- views/ReportsView.tsx
|   |   |-- api.ts
|   |   |-- types.ts
|   |   |-- App.tsx
|   |   |-- App.css
|   |-- vite.config.ts
|   |-- package.json
|
|-- data/                       # Document Data Directories
|   |-- references/             # Local reference documents & inverted index cache
|   |-- test_corpus/            # Synthetic test corpus (identical, paraphrased, etc.)
|   |-- input/                  # Ingested upload files
|   |-- cache/                  # Academic query cache
|
|-- output/                     # Generated HTML & JSON reports
|
|-- tests/                      # Automated Test Suite
|   |-- test_main.cpp           # 46 Automated unit & integration tests
|   |-- benchmark.cpp           # Concurrency & accuracy benchmark executable
|
|-- docs/                       # Technical Documentation
|   |-- architecture.md         # Mermaid system diagrams & concurrency model
|   |-- algorithms.md           # Mathematical formulations & complexities
|   |-- api_setup.md            # REST API & external service integration
|   |-- testing.md              # Test matrix, benchmarks & evaluation metrics
|   |-- viva_questions.md       # 35+ Viva-voce questions and answers
|   |-- cpp_syllabus_mapping.md # Mapping to B.Tech III Sem C++ syllabus
|
|-- scripts/                    # Automation Scripts
|   |-- build.ps1               # Complete build script
|   |-- run_server.ps1          # Launch C++ REST API server
|   |-- run_ai_service.ps1      # Launch Python AI microservice
|   |-- run_frontend.ps1        # Launch React web dashboard
|   |-- run_tests.ps1           # Execute automated test suite
|   |-- run_benchmark.ps1       # Execute benchmark suite
|   |-- fetch_academic.py       # OpenAlex & Crossref fetcher helper
|   |-- extract_text.py         # DOCX/PDF text extraction helper
```

---

## 4. Quick Start: Build & Execution

### Step 1: Build the C++ Application & Frontend
```powershell
.\scripts\build.ps1
```
Or build manually:
```powershell
cmake -B build -G Ninja
cmake --build build
```
This generates four executables in `build/`:
- `docutrace_cli.exe`: Command-line interface.
- `docutrace_server.exe`: C++ REST API server.
- `docutrace_tests.exe`: Automated test suite.
- `docutrace_benchmark.exe`: Multithreading & accuracy benchmark.

---

### Step 2: Run Automated Tests
```powershell
.\scripts\run_tests.ps1
```
*Expected Output*: `TEST RESULTS: 46 PASSED, 0 FAILED`.

---

### Step 3: Run Concurrency & Accuracy Benchmarks
```powershell
.\scripts\run_benchmark.ps1
```
*Outputs*:
- Concurrency throughput across 1, 2, 4, 8 threads.
- Precision ($100\%$), Recall ($100\%$), F1 Score ($1.000$) on the labeled test corpus.

---

### Step 4: Run the CLI
Analyze an existing document:
```powershell
.\build\docutrace_cli.exe analyze data/test_corpus/query_identical.txt
```
Manage reference index:
```powershell
.\build\docutrace_cli.exe index-list
.\build\docutrace_cli.exe index-add data/references/operating_systems.txt
.\build\docutrace_cli.exe index-rebuild data/references
```

---

### Step 5: Run the Full-Stack Web Dashboard

Open three terminal windows (or run via background processes):

**Terminal 1 — C++ REST API Server (Port 8080)**:
```powershell
.\scripts\run_server.ps1
```

**Terminal 2 — Python AI Microservice (Port 8000)**:
```powershell
.\scripts\run_ai_service.ps1
```

**Terminal 3 — React TypeScript Frontend (Port 5173)**:
```powershell
.\scripts\run_frontend.ps1
```

Open your browser to: **`http://localhost:5173`**

---

## 5. Web Dashboard Feature Tour

1. **Dashboard View**: Overview metrics of indexed documents, analysis count, C++ core status, and syllabus coverage overview.
2. **Ingestion & Upload View**: Drag-and-drop or paste document text, configure module checkboxes (Exact, N-gram, Jaccard, Semantic, Academic Search, AI Classifier, Stylometrics), adjust sensitivity threshold sliders, and choose worker thread count.
3. **Live Progress View**: Observes actual processing stages (`EXTRACTION` $\rightarrow$ `PREPROCESSING` $\rightarrow$ `LOCAL_MATCHING` $\rightarrow$ `ACADEMIC_SEARCH` $\rightarrow$ `AI_ANALYSIS` $\rightarrow$ `REPORT_GENERATION`).
4. **Analysis Results View**:
   - High-contrast similarity cards (Overall, Exact, N-Gram, Jaccard, Semantic).
   - Side-by-side passage comparison inspector showing submitted passages alongside reference passages with matched token chips.
   - Filter matches by algorithm type (`EXACT_MATCH`, `NGRAM_MATCH`, `JACCARD_MATCH`, `SEMANTIC_MATCH`).
   - Discovered academic publications table with DOIs and links.
   - AI-writing classification probability bar and explanation.
   - Explainable stylometric radar: Type-Token Ratio, burstiness, and Shannon entropy.
   - Buttons to open the standalone HTML report or download the JSON report.
5. **Reference Library View**: Search, view, add, delete, or rebuild indexed documents.
6. **Reports Archive**: Historical repository of all completed analysis audits.

---

## 6. Academic Integrity & Ethical Disclaimer

1. **Similarity $\neq$ Plagiarism**: A high similarity score reflects textual, structural, or lexical overlap. Standard definitions, mathematical formulas, software licenses, idioms, and citations generate legitimate similarity.
2. **AI Classification Disclaimer**: Pretrained AI text classifiers are probabilistic statistical models. They cannot definitively determine human or AI authorship and should only be used as experimental indicators accompanied by human academic review.
