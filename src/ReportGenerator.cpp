#include "../include/ReportGenerator.h"
#include "../include/Exceptions.h"
#include <fstream>
#include <sstream>
#include <filesystem>
#include <iomanip>
#include <algorithm>
#include <cmath>

namespace docutrace {

ReportGenerator::ReportGenerator(const std::string& outputDir)
    : outputDirectory_(outputDir) {
    std::filesystem::create_directories(outputDirectory_);
}

std::string ReportGenerator::escapeHtml(const std::string& data) const {
    std::string buffer;
    buffer.reserve(data.size() + 20);
    for (char c : data) {
        switch (c) {
            case '&':  buffer.append("&amp;");       break;
            case '\"': buffer.append("&quot;");      break;
            case '\'': buffer.append("&apos;");      break;
            case '<':  buffer.append("&lt;");        break;
            case '>':  buffer.append("&gt;");        break;
            case '\n': buffer.append("<br/>");       break;
            default:   buffer.push_back(c);          break;
        }
    }
    return buffer;
}

std::string ReportGenerator::generateHighlightedTextHtml(
    const Document& queryDoc,
    const std::vector<MatchRecord>& matches) const {

    const std::string& orig = queryDoc.getOriginalText();
    if (orig.empty()) return "<p>No document text available.</p>";

    // Build interval markers for highlighted character spans
    struct Span {
        size_t start;
        size_t end;
        MatchType type;
        std::string label;
    };
    std::vector<Span> spans;

    for (const auto& m : matches) {
        if (m.queryPassage.endChar > m.queryPassage.startChar && m.queryPassage.endChar <= orig.size()) {
            spans.push_back({m.queryPassage.startChar, m.queryPassage.endChar, m.matchType, m.reviewStatus});
        }
    }

    // Sort spans by start offset
    std::sort(spans.begin(), spans.end(), [](const Span& a, const Span& b) {
        return a.start < b.start;
    });

    std::ostringstream oss;
    size_t curr = 0;

    for (const auto& s : spans) {
        if (s.start > curr) {
            oss << escapeHtml(orig.substr(curr, s.start - curr));
        }
        if (s.end > curr) {
            size_t actualStart = std::max(curr, s.start);
            size_t actualLen = s.end - actualStart;
            
            std::string cssClass = "match-exact";
            if (s.type == MatchType::NGRAM_MATCH) cssClass = "match-ngram";
            else if (s.type == MatchType::JACCARD_MATCH) cssClass = "match-jaccard";
            else if (s.type == MatchType::SEMANTIC_MATCH) cssClass = "match-semantic";

            oss << "<mark class=\"" << cssClass << "\" title=\"" << escapeHtml(s.label) << "\">"
                << escapeHtml(orig.substr(actualStart, actualLen))
                << "</mark>";

            curr = s.end;
        }
    }

    if (curr < orig.size()) {
        oss << escapeHtml(orig.substr(curr));
    }

    return oss.str();
}

std::string ReportGenerator::generateJsonReport(const AnalysisSummary& summary) const {
    std::string filename = "report_" + summary.submissionId + ".json";
    std::string fullPath = outputDirectory_ + "/" + filename;

    nlohmann::json root;
    to_json(root, summary);

    std::ofstream out(fullPath);
    if (!out.is_open()) {
        throw ReportGenerationException("Failed to open JSON report destination", fullPath);
    }
    out << root.dump(2);
    out.close();

    return fullPath;
}

std::string ReportGenerator::generateHtmlReport(const Document& queryDoc, const AnalysisSummary& summary) const {
    std::string filename = "report_" + summary.submissionId + ".html";
    std::string fullPath = outputDirectory_ + "/" + filename;

    std::ofstream out(fullPath);
    if (!out.is_open()) {
        throw ReportGenerationException("Failed to open HTML report destination", fullPath);
    }

    int overallPct = static_cast<int>(std::round(summary.overallSimilarityScore * 100.0));
    int exactPct = static_cast<int>(std::round(summary.exactSimilarityScore * 100.0));
    int ngramPct = static_cast<int>(std::round(summary.ngramSimilarityScore * 100.0));
    int jaccardPct = static_cast<int>(std::round(summary.jaccardSimilarityScore * 100.0));
    int semanticPct = static_cast<int>(std::round(summary.semanticSimilarityScore * 100.0));

    std::string statusColor = overallPct > 50 ? "#ef4444" : (overallPct > 25 ? "#f59e0b" : "#10b981");

    out << R"(<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8"/>
    <meta name="viewport" content="width=device-width, initial-scale=1.0"/>
    <title>Document Tracer Analysis Report - )" << escapeHtml(summary.submissionFilename) << R"(</title>
    <style>
        :root {
            --bg: #0f172a;
            --surface: #1e293b;
            --surface-hover: #334155;
            --border: #475569;
            --text-main: #f8fafc;
            --text-muted: #94a3b8;
            --primary: #38bdf8;
            --accent: #818cf8;
            --danger: #f87171;
            --warning: #fbbf24;
            --success: #34d399;
        }
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
            background: var(--bg);
            color: var(--text-main);
            line-height: 1.6;
            padding: 2rem 1rem;
        }
        .container { max-width: 1200px; margin: 0 auto; }
        header {
            background: linear-gradient(135deg, #1e293b, #0f172a);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 2rem;
            margin-bottom: 2rem;
            box-shadow: 0 10px 25px -5px rgba(0,0,0,0.5);
        }
        .badge {
            display: inline-block;
            padding: 0.35rem 0.75rem;
            border-radius: 9999px;
            font-size: 0.75rem;
            font-weight: 700;
            text-transform: uppercase;
            letter-spacing: 0.05em;
            background: rgba(56, 189, 248, 0.15);
            color: var(--primary);
            border: 1px solid rgba(56, 189, 248, 0.3);
            margin-bottom: 0.75rem;
        }
        h1 { font-size: 2rem; margin-bottom: 0.5rem; color: #fff; }
        .meta-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
            gap: 1rem;
            margin-top: 1.5rem;
            padding-top: 1.5rem;
            border-top: 1px solid var(--border);
            font-size: 0.9rem;
            color: var(--text-muted);
        }
        .meta-val { color: var(--text-main); font-weight: 600; }
        
        .grid-cards {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(220px, 1fr));
            gap: 1.25rem;
            margin-bottom: 2rem;
        }
        .card {
            background: var(--surface);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 1.5rem;
            transition: transform 0.2s;
        }
        .card:hover { transform: translateY(-2px); }
        .card-label { font-size: 0.85rem; color: var(--text-muted); text-transform: uppercase; font-weight: 600; }
        .card-value { font-size: 2.2rem; font-weight: 800; margin: 0.5rem 0; color: #fff; }
        .card-desc { font-size: 0.8rem; color: var(--text-muted); }
        
        .section-box {
            background: var(--surface);
            border: 1px solid var(--border);
            border-radius: 12px;
            padding: 1.75rem;
            margin-bottom: 2rem;
        }
        .section-title {
            font-size: 1.25rem;
            font-weight: 700;
            margin-bottom: 1.25rem;
            display: flex;
            align-items: center;
            gap: 0.5rem;
            color: #fff;
            border-bottom: 1px solid var(--border);
            padding-bottom: 0.75rem;
        }
        
        mark.match-exact { background: rgba(248, 113, 113, 0.4); color: #fff; border-bottom: 2px solid #ef4444; padding: 2px 4px; border-radius: 3px; }
        mark.match-ngram { background: rgba(251, 191, 36, 0.4); color: #fff; border-bottom: 2px solid #f59e0b; padding: 2px 4px; border-radius: 3px; }
        mark.match-jaccard { background: rgba(56, 189, 248, 0.4); color: #fff; border-bottom: 2px solid #0284c7; padding: 2px 4px; border-radius: 3px; }
        mark.match-semantic { background: rgba(168, 85, 247, 0.4); color: #fff; border-bottom: 2px solid #a855f7; padding: 2px 4px; border-radius: 3px; }

        .text-view {
            background: #090d16;
            padding: 1.5rem;
            border-radius: 8px;
            font-size: 0.95rem;
            line-height: 1.8;
            max-height: 500px;
            overflow-y: auto;
            border: 1px solid #334155;
            white-space: pre-wrap;
            word-break: break-word;
        }
        
        .match-table, .sources-table {
            width: 100%;
            border-collapse: collapse;
            font-size: 0.9rem;
            margin-top: 1rem;
        }
        .match-table th, .sources-table th {
            text-align: left;
            padding: 0.75rem 1rem;
            background: #0f172a;
            color: var(--text-muted);
            font-weight: 600;
            border-bottom: 1px solid var(--border);
        }
        .match-table td, .sources-table td {
            padding: 1rem;
            border-bottom: 1px solid #334155;
            vertical-align: top;
        }
        .match-table tr:hover, .sources-table tr:hover { background: var(--surface-hover); }
        
        .pill {
            display: inline-block;
            padding: 0.2rem 0.5rem;
            border-radius: 4px;
            font-size: 0.75rem;
            font-weight: 600;
        }
        .pill-exact { background: rgba(248, 113, 113, 0.2); color: #f87171; }
        .pill-ngram { background: rgba(251, 191, 36, 0.2); color: #fbbf24; }
        .pill-semantic { background: rgba(168, 85, 247, 0.2); color: #c084fc; }
        
        .disclaimer-box {
            background: rgba(56, 189, 248, 0.08);
            border-left: 4px solid var(--primary);
            padding: 1rem 1.25rem;
            border-radius: 0 8px 8px 0;
            margin-top: 1.5rem;
            font-size: 0.85rem;
            color: #bae6fd;
        }

        footer {
            text-align: center;
            font-size: 0.85rem;
            color: var(--text-muted);
            margin-top: 3rem;
            padding-top: 1.5rem;
            border-top: 1px solid var(--border);
        }
    </style>
</head>
<body>
<div class="container">
    <header>
        <span class="badge">Document Tracer Comprehensive Analysis Report</span>
        <h1>)" << escapeHtml(summary.submissionFilename) << R"(</h1>
        <div class="meta-grid">
            <div>Analysis ID: <span class="meta-val">)" << escapeHtml(summary.submissionId) << R"(</span></div>
            <div>Generated: <span class="meta-val">)" << escapeHtml(summary.timestamp) << R"(</span></div>
            <div>References Checked: <span class="meta-val">)" << summary.referencesCheckedCount << R"( docs</span></div>
            <div>Processing Time: <span class="meta-val">)" << std::round(summary.processingTimeMs) << R"( ms</span></div>
        </div>
    </header>

    <!-- Executive Summary Cards -->
    <div class="grid-cards">
        <div class="card" style="border-top: 4px solid )" << statusColor << R"(;">
            <div class="card-label">Overall Similarity</div>
            <div class="card-value" style="color: )" << statusColor << R"(;">)" << overallPct << R"(%</div>
            <div class="card-desc">Combined non-overlapping content overlap</div>
        </div>
        <div class="card" style="border-top: 4px solid #ef4444;">
            <div class="card-label">Exact Match</div>
            <div class="card-value">)" << exactPct << R"(%</div>
            <div class="card-desc">Identical sequence overlap</div>
        </div>
        <div class="card" style="border-top: 4px solid #f59e0b;">
            <div class="card-label">3-Gram Overlap</div>
            <div class="card-value">)" << ngramPct << R"(%</div>
            <div class="card-desc">Multi-token phrase containment</div>
        </div>
        <div class="card" style="border-top: 4px solid #0284c7;">
            <div class="card-label">Jaccard Metric</div>
            <div class="card-value">)" << jaccardPct << R"(%</div>
            <div class="card-desc">Token set intersection over union</div>
        </div>
        <div class="card" style="border-top: 4px solid #a855f7;">
            <div class="card-label">Semantic Cosine</div>
            <div class="card-value">)" << semanticPct << R"(%</div>
            <div class="card-desc">Paraphrase candidate similarity</div>
        </div>
        <div class="card" style="border-top: 4px solid #38bdf8;">
            <div class="card-label">AI Classification</div>
            <div class="card-value" style="font-size: 1.5rem; line-height: 2.2rem;">)" 
            << escapeHtml(summary.aiResult.label) << R"(</div>
            <div class="card-desc">)" << (summary.aiResult.serviceAvailable ? "Confidence: " + std::to_string(static_cast<int>(summary.aiResult.confidence * 100)) + "%" : "Service Offline") << R"(</div>
        </div>
    </div>

    <!-- Highlighted Document Passage Inspection -->
    <div class="section-box">
        <div class="section-title">
            <span>&#128269; Submitted Document Inspection (Color-Coded Matches)</span>
        </div>
        <div style="margin-bottom: 1rem; display: flex; gap: 1rem; font-size: 0.85rem;">
            <span><mark class="match-exact">&#9632; Exact Match</mark></span>
            <span><mark class="match-ngram">&#9632; N-Gram Overlap</mark></span>
            <span><mark class="match-jaccard">&#9632; Jaccard Overlap</mark></span>
            <span><mark class="match-semantic">&#9632; Semantic Match</mark></span>
        </div>
        <div class="text-view">)"
        << generateHighlightedTextHtml(queryDoc, summary.matches)
        << R"(</div>
    </div>

    <!-- Side-by-Side Passage Findings Table -->
    <div class="section-box">
        <div class="section-title">
            <span>&#9878; Passage Evidence &amp; Side-by-Side Match Inspector</span>
        </div>)";

    if (summary.matches.empty()) {
        out << "<p style='color: var(--text-muted);'>No matching passages exceeded the configured similarity thresholds.</p>";
    } else {
        out << R"(<table class="match-table">
            <thead>
                <tr>
                    <th style="width: 12%;">Type / Score</th>
                    <th style="width: 38%;">Submitted Passage</th>
                    <th style="width: 38%;">Matched Reference Passage</th>
                    <th style="width: 12%;">Source / Status</th>
                </tr>
            </thead>
            <tbody>)";

        for (const auto& m : summary.matches) {
            std::string pillClass = "pill-exact";
            if (m.matchType == MatchType::NGRAM_MATCH) pillClass = "pill-ngram";
            else if (m.matchType == MatchType::SEMANTIC_MATCH) pillClass = "pill-semantic";

            out << "<tr>"
                << "<td><span class=\"pill " << pillClass << "\">" << matchTypeToString(m.matchType) << "</span><br/>"
                << "<strong style='font-size:1.1rem;'>" << static_cast<int>(std::round(m.score * 100)) << "%</strong></td>"
                << "<td><blockquote style='border-left: 2px solid var(--primary); padding-left: 0.5rem; font-style: italic;'>"
                << escapeHtml(m.queryPassage.originalText) << "</blockquote></td>"
                << "<td><blockquote style='border-left: 2px solid var(--accent); padding-left: 0.5rem; font-style: italic;'>"
                << escapeHtml(m.targetPassage.originalText) << "</blockquote></td>"
                << "<td><strong>" << escapeHtml(m.targetDocName) << "</strong><br/><span style='font-size:0.8rem; color:var(--text-muted);'>"
                << escapeHtml(m.reviewStatus) << "</span></td>"
                << "</tr>";
        }

        out << R"(</tbody></table>)";
    }

    out << R"(</div>

    <!-- Discovered Academic Literature -->
    <div class="section-box">
        <div class="section-title">
            <span>&#128218; Candidate Academic Publications Discovered (OpenAlex &amp; Crossref)</span>
        </div>)";

    if (summary.discoveredSources.empty()) {
        out << "<p style='color: var(--text-muted);'>No external publications required retrieval or external discovery was offline.</p>";
    } else {
        out << R"(<table class="sources-table">
            <thead>
                <tr>
                    <th>Title &amp; Authors</th>
                    <th>Date / DOI</th>
                    <th>Provider</th>
                    <th>Status</th>
                </tr>
            </thead>
            <tbody>)";

        for (const auto& s : summary.discoveredSources) {
            std::string authorStr;
            for (size_t a = 0; a < s.authors.size() && a < 3; ++a) {
                authorStr += s.authors[a] + (a + 1 < s.authors.size() ? ", " : "");
            }
            if (s.authors.size() > 3) authorStr += " et al.";

            out << "<tr>"
                << "<td><strong>" << escapeHtml(s.title) << "</strong><br/><span style='color: var(--text-muted); font-size: 0.85rem;'>"
                << escapeHtml(authorStr) << "</span></td>"
                << "<td>" << escapeHtml(s.publicationDate) << "<br/>"
                << (s.doi.empty() ? "" : "<a href=\"https://doi.org/" + escapeHtml(s.doi) + "\" target=\"_blank\" style=\"color: var(--primary);\">" + escapeHtml(s.doi) + "</a>") << "</td>"
                << "<td><span class=\"badge\" style=\"margin:0;\">" << escapeHtml(s.provider) << "</span></td>"
                << "<td>" << escapeHtml(s.retrievalStatus) << "</td>"
                << "</tr>";
        }

        out << R"(</tbody></table>)";
    }

    out << R"(</div>

    <!-- Explainable Writing Pattern Statistics -->
    <div class="section-box">
        <div class="section-title">
            <span>&#128202; Stylometric &amp; Writing Pattern Statistics</span>
        </div>
        <div class="grid-cards" style="margin-bottom: 1rem;">
            <div class="card">
                <div class="card-label">Vocabulary Diversity (TTR)</div>
                <div class="card-value">)" << std::round(summary.writingStats.typeTokenRatio * 1000.0) / 10.0 << R"(%</div>
                <div class="card-desc">Type-Token Ratio ()" << summary.writingStats.uniqueWords << " unique / " << summary.writingStats.totalWords << R"( total)</div>
            </div>
            <div class="card">
                <div class="card-label">Sentence Burstiness</div>
                <div class="card-value">)" << std::round(summary.writingStats.burstiness * 100.0) / 100.0 << R"(</div>
                <div class="card-desc">Mean: )" << std::round(summary.writingStats.avgSentenceLength * 10.0) / 10.0 << " w/sent | StdDev: " << std::round(summary.writingStats.sentenceLengthStdDev * 10.0) / 10.0 << R"(</div>
            </div>
            <div class="card">
                <div class="card-label">Shannon Token Entropy</div>
                <div class="card-value">)" << std::round(summary.writingStats.characterEntropy * 100.0) / 100.0 << R"(</div>
                <div class="card-desc">Bits of information per token</div>
            </div>
            <div class="card">
                <div class="card-label">Transition Phrases</div>
                <div class="card-value">)" << summary.writingStats.transitionPhraseFrequencies.size() << R"(</div>
                <div class="card-desc">Distinct discourse markers detected</div>
            </div>
        </div>
        <p style="font-size: 0.9rem; color: var(--text-muted); margin-top: 0.5rem;">)"
        << escapeHtml(summary.writingStats.interpretation) << R"(</p>
    </div>

    <!-- AI Writing Classifier Section -->
    <div class="section-box">
        <div class="section-title">
            <span>&#129302; Pretrained AI-Writing Classifier Evaluation</span>
        </div>
        <p><strong>Model:</strong> )" << escapeHtml(summary.aiResult.modelName) << R"(</p>
        <p><strong>Classification:</strong> )" << escapeHtml(summary.aiResult.label) << R"(</p>)";

