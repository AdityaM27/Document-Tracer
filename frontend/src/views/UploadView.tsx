import React, { useState } from 'react';
import {
  Upload,
  FileText,
  SlidersHorizontal,
  ChevronDown,
  ChevronUp,
  AlertCircle,
  ArrowRight,
  Check
} from 'lucide-react';
import type { AnalysisOptions, AnalysisSummary } from '../types';
import { api } from '../api';

interface UploadViewProps {
  onStartAnalysis: (summary: AnalysisSummary) => void;
  onAnalysisInitiated: (submissionId: string) => void;
}

export const UploadView: React.FC<UploadViewProps> = ({
  onStartAnalysis,
  onAnalysisInitiated,
}) => {
  const [selectedFile, setSelectedFile] = useState<File | null>(null);
  const [pastedText, setPastedText] = useState('');
  const [useRawText, setUseRawText] = useState(false);
  const [customFilename, setCustomFilename] = useState('submission.txt');
  const [isLoading, setIsLoading] = useState(false);
  const [errorMessage, setErrorMessage] = useState('');
  const [showSettings, setShowSettings] = useState(false);

  // Analysis Options
  const [options, setOptions] = useState<AnalysisOptions>({
    enableExactMatch: true,
    enableNGram: true,
    enableJaccard: true,
    enableSemantic: true,
    enableAcademicSearch: true,
    enableWebSearch: false,
    enableAIClassifier: true,
    enableWritingPatterns: true,
    ngramSize: 3,
    exactThreshold: 0.80,
    ngramThreshold: 0.40,
    jaccardThreshold: 0.35,
    semanticThreshold: 0.65,
    maxConcurrentThreads: 4,
  });

  const handleFileChange = (e: React.ChangeEvent<HTMLInputElement>) => {
    if (e.target.files && e.target.files[0]) {
      const f = e.target.files[0];
      setSelectedFile(f);
      setErrorMessage('');
    }
  };

  const handleDrop = (e: React.DragEvent<HTMLDivElement>) => {
    e.preventDefault();
    if (e.dataTransfer.files && e.dataTransfer.files[0]) {
      setSelectedFile(e.dataTransfer.files[0]);
      setErrorMessage('');
    }
  };

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    setErrorMessage('');

    if (!useRawText && !selectedFile) {
      setErrorMessage('Please select a document file (.txt, .docx, .pdf) or paste document text.');
      return;
    }

    if (useRawText && pastedText.trim().length === 0) {
      setErrorMessage('Pasted document text cannot be empty.');
      return;
    }

    setIsLoading(true);

    try {
      let uploadResult;
      if (useRawText) {
        uploadResult = await api.uploadText(pastedText, customFilename);
      } else if (selectedFile) {
        uploadResult = await api.uploadFile(selectedFile);
      }

      if (!uploadResult) {
        throw new Error('Upload failed');
      }

      onAnalysisInitiated(uploadResult.docId);

      // Trigger analysis (runs asynchronously in background; ProgressView polls status)
      const res = await api.startAnalysis(uploadResult.filepath, uploadResult.docId, options);
      if (res && res.overallSimilarityScore !== undefined) {
        onStartAnalysis(res);
      }
    } catch (err: any) {
      setErrorMessage(err.message || 'Analysis failed. Please check inputs.');
      setIsLoading(false);
    }
  };

  return (
    <div className="view-container upload-container">
      <div style={{ textAlign: 'center', marginBottom: '2rem' }}>
        <h1 style={{ fontSize: '1.75rem', fontWeight: 700, color: 'var(--text-primary)', marginBottom: '0.4rem' }}>
          Analyze Document
        </h1>
        <p style={{ color: 'var(--text-secondary)', fontSize: '0.95rem' }}>
          Upload a file or paste text to perform comprehensive similarity matching and stylometric analysis.
        </p>
      </div>

      {errorMessage && (
        <div className="alert-box alert-error">
          <AlertCircle size={16} />
          <span>{errorMessage}</span>
        </div>
      )}

      <form onSubmit={handleSubmit}>
        <div className="section-card" style={{ padding: '2rem' }}>
          {/* Mode Switcher */}
          <div style={{ display: 'flex', justifyContent: 'center', marginBottom: '1.5rem' }}>
            <div className="upload-mode-toggle">
              <button
                type="button"
                className={`mode-tab ${!useRawText ? 'active' : ''}`}
                onClick={() => setUseRawText(false)}
              >
                Upload Document
              </button>
              <button
                type="button"
                className={`mode-tab ${useRawText ? 'active' : ''}`}
                onClick={() => setUseRawText(true)}
              >
                Paste Text
              </button>
            </div>
          </div>

          {!useRawText ? (
            <div
              className="upload-dropzone"
              onDragOver={(e) => e.preventDefault()}
              onDrop={handleDrop}
            >
              <Upload size={36} className="dropzone-icon" />
              <div className="dropzone-title">
                {selectedFile ? selectedFile.name : 'Choose a file or drag & drop it here'}
              </div>
              <div className="dropzone-instructions">
                {selectedFile
                  ? `${(selectedFile.size / 1024).toFixed(1)} KB — Click to choose a different file`
                  : 'Supports TXT, DOCX, PDF, and code files up to 15 MB'}
              </div>
              <input
                type="file"
                className="file-input-hidden"
                accept=".txt,.docx,.pdf,.md,.cpp,.h"
                onChange={handleFileChange}
              />
            </div>
          ) : (
            <div>
              <div style={{ marginBottom: '1rem' }}>
                <label style={{ display: 'block', fontSize: '0.82rem', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '0.35rem' }}>
                  Document Name
                </label>
                <input
                  type="text"
                  className="form-input-clean"
                  value={customFilename}
                  onChange={(e) => setCustomFilename(e.target.value)}
                  placeholder="e.g. essay_submission.txt"
                />
              </div>
              <div>
                <label style={{ display: 'block', fontSize: '0.82rem', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '0.35rem' }}>
                  Document Text
                </label>
                <textarea
                  className="text-paste-area"
                  rows={9}
                  value={pastedText}
                  onChange={(e) => setPastedText(e.target.value)}
                  placeholder="Paste document text here to examine..."
                />
              </div>
            </div>
          )}

          {/* Organized Analysis Settings Panel */}
          <div className="settings-panel">
            <div
              className="settings-toggle-header"
              onClick={() => setShowSettings(!showSettings)}
            >
              <span style={{ display: 'flex', alignItems: 'center', gap: '0.5rem' }}>
                <SlidersHorizontal size={15} color="var(--primary-accent)" />
                <span>Analysis Parameters &amp; Thresholds</span>
              </span>
              <span style={{ display: 'flex', alignItems: 'center', gap: '0.35rem', fontSize: '0.8rem', color: 'var(--text-secondary)' }}>
                <span>{showSettings ? 'Hide Settings' : 'Configure Parameters'}</span>
                {showSettings ? <ChevronUp size={16} /> : <ChevronDown size={16} />}
              </span>
            </div>

            {showSettings && (
              <div className="settings-body">
                <div className="setting-row">
                  <div className="setting-info">
                    <label className="setting-label">
                      <input
                        type="checkbox"
                        checked={options.enableExactMatch}
                        onChange={(e) => setOptions({ ...options, enableExactMatch: e.target.checked })}
                      />
                      <span>Exact Phrase Matching</span>
                    </label>
                    <div className="setting-description">Flags contiguous identical sequences of words.</div>
                  </div>
                  <div className="slider-container">
                    <input
                      type="range"
                      min="0.5"
                      max="1.0"
                      step="0.05"
                      className="clean-slider"
                      value={options.exactThreshold}
                      onChange={(e) => setOptions({ ...options, exactThreshold: parseFloat(e.target.value) })}
                    />
                    <span style={{ fontSize: '0.82rem', fontWeight: 600, minWidth: '40px' }}>
                      {Math.round(options.exactThreshold * 100)}%
                    </span>
                  </div>
                </div>

                <div className="setting-row">
                  <div className="setting-info">
                    <label className="setting-label">
                      <input
                        type="checkbox"
                        checked={options.enableNGram}
                        onChange={(e) => setOptions({ ...options, enableNGram: e.target.checked })}
                      />
                      <span>N-Gram Phrase Overlap ({options.ngramSize}-grams)</span>
                    </label>
                    <div className="setting-description">Measures sliding multi-word tuple containment.</div>
                  </div>
                  <div className="slider-container">
                    <input
                      type="range"
                      min="0.2"
                      max="0.8"
                      step="0.05"
                      className="clean-slider"
                      value={options.ngramThreshold}
                      onChange={(e) => setOptions({ ...options, ngramThreshold: parseFloat(e.target.value) })}
                    />
                    <span style={{ fontSize: '0.82rem', fontWeight: 600, minWidth: '40px' }}>
                      {Math.round(options.ngramThreshold * 100)}%
                    </span>
                  </div>
                </div>

                <div className="setting-row">
                  <div className="setting-info">
                    <label className="setting-label">
                      <input
                        type="checkbox"
                        checked={options.enableJaccard}
                        onChange={(e) => setOptions({ ...options, enableJaccard: e.target.checked })}
                      />
                      <span>Jaccard Lexical Overlap</span>
                    </label>
                    <div className="setting-description">Calculates shared vocabulary over total vocabulary union.</div>
                  </div>
                  <div className="slider-container">
                    <input
                      type="range"
                      min="0.2"
                      max="0.8"
                      step="0.05"
                      className="clean-slider"
                      value={options.jaccardThreshold}
                      onChange={(e) => setOptions({ ...options, jaccardThreshold: parseFloat(e.target.value) })}
                    />
                    <span style={{ fontSize: '0.82rem', fontWeight: 600, minWidth: '40px' }}>
                      {Math.round(options.jaccardThreshold * 100)}%
                    </span>
                  </div>
                </div>

                <div className="setting-row">
                  <div className="setting-info">
                    <label className="setting-label">
                      <input
                        type="checkbox"
                        checked={options.enableSemantic}
                        onChange={(e) => setOptions({ ...options, enableSemantic: e.target.checked })}
                      />
                      <span>Semantic Cosine Similarity</span>
                    </label>
                    <div className="setting-description">Identifies potential paraphrasing via vector embeddings.</div>
                  </div>
                  <div className="slider-container">
                    <input
                      type="range"
                      min="0.5"
                      max="0.9"
                      step="0.05"
                      className="clean-slider"
                      value={options.semanticThreshold}
                      onChange={(e) => setOptions({ ...options, semanticThreshold: parseFloat(e.target.value) })}
                    />
                    <span style={{ fontSize: '0.82rem', fontWeight: 600, minWidth: '40px' }}>
                      {Math.round(options.semanticThreshold * 100)}%
                    </span>
                  </div>
                </div>

                <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(220px, 1fr))', gap: '1rem', paddingTop: '0.75rem', borderTop: '1px solid var(--border-light)' }}>
                  <label className="setting-label">
                    <input
                      type="checkbox"
                      checked={options.enableAcademicSearch}
                      onChange={(e) => setOptions({ ...options, enableAcademicSearch: e.target.checked })}
                    />
                    <span>Academic Literature Search</span>
                  </label>
                  <label className="setting-label">
                    <input
                      type="checkbox"
                      checked={options.enableAIClassifier}
                      onChange={(e) => setOptions({ ...options, enableAIClassifier: e.target.checked })}
                    />
                    <span>Pretrained AI Classifier</span>
                  </label>
                  <label className="setting-label">
                    <input
                      type="checkbox"
                      checked={options.enableWritingPatterns}
                      onChange={(e) => setOptions({ ...options, enableWritingPatterns: e.target.checked })}
                    />
                    <span>Stylometric Analysis</span>
                  </label>
                </div>
              </div>
            )}
          </div>

          {/* Primary Action Button */}
          <div style={{ marginTop: '2rem' }}>
            <button
              type="submit"
              className="btn btn-primary btn-lg btn-block"
              disabled={isLoading}
            >
              {isLoading ? (
                <span>Running C++ Multithreaded Pipeline...</span>
              ) : (
                <>
                  <span>Analyze Document</span>
                  <ArrowRight size={17} />
                </>
              )}
            </button>
          </div>
        </div>
      </form>
    </div>
  );
};
