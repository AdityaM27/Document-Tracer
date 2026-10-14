# Document Tracer AI Microservice

This microservice provides pretrained AI-generated text classification and text embeddings for Document Tracer.

## Supported Pretrained Models
- `roberta-base-openai-detector` (Default)
- `distilbert-base-uncased-finetuned-sst-2-english`
- Any compatible Hugging Face text classification model.

## Quick Start
```bash
# 1. Install dependencies
pip install -r requirements.txt

# 2. Run service
python app.py
```
The service will listen on `http://127.0.0.1:8000`.

## Endpoints
- `GET /health` - Health check & model status
- `POST /analyze` - AI classification (`{"text": "..."}`)
- `POST /embed` - Text embedding vector generation (`{"text": "..."}`)

## Offline & Graceful Degradation
If Hugging Face weights or PyTorch are not installed, the microservice automatically operates in calibrated statistical inference mode using burstiness, perplexity proxy, and vocabulary diversity metrics.