    if (summary.aiResult.serviceAvailable && summary.aiResult.label != "Inference Error" && summary.aiResult.label != "Service Offline") {
        out << R"(
        <p><strong>Model Scores:</strong> AI Probability: )" 
            << static_cast<int>(std::round(summary.aiResult.aiProbability * 100.0)) 
            << R"(% | Human Probability: )" 
            << static_cast<int>(std::round(summary.aiResult.humanProbability * 100.0)) << R"(%</p>)";
    } else {
        out << R"(
        <p><strong>Model Status:</strong> Neural classification unavailable (no fabricated probabilities reported).</p>)";
    }
    out << R"(
        <p style="font-size: 0.9rem; color: var(--text-muted); margin-top: 0.5rem;">)"
        << escapeHtml(summary.aiResult.reasoning) << R"(</p>
        <div class="disclaimer-box">
            <strong>&#9888; Ethical &amp; Scientific Limitations:</strong><br/>
            )" << escapeHtml(summary.aiResult.disclaimer) << R"(
        </div>
    </div>

    <!-- Methodology & Limitations -->
    <div class="section-box">
        <div class="section-title">
            <span>&#128220; Methodology, Algorithmic Definitions &amp; Academic Scope</span>
        </div>
        <div style="font-size: 0.9rem; color: var(--text-muted); display: grid; gap: 0.75rem;">
            <p><strong>Exact Matching:</strong> Computes identical token sequences with window-based boundary protection and offset preservation.</p>
            <p><strong>N-Gram Similarity:</strong> Computes multiset containment of sliding contiguous 3-word n-grams between query passages and indexed references.</p>
            <p><strong>Jaccard Similarity:</strong> Evaluates lexical overlap via set intersection over union: \(J(A, B) = \frac{|A \cap B|}{|A \cup B|}\).</p>
            <p><strong>Semantic Similarity:</strong> Generates subword and sentence embeddings to compute cosine similarity: \(\cos(u, v) = \frac{u \cdot v}{\|u\| \|v\|}\).</p>
            <p><strong>Scientific Attribution:</strong> High similarity does NOT automatically establish intellectual dishonesty. Common technical idioms, standard mathematical definitions, and citations regularly generate legitimate overlap.</p>
        </div>
    </div>

    <footer>
        <p>Document Tracer &copy; 2026 &bull; B.Tech III Semester Object Oriented Programming Through C++ Laboratory Project</p>
        <p>Engineered with C++17, STL Containers, RAII, Multithreading &amp; React / TypeScript</p>
    </footer>
</div>
</body>
</html>)";

    out.close();
    return fullPath;
}

} // namespace docutrace
