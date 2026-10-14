# Document Tracer: Formal Validation Plan

**Project**: Document Tracer Document Intelligence System  
**Level**: B.Tech III Semester OOP C++ Lab Project  
**Author**: Quality Assurance & Verification Engineering  
**Version**: 1.0.0  

---

## 1. Validation Objectives

The objective of this validation plan is to establish a rigorous, reproducible, and verifiable testing methodology for Document Tracer. The system must be tested against:
1. Controlled corpus scenarios with known mathematical properties.
2. Boundary conditions, edge cases, and malformed inputs.
3. Concurrency stress tests verifying thread safety under load.
4. AI microservice integrity verifying genuine neural weights vs. fallback.
5. End-to-end task polling and asynchronous state synchronization.

---

## 2. Mathematical Formulations Under Test

### 2.1 Exact Match Metric
Evaluates contiguous token substrings of length $L \ge k$ (default $k=6$ words or 30 characters).
$$\text{Score}_{\text{exact}} = \frac{\sum_{\text{spans}} \text{length}(\text{span})}{\text{Total Document Length}}$$

### 2.2 N-Gram Phrase Containment
Evaluates overlapping sliding windows of $n$ tokens (default $n=3$ trigrams).
$$\text{Score}_{\text{ngram}}(Q, R) = \frac{|N_n(Q) \cap N_n(R)|}{|N_n(Q)|}$$
where $N_n(D)$ is the multiset of $n$-grams extracted from document $D$.

### 2.3 Jaccard Set Similarity
Evaluates bag-of-words token sets independent of order.
$$J(A, B) = \frac{|A \cap B|}{|A \cup B|} = \frac{|A \cap B|}{|A| + |B| - |A \cap B|}$$
*Edge case constraint*: If $|A \cup B| = 0$, $J(A, B) = 0.0$ (division by zero avoided).

### 2.4 Semantic Cosine Similarity
Evaluates 128-dimensional hashed feature proxy vectors $\mathbf{u}, \mathbf{v}$ generated via character trigram and term hashing:
$$\cos(\theta) = \frac{\mathbf{u} \cdot \mathbf{v}}{\|\mathbf{u}\|_2 \|\mathbf{v}\|_2} = \frac{\sum_{i=1}^{d} u_i v_i}{\sqrt{\sum_{i=1}^{d} u_i^2} \sqrt{\sum_{i=1}^{d} v_i^2}}$$
*Edge case constraint*: If $\|\mathbf{u}\| = 0$ or $\|\mathbf{v}\| = 0$, $\cos(\theta) = 0.0$.

### 2.5 Aggregated Document Similarity Coverage
To prevent double-counting overlapping passages matched by multiple strategies, the engine projects all matched character intervals $[s_i, e_i)$ onto a boolean coverage bitmask array of length $N$:
$$\text{Covered}(c) = \bigvee_{m \in M} (c \ge s_m \land c < e_m)$$
$$\text{Overall Similarity} = \frac{\sum_{c=0}^{N-1} \mathbb{I}(\text{Covered}(c))}{N}$$

### 2.6 Stylometric Metrics
- **Type-Token Ratio (TTR)**: $\text{TTR} = \frac{V}{N}$ (unique words $V$ / total words $N$).
- **Root TTR (Guiraud's Index)**: $\text{RTTR} = \frac{V}{\sqrt{N}}$ (length-independent lexical richness).
- **Sentence Burstiness**: $B = \frac{\sigma_{\text{len}} - \mu_{\text{len}}}{\sigma_{\text{len}} + \mu_{\text{len}}}$ bounded in $[-1, 1]$.
- **Shannon Token Entropy**: $H = -\sum_{i=1}^V p(w_i) \log_2 p(w_i)$ (information density in bits).

---

## 3. Controlled Corpus Test Scenarios

| Scenario ID | Test Document | Target Reference | Hypothesis & Expected Thresholds | Rationale |
|---|---|---|---|---|
| **SC-01: Verbatim Identical** | `query_identical.txt` | `oop_cpp.txt` | Overall $\ge 85\%$, Exact $\ge 85\%$, Jaccard $\ge 90\%$ | Complete duplication should trigger maximal scores across all metrics. |
| **SC-02: Punctuation & Case** | `query_punctuation_case.txt` | `data_structures.txt` | Jaccard $\ge 70\%$, Tokenizer normalization verified | Case changes (`Class` vs `class`) and punctuation must not artificially depress similarity. |
| **SC-03: Partial Overlap** | `query_partial_overlap.txt` | `oop_cpp.txt` | Overall $\in [15\%, 85\%]$, Exact $> 0\%$ | A single copied paragraph embedded in original text must be bounded proportionally. |
| **SC-04: Paraphrased Text** | `query_paraphrased.txt` | `operating_systems.txt` | Exact $< 40\%$, Semantic $\ge 35\%$ or Jaccard $\ge 15\%$ | Paraphrasing alters syntax and exact wording; semantic metric should detect conceptual alignment. |
| **SC-05: Unrelated Control** | `query_unrelated.txt` | `oop_cpp.txt` | Overall $\le 15\%$, Exact $\le 5\%$ | Completely disjoint topic (marine biology vs C++) must yield near-zero overlap. |
| **SC-06: Empty File Edge Case** | `query_empty.txt` | Any | Throws `EmptyDocumentException` (Code 1003) | Zero-byte or pure whitespace inputs must be rejected gracefully with clear user error. |
| **SC-07: Micro-Document** | 10 words text | Any | Processed without crash or underflow | Small inputs must not trigger buffer underflows or NaN values. |
| **SC-08: Highly Repetitive** | Single word repeated 100x | Any | Processed without infinite loop, Entropy $\approx 0$ bits | Low-entropy pathological text should yield TTR $\approx 0.01$ and Entropy $\approx 0$. |

---

## 4. AI Microservice Verification Protocol

1. **Environment Check**: Ensure `.venv` has `torch`, `transformers`, `huggingface_hub` installed.
2. **Model Weight Verification**: Confirm `roberta-base-openai-detector` weights exist in Hugging Face hub cache.
3. **Inference Audit**:
   - Query human academic passage: Verify `human_score > 0.80`, `mode: "huggingface"`.
   - Query formal AI generated passage: Verify `ai_score > 0.50`, `mode: "huggingface"`.
4. **Fallback Audit**:
   - Temporarily disable microservice: Confirm C++ backend handles connection timeout without crash.
   - Verify UI displays `"Service Offline"` or `"Calibrated Statistical Fallback"`.

---

## 5. Concurrency & Stress Verification Protocol

- **Thread Pool Scaling**: Execute 50 tasks across 4 worker threads.
- **Race Condition Testing**: Verify atomic counter increments and mutex protection on shared match collection.
- **Barrier Synchronization**: Verify `waitAll()` blocks until all worker tasks have completed execution.
