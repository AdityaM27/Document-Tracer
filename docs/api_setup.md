# Document Tracer: API Specification & External Service Setup

This guide documents the C++ HTTP Backend REST API, the Python AI microservice, and external academic integrations.

---

## 1. C++ REST API Specification (Port 8080)

The backend server is implemented in C++ using `cpp-httplib` and links `ws2_32` and `crypt32` on Windows.

### 1.1 Health & Configuration

#### `GET /api/health`
Returns system status, active C++ compiler, reference counts, and external provider statuses.
```json
{
  "status": "ok",
  "version": "1.0.0",
  "cppCore": "Ready (C++17, MinGW-w64)",
  "referencesIndexed": 3,
  "academicProviderOnline": true,
  "aiServiceOnline": true
}
```

#### `GET /api/config`
Returns active algorithmic thresholds and thread pool worker count.

#### `POST /api/config`
Updates dynamic configuration:
```json
{
  "exactThreshold": 0.85,
  "ngramThreshold": 0.40,
  "jaccardThreshold": 0.35,
  "semanticThreshold": 0.65,
  "maxConcurrentThreads": 4
}
```

---

### 1.2 Ingestion & Analysis

#### `POST /api/upload`
Accepts `multipart/form-data` with document file (`.txt`, `.docx`, `.pdf`) or JSON payload `{"text": "...", "filename": "..."}`.
Returns:
```json
{
  "docId": "doc_12589756_c37e",
  "filename": "operating_systems.txt",
  "filepath": "data/input/doc_12589756_c37e.txt",
  "size": 612
}
```

#### `POST /api/analyze`
Executes complete multithreaded analysis.
Request:
```json
{
  "filepath": "data/input/doc_12589756_c37e.txt",
  "enableExactMatch": true,
  "enableNGram": true,
  "enableJaccard": true,
  "enableSemantic": true,
  "enableAcademicSearch": true,
  "enableAIClassifier": true
}
```
Response: Serialized `AnalysisSummary` JSON.

#### `GET /api/analysis/:id/status`
Returns live pipeline progress for tracking:
```json
{
  "submissionId": "doc_12589756_c37e",
  "stage": "LOCAL_MATCHING",
  "percent": 40,
  "message": "Comparing against local reference corpus...",
  "isCompleted": false,
  "hasError": false
}
```

#### `GET /api/analysis/:id`
Returns completed analysis findings and match records.

---

### 1.3 Reports & Reference Library

#### `GET /api/reports`
Lists all historical JSON and HTML audit reports in `output/`.

#### `GET /api/reports/:id/html`
Serves standalone HTML report.

#### `GET /api/reports/:id/json`
Serves structured JSON report.

#### `GET /api/references`
Lists all reference documents in the inverted index.

#### `POST /api/references/upload`
Uploads and indexes a new document into `data/references/`.

#### `DELETE /api/references/:id`
Removes a reference document by ID and cleans up the inverted index.

#### `POST /api/references/reindex`
Forces a complete rebuild of the reference index from `data/references/`.

---

## 2. Python AI Microservice Setup (Port 8000)

The AI microservice resides in `ai_service/app.py`.

### 2.1 Dependencies
```bash
pip install -r requirements.txt
```
*(Requires Flask, transformers, torch, requests, numpy)*

### 2.2 Model Configuration
The service defaults to `roberta-base-openai-detector`:
```bash
export DOCUTRACE_AI_MODEL=roberta-base-openai-detector
python ai_service/app.py
```

### 2.3 Offline & Graceful Degradation
If Hugging Face weights or PyTorch are not installed, the microservice automatically operates in **calibrated statistical inference mode** using token burstiness, vocabulary diversity, and structural transition markers. The primary C++ core never crashes even if the AI service is completely offline.

---

## 3. Academic Literature Integration (OpenAlex & Crossref)

### 3.1 OpenAlex API
* **Endpoint**: `https://api.openalex.org/works?search={query}&per_page={max}`
* **Header**: `User-Agent: DocuTrace-Academic/1.0 (mailto:docutrace-lab@university.edu)`
* **Abstract Extraction**: OpenAlex stores abstracts as an inverted index. `scripts/fetch_academic.py` reconstructs the abstract string by ordering words by position index.

### 3.2 Crossref API
* **Endpoint**: `https://api.crossref.org/works?query={query}&rows={max}`
* **Header**: `User-Agent: DocuTrace-Academic/1.0 (mailto:docutrace-lab@university.edu)`
* **Response**: Extracts publication title, authors, DOI, and abstract.

### 3.3 Disk Caching
All academic queries and results are cached locally in `data/cache/academic_cache.json` to prevent duplicate network calls and support offline test execution.
