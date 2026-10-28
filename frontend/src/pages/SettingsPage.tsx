import React, { useState } from 'react';
import { useApp } from '../context/AppContext';
import { api } from '../services/api';
import { Settings, Server, Globe, RefreshCw, CheckCircle, Database } from 'lucide-react';

export const SettingsPage: React.FC = () => {
  const { isConnected, health, refreshAll, addToast } = useApp();
  const [endpointUrl, setEndpointUrl] = useState(api.baseUrl);
  const [isTesting, setIsTesting] = useState(false);

  const handleSaveEndpoint = () => {
    api.baseUrl = endpointUrl.trim();
    addToast('info', 'Endpoint Updated', `API base URL set to: ${api.baseUrl}`);
    refreshAll();
  };

  const handleTestConnection = async () => {
    setIsTesting(true);
    try {
      const h = await api.getHealth();
      addToast('success', 'Connection Verified', `Backend v${h.version} responsive with ${h.hospitalCount} hospital(s).`);
    } catch (err: any) {
      addToast('error', 'Connection Failed', err.message);
    } finally {
      setIsTesting(false);
    }
  };

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
      {/* Header */}
      <div className="content-card" style={{ padding: '24px' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
          <div
            style={{
              width: '36px',
              height: '36px',
              borderRadius: '10px',
              backgroundColor: '#EFF6FF',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
            }}
          >
            <Settings size={20} color="#2563EB" />
          </div>
          <div>
            <h1 style={{ fontSize: '1.5rem', fontWeight: 800, color: '#0F172A', margin: 0 }}>
              System Settings &amp; API Configuration
            </h1>
            <p style={{ color: '#64748B', fontSize: '0.875rem', marginTop: '6px', margin: 0 }}>
              Configure your C++ REST API endpoints and verify SQLite persistence status.
            </p>
          </div>
        </div>
      </div>

      {/* Connection Card */}
      <div className="content-card" style={{ padding: '24px' }}>
        <h2 className="card-title" style={{ marginBottom: '16px', display: 'flex', alignItems: 'center', gap: '8px' }}>
          <Server size={18} color="#2563EB" />
          <span>C++ REST API Server Connection</span>
        </h2>

        <div style={{ display: 'flex', flexDirection: 'column', gap: '16px', maxWidth: '600px' }}>
          <div>
            <label style={{ fontSize: '0.8125rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '6px' }}>
              Base API URL
            </label>
            <div style={{ display: 'flex', gap: '10px' }}>
              <input
                type="text"
                value={endpointUrl}
                onChange={(e) => setEndpointUrl(e.target.value)}
                className="form-input"
                style={{ flex: 1 }}
              />
              <button
                onClick={handleSaveEndpoint}
                className="btn-secondary"
              >
                Apply
              </button>
            </div>
            <p style={{ fontSize: '0.75rem', color: '#64748B', marginTop: '4px', margin: 0 }}>
              Default: <code>http://127.0.0.1:8080/api/v1</code> (served by <code>smarter_server.exe</code>)
            </p>
          </div>

          <div style={{ display: 'flex', alignItems: 'center', gap: '12px', marginTop: '8px' }}>
            <button
              onClick={handleTestConnection}
              disabled={isTesting}
              className="btn-primary"
            >
              <RefreshCw size={15} className={isTesting ? 'animate-spin' : ''} />
              <span>{isTesting ? 'Testing...' : 'Test Connection'}</span>
            </button>
            <div style={{ display: 'flex', alignItems: 'center', gap: '6px', fontSize: '0.8125rem' }}>
              <span className={`pulse-dot ${isConnected ? 'online' : 'offline'}`} />
              <span style={{ fontWeight: 600, color: isConnected ? '#16A34A' : '#EF4444' }}>
                {isConnected ? 'Backend Live & Responsive' : 'Backend Disconnected'}
              </span>
            </div>
          </div>
        </div>
      </div>

      {/* Server Diagnostic Info */}
      <div className="content-card" style={{ padding: '24px' }}>
        <h2 className="card-title" style={{ marginBottom: '16px', display: 'flex', alignItems: 'center', gap: '8px' }}>
          <Database size={18} color="#2563EB" />
          <span>Server Health Diagnostic</span>
        </h2>

        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(220px, 1fr))', gap: '16px', fontSize: '0.8125rem' }}>
          <div style={{ backgroundColor: '#F8FAFC', padding: '12px 16px', borderRadius: '8px', border: '1px solid #E2E8F0' }}>
            <div style={{ color: '#64748B' }}>Status</div>
            <div style={{ fontWeight: 700, color: '#0F172A', marginTop: '2px' }}>{health?.status || 'Unknown'}</div>
          </div>
          <div style={{ backgroundColor: '#F8FAFC', padding: '12px 16px', borderRadius: '8px', border: '1px solid #E2E8F0' }}>
            <div style={{ color: '#64748B' }}>API Version</div>
            <div style={{ fontWeight: 700, color: '#0F172A', marginTop: '2px' }}>{health?.version || '1.0.0'}</div>
          </div>
          <div style={{ backgroundColor: '#F8FAFC', padding: '12px 16px', borderRadius: '8px', border: '1px solid #E2E8F0' }}>
            <div style={{ color: '#64748B' }}>SQLite Persisted</div>
            <div style={{ fontWeight: 700, color: '#16A34A', marginTop: '2px' }}>
              {health?.databaseConnected ? 'Yes (Connected)' : 'Active Memory'}
            </div>
          </div>
          <div style={{ backgroundColor: '#F8FAFC', padding: '12px 16px', borderRadius: '8px', border: '1px solid #E2E8F0' }}>
            <div style={{ color: '#64748B' }}>Active Hospitals</div>
            <div style={{ fontWeight: 700, color: '#0F172A', marginTop: '2px' }}>{health?.hospitalCount ?? 0}</div>
          </div>
        </div>
      </div>
    </div>
  );
};

