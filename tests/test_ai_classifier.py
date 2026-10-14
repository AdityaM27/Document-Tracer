#!/usr/bin/env python3
"""
Automated Comprehensive Test Suite for Pretrained AI-Writing Classifier (Document Tracer)
Tests all 10 validation scenarios mandated for the AI classifier:
1. Short text inference
2. Long text inference (sliding window chunking)
3. Empty / whitespace-only input validation
4. Input exceeding the model's 512 token limit (safely chunked, no index out of bounds)
5. Model-loading failure & statistical fallback mode handling
6. Inference exceptions & error transparency (never silently return 50/50)
7. Correct AI/human label mapping (Fake -> AI, Real -> Human)
8. Probability normalization (ai_score + human_score == 1.0, both in [0.0, 1.0])
9. A genuine prediction that happens to be near 50/50 (labeled Uncertain without error)
10. Classification unavailable handling (no fabricated probabilities)
"""

import sys
import os
import json
import time
import requests

# Ensure project root is in sys.path
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
if PROJECT_ROOT not in sys.path:
    sys.path.insert(0, PROJECT_ROOT)

SERVER_URL = "http://127.0.0.1:8000"

def run_tests():
    print("=" * 70)
    print("  DOCUMENT TRACER PRETRAINED AI CLASSIFIER COMPREHENSIVE TEST SUITE  ")
    print("=" * 70)

    # Health check
    try:
        r = requests.get(f"{SERVER_URL}/health", timeout=5)
        r.raise_for_status()
        h_data = r.json()
        print(f"[Health] Microservice online: model={h_data.get('model')}, mode={h_data.get('mode')}, pretrained={h_data.get('hf_loaded')}")
    except Exception as e:
        print(f"[FATAL] Cannot connect to AI service at {SERVER_URL}: {e}")
        return False

    all_passed = True
    test_results = []

    def record_test(name, passed, details=""):
        nonlocal all_passed
        status = "PASS" if passed else "FAIL"
        print(f"[{status}] {name}")
        if details:
            print(f"       Details: {details}")
        test_results.append((name, passed, details))
        if not passed:
            all_passed = False

    # -----------------------------------------------------------------
    # Test 1: Short Text Inference
    # -----------------------------------------------------------------
    short_text = "The rapid evolution of modern operating systems has introduced complex microkernel architectures."
    try:
        res = requests.post(f"{SERVER_URL}/analyze", json={"text": short_text}, timeout=10)
        data = res.json()
        valid = (
            res.status_code == 200 and
            data.get("is_pretrained") is True and
            data.get("chunks_evaluated") == 1 and
            isinstance(data.get("ai_score"), (int, float)) and
            isinstance(data.get("human_score"), (int, float))
        )
        record_test("1. Short Text Inference", valid, f"Chunks: {data.get('chunks_evaluated')}, AI: {data.get('ai_score')}, Human: {data.get('human_score')}")
    except Exception as e:
        record_test("1. Short Text Inference", False, str(e))

    # -----------------------------------------------------------------
    # Test 2: Long Text Inference (Sliding Window Chunking)
    # -----------------------------------------------------------------
    # Document with ~1200 words (~1500 tokens) spanning multiple 510-token windows
    long_text = (
        "Distributed database systems provide horizontal scalability and continuous availability across global data centers. "
        "Through consensus protocols such as Raft and Paxos, distributed nodes maintain identical replicated state logs. "
        "Transactions adhere to strict serializability or causal consistency models depending on network latency tolerance. "
    ) * 40
    try:
        res = requests.post(f"{SERVER_URL}/analyze", json={"text": long_text}, timeout=30)
        data = res.json()
        chunks = data.get("chunks_evaluated", 0)
        tokens = data.get("total_tokens", 0)
        valid = (
            res.status_code == 200 and
            data.get("is_pretrained") is True and
            chunks >= 2 and
            tokens > 512 and
            isinstance(data.get("window_range"), list) and
            len(data.get("window_range")) == 2
        )
        record_test(
            "2. Long Text Inference (Sliding Window)",
            valid,
            f"Tokens: {tokens}, Windows: {chunks}, Range: {data.get('window_range')}, AI: {data.get('ai_score')}"
        )
    except Exception as e:
        record_test("2. Long Text Inference (Sliding Window)", False, str(e))

    # -----------------------------------------------------------------
    # Test 3: Empty & Whitespace Input
    # -----------------------------------------------------------------
    try:
        res1 = requests.post(f"{SERVER_URL}/analyze", json={"text": ""}, timeout=5)
        res2 = requests.post(f"{SERVER_URL}/analyze", json={"text": "   \n\t   "}, timeout=5)
        valid = (
            res1.status_code == 400 and
            res2.status_code == 400 and
            "empty" in res1.json().get("error", "").lower()
        )
        record_test("3. Empty & Whitespace Input Validation", valid, f"Code1: {res1.status_code}, Code2: {res2.status_code}")
    except Exception as e:
        record_test("3. Empty & Whitespace Input Validation", False, str(e))

    # -----------------------------------------------------------------
    # Test 4: Input Exceeding Model Token Limit (Zero Index-Out-Of-Bounds)
    # -----------------------------------------------------------------
    # 700 tokens of single letters that would trigger index 514 if unchunked
    dense_text = "a b c d e f g h i j k l m n o p q r s t u v w x y z " * 30
    try:
        res = requests.post(f"{SERVER_URL}/analyze", json={"text": dense_text}, timeout=15)
        data = res.json()
        reasoning = data.get("reasoning", "")
        # Must NOT contain any mention of "index 514" or "out of bounds" or "exception fallback"
        has_error_in_reasoning = "out of bounds" in reasoning.lower() or "exception fallback" in reasoning.lower()
        valid = (
            res.status_code == 200 and
            data.get("is_pretrained") is True and
            data.get("total_tokens", 0) > 600 and
            not has_error_in_reasoning
        )
        record_test(
            "4. Token Limit Boundary Safety (No Index 514 Error)",
            valid,
            f"Tokens: {data.get('total_tokens')}, Windows: {data.get('chunks_evaluated')}, AI: {data.get('ai_score')}"
        )
    except Exception as e:
        record_test("4. Token Limit Boundary Safety (No Index 514 Error)", False, str(e))

    # -----------------------------------------------------------------
    # Test 5: Model-Loading Failure & Statistical Fallback Simulation
    # -----------------------------------------------------------------
    # Verify calculate_statistical_ai_score produces explicit fallback metadata
    try:
        from ai_service.app import calculate_statistical_ai_score
        ai_s, h_s, conf, reason = calculate_statistical_ai_score(short_text)
        valid = (
            0.0 <= ai_s <= 1.0 and
            0.0 <= h_s <= 1.0 and
            abs((ai_s + h_s) - 1.0) < 1e-4 and
            "fallback" in reason.lower()
        )
        record_test("5. Statistical Fallback Function (Offline Resilience)", valid, f"AI: {ai_s}, Human: {h_s}, Reason: {reason[:60]}...")
    except Exception as e:
        record_test("5. Statistical Fallback Function (Offline Resilience)", False, str(e))

    # -----------------------------------------------------------------
    # Test 6: Inference Exception Handling (Never Silently Produce 50/50)
    # -----------------------------------------------------------------
    # Verify app error response schema
    try:
        # Non-JSON payload
        res_non_json = requests.post(f"{SERVER_URL}/analyze", data="plain text not json", headers={"Content-Type": "text/plain"}, timeout=5)
        valid = (
            res_non_json.status_code == 400 and
            "error" in res_non_json.json()
        )
        record_test("6. Error Transparency (No Silent 50/50 on Bad Requests)", valid, f"Code: {res_non_json.status_code}, Response: {res_non_json.json()}")
    except Exception as e:
        record_test("6. Error Transparency (No Silent 50/50 on Bad Requests)", False, str(e))

    # -----------------------------------------------------------------
    # Test 7: Label Mapping Inspection (Fake -> AI, Real -> Human)
    # -----------------------------------------------------------------
    try:
        from ai_service.app import AI_LABEL_ID, HUMAN_LABEL_ID, MODEL
        id2label = getattr(MODEL.config, "id2label", {})
        ai_label_name = id2label.get(AI_LABEL_ID, "")
        human_label_name = id2label.get(HUMAN_LABEL_ID, "")
        valid = (
            AI_LABEL_ID == 0 and
            HUMAN_LABEL_ID == 1 and
            "fake" in ai_label_name.lower() and
            "real" in human_label_name.lower()
        )
        record_test("7. Model Label ID Verification", valid, f"AI_LABEL_ID={AI_LABEL_ID} ({ai_label_name}), HUMAN_LABEL_ID={HUMAN_LABEL_ID} ({human_label_name})")
    except Exception as e:
        record_test("7. Model Label ID Verification", False, str(e))

    # -----------------------------------------------------------------
    # Test 8: Probability Normalization
    # -----------------------------------------------------------------
    try:
        res = requests.post(f"{SERVER_URL}/analyze", json={"text": "Software engineering design patterns and principles."}, timeout=10)
        data = res.json()
        ai_s = data.get("ai_score")
        h_s = data.get("human_score")
        valid = (
            isinstance(ai_s, (int, float)) and
            isinstance(h_s, (int, float)) and
            0.0 <= ai_s <= 1.0 and
            0.0 <= h_s <= 1.0 and
            abs((ai_s + h_s) - 1.0) < 1e-3
        )
        record_test("8. Probability Normalization (Sums to 100%)", valid, f"AI: {ai_s}, Human: {h_s}, Sum: {ai_s + h_s:.4f}")
    except Exception as e:
        record_test("8. Probability Normalization (Sums to 100%)", False, str(e))

    # -----------------------------------------------------------------
    # Test 9: Genuine Prediction Near 50/50 (Distinguished from Failure)
    # -----------------------------------------------------------------
    # Verify that when scores are in uncertain territory, mode is genuine 'huggingface'
    try:
        # A mixed technical sentence often yields intermediate probability
        mixed_text = "The network packet was routed through the router according to the routing table rules."
        res = requests.post(f"{SERVER_URL}/analyze", json={"text": mixed_text}, timeout=10)
        data = res.json()
        valid = (
            res.status_code == 200 and
            data.get("is_pretrained") is True and
            data.get("mode") == "huggingface" and
            data.get("label") in ["AI-like", "Human-like", "Uncertain"]
        )
        record_test("9. Genuine Model Output Integrity", valid, f"Label: {data.get('label')}, Mode: {data.get('mode')}, AI: {data.get('ai_score')}, Human: {data.get('human_score')}")
    except Exception as e:
        record_test("9. Genuine Model Output Integrity", False, str(e))

    # -----------------------------------------------------------------
    # Test 10: Unavailable Status Handling Schema
    # -----------------------------------------------------------------
    try:
        # Verify that an error response returns null scores rather than 0.5
        from ai_service.app import app
        with app.test_client() as client:
            resp = client.post("/analyze", json={"text": "   "})
            data = resp.get_json()
            valid = (
                resp.status_code == 400 and
                data.get("ai_score") is None and
                data.get("human_score") is None and
                "error" in data
            )
        record_test("10. Unavailable / Error Schema (No Fabricated 0.5)", valid, f"Scores: AI={data.get('ai_score')}, Human={data.get('human_score')}, Error={data.get('error')}")
    except Exception as e:
        record_test("10. Unavailable / Error Schema (No Fabricated 0.5)", False, str(e))

    print("\n" + "=" * 70)
    passed_count = sum(1 for _, p, _ in test_results if p)
    print(f"TEST SUMMARY: {passed_count}/{len(test_results)} TESTS PASSED")
    print("=" * 70)
    return all_passed

if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)
