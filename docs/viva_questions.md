# Document Tracer: Comprehensive Laboratory Project Viva-Voce Guide

## Course: B.Tech III Semester &bull; Object Oriented Programming Through C++

---

## Unit I: C++ Fundamentals, Memory & STL

### Q1. What is RAII, and where is it demonstrated in Document Tracer?
**Answer**:
RAII (Resource Acquisition Is Initialization) is a C++ idiom where resource allocation (memory, mutex locks, file handles, sockets) is bound to object lifetime. When an object is created on the stack, its constructor acquires the resource; when it goes out of scope, its destructor deterministically releases it.
*In Document Tracer*:
- `std::unique_lock<std::mutex>` in `ThreadPool::enqueue()` and `ThreadPool::workerLoop()` automatically unlocks mutexes even if an exception occurs.
- `std::ifstream` and `std::ofstream` in `DocumentLoader` and `ReportGenerator` automatically close file descriptors upon function exit.
- `ThreadPool::~ThreadPool()` stops worker threads and joins them to prevent zombie threads.

### Q2. Why prefer `std::unique_ptr` over raw pointers (`new`/`delete`)?
**Answer**:
`std::unique_ptr` provides exclusive ownership semantics with zero runtime overhead. When a `unique_ptr` goes out of scope, it automatically invokes `delete` on the managed object, completely eliminating memory leaks and dangling pointers. In Document Tracer, polymorphic similarity strategies in `AnalysisService` are stored as `std::vector<std::unique_ptr<SimilarityStrategy>>`.

### Q3. Explain the difference between pass-by-value, pass-by-reference, and pass-by-const-reference.
**Answer**:
- **Pass-by-value** (`void f(Document d)`): Copies the entire object, causing heap allocations for internal vectors/strings.
- **Pass-by-reference** (`void f(Document& d)`): Passes an alias to the caller's object without copying; mutations affect the caller.
- **Pass-by-const-reference** (`void f(const Document& d)`): Passes an alias with zero copy overhead while enforcing read-only safety. In Document Tracer, all calculation methods (e.g. `calculateSimilarity(const Document& docA, const Document& docB)`) use const references.

---

## Unit II: Object Oriented Programming & Encapsulation

### Q4. What is encapsulation, and how does the `Document` class enforce it?
**Answer**:
Encapsulation bundles data members and the functions that manipulate them inside a single class, hiding internal implementation details behind access specifiers (`private` / `public`).
In `Document`:
- Member variables (`id_`, `originalText_`, `tokens_`, `sentences_`) are `private`.
- Clients access them through `const` getters (`getOriginalText() const noexcept`).
- Mutators like `setTokens()` update dependent derived state (`wordCount_ = tokens.size()`) and manage `status_`.

### Q5. What is the significance of the `const` keyword on member functions?
**Answer**:
A `const` member function promises not to modify any non-mutable member variables of the calling instance (`this`). It allows the function to be invoked on `const` objects or references (`const Document&`). For example, `Document::extractSnippet(...) const` can be called safely without mutating document text.

### Q6. Why are virtual destructors mandatory when using inheritance?
**Answer**:
If a derived class object is deleted through a pointer to a base class (`delete basePtr`) and the base class lacks a virtual destructor, the compiler only invokes the base class destructor (static dispatch). The derived class destructor is skipped, leaking any resources allocated by the derived class. Declaring `virtual ~SimilarityStrategy() = default;` ensures dynamic dispatch invokes destructors in bottom-up order.

---

## Unit III: Inheritance & Runtime Polymorphism

### Q7. What is an Abstract Base Class (ABC) and a Pure Virtual Function?
**Answer**:
An Abstract Base Class contains at least one pure virtual function (syntax: `virtual return_type function_name(...) = 0;`). It defines an interface and cannot be directly instantiated. Derived classes must override all pure virtual functions to become concrete classes.
In Document Tracer: `SimilarityStrategy` is an ABC with pure virtual functions `calculateSimilarity()` and `findPassageMatches()`.

### Q8. How does the Strategy Design Pattern work in Document Tracer?
**Answer**:
The Strategy Pattern defines a family of interchangeable algorithms, encapsulates each one, and makes them interchangeable at runtime.
`AnalysisService` holds a collection of strategies:
```cpp
std::vector<std::unique_ptr<SimilarityStrategy>> strategies_;
strategies_.push_back(std::make_unique<ExactMatchStrategy>());
strategies_.push_back(std::make_unique<NGramSimilarity>());
strategies_.push_back(std::make_unique<JaccardSimilarity>());
strategies_.push_back(std::make_unique<SemanticSimilarity>());
```
Each algorithm is invoked polymorphically without the caller needing to know internal algorithm details.

