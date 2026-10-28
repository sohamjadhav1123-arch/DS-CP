import React from 'react';
import { useApp } from '../context/AppContext';
import { WelcomeBanner } from '../components/layout/WelcomeBanner';
import { SummaryCards } from '../components/dashboard/SummaryCards';
import { EmergencyQueueTable } from '../components/dashboard/EmergencyQueueTable';
import { RecentPatientsTable } from '../components/dashboard/RecentPatientsTable';
import { HospitalOverviewCard } from '../components/dashboard/HospitalOverviewCard';
import { PatientDistributionChart } from '../components/dashboard/PatientDistributionChart';
import { QuickActionsCard } from '../components/dashboard/QuickActionsCard';
import { AlertCircle, RefreshCw, Server, Plus } from 'lucide-react';

export const DashboardPage: React.FC = () => {
  const {
    isConnected,
    isInitialLoading,
    hospitals,
    refreshAll,
    isRefreshing,
    lastErrorMessage,
    setActiveTab,
  } = useApp();

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '20px', width: '100%' }}>
      {/* Backend Disconnected Notice Banner if server is unreachable */}
      {!isConnected && !isInitialLoading && (
        <div
          style={{
            backgroundColor: '#FEF2F2',
            border: '1px solid #FEE2E2',
            borderRadius: '12px',
            padding: '14px 18px',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            flexWrap: 'wrap',
            gap: '12px',
          }}
        >
          <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
            <div
              style={{
                width: '32px',
                height: '32px',
                borderRadius: '8px',
                backgroundColor: '#FEE2E2',
                display: 'flex',
                alignItems: 'center',
                justifyContent: 'center',
                flexShrink: 0,
              }}
            >
              <AlertCircle size={18} color="#EF4444" />
            </div>
            <div>
              <div style={{ fontWeight: 700, fontSize: '0.875rem', color: '#991B1B' }}>
                Backend REST API Server Offline
              </div>
              <div style={{ fontSize: '0.75rem', color: '#B91C1C', marginTop: '2px' }}>
                {lastErrorMessage || 'Cannot connect to C++ backend at http://127.0.0.1:8080/api/v1. Launch smarter_server.exe to connect.'}
              </div>
            </div>
          </div>
          <button
            onClick={refreshAll}
            disabled={isRefreshing}
            className="btn-secondary"
            style={{ fontSize: '0.8125rem', padding: '6px 14px' }}
          >
            <RefreshCw size={14} className={isRefreshing ? 'animate-spin' : ''} />
            <span>Retry Connection</span>
          </button>
        </div>
      )}

      {/* Zero Hospitals State if database is empty */}
      {isConnected && hospitals.length === 0 && !isInitialLoading && (
        <div
          style={{
            backgroundColor: '#EFF6FF',
            border: '1px solid #DBEAFE',
            borderRadius: '12px',
            padding: '16px 20px',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            flexWrap: 'wrap',
            gap: '12px',
          }}
        >
          <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
            <Server size={20} color="#2563EB" />
            <div>
              <div style={{ fontWeight: 700, fontSize: '0.875rem', color: '#1E40AF' }}>
                No Hospitals Registered in SQLite Persistence
              </div>
              <div style={{ fontSize: '0.75rem', color: '#3B82F6', marginTop: '2px' }}>
                Initialize your primary healthcare facility to activate the Min-Heap queue and patient registry.
              </div>
            </div>
          </div>
          <button
            onClick={() => setActiveTab('hospitals')}
            className="btn-primary"
            style={{ fontSize: '0.8125rem', padding: '6px 14px' }}
          >
            <Plus size={14} />
            <span>Create Hospital Facility</span>
          </button>
        </div>
      )}

      {/* 1. Welcome Banner */}
      <WelcomeBanner />

      {/* 2. Top Summary Metric Cards */}
      <SummaryCards />

      {/* 3. Main Dashboard Grid */}
      <div className="grid-dashboard-main">
        {/* Left Column (Tables) */}
        <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
          <EmergencyQueueTable limit={5} showViewAll={true} />
          <RecentPatientsTable limit={4} showViewAll={true} />
        </div>

        {/* Right Column (Widgets) */}
        <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
          <HospitalOverviewCard />
          <PatientDistributionChart />
          <QuickActionsCard />
        </div>
      </div>
    </div>
  );
};
