import React, { useEffect, useState } from 'react';
import {
  CheckCircle,
  Clock,
  Cpu,
  Layers,
  Search,
  BookOpen,
  FileCheck,
  Loader2
} from 'lucide-react';
import { AnalysisProgress, AnalysisSummary } from '../types';
import { api } from '../api';

interface ProgressViewProps {
  submissionId: string;
  onCompleted: (summary: AnalysisSummary) => void;
  onError: (msg: string) => void;
}

const STAGES = [
  { key: 'EXTRACTION', label: 'File Extraction & Validation', icon: FileCheck },
  { key: 'PREPROCESSING', label: 'Text Normalization & Token Segmentation', icon: Layers },
  { key: 'LOCAL_MATCHING', label: 'Local Reference Matching (Multithreaded)', icon: Cpu },
  { key: 'ACADEMIC_SEARCH', label: 'Scholarly Literature Discovery (OpenAlex & Crossref)', icon: BookOpen },
  { key: 'AI_ANALYSIS', label: 'Pretrained AI-Writing Classification', icon: Search },
  { key: 'WRITING_PATTERNS', label: 'Stylometric Writing Pattern Analysis', icon: Cpu },
  { key: 'REPORT_GENERATION', label: 'Report Serialization & Audit Assembly', icon: CheckCircle },
];

export const ProgressView: React.FC<ProgressViewProps> = ({
  submissionId,
  onCompleted,
  onError,
}) => {
  const [progress, setProgress] = useState<AnalysisProgress | null>(null);

  useEffect(() => {
    let interval: any = null;
    let failureCount = 0;

    const pollStatus = async () => {
      try {
        const p = await api.getAnalysisStatus(submissionId);
        setProgress(p);

        if (p.hasError) {
          if (interval) clearInterval(interval);
          onError(p.errorMessage || 'An error occurred during analysis.');
        } else if (p.isCompleted || p.percent >= 100) {
          if (interval) clearInterval(interval);
          const result = await api.getAnalysisResult(submissionId);
          onCompleted(result);
        }
      } catch (err: any) {
        failureCount++;
        if (failureCount > 15) {
          if (interval) clearInterval(interval);
          onError(err.message || `Unable to retrieve analysis status for task: ${submissionId}`);
        }
      }
    };

    pollStatus();
    interval = setInterval(pollStatus, 400);

    return () => {
      if (interval) clearInterval(interval);
    };
  }, [submissionId, onCompleted, onError]);

  const currentPercent = progress && typeof progress.percent === 'number' ? progress.percent : 5;

  return (
    <div className="view-container">
      <div className="progress-box">
        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'center', gap: '0.6rem', marginBottom: '0.5rem' }}>
          <Loader2 className="spin-icon" size={24} color="var(--primary-accent)" />
          <h2 style={{ fontSize: '1.4rem', fontWeight: 700, color: 'var(--text-primary)', margin: 0 }}>
            Analyzing Document
          </h2>
        </div>
        <p style={{ color: 'var(--text-secondary)', fontSize: '0.88rem', margin: '0 0 1.5rem' }}>
          Submission ID: <code>{submissionId}</code>
        </p>

        {/* Minimal Progress Bar */}
        <div className="progress-meter">
          <div
            className="progress-fill"
            style={{ width: `${currentPercent}%` }}
          />
        </div>
        <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.82rem', color: 'var(--text-secondary)' }}>
          <span>Current Stage: <strong style={{ color: 'var(--text-primary)' }}>{progress?.stage || 'INITIALIZING'}</strong></span>
          <span style={{ fontWeight: 600, color: 'var(--primary-accent)' }}>{currentPercent}%</span>
        </div>

        {progress?.message && (
          <p style={{ marginTop: '1.25rem', fontSize: '0.88rem', color: 'var(--text-secondary)', fontStyle: 'italic' }}>
            "{progress.message}"
          </p>
        )}

        {/* Clean Minimal Stage Checklist */}
        <div className="stages-clean">
          {STAGES.map((s, idx) => {
            const Icon = s.icon;
            const isDone = currentPercent > (idx + 1) * 14 || progress?.isCompleted;
            const isCurrent = progress?.stage === s.key;

            return (
              <div
                key={s.key}
                className={`stage-clean-item ${isCurrent ? 'active' : ''}`}
              >
                <div style={{ display: 'flex', alignItems: 'center', gap: '0.65rem' }}>
                  {isDone ? (
                    <CheckCircle size={16} color="var(--success)" />
                  ) : isCurrent ? (
                    <Clock size={16} color="var(--primary-accent)" />
                  ) : (
                    <Icon size={16} color="var(--text-muted)" />
                  )}
                  <span style={{ color: isDone || isCurrent ? 'var(--text-primary)' : 'var(--text-secondary)' }}>
                    {s.label}
                  </span>
                </div>

                <div>
                  {isCurrent && <span className="pill pill-info">In Progress</span>}
                  {isDone && <span className="pill pill-success">Completed</span>}
                </div>
              </div>
            );
          })}
        </div>
      </div>
    </div>
  );
};
