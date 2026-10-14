import React from 'react';
import {
  Cpu,
  Layers,
  FileCheck2,
  BookOpen,
  Archive,
  BarChart3
} from 'lucide-react';
import type { SystemHealth } from '../types';

interface NavbarProps {
  activeTab: string;
  setActiveTab: (tab: string) => void;
  health: SystemHealth | null;
  hasActiveResult?: boolean;
}

export const Navbar: React.FC<NavbarProps> = ({
  activeTab,
  setActiveTab,
  health,
  hasActiveResult = false,
}) => {
  return (
    <header className="navbar-container">
      <div className="navbar-content">
        {/* Brand / Logo */}
        <div className="brand" onClick={() => setActiveTab('dashboard')}>
          <div className="brand-logo">
            <Cpu size={18} />
          </div>
          <span className="brand-title">Document Tracer</span>
        </div>

        {/* Clean Navigation Items */}
        <nav className="nav-links">
          <button
            className={`nav-button ${activeTab === 'dashboard' ? 'active' : ''}`}
            onClick={() => setActiveTab('dashboard')}
          >
            <BarChart3 size={15} />
            <span>Dashboard</span>
          </button>
          <button
            className={`nav-button ${activeTab === 'upload' ? 'active' : ''}`}
            onClick={() => setActiveTab('upload')}
          >
            <FileCheck2 size={15} />
            <span>Analyze Documents</span>
          </button>
          {hasActiveResult && (
            <button
              className={`nav-button ${activeTab === 'results' ? 'active' : ''}`}
              onClick={() => setActiveTab('results')}
            >
              <Layers size={15} />
              <span>Results</span>
            </button>
          )}
          <button
            className={`nav-button ${activeTab === 'references' ? 'active' : ''}`}
            onClick={() => setActiveTab('references')}
          >
            <BookOpen size={15} />
            <span>Reference Library</span>
          </button>
          <button
            className={`nav-button ${activeTab === 'reports' ? 'active' : ''}`}
            onClick={() => setActiveTab('reports')}
          >
            <Archive size={15} />
            <span>Reports</span>
          </button>
          <button
            className={`nav-button ${activeTab === 'selftest' ? 'active' : ''}`}
            onClick={() => setActiveTab('selftest')}
          >
            <Cpu size={15} />
            <span>Self-Test</span>
          </button>
        </nav>

        {/* Compact Service-Status Indicators */}
        <div className="status-pills">
          <span className="pill pill-success" title="C++ Core Engine is running (port 8080)">
            <span className="pill-dot"></span> C++ Core: Online
          </span>
          <span
            className={`pill ${health?.aiHfLoaded ? 'pill-success' : health?.aiServiceOnline ? 'pill-warning' : 'pill-muted'}`}
            title={health?.aiHfLoaded ? 'Genuine Hugging Face neural transformer online' : health?.aiServiceOnline ? 'Python microservice running in calibrated statistical fallback mode' : 'AI microservice offline'}
          >
            <span className="pill-dot"></span> AI: {health?.aiHfLoaded ? 'Neural HF' : health?.aiServiceOnline ? 'Fallback' : 'Offline'}
          </span>
          <span
            className={`pill ${health?.academicProviderOnline ? 'pill-info' : 'pill-muted'}`}
            title="OpenAlex & Crossref literature search"
          >
            <span className="pill-dot"></span> Academic: {health?.academicProviderOnline ? 'Ready' : 'Offline'}
          </span>
        </div>
      </div>
    </header>
  );
};
