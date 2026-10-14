import React, { useState } from 'react';
import {
  CheckCircle,
  XCircle,
  AlertTriangle,
  Play,
  RotateCcw,
  Cpu,
  Layers,
  Search,
  BookOpen,
  Download,
  Clock,
  Sparkles,
  Info
} from 'lucide-react';
import { SelfTestReport, SelfTestCase } from '../types';
import { api } from '../api';

interface SelfTestViewProps {
  onBackToDashboard?: () => void;
}

export const SelfTestView: React.FC<SelfTestViewProps> = ({ onBackToDashboard }) => {
  const [report, setReport] = useState<SelfTestReport | null>(null);
  const [isRunning, setIsRunning] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');
  const [showRawJson, setShowRawJson] = useState(false);

  const runTest = async () => {
    setIsRunning(true);
    setErrorMessage('');
    try {
      const data = await api.runSelfTest();
      setReport(data);
    } catch (err: any) {
      setErrorMessage(err.message || 'Self-test execution failed.');
    } finally {
      setIsRunning(false);
    }
  };

  return (
    <div className="view-container">
      {/* Header Card */}
      <div className="section-card">
        <div className="section-header" style={{ borderBottom: 'none', paddingBottom: 0, marginBottom: '1rem', flexWrap: 'wrap', gap: '1rem' }}>
          <div>
            <h1 style={{ fontSize: '1.6rem', fontWeight: 700, color: 'var(--text-primary)', margin: 0 }}>
              System Validation &amp; Self-Test
            </h1>
            <p className="section-desc">
              Execute live validation tests against the C++ similarity engine across identical, partial, paraphrased, and unrelated documents, and inspect AI model mode.
            </p>
          </div>

          <div style={{ display: 'flex', gap: '0.65rem' }}>
            <button
              className="btn btn-primary btn-lg"
              onClick={runTest}
              disabled={isRunning}
            >
              {isRunning ? (
                <>
                  <RotateCcw size={16} className="spin-icon" />
                  <span>Running C++ Engine Tests...</span>
                </>
              ) : (
                <>
                  <Play size={16} />
                  <span>{report ? 'Re-run Self-Test' : 'Run Self-Test Now'}</span>
                </>
              )}
            </button>
          </div>
        </div>
      </div>

      {errorMessage && (
        <div className="alert-box alert-error">
          <XCircle size={18} />
          <span>{errorMessage}</span>
        </div>
      )}

      {/* Running State Banner */}
      {isRunning && (
        <div className="section-card" style={{ textAlign: 'center', padding: '3rem 1.5rem' }}>
          <RotateCcw size={32} color="var(--primary-accent)" className="spin-icon" style={{ marginBottom: '1rem' }} />
          <h2 style={{ fontSize: '1.25rem', fontWeight: 600, color: 'var(--text-primary)', margin: '0 0 0.5rem' }}>
            Running Live Analysis Pipeline
          </h2>
          <p style={{ color: 'var(--text-secondary)', fontSize: '0.9rem', maxWidth: '520px', margin: '0 auto' }}>
            Evaluating test documents through tokenizer, inverted reference index, multithreaded similarity strategies, and AI classifier client...
          </p>
        </div>
      )}

      {/* Report Results */}
      {report && !isRunning && (
        <>
          {/* Summary Status Bar */}
          <div className="results-summary-bar">
            <div>
              <div style={{ display: 'flex', alignItems: 'center', gap: '0.5rem', marginBottom: '0.35rem' }}>
                <span
                  className={`pill ${
                    report.failedCount === 0 ? 'pill-success' : 'pill-error'
                  }`}
                  style={{ fontSize: '0.85rem', fontWeight: 700 }}
                >
                  {report.failedCount === 0 ? 'ALL ENGINE TESTS PASSED' : `${report.failedCount} TESTS FAILED`}
                </span>
                <span style={{ fontSize: '0.82rem', color: 'var(--text-secondary)' }}>
                  Total Tests: <strong>{report.totalTests}</strong> &bull; Passed: <strong style={{ color: 'var(--success)' }}>{report.passedCount}</strong> &bull; Failed: <strong style={{ color: report.failedCount > 0 ? 'var(--error)' : 'inherit' }}>{report.failedCount}</strong>
                </span>
              </div>
              <div className="results-meta">
                Benchmark execution completed in <strong>{report.elapsedMs} ms</strong> &bull; Timestamp: {new Date(report.timestamp * 1000).toLocaleTimeString()}
              </div>
            </div>

            <div style={{ display: 'flex', gap: '0.65rem' }}>
              <button
                className="btn btn-secondary btn-sm"
                onClick={() => setShowRawJson(!showRawJson)}
              >
                {showRawJson ? 'Hide Audit JSON' : 'Inspect Audit JSON'}
              </button>
            </div>
          </div>

          {/* AI Classifier Genuine vs Fallback Audit Card */}
          <div className="section-card" style={{ borderLeft: `4px solid ${report.aiServiceAudit.online ? (report.aiServiceAudit.isPretrainedModel ? 'var(--success)' : 'var(--warning)') : 'var(--error)'}` }}>
            <div className="section-header" style={{ marginBottom: '0.75rem', paddingBottom: '0.5rem' }}>
              <div className="section-title">
                <Cpu size={18} color="var(--primary-accent)" />
                <span>AI Classification Service Verification</span>
              </div>
              <div>
                {report.aiServiceAudit.online ? (
                  report.aiServiceAudit.isPretrainedModel ? (
                    <span className="pill pill-success" style={{ fontSize: '0.82rem' }}>
                      <span className="pill-dot"></span> Genuine Pretrained Neural Model
                    </span>
                  ) : (
                    <span className="pill pill-warning" style={{ fontSize: '0.82rem' }}>
                      <span className="pill-dot"></span> Calibrated Statistical Fallback Mode
                    </span>
                  )
                ) : (
                  <span className="pill pill-error" style={{ fontSize: '0.82rem' }}>
                    <span className="pill-dot"></span> Microservice Offline
                  </span>
                )}
              </div>
            </div>

            <div style={{ fontSize: '0.88rem', color: 'var(--text-secondary)', lineHeight: '1.6' }}>
              <p style={{ margin: '0 0 0.5rem' }}>
                <strong>Configured Model:</strong> <code>{report.aiServiceAudit.modelName}</code> &bull;{' '}
                <strong>Inference Mode:</strong> <code>{report.aiServiceAudit.mode}</code>
              </p>
              {report.aiServiceAudit.online ? (
                report.aiServiceAudit.isPretrainedModel ? (
                  <p style={{ margin: 0, color: 'var(--text-primary)' }}>
                    &#10004; The Hugging Face transformer model is genuinely loaded into memory and executing neural inference on CPU/GPU.
                  </p>
                ) : (
                  <div style={{ backgroundColor: 'var(--bg-secondary)', padding: '0.75rem 1rem', borderRadius: '6px', borderLeft: '3px solid var(--warning)' }}>
                    <strong>Fallback Operational Notice:</strong> The Python microservice (port 8000) is running and serving calibrated statistical predictions (burstiness, Shannon entropy, vocabulary density). The neural model weights are offline because <code>transformers</code> / <code>torch</code> are not installed in the Python environment.
                    <div style={{ marginTop: '0.35rem', fontSize: '0.82rem' }}>
                      <em>To load genuine Hugging Face pretrained weights:</em> run <code>pip install transformers torch</code> and restart <code>python ai_service/app.py</code>.
                    </div>
                  </div>
                )
              ) : (
                <div style={{ backgroundColor: 'var(--error-light)', padding: '0.75rem 1rem', borderRadius: '6px', color: 'var(--error)' }}>
                  The Python AI microservice is not reachable on port 8000. Start it by executing <code>python ai_service/app.py</code> in a terminal.
                </div>
              )}
            </div>
          </div>

          {/* Test Cases Comparison Table */}
          <div className="section-card">
            <div className="section-header">
              <div>
                <div className="section-title">
                  <CheckCircle size={18} color="var(--primary-accent)" />
                  <span>C++ Similarity Engine Test Results</span>
                </div>
                <div className="section-desc">
                  Verification of actual engine similarity scores against expected mathematical bounds
                </div>
              </div>
            </div>

            <table className="clean-table">
              <thead>
                <tr>
                  <th style={{ width: '10%' }}>Status</th>
                  <th style={{ width: '25%' }}>Test Category</th>
                  <th style={{ width: '25%' }}>Expected Criteria</th>
                  <th style={{ width: '25%' }}>Actual Engine Score</th>
                  <th style={{ width: '15%', textAlign: 'right' }}>Passages</th>
                </tr>
              </thead>
              <tbody>
                {report.tests.map((t) => (
                  <tr key={t.id}>
                    <td>
                      {t.passed ? (
                        <span className="pill pill-success" style={{ fontWeight: 700 }}>
                          PASS
                        </span>
                      ) : (
                        <span className="pill pill-error" style={{ fontWeight: 700 }}>
                          FAIL
                        </span>
                      )}
                    </td>
                    <td>
                      <strong style={{ color: 'var(--text-primary)', display: 'block' }}>{t.name}</strong>
                      <span style={{ fontSize: '0.8rem', color: 'var(--text-secondary)' }}>
                        {t.description}
                      </span>
                    </td>
                    <td>
                      <code style={{ fontSize: '0.82rem' }}>{t.expected}</code>
                    </td>
                    <td>
                      {t.overallScore !== undefined ? (
                        <div>
                          <div style={{ display: 'flex', alignItems: 'center', gap: '0.45rem', marginBottom: '0.2rem' }}>
                            <span style={{ fontWeight: 700, fontSize: '0.95rem', color: 'var(--text-primary)' }}>
                              Overall: {t.overallScore}%
                            </span>
                            <span style={{ fontSize: '0.8rem', color: 'var(--text-secondary)' }}>
                              (Exact: {t.exactScore}%)
                            </span>
                          </div>
                          <div style={{ fontSize: '0.75rem', color: 'var(--text-muted)' }}>
                            3-Gram: {t.ngramScore}% &bull; Jaccard: {t.jaccardScore}% &bull; Cosine: {t.semanticScore}%
                          </div>
                        </div>
                      ) : (
                        <span style={{ fontSize: '0.82rem', color: 'var(--text-secondary)' }}>
                          {t.details || t.mode}
                        </span>
                      )}
                    </td>
                    <td style={{ textAlign: 'right' }}>
                      {t.matchesFound !== undefined ? (
                        <span className="pill pill-info">{t.matchesFound} matched</span>
                      ) : (
                        <span style={{ fontSize: '0.8rem', color: 'var(--text-muted)' }}>N/A</span>
                      )}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>

          {/* Raw JSON Audit Modal/Box */}
          {showRawJson && (
            <div className="section-card">
              <div className="section-header">
                <div className="section-title">
                  <span>Raw Engine Audit JSON</span>
                </div>
              </div>
              <pre style={{
                backgroundColor: 'var(--bg-secondary)',
                padding: '1rem',
                borderRadius: '6px',
                fontSize: '0.8rem',
                maxHeight: '320px',
                overflowY: 'auto',
                border: '1px solid var(--border-color)',
                fontFamily: 'monospace'
              }}>
                {JSON.stringify(report, null, 2)}
              </pre>
            </div>
          )}
        </>
      )}

      {/* Initial Call-to-Action if no test run yet */}
      {!report && !isRunning && (
        <div className="section-card" style={{ textAlign: 'center', padding: '3.5rem 1.5rem' }}>
          <Sparkles size={36} color="var(--primary-accent)" style={{ marginBottom: '1rem' }} />
          <h2 style={{ fontSize: '1.35rem', fontWeight: 700, color: 'var(--text-primary)', marginBottom: '0.5rem' }}>
            Ready to Validate Document Tracer
          </h2>
          <p style={{ color: 'var(--text-secondary)', fontSize: '0.92rem', maxWidth: '580px', margin: '0 auto 1.75rem', lineHeight: '1.6' }}>
            Click the button below to initiate an automated self-test run. The test suite loads four distinct documents (identical text, partial overlap, semantic paraphrase, and unrelated negative control) and executes them through the genuine C++ engine.
          </p>
          <button className="btn btn-primary btn-lg" onClick={runTest}>
            <Play size={16} />
            <span>Launch Engine Self-Test</span>
          </button>
        </div>
      )}
    </div>
  );
};
