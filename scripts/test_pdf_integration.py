import requests
import json
import os
import sys
import time

SERVER_URL = "http://localhost:8080"
CORPUS_DIR = os.path.join(os.path.dirname(__file__), "..", "data", "test_corpus")

def run_tests():
    print("=" * 60)
    print("   DOCUMENT TRACER PDF REFERENCE LIVE INTEGRATION TESTS     ")
    print("=" * 60)

    # Check health
    try:
        r = requests.get(f"{SERVER_URL}/api/health", timeout=5)
        r.raise_for_status()
        print(f"[OK] Backend health check passed: {r.json().get('status')}")
    except Exception as e:
        print(f"[FAIL] Could not connect to backend at {SERVER_URL}: {e}")
        return False

    all_passed = True
    uploaded_ids = []

    # -------------------------------------------------------------
    # Scenario 1: Upload Normal Searchable PDF
    # -------------------------------------------------------------
    normal_pdf = os.path.join(CORPUS_DIR, "test_ref_normal.pdf")
    print("\n--- Test 1: Upload Searchable PDF (test_ref_normal.pdf) ---")
    try:
        with open(normal_pdf, "rb") as f:
            files = {"file": ("test_ref_normal.pdf", f, "application/pdf")}
            res = requests.post(f"{SERVER_URL}/api/references/upload", files=files, timeout=10)
        
        print(f"Status Code: {res.status_code}")
        data = res.json()
        print(f"Response: {data}")

        if res.status_code == 200 and data.get("status") == "indexed":
            if data.get("filename") == "test_ref_normal.pdf" and data.get("wordCount", 0) > 20:
                print(f"[PASS] Searchable PDF uploaded & indexed successfully! Words: {data.get('wordCount')}")
                uploaded_ids.append(data.get("id"))
            else:
                print(f"[FAIL] PDF indexed but metadata incorrect: {data}")
                all_passed = False
        else:
            print(f"[FAIL] Upload returned unexpected status: {res.status_code}")
            all_passed = False
    except Exception as e:
        print(f"[FAIL] Exception during Test 1: {e}")
        all_passed = False

    # -------------------------------------------------------------
    # Scenario 2: Upload Normal DOCX
    # -------------------------------------------------------------
    normal_docx = os.path.join(CORPUS_DIR, "test_ref_normal.docx")
    print("\n--- Test 2: Upload DOCX (test_ref_normal.docx) ---")
    try:
        with open(normal_docx, "rb") as f:
            files = {"file": ("test_ref_normal.docx", f, "application/vnd.openxmlformats-officedocument.wordprocessingml.document")}
            res = requests.post(f"{SERVER_URL}/api/references/upload", files=files, timeout=10)
        
        print(f"Status Code: {res.status_code}")
        data = res.json()
        print(f"Response: {data}")

        if res.status_code == 200 and data.get("status") == "indexed":
            print(f"[PASS] DOCX uploaded & indexed successfully! Words: {data.get('wordCount')}")
            uploaded_ids.append(data.get("id"))
        else:
            print(f"[FAIL] DOCX upload failed: {res.status_code} {data}")
            all_passed = False
    except Exception as e:
        print(f"[FAIL] Exception during Test 2: {e}")
        all_passed = False

    # -------------------------------------------------------------
    # Scenario 3: Upload Normal TXT
    # -------------------------------------------------------------
    normal_txt = os.path.join(CORPUS_DIR, "test_ref_normal.txt")
    print("\n--- Test 3: Upload TXT (test_ref_normal.txt) ---")
    try:
        with open(normal_txt, "rb") as f:
            files = {"file": ("test_ref_normal.txt", f, "text/plain")}
            res = requests.post(f"{SERVER_URL}/api/references/upload", files=files, timeout=10)
        
        print(f"Status Code: {res.status_code}")
        data = res.json()
        print(f"Response: {data}")

        if res.status_code == 200 and data.get("status") == "indexed":
            print(f"[PASS] TXT uploaded & indexed successfully! Words: {data.get('wordCount')}")
            uploaded_ids.append(data.get("id"))
        else:
            print(f"[FAIL] TXT upload failed: {res.status_code} {data}")
            all_passed = False
    except Exception as e:
        print(f"[FAIL] Exception during Test 3: {e}")
        all_passed = False

    # -------------------------------------------------------------
    # Scenario 4: Upload Corrupted PDF
    # -------------------------------------------------------------
    corrupt_pdf = os.path.join(CORPUS_DIR, "test_ref_corrupt.pdf")
    print("\n--- Test 4: Upload Corrupted PDF (test_ref_corrupt.pdf) ---")
    try:
        with open(corrupt_pdf, "rb") as f:
            files = {"file": ("test_ref_corrupt.pdf", f, "application/pdf")}
            res = requests.post(f"{SERVER_URL}/api/references/upload", files=files, timeout=10)
        
        print(f"Status Code: {res.status_code}")
        data = res.json()
        print(f"Response: {data}")

        if res.status_code == 400 and ("corrupt" in data.get("error", "").lower() or "header" in data.get("error", "").lower()):
            print(f"[PASS] Corrupted PDF rejected properly with 400 error: '{data.get('error')}'")
        else:
            print(f"[FAIL] Corrupted PDF not rejected as expected: {res.status_code} {data}")
            all_passed = False
    except Exception as e:
        print(f"[FAIL] Exception during Test 4: {e}")
        all_passed = False

    # -------------------------------------------------------------
    # Scenario 5: Upload Scanned / Image-only PDF
    # -------------------------------------------------------------
    scanned_pdf = os.path.join(CORPUS_DIR, "test_ref_scanned.pdf")
    print("\n--- Test 5: Upload Scanned / Image-only PDF (test_ref_scanned.pdf) ---")
    try:
        with open(scanned_pdf, "rb") as f:
            files = {"file": ("test_ref_scanned.pdf", f, "application/pdf")}
            res = requests.post(f"{SERVER_URL}/api/references/upload", files=files, timeout=10)
        
        print(f"Status Code: {res.status_code}")
        data = res.json()
        print(f"Response: {data}")

        if res.status_code == 400 and ("scanned" in data.get("error", "").lower() or "ocr" in data.get("error", "").lower()):
            print(f"[PASS] Scanned PDF rejected properly with 400 error: '{data.get('error')}'")
        else:
            print(f"[FAIL] Scanned PDF not rejected as expected: {res.status_code} {data}")
            all_passed = False
    except Exception as e:
        print(f"[FAIL] Exception during Test 5: {e}")
        all_passed = False

    # -------------------------------------------------------------
    # Scenario 6: Verify GET /api/references
    # -------------------------------------------------------------
    print("\n--- Test 6: Verify References List (GET /api/references) ---")
    try:
        res = requests.get(f"{SERVER_URL}/api/references", timeout=5)
        refs = res.json()
        pdf_ref = next((r for r in refs if r.get("filename") == "test_ref_normal.pdf"), None)
        if pdf_ref:
            print(f"[PASS] test_ref_normal.pdf is listed in library: ID={pdf_ref['id']}, Filename='{pdf_ref['filename']}', Words={pdf_ref['wordCount']}")
        else:
            print(f"[FAIL] test_ref_normal.pdf not found in references list: {[r.get('filename') for r in refs]}")
            all_passed = False
    except Exception as e:
        print(f"[FAIL] Exception during Test 6: {e}")
        all_passed = False

    # -------------------------------------------------------------
    # Scenario 7: Document Analysis Matching Against Uploaded PDF
    # -------------------------------------------------------------
    print("\n--- Test 7: Similarity Analysis Matching Against Uploaded PDF ---")
    query_text = (
        "Distributed systems consist of autonomous computing nodes that communicate over a network to coordinate actions. "
        "Consensus protocols like Raft and Paxos ensure fault tolerance and consistency across distributed state machines, "
        "allowing clusters to maintain state even when individual nodes crash."
    )
    try:
        # Step 1: Upload query document
        up_res = requests.post(
            f"{SERVER_URL}/api/upload",
            json={"text": query_text, "filename": "query_test_distributed.txt"},
            timeout=10
        )
        print(f"Upload Document Status: {up_res.status_code}")
        up_data = up_res.json()
        doc_id = up_data.get("docId")
        filepath = up_data.get("filepath")
        print(f"Uploaded Doc ID: {doc_id}, Filepath: {filepath}")

        # Step 2: Start Analysis
        analyze_payload = {
            "docId": doc_id,
            "filepath": filepath,
            "enableAcademicSearch": False,
            "sync": True
        }
        res = requests.post(f"{SERVER_URL}/api/analyze", json=analyze_payload, timeout=20)
        print(f"Run Analysis Status: {res.status_code}")
        result = res.json()
        overall = result.get("overallSimilarityScore", 0.0)
        matches = result.get("matches", [])
        print(f"Overall Similarity Score: {overall * 100:.1f}%")
        matched_targets = [m.get("targetDocName", "") for m in matches]
        print(f"Matched Target Document Names: {set(matched_targets)}")

        # Check if test_ref_normal.pdf was matched
        pdf_matched = any("test_ref_normal" in t.lower() for t in matched_targets)
        if pdf_matched and overall > 0.50:
            print(f"[PASS] Successfully matched against uploaded PDF reference with high similarity ({overall * 100:.1f}%)!")
        else:
            print(f"[FAIL] Uploaded PDF was not matched as expected. Overall: {overall}, Matches: {matched_targets}")
            all_passed = False
    except Exception as e:
        print(f"[FAIL] Exception during Test 7: {e}")
        all_passed = False

    # -------------------------------------------------------------
    # Cleanup: Delete uploaded test references
    # -------------------------------------------------------------
    print("\n--- Cleanup: Remove Test References ---")
    for rid in uploaded_ids:
        try:
            requests.delete(f"{SERVER_URL}/api/references/{rid}", timeout=5)
            print(f"Cleaned up reference {rid}")
        except Exception as e:
            print(f"Warning: could not delete {rid}: {e}")

    print("\n" + "=" * 60)
    if all_passed:
        print(">>> ALL 7 LIVE INTEGRATION TESTS PASSED PERFECTLY! <<<")
    else:
        print(">>> SOME INTEGRATION TESTS FAILED <<<")
    print("=" * 60)
    return all_passed

if __name__ == "__main__":
    success = run_tests()
    sys.exit(0 if success else 1)
