#!/usr/bin/env python3
"""
Generate test documents for testing reference upload:
- test_ref_normal.pdf (Normal text PDF)
- test_ref_scanned.pdf (Scanned PDF with 0 extractable text)
- test_ref_corrupt.pdf (Corrupted invalid PDF)
- test_ref_normal.docx (Valid DOCX document)
- test_ref_normal.txt (Valid TXT document)
"""
import os
import io
import zipfile
import pypdf

out_dir = os.path.join("data", "test_corpus")
os.makedirs(out_dir, exist_ok=True)

# 1. Normal Text PDF
pdf_text_bytes = b"""%PDF-1.4
1 0 obj
<< /Type /Catalog /Pages 2 0 R >>
endobj
2 0 obj
<< /Type /Pages /Kids [3 0 R] /Count 1 >>
endobj
3 0 obj
<< /Type /Page /Parent 2 0 R /MediaBox [0 0 612 792] /Resources << /Font << /F1 4 0 R >> >> /Contents 5 0 R >>
endobj
4 0 obj
<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>
endobj
5 0 obj
<< /Length 260 >>
stream
BT
/F1 12 Tf
72 710 Td
(Distributed systems consist of autonomous computing nodes that communicate over network channels.) Tj
0 -20 Td
(Consensus algorithms like Raft and Paxos ensure fault tolerance and consistency across replicas.) Tj
ET
endstream
endobj
xref
0 6
0000000000 65535 f 
0000000010 00000 n 
0000000060 00000 n 
0000000117 00000 n 
0000000234 00000 n 
0000000305 00000 n 
trailer
<< /Size 6 /Root 1 0 R >>
startxref
615
%%EOF"""

with open(os.path.join(out_dir, "test_ref_normal.pdf"), "wb") as f:
    f.write(pdf_text_bytes)

# 2. Scanned / Blank PDF (Pages exist, but no selectable text layer)
writer_blank = pypdf.PdfWriter()
writer_blank.add_blank_page(width=612, height=792)
with open(os.path.join(out_dir, "test_ref_scanned.pdf"), "wb") as f:
    writer_blank.write(f)

# 3. Corrupted PDF
with open(os.path.join(out_dir, "test_ref_corrupt.pdf"), "wb") as f:
    f.write(b"%PDF-1.4 CORRUPTED TRUNCATED DATA THAT IS NOT A VALID PDF OBJECT STREAM OR HEADER")

# 4. Normal DOCX
docx_xml = """<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<w:document xmlns:w="http://schemas.openxmlformats.org/wordprocessingml/2006/main">
  <w:body>
    <w:p><w:r><w:t>Software engineering principles emphasize modularity, cohesion, and loose coupling.</w:t></w:r></w:p>
    <w:p><w:r><w:t>Design patterns provide reusable architectural templates to solve common design challenges.</w:t></w:r></w:p>
  </w:body>
</w:document>"""
docx_bytes = io.BytesIO()
with zipfile.ZipFile(docx_bytes, "w") as z:
    z.writestr("word/document.xml", docx_xml)
with open(os.path.join(out_dir, "test_ref_normal.docx"), "wb") as f:
    f.write(docx_bytes.getvalue())

# 5. Normal TXT
with open(os.path.join(out_dir, "test_ref_normal.txt"), "w", encoding="utf-8") as f:
    f.write("Machine learning algorithms optimize objective functions across training datasets using gradient descent.")

print("All test corpus files generated successfully!")
