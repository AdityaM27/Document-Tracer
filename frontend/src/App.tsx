import React, { useState, useEffect } from 'react';
import { Navbar } from './components/Navbar';
import { DashboardView } from './views/DashboardView';
import { UploadView } from './views/UploadView';
import { ProgressView } from './views/ProgressView';
import { ResultsView } from './views/ResultsView';
import { ReferenceLibraryView } from './views/ReferenceLibraryView';
import { ReportsView } from './views/ReportsView';
import { SelfTestView } from './views/SelfTestView';
import {
  SystemHealth,
  AnalysisSummary,
  ReferenceDoc,
  ReportItem
} from './types';
import { api } from './api';
import './App.css';

export const App: React.FC = () => {
  const [activeTab, setActiveTab] = useState<string>('dashboard');
  const [health, setHealth] = useState<SystemHealth | null>(null);
  const [references, setReferences] = useState<ReferenceDoc[]>([]);
  const [reports, setReports] = useState<ReportItem[]>([]);
  const [activeSubmissionId, setActiveSubmissionId] = useState<string | null>(null);
  const [currentSummary, setCurrentSummary] = useState<AnalysisSummary | null>(null);
  const [appError, setAppError] = useState<string | null>(null);

  const fetchAppData = async () => {
    try {
      const [h, refs, reps] = await Promise.all([
        api.getHealth().catch(() => null),
        api.getReferences().catch(() => []),
        api.getReports().catch(() => [])
      ]);
      if (h) setHealth(h);
      setReferences(refs);
      setReports(reps);
    } catch (err) {
      console.error('Error loading app data:', err);
    }
  };

  useEffect(() => {
    fetchAppData();
    const interval = setInterval(fetchAppData, 6000);
    return () => clearInterval(interval);
  }, []);

  const handleStartAnalysis = (summary: AnalysisSummary) => {
    setCurrentSummary(summary);
    setActiveSubmissionId(null);
    setActiveTab('results');
    fetchAppData();
  };

  const handleAnalysisInitiated = (submissionId: string) => {
    setActiveSubmissionId(submissionId);
    setActiveTab('progress');
  };

  const handleProgressCompleted = (summary: AnalysisSummary) => {
    setCurrentSummary(summary);
    setActiveSubmissionId(null);
    setActiveTab('results');
    fetchAppData();
  };

  const handleProgressError = (msg: string) => {
    setAppError(msg);
    setActiveSubmissionId(null);
    setActiveTab('upload');
  };

  return (
    <div className="app-layout">
      <Navbar
        activeTab={activeTab}
        setActiveTab={setActiveTab}
        health={health}
        hasActiveResult={!!currentSummary}
      />

      <main className="main-content">
        {appError && (
          <div className="view-container" style={{ paddingBottom: 0 }}>
            <div className="alert-box alert-error" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
              <span>{appError}</span>
              <button className="btn btn-sm btn-secondary" onClick={() => setAppError(null)}>Dismiss</button>
            </div>
          </div>
        )}

        {activeTab === 'dashboard' && (
          <DashboardView
            health={health}
            references={references}
            reports={reports}
            setActiveTab={setActiveTab}
          />
        )}

        {activeTab === 'upload' && (
          <UploadView
            onStartAnalysis={handleStartAnalysis}
            onAnalysisInitiated={handleAnalysisInitiated}
          />
        )}

        {activeTab === 'progress' && activeSubmissionId && (
          <ProgressView
            submissionId={activeSubmissionId}
            onCompleted={handleProgressCompleted}
            onError={handleProgressError}
          />
        )}

        {activeTab === 'results' && (
          <ResultsView
            summary={currentSummary}
            onNewAnalysis={() => setActiveTab('upload')}
          />
        )}

        {activeTab === 'references' && (
          <ReferenceLibraryView
            references={references}
            onRefresh={fetchAppData}
          />
        )}

        {activeTab === 'reports' && (
          <ReportsView reports={reports} />
        )}

        {activeTab === 'selftest' && (
          <SelfTestView onBackToDashboard={() => setActiveTab('dashboard')} />
        )}
      </main>

      <footer className="app-footer">
        <div>Document Tracer &copy; 2026 &bull; B.Tech III Semester Object Oriented Programming Through C++ Laboratory Project</div>
        <div style={{ fontSize: '0.8rem', color: 'var(--text-muted)', marginTop: '0.25rem' }}>
          Engineered with C++17, MinGW-w64, Producer-Consumer ThreadPool, cpp-httplib &amp; React / TypeScript
        </div>
      </footer>
    </div>
  );
};

export default App;
