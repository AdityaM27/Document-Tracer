import React from 'react';
import {
  FileText,
  ArrowRight,
  ExternalLink,
  BookOpen,
  Cpu,
  Layers,
  Sparkles,
  CheckCircle
} from 'lucide-react';
import type { SystemHealth, ReportItem, ReferenceDoc } from '../types';

interface DashboardViewProps {
  health: SystemHealth | null;
  references: ReferenceDoc[];
  reports: ReportItem[];
  setActiveTab: (tab: string) => void;
}

export const DashboardView: React.FC<DashboardViewProps> = ({
  health,
  references,
  reports,
  setActiveTab,
}) => {
  return (
    <div className="view-container">
      {/* Centered Clean Introductory Section */}
      <section className="dashboard-hero">
        <h1 className="hero-title">Document Intelligence</h1>
        <p className="hero-description">
          Analyze document similarity, discover potential source matches,
          and examine AI-writing indicators.
        </p>
        <div className="hero-cta" style={{ display: 'inline-flex', gap: '0.75rem', flexWrap: 'wrap', justifyContent: 'center' }}>
          <button
            className="btn btn-primary btn-lg"
            onClick={() => setActiveTab('upload')}
          >
            <span>Start New Analysis</span>
            <ArrowRight size={17} />
          </button>
          <button
            className="btn btn-secondary btn-lg"
            onClick={() => setActiveTab('selftest')}
          >
            <CheckCircle size={17} color="var(--primary-accent)" />
            <span>Run Self-Test</span>
          </button>
        </div>
      </section>

      {/* Compact Statistics Grid (Real Backend Data) */}
      <div className="stats-grid">
        <div className="stat-card">
          <div className="stat-label">Indexed References</div>
          <div className="stat-value">{references.length}</div>
          <div className="stat-subtext">Local reference documents ready for comparison</div>
        </div>

        <div className="stat-card">
          <div className="stat-label">Completed Analyses</div>
          <div className="stat-value">{reports.length}</div>
          <div className="stat-subtext">Document audits stored in local output directory</div>
        </div>

        <div className="stat-card">
          <div className="stat-label">Analysis Engine</div>
          <div className="stat-value" style={{ fontSize: '1.35rem' }}>
            C++17 Engine
          </div>
          <div className="stat-subtext">Multithreaded Producer-Consumer pool (port 8080)</div>
        </div>

        <div className="stat-card">
          <div className="stat-label">AI Classifier Status</div>
          <div
            className="stat-value"
            style={{
              fontSize: '1.35rem',
              color: health?.aiHfLoaded ? 'var(--success)' : (health?.aiServiceOnline ? 'var(--warning)' : 'var(--text-secondary)'),
            }}
          >
            {health?.aiHfLoaded ? 'Neural (HF)' : (health?.aiServiceOnline ? 'Statistical Fallback' : 'Offline')}
          </div>
          <div className="stat-subtext">
            {health?.aiHfLoaded ? 'Pretrained RoBERTa weights active' : (health?.aiServiceOnline ? 'Stylometric heuristics' : 'Service disconnected')}
          </div>
        </div>
      </div>

      {/* Recent Analyses Section (Clean Table) */}
      <div className="section-card">
        <div className="section-header">
          <div>
            <h2 className="section-title">
              <FileText size={18} /> Recent Analyses
            </h2>
            <div className="section-desc">Audit summaries from the most recent document runs.</div>
          </div>
          {reports.length > 0 && (
            <button
              className="btn btn-secondary btn-sm"
              onClick={() => setActiveTab('reports')}
            >
              View All ({reports.length})
            </button>
          )}
        </div>

        {reports.length === 0 ? (
          <div style={{ textAlign: 'center', padding: '2.5rem 1rem', color: 'var(--text-secondary)' }}>
            <p style={{ marginBottom: '1rem' }}>No document analyses recorded yet.</p>
            <button
              className="btn btn-secondary btn-sm"
              onClick={() => setActiveTab('upload')}
            >
              Analyze Your First Document
            </button>
          </div>
        ) : (
          <table className="clean-table">
            <thead>
              <tr>
                <th>Document</th>
                <th>Analyzed Date</th>
                <th>Overall Overlap</th>
                <th>AI Classification</th>
                <th style={{ textAlign: 'right' }}>Actions</th>
              </tr>
            </thead>
            <tbody>
              {reports.slice(0, 5).map((r) => {
                const pct = Math.round(r.overallSimilarity * 100);
                const badgeStyle =
                  pct > 50
                    ? { background: 'var(--error-light)', color: 'var(--error)' }
                    : pct > 25
                    ? { background: 'var(--warning-light)', color: 'var(--warning)' }
                    : { background: 'var(--success-light)', color: 'var(--success)' };

                return (
                  <tr key={r.id}>
                    <td>
                      <span style={{ fontWeight: 600, color: 'var(--text-primary)' }}>
                        {r.filename}
                      </span>
                    </td>
                    <td style={{ color: 'var(--text-secondary)' }}>{r.timestamp}</td>
                    <td>
                      <span className="clean-badge" style={badgeStyle}>
                        {pct}% similarity
                      </span>
                    </td>
                    <td>
                      <span className="pill pill-info">{r.aiLabel || 'Evaluated'}</span>
                    </td>
                    <td style={{ textAlign: 'right' }}>
                      <a
                        href={`/api/reports/${r.id}/html`}
                        target="_blank"
                        rel="noopener noreferrer"
                        className="btn btn-secondary btn-sm"
                      >
                        <ExternalLink size={13} />
                        <span>View HTML</span>
                      </a>
                    </td>
                  </tr>
                );
              })}
            </tbody>
          </table>
        )}
      </div>
    </div>
  );
};