### Q9. What is the vtable (Virtual Method Table) and vptr?
**Answer**:
When a class has virtual functions, the compiler generates a static array of function pointers called the **vtable**. Each instance of that class contains an invisible pointer (**vptr**) pointing to its class's vtable. At runtime, calling `strategy->calculateSimilarity(...)` resolves to:
$$\text{vptr} \rightarrow \text{vtable}[\text{slot}] \rightarrow \text{DerivedFunctionAddress}$$
This enables dynamic binding at the cost of a single pointer dereference.

---

## Unit IV: C++ I/O Streams & Persistence

### Q10. How does DocuTrace handle binary file streams versus text streams?
**Answer**:
When reading documents (`DocumentLoader`), files are opened with `std::ios::binary` to preserve exact byte counts and line endings across Windows (`\r\n`) and Linux (`\n`), preventing character offset misalignments. `std::istreambuf_iterator<char>` is used for fast bulk byte ingestion.

### Q11. How is index persistence implemented in `ReferenceIndex`?
**Answer**:
`ReferenceIndex` serializes the document corpus and inverted indices to `data/references/index_cache.json`. When the application restarts, `loadFromDisk()` checks the index version (`CURRENT_INDEX_VERSION = 2`). If compatible, it populates in-memory hash tables in $O(N)$ time without re-extracting raw files from scratch.

---

## Unit V: Exceptions, Concurrency & Multithreading

### Q12. Explain the custom exception hierarchy in Document Tracer.
**Answer**:
All custom exceptions inherit from `DocuTraceException`, which inherits from `std::exception`:
- `DocuTraceException` (base: holds error message, context, and error code)
  - `DocumentExtractionException` (code 1001)
  - `InvalidFormatException` (code 1002)
  - `EmptyDocumentException` (code 1003)
  - `FileSizeLimitException` (code 1004)
  - `IndexException` (code 2001)
  - `StrategyException` (code 3001)
  - `NetworkException` (code 4001)
  - `AIAnalysisException` (code 5001)
  - `ReportGenerationException` (code 6001)

### Q13. Describe the Producer-Consumer pattern in `ThreadPool`.
**Answer**:
1. **Producer**: `AnalysisService` breaks document comparisons into tasks and calls `ThreadPool::enqueue()`.
2. **Buffer / Queue**: Tasks are stored in `std::queue<std::function<void()>> tasks_`, guarded by `std::mutex queueMutex_`.
3. **Consumers**: A fixed pool of `std::thread` worker threads wait on `std::condition_variable cvTask_`. When signaled, a worker pops a task, increments `activeTasks_`, executes the task, decrements `activeTasks_`, and signals `cvFinished_` when idle.

### Q14. What is a Race Condition, and how does DocuTrace prevent them?
**Answer**:
A race condition occurs when multiple threads concurrently read and write shared data without synchronization, causing nondeterministic bugs.
In Document Tracer:
- `tasks_` queue access is protected by `queueMutex_`.
- Worker task counting uses `std::atomic<size_t> activeTasks_`.
- Assembling match results from concurrent comparisons is protected by `std::mutex matchCollectionMutex`.

### Q15. How does DocuTrace prevent deadlocks?
**Answer**:
1. **Lock ordering**: Threads never hold multiple conflicting mutexes simultaneously.
2. **RAII Locks**: `std::unique_lock` and `std::lock_guard` guarantee mutex release even when exceptions are thrown.
3. **Condition Variable predicates**: Spurious wakeups are prevented using lambda predicates:
   `cvTask_.wait(lock, [this]() { return stop_.load() || !tasks_.empty(); });`

---

## Domain & NLP Concepts

### Q16. What is the mathematical definition of Jaccard similarity?
**Answer**:
$$J(A, B) = \frac{|A \cap B|}{|A \cup B|}$$
In Document Tracer, it measures the ratio of shared vocabulary tokens over total combined unique vocabulary tokens. It safely handles empty sets by returning $0.0$.

### Q17. How does N-Gram similarity differ from Exact Matching?
**Answer**:
- **Exact matching** looks for identical, unbroken sequences of sentences or phrases.
- **N-Gram similarity** computes containment of overlapping $n$-word tuples (default trigrams). It is robust against synonym substitution, punctuation variations, and minor sentence restructuring.

