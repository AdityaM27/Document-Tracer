#!/usr/bin/env python3
"""
DocuTrace Academic Source Discovery Fetcher
Queries OpenAlex and Crossref using official REST APIs with polite headers and caching.
"""

import sys
import os
import json
import argparse
import urllib.request
import urllib.parse

def reconstruct_openalex_abstract(inverted_index):
    if not inverted_index or not isinstance(inverted_index, dict):
        return ""
    try:
        positions = []
        for word, idxs in inverted_index.items():
            for idx in idxs:
                positions.append((idx, word))
        positions.sort(key=lambda x: x[0])
        return " ".join([word for _, word in positions])
    except Exception:
        return ""

def query_openalex(query, max_results=3):
    results = []
    try:
        encoded_q = urllib.parse.quote(query)
        url = f"https://api.openalex.org/works?search={encoded_q}&per_page={max_results}"
        req = urllib.request.Request(
            url,
            headers={
                "User-Agent": "DocuTrace-Academic/1.0 (mailto:docutrace-lab@university.edu)",
                "Accept": "application/json"
            }
        )
        with urllib.request.urlopen(req, timeout=5) as resp:
            data = json.loads(resp.read().decode('utf-8'))
            for item in data.get("results", []):
                title = item.get("display_name") or "Untitled Publication"
                authors = []
                for a in item.get("authorships", []):
                    name = a.get("author", {}).get("display_name")
                    if name:
                        authors.append(name)
                pub_date = item.get("publication_date") or ""
                doi = item.get("doi") or ""
                doc_url = item.get("id") or ""
                abstract = reconstruct_openalex_abstract(item.get("abstract_inverted_index"))

                results.append({
                    "title": title,
                    "authors": authors,
                    "publication_date": pub_date,
                    "doi": doi,
                    "url": doc_url,
                    "abstract": abstract,
                    "provider": "OpenAlex"
                })
    except Exception as e:
        sys.stderr.write(f"OpenAlex fetch error (query='{query}'): {e}\n")
    return results

def query_crossref(query, max_results=3):
    results = []
    try:
        encoded_q = urllib.parse.quote(query)
        url = f"https://api.crossref.org/works?query={encoded_q}&rows={max_results}"
        req = urllib.request.Request(
            url,
            headers={
                "User-Agent": "DocuTrace-Academic/1.0 (mailto:docutrace-lab@university.edu)",
                "Accept": "application/json"
            }
        )
        with urllib.request.urlopen(req, timeout=5) as resp:
            data = json.loads(resp.read().decode('utf-8'))
            items = data.get("message", {}).get("items", [])
            for item in items:
                title_list = item.get("title", [])
                title = title_list[0] if title_list else "Untitled"
                authors = []
                for a in item.get("author", []):
                    name = f"{a.get('given', '')} {a.get('family', '')}".strip()
                    if name:
                        authors.append(name)
                pub_parts = item.get("issued", {}).get("date-parts", [[]])
                pub_date = "-".join([str(p) for p in pub_parts[0]]) if pub_parts and pub_parts[0] else ""
                doi = item.get("DOI") or ""
                doc_url = item.get("URL") or ""
                abstract = item.get("abstract") or ""

                results.append({
                    "title": title,
                    "authors": authors,
                    "publication_date": pub_date,
                    "doi": doi,
                    "url": doc_url,
                    "abstract": abstract,
                    "provider": "Crossref"
                })
    except Exception as e:
        sys.stderr.write(f"Crossref fetch error (query='{query}'): {e}\n")
    return results

def main():
    parser = argparse.ArgumentParser(description="Fetch academic sources from OpenAlex and Crossref")
    parser.add_argument("--provider", choices=["openalex", "crossref"], required=True)
    parser.add_argument("--query", required=True)
    parser.add_argument("--max", type=int, default=3)
    parser.add_argument("--out", required=True)

    args = parser.parse_args()

    results = []
    if args.provider == "openalex":
        results = query_openalex(args.query, args.max)
    elif args.provider == "crossref":
        results = query_crossref(args.query, args.max)

    os.makedirs(os.path.dirname(os.path.abspath(args.out)), exist_ok=True)
    with open(args.out, "w", encoding="utf-8") as f:
        json.dump(results, f, indent=2)

    sys.exit(0)

if __name__ == "__main__":
    main()
