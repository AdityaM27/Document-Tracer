import {
  SystemHealth,
  AnalysisOptions,
  AnalysisSummary,
  AnalysisProgress,
  ReferenceDoc,
  ReportItem
} from './types';

const API_BASE = '/api';

export const api = {
  async getHealth(): Promise<SystemHealth> {
    const res = await fetch(`${API_BASE}/health`);
    if (!res.ok) throw new Error('Health check failed');
    return res.json();
  },

  async getConfig(): Promise<AnalysisOptions> {
    const res = await fetch(`${API_BASE}/config`);
    if (!res.ok) throw new Error('Failed to load configuration');
    return res.json();
  },

  async updateConfig(options: Partial<AnalysisOptions>): Promise<void> {
    const res = await fetch(`${API_BASE}/config`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(options),
    });
    if (!res.ok) throw new Error('Failed to update config');
  },

  async uploadFile(file: File): Promise<{ docId: string; filename: string; filepath: string; size: number }> {
    const formData = new FormData();
    formData.append('file', file);
    const res = await fetch(`${API_BASE}/upload`, {
      method: 'POST',
      body: formData,
    });
    if (!res.ok) {
      const err = await res.json().catch(() => ({}));
      throw new Error(err.error || 'Failed to upload document file');
    }
    return res.json();
  },

  async uploadText(text: string, filename: string = 'submission.txt'): Promise<{ docId: string; filename: string; filepath: string; size: number }> {
    const res = await fetch(`${API_BASE}/upload`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ text, filename }),
    });
    if (!res.ok) {
      const err = await res.json().catch(() => ({}));
      throw new Error(err.error || 'Failed to submit document text');
    }
    return res.json();
  },

  async startAnalysis(filepath: string, docId: string, options: Partial<AnalysisOptions>): Promise<any> {
    const res = await fetch(`${API_BASE}/analyze`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ filepath, docId, ...options }),
    });
    if (!res.ok) {
      const err = await res.json().catch(() => ({}));
      throw new Error(err.error || 'Analysis execution failed');
    }
    return res.json();
  },

  async getAnalysisStatus(submissionId: string): Promise<AnalysisProgress> {
    const res = await fetch(`${API_BASE}/analysis/${submissionId}/status`);
    if (!res.ok) {
      const err = await res.json().catch(() => ({}));
      return {
        submissionId,
        stage: 'ERROR',
        percent: 0,
        message: err.message || err.error || 'Task not found',
        isCompleted: false,
        hasError: true,
        errorMessage: err.errorMessage || err.error || `Analysis task ID not found (${submissionId})`
      };
    }
    return res.json();
  },

  async getAnalysisResult(submissionId: string): Promise<AnalysisSummary> {
    const res = await fetch(`${API_BASE}/analysis/${submissionId}`);
    if (!res.ok) throw new Error('Failed to retrieve analysis result');
    return res.json();
  },

  async getReferences(): Promise<ReferenceDoc[]> {
    const res = await fetch(`${API_BASE}/references`);
    if (!res.ok) throw new Error('Failed to fetch reference library');
    return res.json();
  },

  async uploadReference(file: File): Promise<{ status: string; id: string; filename: string }> {
    const formData = new FormData();
    formData.append('file', file);
    const res = await fetch(`${API_BASE}/references/upload`, {
      method: 'POST',
      body: formData,
    });
    if (!res.ok) {
      const err = await res.json().catch(() => ({}));
      throw new Error(err.error || `Upload failed (HTTP ${res.status})`);
    }
    return res.json();
  },

  async uploadReferenceText(title: string, text: string): Promise<{ status: string; id: string; filename: string }> {
    const res = await fetch(`${API_BASE}/references/upload`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ filename: title, text }),
    });
    if (!res.ok) {
      const err = await res.json().catch(() => ({}));
      throw new Error(err.error || `Text submission failed (HTTP ${res.status})`);
    }
    return res.json();
  },

  async seedReferences(): Promise<number> {
    const res = await fetch(`${API_BASE}/references/seed`, {
      method: 'POST',
    });
    if (!res.ok) throw new Error('Failed to seed reference corpus');
    const data = await res.json();
    return data.indexedCount || 0;
  },

  async deleteReference(id: string): Promise<void> {
    const res = await fetch(`${API_BASE}/references/${id}`, {
      method: 'DELETE',
    });
    if (!res.ok) throw new Error('Failed to delete reference document');
  },

  async reindexReferences(): Promise<number> {
    const res = await fetch(`${API_BASE}/references/reindex`, {
      method: 'POST',
    });
    if (!res.ok) throw new Error('Failed to rebuild index');
    const data = await res.json();
    return data.indexedCount || 0;
  },

  async getReports(): Promise<ReportItem[]> {
    const res = await fetch(`${API_BASE}/reports`);
    if (!res.ok) throw new Error('Failed to fetch reports archive');
    return res.json();
  },

  async runSelfTest(): Promise<import('./types').SelfTestReport> {
    const res = await fetch(`${API_BASE}/self-test`, {
      method: 'POST',
    });
    if (!res.ok) {
      const err = await res.json().catch(() => ({}));
      throw new Error(err.error || `Self-test execution failed (HTTP ${res.status})`);
    }
    return res.json();
  },
};
