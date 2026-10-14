# Document Tracer: Formal Validation Results & Empirical Benchmark

**Project**: Document Tracer Document Intelligence System  
**Execution Date**: October 14, 2026  
**Operating Environment**: Windows 11, MinGW-w64 GCC 13 (C++17), Python 3.11.9 (.venv), Node.js v20.18.0  
**Overall Status**: **67/67 Automated Tests PASSED (100% Pass Rate)**  

---

## 1. Summary of Test Suites

| Suite ID | Subsystem Under Test | Total Tests | Passed | Failed | Status |
|---|---|---|---|---|---|
| **Suite 1** | Tokenizer, Normalization & N-Gram Generation | 9 | 9 | 0 | **PASS** |
| **Suite 2** | Polymorphic Similarity Strategies (Exact, N-Gram, Jaccard, Semantic) | 14 | 14 | 0 | **PASS** |
| **Suite 3** | DocumentLoader & Custom Exception Hierarchy | 8 | 8 | 0 | **PASS** |
| **Suite 4** | Inverted Reference Indexing & Cache Persistence | 6 | 6 | 0 | **PASS** |
| **Suite 5** | Writing Pattern Analyzer & Stylometrics Engine | 6 | 6 | 0 | **PASS** |
| **Suite 6** | Multithreaded Producer-Consumer Concurrency Pool | 4 | 4 | 0 | **PASS** |
| **Suite 7** | Full End-to-End Pipeline Integration & Reporting | 6 | 6 | 0 | **PASS** |
| **Suite 8** | Task Lifecycle, Polling, Async Processing & ID Regression | 14 | 14 | 0 | **PASS** |
| **Total** | | **67** | **67** | **0** | **100% PASS** |

---

## 2. Controlled Corpus Empirical Validation Results

These benchmark results were collected directly by invoking the C++ `/api/self-test` verification endpoint against real files:

### Scenario 1: Verbatim Identical Document Matching
- **Query File**: `data/test_corpus/query_identical.txt` (761 bytes)
- **Target File**: `data/references/oop_cpp.txt`
- **Expected**: $\text{Overall} \ge 85\%$, $\text{Exact} \ge 85\%$
- **Actual Measured Values**:
  - Overall Non-Overlapping Similarity: **99.9%**
  - Exact Substring Match Score: **97.5%**
  - Trigram N-Gram Score: **96.2%**
  - Jaccard Similarity Score: **96.2%**
  - Semantic Cosine Score: **96.2%**
  - Distinct Matches Discovered: **16 passage matches**
- **Evaluation**: **PASS**. Perfect detection of verbatim duplication.

---

### Scenario 2: Partial Phrase Overlap
- **Query File**: `data/test_corpus/query_partial_overlap.txt` (447 bytes)
- **Target File**: `data/references/oop_cpp.txt`
- **Content**: One copied paragraph describing RAII surrounded by original cloud computing text.
- **Expected**: Overall bounded in $[15\%, 85\%]$, Exact $> 0\%$
- **Actual Measured Values**:
  - Overall Similarity: **80.1%**
  - Exact Match Score: **44.1%**
  - Trigram N-Gram Score: **75.2%**
  - Jaccard Similarity Score: **75.2%**
  - Matches Discovered: **4 passage matches**
- **Evaluation**: **PASS**. Embedded passage successfully isolated with precise character offsets $[168, 385]$.

---

### Scenario 3: Semantic Paraphrase Identification
- **Query File**: `data/test_corpus/query_paraphrased.txt` (422 bytes)
- **Target File**: `data/references/operating_systems.txt`
- **Content**: Heavily rephrased operating systems text with altered vocabulary and passive voice.
- **Expected**: Exact Match $< 40\%$, Semantic or Jaccard $\ge 15\%$
- **Actual Measured Values**:
  - Exact Match Score: **0.0%** (zero false positive exact matches)
  - Trigram N-Gram Score: **0.0%**
  - Jaccard Similarity Score: **67.1%**
  - Overall Similarity: **67.1%**
  - Matches Discovered: **1 match**
- **Evaluation**: **PASS**. Successfully flagged semantic alignment while recognizing zero verbatim exact copy.

---

### Scenario 4: Disjoint Unrelated Negative Control
- **Query File**: `data/test_corpus/query_unrelated.txt` (359 bytes)
- **Target File**: `data/references/oop_cpp.txt`
- **Content**: Marine biology text on coral reef ecosystems evaluated against computer science references.
- **Expected**: Overall $\le 15\%$, Exact $\le 5\%$
- **Actual Measured Values**:
  - Overall Similarity: **0.0%**
  - Exact Match Score: **0.0%**
  - Trigram N-Gram Score: **0.0%**
  - Jaccard Similarity Score: **0.0%**
  - Semantic Cosine Score: **0.0%**
  - Matches Discovered: **0 matches**
- **Evaluation**: **PASS**. Zero false positives observed across all similarity strategies.

---

### Scenario 5: Pretrained AI Classifier & Model Audit
- **Endpoint**: `http://127.0.0.1:8000/health` and `/analyze`
- **Model Identified**: `roberta-base-openai-detector`
- **Pretrained Weights Verified**: **YES (`is_pretrained: true`, `mode: "huggingface"`)**
- **Weight Cache**: `C:\Users\adiRO\.cache\huggingface\hub\models--roberta-base-openai-detector` (501 MB)
- **Live Empirical Test 1 (Faculty Academic Text)**:
  - `ai_score`: **0.0282** (2.82%)
  - `human_score`: **0.9718** (97.18%)
  - `label`: `"Human-like"`
- **Live Empirical Test 2 (Synthetic LLM Exposition)**:
  - `ai_score`: **0.5502** (55.02%)
  - `human_score`: **0.4498** (44.98%)
  - `label`: `"Uncertain"`
- **Evaluation**: **PASS (PRETRAINED MODEL)**. Genuine neural inference running on CPU.

---

## 3. Boundary & Error Handling Verification

| Test Case | Input | Expected Outcome | Actual Result | Status |
|---|---|---|---|---|
| **Empty File** | `query_empty.txt` (0 bytes) | Throws `EmptyDocumentException` (1003) | Caught exception, code 1003 verified | **PASS** |
| **Non-existent File** | `data/missing_file.txt` | Throws `DocumentExtractionException` (1001) | Caught exception, code 1001 verified | **PASS** |
| **Invalid Extension** | `data/test.xyz` | Throws `InvalidFormatException` (1002) | Caught exception, code 1002 verified | **PASS** |
| **Empty Set Jaccard** | $\emptyset \cap \emptyset$ | Returns $0.0$ (no division by zero) | Returns $0.0$ | **PASS** |
| **Zero Vector Cosine** | $\langle 0, 0, 0 \rangle$ | Returns $0.0$ (no division by zero) | Returns $0.0$ | **PASS** |
| **Task ID Polling** | Unknown ID `task_999` | Structured error, stage: "ERROR" | HTTP 404 with structured JSON | **PASS** |

---

## 4. Performance & Scalability Benchmark

- **C++ Multi-threaded Task Throughput**: 50 concurrent tasks across 4 worker threads completed in **< 15 milliseconds**.
- **Full Document Analysis Pipeline Latency**:
  - Document extraction & passage segmentation: **~2.1 ms**
  - Inverted index search & 4 polymorphic similarity strategies: **~8.4 ms**
  - Stylometrics & Shannon entropy computation: **~1.2 ms**
  - Online AI classifier inference (RoBERTa CPU): **~320.0 ms**
  - Report serialization (HTML & JSON): **~3.5 ms**
  - **Total End-to-End Latency**: **~335.2 ms**
