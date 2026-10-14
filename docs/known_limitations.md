# Document Tracer: Known Limitations and Ethical Boundaries

This document provides a rigorous, transparent account of the architectural, statistical, and operational boundaries of the Document Tracer Document Similarity and AI-Text Analysis System.

---

## 1. Textual Similarity vs. Plagiarism

1. **Similarity Is Not Proof of Plagiarism:**
   - Document Tracer calculates mathematical and lexical overlap between texts using exact substring matching, character/word n-gram containment, and Jaccard intersection over union.
   - A high similarity percentage indicates textual overlap, which may be caused by:
     - Properly cited quotations.
     - Common idioms, formulas, and boilerplate terminology (e.g., standard legal disclaimers, academic paper structures, mathematical proofs).
     - Standard definitions in technical fields (e.g., "polymorphism in object-oriented programming").
   - Document Tracer reports **evidence and alignments**, but determination of academic dishonesty or copyright infringement requires human review and context evaluation.

2. **Passage Granularity Trade-offs:**
   - Rolling passage windows are partitioned into 50-word spans with 25-word overlaps. Short copied fragments (< 6 words) or heavily interleaved words can fall below threshold boundaries.

---

## 2. Pretrained AI-Text Classifier Limitations

1. **Probabilistic Classification, Not Authorship Proof:**
   - Pretrained AI detectors (such as `roberta-base-openai-detector`) produce a probabilistic score based on statistical patterns learned during training on specific generator corpora (e.g., GPT-2).
   - They **cannot definitively prove or disprove human authorship**.

2. **Known Failure Modes and Biases:**
   - **False Positives on Non-Native English:** Studies show AI classifiers disproportionately flag writing by non-native English speakers as AI-generated due to lower perplexity and more uniform grammatical choices.
   - **False Positives on Highly Formal or Formulaic Writing:** Technical specifications, legal prose, and structured laboratory reports naturally exhibit high regularity and lower burstiness.
   - **Evasion via Light Paraphrasing:** Light paraphrasing, prompt engineering, or inserting typos can drastically lower classifier confidence.
   - **Out-of-Distribution Degradation:** Models trained on older LLMs (e.g., GPT-2/3) exhibit degraded accuracy on newer reasoning models (e.g., Gemini, GPT-4, Claude).

3. **Fallback Operational Modes:**
   - If PyTorch or Hugging Face transformers are missing or offline, the microservice falls back to **Explainable Statistical Mode** (measuring token diversity, burstiness, and discourse markers).
   - Statistical proxies must **never be mislabeled as neural transformer outputs**.

---

## 3. Semantic Similarity Implementation Boundaries

1. **Feature-Hashing Embedding Proxy:**
   - The native C++ semantic similarity module utilizes a 128-dimensional feature-hashing character trigram and word vector with L2 normalization, followed by cosine similarity.
   - While this provides microsecond offline semantic approximations without requiring gigabyte-scale language model weights, it does not capture deep syntactic paraphrases or cross-lingual semantics that a dense transformer model (e.g., `all-MiniLM-L6-v2`) provides.

---

## 4. External Literature & Web Discovery Boundaries

1. **Scholarly API Limits:**
   - OpenAlex and Crossref discovery relies on public REST APIs. Networks without internet connectivity or experiencing rate limits rely on local disk cache (`data/cache/academic_cache.json`).
2. **Abstract-Level Text Availability:**
   - Open-access APIs primarily return titles, authors, DOIs, and abstracts. Full text is frequently behind paywalls or closed publisher platforms. Document Tracer only verifies passage alignments against accessible abstract text.

---

## 5. Document Extraction Boundaries

1. **Scanned / Image-Only Documents:**
   - `DocumentLoader` extracts text from UTF-8/ASCII `.txt`, XML text streams (`<w:t>`) in `.docx`, and text literal streams (`(Text) Tj`) in `.pdf`.
   - Scanned, rasterized image-only PDFs requiring Optical Character Recognition (OCR) or encrypted/password-protected files will throw a `DocumentExtractionException`.
2. **File Size Limits:**
   - By default, files exceeding 15 MB are rejected via `FileSizeLimitException` to prevent denial-of-service and memory exhaustion.
