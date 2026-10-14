# Document Tracer: C++ Syllabus Mapping Guide

## Course: B.Tech III Semester &bull; Object Oriented Programming Through C++

This document maps all theoretical and laboratory concepts across the five prescribed syllabus units directly to specific source code files and implementations within **Document Tracer**.

---

## Unit I: C++ Fundamentals, Memory & Algorithms

| Syllabus Topic | Concept Description | Implemented In Source File | Key Code Elements |
|---|---|---|---|
| **Functions & Parameter Passing** | Pass-by-value, pass-by-reference, pass-by-const-reference | [`include/SimilarityStrategy.h`](file:///c:/C++/CBP/include/SimilarityStrategy.h)<br/>[`src/TextProcessor.cpp`](file:///c:/C++/CBP/src/TextProcessor.cpp) | `calculateSimilarity(const Document& docA, const Document& docB) const`<br/>`void processDocument(Document& doc)` |
| **Strings & Vectors** | Dynamic text manipulation and sequence storage | [`include/Document.h`](file:///c:/C++/CBP/include/Document.h)<br/>[`src/Tokenizer.cpp`](file:///c:/C++/CBP/src/Tokenizer.cpp) | `std::string originalText_`, `std::vector<Token> tokens_`, `std::vector<Passage> sentences_` |
| **Smart Pointers & Dynamic Memory** | Modern memory ownership without leaks | [`include/AnalysisService.h`](file:///c:/C++/CBP/include/AnalysisService.h)<br/>[`include/ThreadPool.h`](file:///c:/C++/CBP/include/ThreadPool.h) | `std::vector<std::unique_ptr<SimilarityStrategy>> strategies_`<br/>`std::shared_ptr<ReferenceIndex> referenceIndex_`<br/>`std::make_shared<std::packaged_task<...>>` |
| **RAII (Resource Management)** | Automatic acquisition and deterministic cleanup | [`src/ThreadPool.cpp`](file:///c:/C++/CBP/src/ThreadPool.cpp)<br/>[`src/DocumentLoader.cpp`](file:///c:/C++/CBP/src/DocumentLoader.cpp) | `std::unique_lock<std::mutex> lock(queueMutex_)`<br/>`std::ifstream` scope-bound file closure |
| **STL Containers & Algorithms** | Set operations, maps, hash tables, sorting, iteration | [`src/ReferenceIndex.cpp`](file:///c:/C++/CBP/src/ReferenceIndex.cpp)<br/>[`src/WritingPatternAnalyzer.cpp`](file:///c:/C++/CBP/src/WritingPatternAnalyzer.cpp) | `std::unordered_map<std::string, std::unordered_set<std::string>> invertedIndex_`<br/>`std::accumulate()`, `std::sort()`, `std::transform()` |

---

## Unit II: Object-Oriented Programming & Encapsulation

| Syllabus Topic | Concept Description | Implemented In Source File | Key Code Elements |
|---|---|---|---|
| **Classes & Objects** | User-defined data types encapsulating state and operations | [`include/Document.h`](file:///c:/C++/CBP/include/Document.h)<br/>[`src/Document.cpp`](file:///c:/C++/CBP/src/Document.cpp) | `class Document`, `class Tokenizer`, `class ReferenceIndex` |
| **Constructors & Destructors** | Lifecycle management, default values, cleanup | [`src/Document.cpp`](file:///c:/C++/CBP/src/Document.cpp)<br/>[`src/ThreadPool.cpp`](file:///c:/C++/CBP/src/ThreadPool.cpp) | `Document::Document(const std::string& id, ...)`<br/>`ThreadPool::~ThreadPool()` joins worker threads |
| **Encapsulation & Access Specifiers** | Data hiding (`private` state, `public` interfaces) | [`include/Document.h`](file:///c:/C++/CBP/include/Document.h) | `private:` member variables (`id_`, `originalText_`, `tokens_`)<br/>`public:` accessors (`getOriginalText()`, `getId()`) |
| **Const-Correctness** | Enforcing immutable guarantees on member functions | [`include/Document.h`](file:///c:/C++/CBP/include/Document.h)<br/>[`include/SimilarityStrategy.h`](file:///c:/C++/CBP/include/SimilarityStrategy.h) | `const std::string& getFilename() const noexcept`<br/>`double calculateSimilarity(...) const = 0` |
| **Static Methods & Members** | Class-level utilities not bound to specific instances | [`include/Tokenizer.h`](file:///c:/C++/CBP/include/Tokenizer.h)<br/>[`include/JaccardSimilarity.h`](file:///c:/C++/CBP/include/JaccardSimilarity.h) | `static bool isWhitespace(char c)`<br/>`static double computeJaccard(const std::unordered_set<...>&, ...)` |

---

## Unit III: Inheritance & Runtime Polymorphism

| Syllabus Topic | Concept Description | Implemented In Source File | Key Code Elements |
|---|---|---|---|
| **Abstract Base Classes (ABC)** | Interface definition requiring derived overrides | [`include/SimilarityStrategy.h`](file:///c:/C++/CBP/include/SimilarityStrategy.h)<br/>[`include/SourceProvider.h`](file:///c:/C++/CBP/include/SourceProvider.h) | `class SimilarityStrategy { virtual ~SimilarityStrategy() = default; ... };`<br/>`class SourceProvider { ... };` |
| **Pure Virtual Functions** | Dynamic dispatch contracts | [`include/SimilarityStrategy.h`](file:///c:/C++/CBP/include/SimilarityStrategy.h) | `virtual double calculateSimilarity(...) const = 0;`<br/>`virtual std::vector<MatchRecord> findPassageMatches(...) const = 0;` |
| **Inheritance Hierarchies** | Derived classes inheriting and specializing base behaviors | [`include/ExactMatchStrategy.h`](file:///c:/C++/CBP/include/ExactMatchStrategy.h)<br/>[`include/NGramSimilarity.h`](file:///c:/C++/CBP/include/NGramSimilarity.h)<br/>[`include/JaccardSimilarity.h`](file:///c:/C++/CBP/include/JaccardSimilarity.h)<br/>[`include/SemanticSimilarity.h`](file:///c:/C++/CBP/include/SemanticSimilarity.h) | `class ExactMatchStrategy : public SimilarityStrategy`<br/>`class NGramSimilarity : public SimilarityStrategy`<br/>`class JaccardSimilarity : public SimilarityStrategy`<br/>`class SemanticSimilarity : public SimilarityStrategy` |
| **Runtime Polymorphism** | Calling derived implementations through base pointers | [`src/AnalysisService.cpp`](file:///c:/C++/CBP/src/AnalysisService.cpp) | `std::vector<std::unique_ptr<SimilarityStrategy>> strategies_`<br/>Iterating and executing `strategy->findPassageMatches(...)` |
| **Virtual Destructors** | Correct object destruction order in inheritance trees | [`include/SimilarityStrategy.h`](file:///c:/C++/CBP/include/SimilarityStrategy.h)<br/>[`include/SourceProvider.h`](file:///c:/C++/CBP/include/SourceProvider.h) | `virtual ~SimilarityStrategy() = default;`<br/>`virtual ~SourceProvider() = default;` |

---

## Unit IV: C++ File I/O Streams & Serialization

| Syllabus Topic | Concept Description | Implemented In Source File | Key Code Elements |
|---|---|---|---|
| **File Input Streams** | Bulk binary reading and text parsing from disk | [`src/DocumentLoader.cpp`](file:///c:/C++/CBP/src/DocumentLoader.cpp) | `std::ifstream file(filepath, std::ios::binary)`<br/>`std::istreambuf_iterator<char>` reading |
| **File Output Streams** | Formatted report generation and index writing | [`src/ReportGenerator.cpp`](file:///c:/C++/CBP/src/ReportGenerator.cpp)<br/>[`src/ReferenceIndex.cpp`](file:///c:/C++/CBP/src/ReferenceIndex.cpp) | `std::ofstream out(fullPath)` writing HTML and JSON |
| **String Streams** | In-memory string formatting and parsing | [`src/ReportGenerator.cpp`](file:///c:/C++/CBP/src/ReportGenerator.cpp)<br/>[`src/AnalysisService.cpp`](file:///c:/C++/CBP/src/AnalysisService.cpp) | `std::ostringstream oss`<br/>`std::stringstream timeSs; timeSs << std::put_time(...)` |
| **Structured Serialization** | Persisting object state to structured files | [`src/ReportGenerator.cpp`](file:///c:/C++/CBP/src/ReportGenerator.cpp)<br/>[`include/Models.h`](file:///c:/C++/CBP/include/Models.h) | `to_json()`, `from_json()` implementing JSON serialization |

---

## Unit V: Exceptions, Concurrency & Multithreading

| Syllabus Topic | Concept Description | Implemented In Source File | Key Code Elements |
|---|---|---|---|
| **Custom Exception Hierarchy** | Structured error handling inheriting `std::exception` | [`include/Exceptions.h`](file:///c:/C++/CBP/include/Exceptions.h) | `class DocuTraceException : public std::exception`<br/>`class DocumentExtractionException`<br/>`class EmptyDocumentException`<br/>`class InvalidFormatException`<br/>`class FileSizeLimitException` |
| **Exception Propagation** | Throwing and catching typed exceptions | [`src/DocumentLoader.cpp`](file:///c:/C++/CBP/src/DocumentLoader.cpp)<br/>[`src/main.cpp`](file:///c:/C++/CBP/src/main.cpp) | `throw EmptyDocumentException(filepath);`<br/>`catch (const DocuTraceException& e) { ... }` |
| **Worker Threads** | Native OS threads executing concurrent tasks | [`src/ThreadPool.cpp`](file:///c:/C++/CBP/src/ThreadPool.cpp) | `std::vector<std::thread> workers_`<br/>`workers_.emplace_back(&ThreadPool::workerLoop, this)` |
| **Mutex & Synchronization** | Mutual exclusion protecting shared memory | [`src/ThreadPool.cpp`](file:///c:/C++/CBP/src/ThreadPool.cpp)<br/>[`src/AnalysisService.cpp`](file:///c:/C++/CBP/src/AnalysisService.cpp) | `std::mutex queueMutex_`<br/>`std::unique_lock<std::mutex> lock(queueMutex_)`<br/>`std::lock_guard<std::mutex> lock(matchCollectionMutex)` |
| **Condition Variables** | Thread signaling and sleep-wait mechanics | [`src/ThreadPool.cpp`](file:///c:/C++/CBP/src/ThreadPool.cpp) | `std::condition_variable cvTask_`<br/>`cvTask_.wait(lock, [this]() { ... })`<br/>`cvFinished_.notify_all()` |
| **Atomic Operations** | Lock-free counters for thread metrics | [`include/ThreadPool.h`](file:///c:/C++/CBP/include/ThreadPool.h)<br/>[`src/ThreadPool.cpp`](file:///c:/C++/CBP/src/ThreadPool.cpp) | `std::atomic<bool> stop_{false}`<br/>`std::atomic<size_t> activeTasks_{0}` |
| **Producer-Consumer Pattern** | Asynchronous task queue dispatching to worker pool | [`include/ThreadPool.h`](file:///c:/C++/CBP/include/ThreadPool.h)<br/>[`src/ThreadPool.cpp`](file:///c:/C++/CBP/src/ThreadPool.cpp) | `std::queue<std::function<void()>> tasks_`<br/>`enqueue()` and `waitAll()` barrier synchronization |
