import React, { useState } from 'react';
import {
  Download,
  ExternalLink,
  Cpu,
  BookOpen,
  BarChart2,
  FileText,
  Layers,
  ArrowRight
} from 'lucide-react';
import { AnalysisSummary, MatchRecord } from '../types';

interface ResultsViewProps {
  summary: AnalysisSummary | null;
  onNewAnalysis: () => void;
}

export const ResultsView: React.FC<ResultsViewProps> = ({ summary, onNewAnalysis }) => {
  const [filterType, setFilterType] = useState<string>('ALL');

  if (!summary) {
    return (
      <div className="view-container">
        <div className="section-card empty-state">
          <h2>No Active Analysis Results</h2>
          <p>Analyze or upload a document to inspect similarity metrics, AI classification, and source matches.</p>
          <button className="btn btn-primary" onClick={onNewAnalysis}>
            Start New Analysis
          </button>
        </div>
      </div>
    );
  }

  const overallPct = Math.round(summary.overallSimilarityScore * 100);
  const exactPct = Math.round(summary.exactSimilarityScore * 100);
  const ngramPct = Math.round(summary.ngramSimilarityScore * 100);
  const jaccardPct = Math.round(summary.jaccardSimilarityScore * 100);
  const semanticPct = Math.round(summary.semanticSimilarityScore * 100);

  const exactMatchesCount = summary.matches.filter((m) => m.matchType === 'EXACT_MATCH').length;
  const ngramMatchesCount = summary.matches.filter((m) => m.matchType === 'NGRAM_MATCH').length;
  const jaccardMatchesCount = summary.matches.filter((m) => m.matchType === 'JACCARD_MATCH').length;
  const semanticMatchesCount = summary.matches.filter((m) => m.matchType === 'SEMANTIC_MATCH').length;

  const filteredMatches = summary.matches.filter((m) => {
    if (filterType === 'ALL') return true;
    return m.matchType === filterType;
  });

  const getPillForMatchType = (type: string) => {
    switch (type) {
      case 'EXACT_MATCH':
        return 'pill-error';
      case 'NGRAM_MATCH':
        return 'pill-warning';
      case 'SEMANTIC_MATCH':
        return 'pill-purple';
      default:
        return 'pill-info';
    }
  };

  const getHighlightClass = (type: string) => {
    switch (type) {
      case 'EXACT_MATCH':
        return 'highlight-exact';
      case 'NGRAM_MATCH':
        return 'highlight-ngram';
      case 'SEMANTIC_MATCH':
        return 'highlight-semantic';
      default:
        return 'highlight-jaccard';
    }
  };

  return (
    <div className="view-container">
      {/* Top Banner / Summary Bar */}
      <div className="results-summary-bar">
        <div>
          <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', marginBottom: '0.4rem' }}>
            <span className="pill pill-info">Audit Record</span>
            <span style={{ fontSize: '0.82rem', color: 'var(--text-secondary)' }}>ID: <code>{summary.submissionId}</code></span>
          </div>
          <h1 className="results-title">{summary.submissionFilename}</h1>
          <div className="results-meta">
            Analyzed on {summary.timestamp} &bull; Processed in {Math.round(summary.processingTimeMs)} ms &bull;{' '}
            {summary.referencesCheckedCount} Reference Documents Checked
          </div>
        </div>

        <div style={{ display: 'flex', alignItems: 'center', gap: '0.65rem', flexWrap: 'wrap' }}>
          <a
            href={`/api/reports/${summary.submissionId}/html`}
            target="_blank"
            rel="noopener noreferrer"
            className="btn btn-primary"
          >
            <ExternalLink size={15} /> Standalone HTML Report
          </a>
          <a
            href={`/api/reports/${summary.submissionId}/json`}
            download={`report_${summary.submissionId}.json`}
            className="btn btn-secondary"
          >
            <Download size={15} /> Export JSON
          </a>
          <button className="btn btn-secondary" onClick={onNewAnalysis}>
            New Analysis
          </button>
        </div>
      </div>

      {/* Similarity Metric Gauges */}
      <div className="metrics-row">
        <div
          className="metric-gauge"
          style={{
            borderTopColor: overallPct > 50 ? 'var(--error)' : overallPct > 25 ? 'var(--warning)' : 'var(--success)',
          }}
        >
          <div className="metric-gauge-label">Overall Similarity</div>
          <div
            className="metric-gauge-val"
            style={{
              color: overallPct > 50 ? 'var(--error)' : overallPct > 25 ? 'var(--warning)' : 'var(--success)',
            }}
          >
            {overallPct}%
          </div>
          <div className="metric-gauge-sub">Aggregated non-overlapping coverage</div>
        </div>

        <div className="metric-gauge" style={{ borderTopColor: 'var(--error)' }}>
          <div className="metric-gauge-label">Exact Match</div>
          <div className="metric-gauge-val">{exactPct}%</div>
          <div className="metric-gauge-sub">Identical sequence overlap</div>
        </div>

        <div className="metric-gauge" style={{ borderTopColor: 'var(--warning)' }}>
          <div className="metric-gauge-label">3-Gram Overlap</div>
          <div className="metric-gauge-val">{ngramPct}%</div>
          <div className="metric-gauge-sub">Contiguous phrase containment</div>
        </div>

        <div className="metric-gauge" style={{ borderTopColor: '#0284C7' }}>
          <div className="metric-gauge-label">Jaccard Metric</div>
          <div className="metric-gauge-val">{jaccardPct}%</div>
          <div className="metric-gauge-sub">Token set intersection over union</div>
        </div>

        <div className="metric-gauge" style={{ borderTopColor: '#7E22CE' }}>
          <div className="metric-gauge-label">Semantic Cosine</div>
          <div className="metric-gauge-val">{semanticPct}%</div>
          <div className="metric-gauge-sub">Paraphrase candidate similarity</div>
        </div>
      </div>

      {/* Separate AI Classification & Stylometrics Row */}
      <div className="dual-grid">
        {/* Left: AI-Writing Classifier */}
        <div className="ai-classifier-card">
          <div className="section-header">
            <div>
              <div className="section-title">
                <Cpu size={18} color="var(--primary-accent)" /> Pretrained AI-Writing Classifier
              </div>
              <div className="section-desc">
                Model: {summary.aiResult.modelName} &bull; Mode:{' '}
                <span
                  style={{
                    fontWeight: 600,
                    color: summary.aiResult.serviceAvailable && summary.aiResult.label !== 'Inference Error' && summary.aiResult.label !== 'Service Offline'
                      ? summary.aiResult.isPretrainedModel
                        ? 'var(--success)'
                        : 'var(--warning)'
                      : 'var(--error)',
                  }}
                >
                  {summary.aiResult.serviceAvailable && summary.aiResult.label !== 'Inference Error' && summary.aiResult.label !== 'Service Offline'
                    ? summary.aiResult.isPretrainedModel
                      ? 'Genuine Pretrained Neural Weights (HF)'
                      : 'Calibrated Statistical Fallback'
                    : 'Service Offline / Error'}
                </span>
              </div>
            </div>
            <div>
              <span
                className={`pill ${
                  !summary.aiResult.serviceAvailable || summary.aiResult.label === 'Inference Error' || summary.aiResult.label === 'Service Offline'
                    ? 'pill-error'
                    : summary.aiResult.label === 'AI-like'
                    ? 'pill-error'
                    : summary.aiResult.label === 'Human-like'
                    ? 'pill-success'
                    : 'pill-warning'
                }`}
                style={{ fontSize: '0.82rem', padding: '0.25rem 0.65rem' }}
              >
                {summary.aiResult.label}
              </span>
            </div>
          </div>

          {summary.aiResult.serviceAvailable && summary.aiResult.label !== 'Inference Error' && summary.aiResult.label !== 'Service Offline' ? (
            <div style={{ marginBottom: '1.25rem' }}>
              <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.82rem', marginBottom: '0.35rem' }}>
                <span>AI Probability: <strong style={{ color: 'var(--text-primary)' }}>{Math.round(summary.aiResult.aiProbability * 100)}%</strong></span>
                <span>Human Probability: <strong style={{ color: 'var(--text-primary)' }}>{Math.round(summary.aiResult.humanProbability * 100)}%</strong></span>
              </div>
              <div className="ai-prob-track">
                <div
                  className="ai-prob-fill"
                  style={{ width: `${Math.round(summary.aiResult.aiProbability * 100)}%` }}
                />
                <div
                  className="ai-prob-human"
                  style={{ width: `${Math.round(summary.aiResult.humanProbability * 100)}%` }}
                />
              </div>
            </div>
          ) : (
            <div style={{
              backgroundColor: 'var(--bg-secondary)',
              border: '1px solid var(--border)',
              borderRadius: '6px',
              padding: '0.75rem 1rem',
              marginBottom: '1rem',
              fontSize: '0.82rem',
              color: 'var(--text-secondary)'
            }}>
              <strong style={{ color: 'var(--error)' }}>Notice:</strong> Neural text classification is currently unavailable. No fabricated probabilities are generated.
            </div>
          )}

          <p style={{ fontSize: '0.85rem', color: 'var(--text-secondary)', lineHeight: '1.6', margin: 0 }}>
            {summary.aiResult.reasoning}
          </p>

          <div className="disclaimer-clean">
            <strong style={{ color: 'var(--text-primary)' }}>Ethical Notice:</strong> {summary.aiResult.disclaimer}
          </div>
        </div>

        {/* Right: Explainable Stylometrics & Writing Patterns */}
        <div className="section-card" style={{ marginBottom: 0 }}>
          <div className="section-header">
            <div>
              <div className="section-title">
                <BarChart2 size={18} color="var(--primary-accent)" /> Explainable Writing Patterns
              </div>
              <div className="section-desc">Statistical linguistic &amp; stylometric features</div>
            </div>
          </div>

          <div className="stylometrics-grid">
            <div className="stylometric-box">
              <div className="stylometric-label">Vocabulary Diversity (TTR)</div>
              <div className="stylometric-val">{Math.round(summary.writingStats.typeTokenRatio * 1000) / 10}%</div>
              <div className="stylometric-desc">{summary.writingStats.uniqueWords} unique / {summary.writingStats.totalWords} words</div>
            </div>
            <div className="stylometric-box">
              <div className="stylometric-label">Sentence Burstiness</div>
              <div className="stylometric-val">{Math.round(summary.writingStats.burstiness * 100) / 100}</div>
              <div className="stylometric-desc">Mean: {Math.round(summary.writingStats.avgSentenceLength * 10) / 10} words/sent</div>
            </div>
            <div className="stylometric-box">
              <div className="stylometric-label">Shannon Token Entropy</div>
              <div className="stylometric-val">{Math.round(summary.writingStats.characterEntropy * 100) / 100} bits</div>
              <div className="stylometric-desc">Information density per word</div>
            </div>
            <div className="stylometric-box">
              <div className="stylometric-label">Discourse Transitions</div>
              <div className="stylometric-val">{Object.keys(summary.writingStats.transitionPhraseFrequencies).length}</div>
              <div className="stylometric-desc">Unique transition markers</div>
            </div>
          </div>

          <p style={{ fontSize: '0.85rem', color: 'var(--text-secondary)', lineHeight: '1.6', margin: 0 }}>
            {summary.writingStats.interpretation}
          </p>
        </div>
      </div>

      {/* Side-by-Side Passage Match Inspector */}
      <div className="section-card">
        <div className="section-header" style={{ flexWrap: 'wrap', gap: '0.75rem' }}>
          <div>
            <div className="section-title">
              <Layers size={18} color="var(--primary-accent)" /> Side-by-Side Passage Comparisons
            </div>
            <div className="section-desc">
              Detailed evidence matching with restrained highlights ({filteredMatches.length} matches displayed)
            </div>
          </div>

          {/* Filter Tabs */}
          <div className="match-filter-tabs">
            <button
              className={`btn btn-sm ${filterType === 'ALL' ? 'btn-primary' : 'btn-secondary'}`}
              onClick={() => setFilterType('ALL')}
            >
              All ({summary.matches.length})
            </button>
            <button
              className={`btn btn-sm ${filterType === 'EXACT_MATCH' ? 'btn-primary' : 'btn-secondary'}`}
              onClick={() => setFilterType('EXACT_MATCH')}
            >
              Exact ({exactMatchesCount})
            </button>
            <button
              className={`btn btn-sm ${filterType === 'NGRAM_MATCH' ? 'btn-primary' : 'btn-secondary'}`}
              onClick={() => setFilterType('NGRAM_MATCH')}
            >
              N-Gram ({ngramMatchesCount})
            </button>
            <button
              className={`btn btn-sm ${filterType === 'JACCARD_MATCH' ? 'btn-primary' : 'btn-secondary'}`}
              onClick={() => setFilterType('JACCARD_MATCH')}
            >
              Jaccard ({jaccardMatchesCount})
            </button>
            <button
              className={`btn btn-sm ${filterType === 'SEMANTIC_MATCH' ? 'btn-primary' : 'btn-secondary'}`}
              onClick={() => setFilterType('SEMANTIC_MATCH')}
            >
              Semantic ({semanticMatchesCount})
            </button>
          </div>
        </div>

        {filteredMatches.length === 0 ? (
          <div className="empty-state">
            <p>No passage matches found for the selected filter category.</p>
          </div>
        ) : (
          <div>
            {filteredMatches.map((m) => {
              const pillClass = getPillForMatchType(m.matchType);
              const highlightClass = getHighlightClass(m.matchType);

              return (
                <div key={m.id} className="match-item">
                  <div className="match-item-header">
                    <div style={{ display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                      <span className={`pill ${pillClass}`}>{m.matchType}</span>
                      <strong style={{ fontSize: '0.95rem', color: 'var(--text-primary)' }}>
                        {Math.round(m.score * 100)}% Similarity
                      </strong>
                      <span style={{ fontSize: '0.8rem', color: 'var(--text-secondary)' }}>
                        Strategy: {m.strategyName}
                      </span>
                    </div>
                    <div style={{ fontSize: '0.82rem', color: 'var(--text-secondary)' }}>
                      Reference Source: <strong style={{ color: 'var(--text-primary)' }}>{m.targetDocName}</strong>
                    </div>
                  </div>

                  {/* Side-by-Side Passages */}
                  <div className="side-by-side">
                    <div className="passage-pane">
                      <div className="passage-pane-header">
                        Submitted Passage (Characters {m.queryPassage.startChar}–{m.queryPassage.endChar})
                      </div>
                      <div className="passage-content submitted">
                        <mark className={highlightClass}>
                          {m.queryPassage.originalText}
                        </mark>
                      </div>
                    </div>

                    <div className="passage-pane">
                      <div className="passage-pane-header">
                        Matched Reference Passage ({m.targetDocName})
                      </div>
                      <div className="passage-content reference">
                        {m.targetPassage.originalText}
                      </div>
                    </div>
                  </div>

                  {/* Evidence Tokens */}
                  {m.matchedTokens && m.matchedTokens.length > 0 && (
                    <div className="tokens-row">
                      <span style={{ fontSize: '0.78rem', fontWeight: 600, color: 'var(--text-secondary)', marginRight: '0.25rem' }}>
                        Evidence Tokens:
                      </span>
                      {m.matchedTokens.slice(0, 10).map((t, tidx) => (
                        <span key={tidx} className="token-chip">
                          {t}
                        </span>
                      ))}
                      {m.matchedTokens.length > 10 && (
                        <span className="token-chip">+{m.matchedTokens.length - 10} more</span>
                      )}
                    </div>
                  )}
                </div>
              );
            })}
          </div>
        )}
      </div>

      {/* Discovered Academic Literature (OpenAlex & Crossref) */}
      <div className="section-card">
        <div className="section-header">
          <div>
            <div className="section-title">
              <BookOpen size={18} color="var(--primary-accent)" /> Discovered Academic Literature
            </div>
            <div className="section-desc">OpenAlex &amp; Crossref metadata discovery</div>
          </div>
        </div>

        {summary.discoveredSources.length === 0 ? (
          <p style={{ color: 'var(--text-secondary)', fontSize: '0.88rem', margin: 0 }}>
            No external academic literature sources required verification, or external discovery was offline.
          </p>
        ) : (
          <table className="clean-table">
            <thead>
              <tr>
                <th>Publication Title</th>
                <th>Authors</th>
                <th>Date / DOI</th>
                <th>Provider</th>
                <th>Retrieval Status</th>
              </tr>
            </thead>
            <tbody>
              {summary.discoveredSources.map((s, idx) => (
                <tr key={idx}>
                  <td>
                    <strong style={{ color: 'var(--text-primary)' }}>{s.title}</strong>
                    {s.abstractText && (
                      <div style={{ fontSize: '0.8rem', color: 'var(--text-secondary)', marginTop: '0.25rem', lineHeight: '1.4' }}>
                        {s.abstractText.slice(0, 180)}...
                      </div>
                    )}
                  </td>
                  <td>{s.authors.slice(0, 2).join(', ')}{s.authors.length > 2 ? ' et al.' : ''}</td>
                  <td>
                    <div>{s.publicationDate}</div>
                    {s.doi && (
                      <a
                        href={`https://doi.org/${s.doi}`}
                        target="_blank"
                        rel="noopener noreferrer"
                        style={{ color: 'var(--primary-accent)', fontSize: '0.8rem' }}
                      >
                        {s.doi}
                      </a>
                    )}
                  </td>
                  <td>
                    <span className="pill pill-info">{s.provider}</span>
                  </td>
                  <td>
                    <span style={{ fontSize: '0.82rem', color: 'var(--text-secondary)' }}>
                      {s.retrievalStatus}
                    </span>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        )}
      </div>
    </div>
  );
};
