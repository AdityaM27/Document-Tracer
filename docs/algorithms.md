# Document Tracer: Algorithmic Formulations & Complexity Analysis

This document details the mathematical models, algorithmic procedures, and asymptotic complexity of the similarity strategies and text-processing components in Document Tracer.

---

## 1. Exact Phrase Matching

### Formulation
Exact phrase matching identifies contiguous substrings of $K$ or more identical tokens between query document $Q$ and reference document $R$.

Let $T_Q = [t_1, t_2, \dots, t_n]$ and $T_R = [r_1, r_2, \dots, r_m]$ be normalized token sequences. An exact phrase match of length $k \ge K$ occurs at offsets $(i, j)$ if:

$$t_{i+s} = r_{j+s} \quad \forall s \in [0, k-1]$$

### Overlap Deduplication
To avoid redundantly reporting sub-windows of an already matched passage, a character coverage interval array $C_Q[0 \dots |Q|-1]$ is maintained:

$$C_Q[x] = \text{true} \quad \forall x \in [\text{startChar}, \text{endChar}]$$

Candidate windows whose midpoints are already marked as covered are pruned.

* **Time Complexity**: $O(n + m)$ using rolling hash or set lookups of $K$-tuples.
* **Space Complexity**: $O(m)$ for reference $K$-tuple hash sets.

---

## 2. N-Gram Multi-Set Containment

### Formulation
Let $\text{ngrams}(D, n)$ represent the multiset of sliding $n$-token sequences generated from document $D$:

$$\text{ngrams}(D, n) = \{ (t_i, t_{i+1}, \dots, t_{i+n-1}) \mid 1 \le i \le |D|-n+1 \}$$

The **containment score** of query passage $P_Q$ within reference passage $P_R$ is:

$$\text{Containment}(P_Q, P_R) = \frac{|\text{ngrams}(P_Q, n) \cap \text{ngrams}(P_R, n)|}{|\text{ngrams}(P_Q, n)|}$$

* **Default parameter**: $n = 3$ (trigrams).
* **Properties**: Robust against minor word reordering, insertion of punctuation, or slight paraphrasing.
* **Time Complexity**: $O(|P_Q| + |P_R|)$ via hash set construction.

---

## 3. Jaccard Set Similarity

### Formulation
Jaccard similarity measures the lexical overlap between the distinct vocabulary sets $V_A$ and $V_B$ of two text segments:

$$J(A, B) = \frac{|V_A \cap V_B|}{|V_A \cup V_B|} = \frac{|V_A \cap V_B|}{|V_A| + |V_B| - |V_A \cap V_B|}$$

### Division-by-Zero Safety
When both sets are empty ($V_A = \emptyset \land V_B = \emptyset$):

$$J(\emptyset, \emptyset) \equiv 0.0$$

### Scientific Interpretation
Jaccard similarity strictly measures shared lexical types relative to union size. It does not reflect semantic meaning or intent, and a low Jaccard score does not preclude semantic paraphrasing.

* **Time Complexity**: $O(\min(|V_A|, |V_B|))$ by iterating through the smaller set and checking membership in the larger hash set.

---

## 4. Semantic Paraphrase Detection (Cosine Similarity)

### Formulation
Given vector embeddings $\mathbf{u}, \mathbf{v} \in \mathbb{R}^d$ corresponding to text passages $P_Q$ and $P_R$:

$$\text{cosine\_similarity}(\mathbf{u}, \mathbf{v}) = \frac{\mathbf{u} \cdot \mathbf{v}}{\|\mathbf{u}\|_2 \|\mathbf{v}\|_2} = \frac{\sum_{i=1}^d u_i v_i}{\sqrt{\sum_{i=1}^d u_i^2} \sqrt{\sum_{i=1}^d v_i^2}}$$

### Zero-Norm Boundary Protection
If $\|\mathbf{u}\|_2 \le 10^{-9}$ or $\|\mathbf{v}\|_2 \le 10^{-9}$, the function safely evaluates to $0.0$:

$$\text{cosine\_similarity}(\mathbf{u}, \mathbf{0}) \equiv 0.0$$

### Local Fallback Embedding
When external model weights are offline, a feature-hashing character n-gram projector generates fixed-dimensional unit vectors ($d = 128$) using the Fowler–Noll–Vo (FNV-1a) hash function:

$$\text{hash}(s) = \left( \prod_{c \in s} (\text{hash} \oplus c) \cdot \text{PRIME} \right) \bmod d$$

* **Time Complexity**: $O(d)$ for vector dot product and norm calculation.

---

## 5. Explainable Stylometrics & Writing Pattern Analysis

### Vocabulary Diversity (Type-Token Ratio)
Given total word count $N$ and unique vocabulary count $V$:

$$\text{TTR} = \frac{V}{N}, \qquad \text{Root TTR (Guiraud's Index)} = \frac{V}{\sqrt{N}}$$

### Sentence Length Burstiness
Given the mean sentence length $\mu$ and sample standard deviation $\sigma$ across all sentences:

$$\text{Burstiness} = \frac{\sigma - \mu}{\sigma + \mu}$$

* **Burstiness $> 0$**: Highly variable sentence structures (typical in human conversational and academic writing).
* **Burstiness $< 0$**: Uniform sentence lengths and pacing (frequently observed in synthetic text).

### Shannon Token Entropy
Measures the information unpredictability of the word distribution:

$$H(X) = -\sum_{i=1}^{V} p(w_i) \log_2 p(w_i)$$

Where $p(w_i) = \frac{\text{count}(w_i)}{N}$.

---

## 6. Algorithmic Complexity Summary

| Component | Time Complexity | Auxiliary Space |
|---|---|---|
| **Tokenizer** | $O(N)$ characters | $O(N)$ |
| **Sentence Segmentation** | $O(M)$ tokens | $O(M)$ |
| **Exact Match** | $O(M_1 + M_2)$ | $O(M_2)$ |
| **N-Gram Similarity** | $O(M_1 + M_2)$ | $O(M_1 + M_2)$ |
| **Jaccard Similarity** | $O(V_1 + V_2)$ | $O(V_1 + V_2)$ |
| **Cosine Similarity** | $O(d)$ | $O(1)$ |
| **Inverted Index Query** | $O(\text{matches} \log \text{matches})$ | $O(\text{matches})$ |
| **Writing Pattern Analysis** | $O(M)$ tokens | $O(V)$ |
| **ThreadPool Dispatch** | $O(T_{\text{task}} / K_{\text{threads}})$ | $O(K_{\text{threads}})$ |
