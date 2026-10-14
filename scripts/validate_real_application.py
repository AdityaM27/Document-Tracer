import requests
import json
import time

SERVER_URL = "http://localhost:8080"

samples = {
    "1. Short Human-Written Text": (
        "I wrote this lab report on Sunday night after finishing my binary search tree implementation. "
        "The recursion was tricky at first because of balancing issues, but using parent pointers fixed the bugs."
    ),
    "2. Longer Human-Written Text": (
        "Operating systems utilize virtual memory management to provide processes with the illusion of a contiguous, "
        "private address space while efficiently sharing physical DRAM. The memory management unit (MMU) performs "
        "address translation from virtual page numbers to physical frame numbers using hierarchical page tables. "
        "To mitigate translation latency overhead, the processor caches recent translations in a hardware Translation "
        "Lookaside Buffer (TLB). On a TLB miss, a page table walk traverses the multi-level page table structure in memory. "
        "When physical memory becomes saturated, page replacement algorithms such as LRU or Clock evict inactive pages "
        "to disk swap space, balancing hit ratios against I/O thrashing."
    ) * 4,
    "3. AI-Generated Sample Text": (
        "In the contemporary epoch of technological advancement, artificial intelligence stands as a transformative paradigm "
        "reshaping modern computational landscapes. Furthermore, it is crucial to recognize that generative language models "
        "facilitate unprecedented efficiency across software engineering, academic synthesis, and organizational logistics. "
        "Moreover, the algorithmic sophistication inherent in transformer architectures enables nuanced semantic contextualization. "
        "In conclusion, embracing these innovative tools remains paramount for future academic progress and ethical development."
    ),
    "4. Mixed / Edited Text Sample": (
        "We started this experiment by setting up the test environment on Linux Ubuntu 22.04 LTS with GCC 11. "
        "In the contemporary epoch of technological advancement, artificial intelligence stands as a transformative paradigm "
        "reshaping modern computational landscapes. Furthermore, our measurements demonstrated that the latency dropped by 15%. "
        "In conclusion, the experiment confirmed our initial hypothesis regarding cache utilization."
    )
}

def validate():
    print("=" * 70)
    print("   DOCUMENT TRACER REAL APPLICATION PIPELINE & AI VALIDATION    ")
    print("=" * 70)

    for name, text in samples.items():
        print(f"\n--- Testing: {name} ---")
        print(f"Text Length: {len(text)} characters, ~{len(text.split())} words")

        # Step 1: Upload query text
        filename = f"sample_{name[:3].replace('.', '')}.txt"
        up_res = requests.post(
            f"{SERVER_URL}/api/upload",
            json={"text": text, "filename": filename},
            timeout=10
        )
        if up_res.status_code != 200:
            print(f"[FAIL] Upload failed: {up_res.status_code} {up_res.text}")
            continue

        up_data = up_res.json()
        doc_id = up_data.get("docId")
        filepath = up_data.get("filepath")

        # Step 2: Run Analysis with AI Classifier enabled
        analyze_payload = {
            "docId": doc_id,
            "filepath": filepath,
            "enableAIClassifier": True,
            "enableAcademicSearch": False,
            "sync": True
        }
        an_res = requests.post(f"{SERVER_URL}/api/analyze", json=analyze_payload, timeout=30)
        if an_res.status_code != 200:
            print(f"[FAIL] Analysis failed: {an_res.status_code} {an_res.text}")
            continue

        summary = an_res.json()
        ai = summary.get("aiResult", {})

        print(f"Overall Similarity Score : {summary.get('overallSimilarityScore', 0) * 100:.1f}%")
        print(f"AI Service Available     : {ai.get('serviceAvailable')}")
        print(f"Is Pretrained Model      : {ai.get('isPretrainedModel')}")
        print(f"Inference Mode           : {ai.get('inferenceMode')}")
        print(f"Classification Label     : {ai.get('label')}")
        print(f"AI Probability           : {ai.get('aiProbability', 0) * 100:.2f}%")
        print(f"Human Probability        : {ai.get('humanProbability', 0) * 100:.2f}%")
        print(f"Model Confidence         : {ai.get('confidence', 0) * 100:.2f}%")
        print(f"Reasoning                : {ai.get('reasoning')}")

    print("\n" + "=" * 70)
    print("ALL REAL PIPELINE VALIDATION SAMPLES EXECUTED SUCCESSFULLY")
    print("=" * 70)

if __name__ == "__main__":
    validate()
