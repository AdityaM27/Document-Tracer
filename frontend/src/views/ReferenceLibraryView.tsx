import React, { useState } from 'react';
import {
  BookOpen,
  Upload,
  Trash2,
  RefreshCw,
  Search,
  CheckCircle,
  AlertCircle,
  Plus,
  FileText,
  Sparkles,
  ChevronDown,
  ChevronUp
} from 'lucide-react';
import { ReferenceDoc } from '../types';
import { api } from '../api';

interface ReferenceLibraryViewProps {
  references: ReferenceDoc[];
  onRefresh: () => void;
}

export const ReferenceLibraryView: React.FC<ReferenceLibraryViewProps> = ({
  references,
  onRefresh,
}) => {
  const [isUploading, setIsUploading] = useState(false);
  const [isReindexing, setIsReindexing] = useState(false);
  const [isSeeding, setIsSeeding] = useState(false);
  const [statusMessage, setStatusMessage] = useState('');
  const [errorMessage, setErrorMessage] = useState('');
  const [searchQuery, setSearchQuery] = useState('');

  // Add Reference Panel State
  const [showAddPanel, setShowAddPanel] = useState(false);
  const [addMode, setAddMode] = useState<'FILE' | 'TEXT'>('FILE');
  const [textTitle, setTextTitle] = useState('');
  const [textContent, setTextContent] = useState('');
  const processFile = async (file: File) => {
    if (file.size === 0) {
      setErrorMessage('The selected file is empty (0 bytes). Please choose a file containing text.');
      return;
    }

    const allowedExts = ['.pdf', '.docx', '.txt', '.md', '.cpp', '.h', '.c', '.py', '.json', '.csv'];
    const ext = file.name.substring(file.name.lastIndexOf('.')).toLowerCase();
    if (!allowedExts.includes(ext)) {
      setErrorMessage(`Unsupported file format "${ext}". Supported formats: PDF (.pdf), Word (.docx), and text files (.txt, .md).`);
      return;
    }

    setIsUploading(true);
    setStatusMessage('');
    setErrorMessage('');
    try {
      const res = await api.uploadReference(file);
      setStatusMessage(`Successfully uploaded and indexed "${res.filename || file.name}".`);
      setShowAddPanel(false);
      onRefresh();
    } catch (err: any) {
      setErrorMessage(err.message || 'Failed to upload reference document.');
    } finally {
      setIsUploading(false);
    }
  };

  const handleFileUpload = async (e: React.ChangeEvent<HTMLInputElement>) => {
    if (e.target.files && e.target.files[0]) {
      const file = e.target.files[0];
      await processFile(file);
      e.target.value = '';
    }
  };

  const handleFileDrop = async (e: React.DragEvent<HTMLDivElement>) => {
    e.preventDefault();
    if (e.dataTransfer.files && e.dataTransfer.files[0]) {
      const file = e.dataTransfer.files[0];
      await processFile(file);
    }
  };

  const handleTextSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!textContent.trim()) {
      setErrorMessage('Reference text content cannot be empty.');
      return;
    }
    const title = textTitle.trim() || 'reference_custom.txt';

    setIsUploading(true);
    setStatusMessage('');
    setErrorMessage('');
    try {
      const res = await api.uploadReferenceText(title, textContent);
      setStatusMessage(`Successfully indexed reference document "${res.filename || title}".`);
      setTextTitle('');
      setTextContent('');
      setShowAddPanel(false);
      onRefresh();
    } catch (err: any) {
      setErrorMessage(err.message || 'Failed to add reference document.');
    } finally {
      setIsUploading(false);
    }
  };

  const handleSeedCorpus = async () => {
    setIsSeeding(true);
    setStatusMessage('');
    setErrorMessage('');
    try {
      const count = await api.seedReferences();
      setStatusMessage(`Seeded benchmark reference documents (OOP, OS, Data Structures). Indexed ${count} documents.`);
      onRefresh();
    } catch (err: any) {
      setErrorMessage(err.message || 'Failed to seed reference corpus.');
    } finally {
      setIsSeeding(false);
    }
  };

  const handleDelete = async (docId: string, filename: string) => {
    if (!window.confirm(`Are you sure you want to remove "${filename}" from the reference library?`)) {
      return;
    }
    try {
      await api.deleteReference(docId);
      setStatusMessage(`Removed "${filename}" from reference index.`);
      onRefresh();
    } catch (err: any) {
      setErrorMessage(err.message || 'Failed to delete reference document.');
    }
  };

  const handleReindex = async () => {
    setIsReindexing(true);
    setStatusMessage('');
    setErrorMessage('');
    try {
      const count = await api.reindexReferences();
      setStatusMessage(`Rebuilt inverted reference index. Re-indexed ${count} documents.`);
      onRefresh();
    } catch (err: any) {
      setErrorMessage(err.message || 'Failed to rebuild index.');
    } finally {
      setIsReindexing(false);
    }
  };

  const filteredReferences = references.filter((r) =>
    r.filename.toLowerCase().includes(searchQuery.toLowerCase()) ||
    r.id.toLowerCase().includes(searchQuery.toLowerCase())
  );

  return (
    <div className="view-container">
      {/* Header & Controls */}
      <div className="section-card">
        <div className="section-header" style={{ borderBottom: 'none', paddingBottom: 0, marginBottom: '1.25rem', flexWrap: 'wrap', gap: '1rem' }}>
          <div>
            <h1 style={{ fontSize: '1.6rem', fontWeight: 700, color: 'var(--text-primary)', margin: 0 }}>
              Reference Library
            </h1>
            <p className="section-desc">
              Manage indexed reference documents used for similarity comparison and inverted index candidate retrieval.
            </p>
          </div>

          <div style={{ display: 'flex', gap: '0.65rem', flexWrap: 'wrap' }}>
            <button
              className="btn btn-secondary"
              onClick={handleSeedCorpus}
              disabled={isSeeding}
              title="Ensure standard reference documents (OOP, OS, Data Structures) are populated"
            >
              <Sparkles size={15} color="var(--primary-accent)" />
              <span>{isSeeding ? 'Seeding...' : 'Restore Benchmark Corpus'}</span>
            </button>

            <button
              className="btn btn-secondary"
              onClick={handleReindex}
              disabled={isReindexing}
              title="Re-parse and rebuild inverted index from data/references"
            >
              <RefreshCw size={15} className={isReindexing ? 'spin-icon' : ''} />
              <span>{isReindexing ? 'Rebuilding...' : 'Rebuild Index'}</span>
            </button>

            <button
              className="btn btn-primary"
              onClick={() => setShowAddPanel(!showAddPanel)}
            >
              <Plus size={15} />
              <span>{showAddPanel ? 'Close Panel' : 'Add Reference'}</span>
              {showAddPanel ? <ChevronUp size={15} /> : <ChevronDown size={15} />}
            </button>
          </div>
        </div>

        {/* Expandable Add Reference Panel */}
        {showAddPanel && (
          <div style={{
            backgroundColor: 'var(--bg-secondary)',
            border: '1px solid var(--border-color)',
            borderRadius: '8px',
            padding: '1.25rem',
            marginBottom: '1.25rem'
          }}>
            <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '1rem' }}>
              <strong style={{ fontSize: '0.95rem', color: 'var(--text-primary)' }}>
                Add New Reference Document
              </strong>
              <div className="upload-mode-toggle" style={{ margin: 0 }}>
                <button
                  type="button"
                  className={`mode-tab ${addMode === 'FILE' ? 'active' : ''}`}
                  onClick={() => setAddMode('FILE')}
                >
                  Upload File
                </button>
                <button
                  type="button"
                  className={`mode-tab ${addMode === 'TEXT' ? 'active' : ''}`}
                  onClick={() => setAddMode('TEXT')}
                >
                  Paste Text
                </button>
              </div>
            </div>

            {addMode === 'FILE' ? (
              <div
                style={{ textAlign: 'center', padding: '1.75rem 1.5rem', backgroundColor: '#FFFFFF', borderRadius: '6px', border: '1.5px dashed var(--border-color)', cursor: 'pointer' }}
                onDragOver={(e) => e.preventDefault()}
                onDrop={handleFileDrop}
              >
                <FileText size={32} color="var(--primary-accent)" style={{ marginBottom: '0.5rem' }} />
                <div style={{ fontWeight: 600, fontSize: '0.95rem', marginBottom: '0.25rem' }}>
                  Select or drag &amp; drop a reference file to index
                </div>
                <div style={{ fontSize: '0.82rem', color: 'var(--text-secondary)', marginBottom: '1rem' }}>
                  Supports PDF (.pdf), Word (.docx), and text files (.txt, .md, .cpp) containing valid text
                </div>
                <label className="btn btn-primary" style={{ cursor: 'pointer', display: 'inline-flex' }}>
                  <Upload size={15} />
                  <span>{isUploading ? 'Indexing Document...' : 'Choose File from Computer'}</span>
                  <input
                    type="file"
                    style={{ display: 'none' }}
                    accept=".pdf,.docx,.txt,.md,.cpp,.h,application/pdf,application/vnd.openxmlformats-officedocument.wordprocessingml.document,text/plain"
                    onChange={handleFileUpload}
                    disabled={isUploading}
                  />
                </label>
              </div>
            ) : (
              <form onSubmit={handleTextSubmit}>
                <div style={{ marginBottom: '0.75rem' }}>
                  <label style={{ display: 'block', fontSize: '0.82rem', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '0.25rem' }}>
                    Document Title / Filename
                  </label>
                  <input
                    type="text"
                    className="form-input-clean"
                    value={textTitle}
                    onChange={(e) => setTextTitle(e.target.value)}
                    placeholder="e.g. computer_networks.txt"
                  />
                </div>
                <div style={{ marginBottom: '0.75rem' }}>
                  <label style={{ display: 'block', fontSize: '0.82rem', fontWeight: 600, color: 'var(--text-secondary)', marginBottom: '0.25rem' }}>
                    Reference Text Content
                  </label>
                  <textarea
                    className="text-paste-area"
                    rows={5}
                    value={textContent}
                    onChange={(e) => setTextContent(e.target.value)}
                    placeholder="Paste reference text passage here to index into the corpus..."
                  />
                </div>
                <div style={{ textAlign: 'right' }}>
                  <button type="submit" className="btn btn-primary" disabled={isUploading}>
                    {isUploading ? 'Indexing...' : 'Index Reference Text'}
                  </button>
                </div>
              </form>
            )}
          </div>
        )}

        {/* Search Input Bar */}
        <div style={{ position: 'relative' }}>
          <Search
            size={16}
            color="var(--text-muted)"
            style={{ position: 'absolute', left: '0.85rem', top: '50%', transform: 'translateY(-50%)' }}
          />
          <input
            type="text"
            className="form-input-clean"
            style={{ paddingLeft: '2.5rem' }}
            placeholder="Search reference corpus by filename or document ID..."
            value={searchQuery}
            onChange={(e) => setSearchQuery(e.target.value)}
          />
        </div>
      </div>

      {statusMessage && (
        <div className="alert-box alert-success">
          <CheckCircle size={18} />
          <span>{statusMessage}</span>
        </div>
      )}

      {errorMessage && (
        <div className="alert-box alert-error">
          <AlertCircle size={18} />
          <span>{errorMessage}</span>
        </div>
      )}

      {/* Clean Table of Indexed Documents */}
      <div className="section-card">
        <div className="section-header">
          <div>
            <div className="section-title">
              <BookOpen size={18} color="var(--primary-accent)" /> Indexed Documents ({filteredReferences.length} of {references.length})
            </div>
            <div className="section-desc">Documents available for candidate retrieval and passage similarity comparison</div>
          </div>
        </div>

        {filteredReferences.length === 0 ? (
          <div className="empty-state">
            <p>No reference documents found in the active index.</p>
            <div style={{ display: 'inline-flex', gap: '0.5rem' }}>
              <button className="btn btn-primary btn-sm" onClick={handleSeedCorpus}>
                Restore Benchmark References
              </button>
              <button className="btn btn-secondary btn-sm" onClick={() => setShowAddPanel(true)}>
                Add a Reference
              </button>
            </div>
          </div>
        ) : (
          <table className="clean-table">
            <thead>
              <tr>
                <th>Document Name</th>
                <th>Document ID</th>
                <th>Word Count</th>
                <th>Sentences</th>
                <th>File Size</th>
                <th style={{ textAlign: 'right' }}>Actions</th>
              </tr>
            </thead>
            <tbody>
              {filteredReferences.map((r) => (
                <tr key={r.id}>
                  <td>
                    <strong style={{ color: 'var(--text-primary)' }}>{r.filename}</strong>
                  </td>
                  <td>
                    <code>{r.id}</code>
                  </td>
                  <td>{r.wordCount.toLocaleString()} words</td>
                  <td>{r.sentencesCount} sentences</td>
                  <td>{(r.fileSize / 1024).toFixed(1)} KB</td>
                  <td style={{ textAlign: 'right' }}>
                    <button
                      className="btn btn-sm btn-danger"
                      onClick={() => handleDelete(r.id, r.filename)}
                      title="Remove from index"
                    >
                      <Trash2 size={14} /> Remove
                    </button>
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
