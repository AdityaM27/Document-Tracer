#!/usr/bin/env python3
"""
DocuTrace Document Extraction Helper Script
Extracts clean plain text from DOCX, PDF, and other document formats.
Provides granular diagnostics for empty, corrupt, encrypted, and scanned documents.
"""

import sys
import os
import zipfile
import xml.etree.ElementTree as ET

def extract_docx(docx_path):
    if not os.path.exists(docx_path):
        return False, "File does not exist: " + docx_path, "NOT_FOUND"

    if os.path.getsize(docx_path) == 0:
        return False, "The DOCX file is empty (0 bytes). Please upload a valid document.", "EMPTY_FILE"

    try:
        with zipfile.ZipFile(docx_path) as z:
            if 'word/document.xml' not in z.namelist():
                return False, "Invalid DOCX format: archive missing word/document.xml.", "INVALID_DOCX"

            xml_content = z.read('word/document.xml')
            tree = ET.fromstring(xml_content)

            # XML namespace for wordprocessingml
            namespaces = {'w': 'http://schemas.openxmlformats.org/wordprocessingml/2006/main'}

            paragraphs = []
            for p in tree.findall('.//w:p', namespaces):
                texts = [node.text for node in p.findall('.//w:t', namespaces) if node.text]
                if texts:
                    paragraphs.append(''.join(texts))

            full_text = '\n\n'.join(paragraphs).strip()
            if not full_text:
                return False, "The DOCX file contains no readable text content.", "EMPTY_DOCX"

            return True, full_text, "OK"
    except zipfile.BadZipFile:
        return False, "The DOCX file is corrupted or not a valid Microsoft Word archive.", "CORRUPT_DOCX"
    except Exception as e:
        return False, f"DOCX extraction error: {e}", "DOCX_ERROR"

def extract_pdf(pdf_path):
    if not os.path.exists(pdf_path):
        return False, "File does not exist: " + pdf_path, "NOT_FOUND"

    file_size = os.path.getsize(pdf_path)
    if file_size == 0:
        return False, "The PDF file is empty (0 bytes). Please upload a valid document.", "EMPTY_FILE"

    # Verify standard %PDF header
    try:
        with open(pdf_path, 'rb') as f:
            header = f.read(1024)
            if b'%PDF-' not in header:
                return False, "The file does not have a valid PDF header (%PDF-). It may be corrupted or not a PDF.", "INVALID_PDF_HEADER"
    except Exception as e:
        return False, f"Unable to read PDF file: {e}", "READ_ERROR"

    # Primary extraction via pypdf
    try:
        import pypdf
        try:
            reader = pypdf.PdfReader(pdf_path)
        except Exception as e:
            return False, f"The PDF file is corrupted or damaged and cannot be opened: {e}", "CORRUPT_PDF"

        if reader.is_encrypted:
            try:
                decrypted = reader.decrypt("")
                if decrypted == 0:
                    return False, "The PDF file is password-protected or encrypted. Please provide an unencrypted document.", "ENCRYPTED_PDF"
            except Exception:
                return False, "The PDF file is password-protected or encrypted. Please provide an unencrypted document.", "ENCRYPTED_PDF"

        num_pages = len(reader.pages)
        if num_pages == 0:
            return False, "The PDF document contains 0 pages.", "EMPTY_PDF"

        extracted_pages = []
        for idx, page in enumerate(reader.pages):
            try:
                page_text = page.extract_text()
                if page_text and page_text.strip():
                    extracted_pages.append(page_text.strip())
            except Exception as pe:
                sys.stderr.write(f"Warning: failed extracting page {idx + 1}: {pe}\n")

        full_text = '\n\n'.join(extracted_pages).strip()
        if not full_text:
            return False, "The PDF file contains pages but no selectable text layer (scanned or image-only document). An OCR or text-based PDF is required.", "SCANNED_PDF"

        return True, full_text, "OK"

    except ImportError:
        pass

    # Native fallback for uncompressed streams if pypdf is unavailable
    try:
        with open(pdf_path, 'rb') as f:
            content = f.read().decode('latin-1', errors='ignore')
            import re
            matches = re.findall(r'\((.*?)\)\s*Tj', content)
            if matches:
                raw_text = " ".join(matches).strip()
                if raw_text:
                    return True, raw_text, "OK"
    except Exception as e:
        sys.stderr.write(f"PDF raw scan fallback error: {e}\n")

    return False, "Unable to extract text from PDF. The document may be scanned (image-only), encrypted, or missing extraction dependencies.", "PDF_EXTRACTION_FAILED"

def main():
    if len(sys.argv) < 3:
        print("Usage: extract_text.py <input_file> <output_file>")
        sys.exit(1)

    in_path = sys.argv[1]
    out_path = sys.argv[2]

    ext = os.path.splitext(in_path)[1].lower()

    if ext == '.docx':
        success, content, code = extract_docx(in_path)
    elif ext == '.pdf':
        success, content, code = extract_pdf(in_path)
    else:
        # Text-based formats (.txt, .md, .cpp, etc.)
        try:
            if not os.path.exists(in_path):
                success, content, code = False, "File does not exist: " + in_path, "NOT_FOUND"
            elif os.path.getsize(in_path) == 0:
                success, content, code = False, "The text file is empty (0 bytes).", "EMPTY_FILE"
            else:
                with open(in_path, 'r', encoding='utf-8', errors='ignore') as f:
                    content = f.read()
                if not content.strip():
                    success, content, code = False, "The text file contains only whitespace.", "EMPTY_FILE"
                else:
                    success, content, code = True, content, "OK"
        except Exception as e:
            success, content, code = False, f"Read error: {e}", "READ_ERROR"

    if success:
        with open(out_path, 'w', encoding='utf-8') as f:
            f.write(content)
        sys.exit(0)
    else:
        with open(out_path, 'w', encoding='utf-8') as f:
            f.write(f"DOCUTRACE_EXTRACTION_ERROR:{code}:{content}")
        sys.stderr.write(f"[{code}] {content}\n")
        sys.exit(1)

if __name__ == '__main__':
    main()