### Q18. How does DocuTrace measure explainable writing patterns?
**Answer**:
`WritingPatternAnalyzer` calculates:
1. **Type-Token Ratio (TTR)**: $V / N$ (vocabulary richness).
2. **Sentence Length Burstiness**: $B = \frac{\sigma - \mu}{\sigma + \mu}$ (variation in sentence cadence).
3. **Shannon Token Entropy**: $-\sum p(w) \log_2 p(w)$ (lexical unpredictability).
4. **Transition Phrase Frequencies**: Counts discourse markers ("moreover", "furthermore", "in conclusion").

### Q19. Does a high similarity score prove plagiarism?
**Answer**:
No. Similarity only measures textual, n-gram, or semantic overlap. Standard technical definitions, software licenses, citations, idioms, and common domain terminology naturally produce similarity without any intellectual dishonesty. DocuTrace report disclaimers explicitly emphasize this distinction.

### Q20. Can AI-writing classifiers definitively determine authorship?
**Answer**:
No. AI text classifiers are statistical and probabilistic models that evaluate pattern regularity. They can produce false positives on human non-native English speakers or highly structured technical writing. DocuTrace presents classifier outputs as probabilistic indicators accompanied by clear scientific disclaimers.

---

## Advanced Architecture & C++ Nuances

### Q21. What is the difference between shallow copy and deep copy, and how does `Document` handle copying?
**Answer**:
- **Shallow copy**: Copies primitive fields and pointer addresses without duplicating the heap memory being pointed to, leading to double-free bugs when destructors run.
- **Deep copy**: Allocates new memory on the heap and duplicates the actual underlying contents.
- *In Document Tracer*: `Document` uses modern standard library types (`std::string`, `std::vector<Token>`, `std::vector<Passage>`). Standard containers manage their own memory and implement deep copy semantics out of the box (Rule of Zero).

### Q22. What is the difference between `std::mutex` and `std::atomic`? Why use `std::atomic<size_t>` for `activeTasks_`?
**Answer**:
- `std::mutex` is a kernel/OS synchronization primitive that puts waiting threads to sleep if the lock is held, which incurs context switch overhead.
- `std::atomic` uses hardware-level atomic CPU instructions (such as `LOCK CMPXCHG` or `LOCK XADD` on x86_64) without blocking or context switches.
- In `ThreadPool`, `activeTasks_` is updated frequently across worker threads (`fetch_add`, `fetch_sub`). Using `std::atomic<size_t>` provides lock-free, thread-safe counter modifications with near-zero latency.

### Q23. What is a "spurious wakeup" in condition variables, and how does DocuTrace prevent it?
**Answer**:
A condition variable may unblock a waiting thread even if no other thread signaled it (due to OS kernel interrupts or thread scheduling). If the thread proceeds assuming the condition is met, it may attempt to pop from an empty queue and crash.
DocuTrace prevents this by always providing a boolean predicate lambda to `cvTask_.wait()`:
```cpp
cvTask_.wait(lock, [this]() { return stop_.load() || !tasks_.empty(); });
```
The thread re-evaluates the predicate and only wakes up if `stop_` is true or `tasks_` is non-empty.

### Q24. What is the difference between static (early) binding and dynamic (late) binding?
**Answer**:
- **Static binding**: Function address is resolved by the compiler and linker at build time based on variable types (e.g., standard non-virtual member functions, overloaded functions, templates). Fast and inlineable.
- **Dynamic binding**: Function address is resolved at runtime through virtual table dispatch (`vptr` -> `vtable`) based on the actual runtime object type. This enables runtime polymorphism in `SimilarityStrategy`.

### Q25. Why use a lightweight Python microservice for AI classification instead of embedding LibTorch directly into C++?
**Answer**:
1. **Separation of Concerns**: The core C++ document analysis engine remains completely self-contained, lightweight, fast to compile, and 100% usable offline without multi-gigabyte neural dependencies.
2. **Ecosystem & Deployment**: Hugging Face `transformers` and tokenizers are natively maintained in Python with high-level pipeline APIs (`roberta-base-openai-detector`).
3. **Resilience & Fault Tolerance**: If the Python service runs out of memory, is killed, or encounters a neural inference failure, the C++ backend traps the socket error and continues delivering accurate textual and stylometric analysis without crashing.

