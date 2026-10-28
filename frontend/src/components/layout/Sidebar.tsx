import React from 'react';
import { useApp, NavigationTab } from '../../context/AppContext';
import {
  LayoutDashboard,
  AlertTriangle,
  Users,
  History,
  Building2,
  BarChart3,
  Settings,
  HeartPulse,
} from 'lucide-react';

export const Sidebar: React.FC = () => {
  const { activeTab, setActiveTab, queue } = useApp();

  const navItems: { id: NavigationTab; label: string; icon: React.ReactNode; badge?: number }[] = [
    { id: 'dashboard', label: 'Dashboard', icon: <LayoutDashboard size={19} /> },
    {
      id: 'queue',
      label: 'Emergency Queue',
      icon: <AlertTriangle size={19} />,
      badge: queue.length > 0 ? queue.length : undefined,
    },
    { id: 'patients', label: 'Patients', icon: <Users size={19} /> },
    { id: 'visits', label: 'Visit History', icon: <History size={19} /> },
    { id: 'hospitals', label: 'Hospitals', icon: <Building2 size={19} /> },
    { id: 'reports', label: 'Reports', icon: <BarChart3 size={19} /> },
    { id: 'settings', label: 'Settings', icon: <Settings size={19} /> },
  ];

  return (
    <aside className="sidebar">
      {/* Brand Header */}
      <div className="sidebar-brand">
        <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
          <div
            style={{
              width: '38px',
              height: '38px',
              borderRadius: '10px',
              background: 'linear-gradient(135deg, #2563EB 0%, #1D4ED8 100%)',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
              boxShadow: '0 4px 12px rgba(37, 99, 235, 0.4)',
            }}
          >
            <HeartPulse size={22} color="#FFFFFF" />
          </div>
          <div style={{ display: 'flex', flexDirection: 'column' }}>
            <span
              style={{
                fontSize: '1.25rem',
                fontWeight: 800,
                color: '#FFFFFF',
                letterSpacing: '-0.02em',
                lineHeight: 1.1,
              }}
            >
              SmartER
            </span>
            <span
              style={{
                fontSize: '0.625rem',
                fontWeight: 600,
                color: '#94A3B8',
                letterSpacing: '0.04em',
                textTransform: 'uppercase',
                marginTop: '2px',
              }}
            >
              C++17 Engine
            </span>
          </div>
        </div>
        <p className="sidebar-sub">
          Hospital Emergency Queue &amp; Patient History Management
        </p>
      </div>

      {/* Nav Menu */}
      <nav className="sidebar-nav">
        {navItems.map((item) => {
          const isActive = activeTab === item.id;
          return (
            <button
              key={item.id}
              onClick={() => setActiveTab(item.id)}
              className={`nav-pill ${isActive ? 'active' : ''}`}
            >
              <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
                <span style={{ opacity: isActive ? 1 : 0.85 }}>{item.icon}</span>
                <span>{item.label}</span>
              </div>
              {item.badge !== undefined && (
                <span
                  style={{
                    backgroundColor: isActive ? '#FFFFFF' : '#EF4444',
                    color: isActive ? '#2563EB' : '#FFFFFF',
                    borderRadius: '12px',
                    fontSize: '0.6875rem',
                    fontWeight: 700,
                    padding: '2px 7px',
                    lineHeight: 1,
                  }}
                >
                  {item.badge}
                </span>
              )}
            </button>
          );
        })}
      </nav>

      {/* Footer Card with Pulse Wave */}
      <div className="sidebar-footer">
        <div className="sidebar-card">
          <div style={{ display: 'flex', alignItems: 'flex-start', gap: '10px' }}>
            <div
              style={{
                width: '32px',
                height: '32px',
                borderRadius: '8px',
                backgroundColor: 'rgba(37, 99, 235, 0.35)',
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                flexShrink: 0,
              }}
            >
              <Users size={16} color="#60A5FA" />
            </div>
            <div>
              <div style={{ fontWeight: 600, color: '#F1F5F9', fontSize: '0.8125rem' }}>
                Better Care
              </div>
              <div style={{ color: '#94A3B8', fontSize: '0.75rem', marginTop: '2px' }}>
                Faster Response
              </div>
              <div style={{ color: '#64748B', fontSize: '0.6875rem', marginTop: '1px' }}>
                Smarter Management
              </div>
            </div>
          </div>

          {/* SVG Pulse Line */}
          <div style={{ marginTop: '12px', height: '24px', opacity: 0.65 }}>
            <svg width="100%" height="24" viewBox="0 0 160 24" fill="none">
              <path
                d="M0 12 H40 L46 3 L52 21 L58 8 L64 16 L70 12 H160"
                stroke="#60A5FA"
                strokeWidth="2"
                strokeLinecap="round"
                strokeLinejoin="round"
              />
            </svg>
          </div>
        </div>
      </div>
    </aside>
  );
};

