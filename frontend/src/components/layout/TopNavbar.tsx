import React, { useState } from 'react';
import { useApp } from '../../context/AppContext';
import { Building2, ChevronDown, Bell, CheckCircle2, AlertCircle, RefreshCw } from 'lucide-react';

export const TopNavbar: React.FC = () => {
  const {
    hospitals,
    selectedHospitalId,
    setSelectedHospitalId,
    isConnected,
    unreadAlertCount,
    refreshAll,
  } = useApp();

  const [isRefreshing, setIsRefreshing] = useState(false);
  const [showNotifications, setShowNotifications] = useState(false);

  const handleRefresh = async () => {
    setIsRefreshing(true);
    await refreshAll();
    setTimeout(() => setIsRefreshing(false), 500);
  };

  return (
    <header className="top-navbar">
      {/* Left: Hospital Selector Dropdown */}
      <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
        <div className="hospital-dropdown-container">
          <Building2 size={18} color="#2563EB" style={{ flexShrink: 0 }} />
          <select
            className="hospital-select"
            value={selectedHospitalId}
            onChange={(e) => setSelectedHospitalId(e.target.value)}
          >
            {hospitals.map((h) => (
              <option key={h.hospitalId} value={h.hospitalId}>
                {h.name} ({h.hospitalId})
              </option>
            ))}
            {hospitals.length === 0 && (
              <option value="">No Hospitals Configured</option>
            )}
          </select>
          <ChevronDown size={16} color="#64748B" style={{ pointerEvents: 'none', marginLeft: '-6px' }} />
        </div>

        <button
          onClick={handleRefresh}
          className="action-icon-btn"
          title="Refresh Data"
          style={{ width: '38px', height: '38px' }}
        >
          <RefreshCw size={16} className={isRefreshing ? 'animate-spin' : ''} />
        </button>
      </div>

      {/* Right Controls */}
      <div style={{ display: 'flex', alignItems: 'center', gap: '20px' }}>
        {/* Connection status indicator */}
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <span className={`pulse-dot ${isConnected ? 'online' : 'offline'}`} />
          <div style={{ display: 'flex', flexDirection: 'column' }}>
            <span style={{ fontSize: '0.8125rem', fontWeight: 600, color: '#1E293B' }}>
              {isConnected ? 'API Connected' : 'Disconnected'}
            </span>
            <span style={{ fontSize: '0.6875rem', color: '#64748B', fontFamily: 'monospace' }}>
              http://127.0.0.1:8080/api/v1
            </span>
          </div>
        </div>

        {/* Notifications */}
        <div style={{ position: 'relative' }}>
          <button
            onClick={() => setShowNotifications(!showNotifications)}
            className="action-icon-btn"
            style={{ width: '38px', height: '38px', position: 'relative' }}
          >
            <Bell size={18} />
            {unreadAlertCount > 0 && (
              <span
                style={{
                  position: 'absolute',
                  top: '-4px',
                  right: '-4px',
                  backgroundColor: '#EF4444',
                  color: '#FFFFFF',
                  borderRadius: '50%',
                  fontSize: '0.6875rem',
                  fontWeight: 700,
                  width: '18px',
                  height: '18px',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  border: '2px solid #FFFFFF',
                }}
              >
                {unreadAlertCount}
              </span>
            )}
          </button>

          {showNotifications && (
            <div
              style={{
                position: 'absolute',
                right: 0,
                top: '46px',
                width: '280px',
                backgroundColor: '#FFFFFF',
                borderRadius: '12px',
                boxShadow: '0 10px 25px rgba(0,0,0,0.12)',
                border: '1px solid #E2E8F0',
                padding: '12px',
                zIndex: 60,
              }}
            >
              <div
                style={{
                  fontSize: '0.8125rem',
                  fontWeight: 700,
                  color: '#0F172A',
                  borderBottom: '1px solid #F1F5F9',
                  paddingBottom: '8px',
                  marginBottom: '8px',
                  display: 'flex',
                  justifyContent: 'space-between',
                }}
              >
                <span>Live Priority Alerts</span>
                <span style={{ color: '#EF4444' }}>{unreadAlertCount} Critical</span>
              </div>
              {unreadAlertCount > 0 ? (
                <div style={{ display: 'flex', flexDirection: 'column', gap: '8px', fontSize: '0.75rem' }}>
                  <div style={{ display: 'flex', gap: '8px', alignItems: 'flex-start' }}>
                    <AlertCircle size={15} color="#EF4444" style={{ flexShrink: 0, marginTop: '2px' }} />
                    <span style={{ color: '#334155' }}>
                      {unreadAlertCount} patient(s) waiting with Critical / Emergency triage status.
                    </span>
                  </div>
                </div>
              ) : (
                <div style={{ display: 'flex', gap: '8px', alignItems: 'center', color: '#10B981', fontSize: '0.75rem' }}>
                  <CheckCircle2 size={16} />
                  <span>No critical triage alerts. Normal flow.</span>
                </div>
              )}
            </div>
          )}
        </div>

        {/* User Badge */}
        <div className="user-profile-badge">
          <div className="avatar-circle" style={{ width: '34px', height: '34px', fontSize: '0.8125rem' }}>
            SS
          </div>
          <div style={{ display: 'flex', flexDirection: 'column' }}>
            <span style={{ fontSize: '0.8125rem', fontWeight: 600, color: '#0F172A', lineHeight: 1.2 }}>
              Soham
            </span>
            <span style={{ fontSize: '0.6875rem', color: '#64748B' }}>
              Hospital Staff
            </span>
          </div>
          <ChevronDown size={14} color="#94A3B8" />
        </div>
      </div>
    </header>
  );
};

