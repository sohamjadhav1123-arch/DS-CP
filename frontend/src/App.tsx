import React from 'react';
import { useApp } from './context/AppContext';
import { Sidebar } from './components/layout/Sidebar';
import { TopNavbar } from './components/layout/TopNavbar';
import { ToastContainer } from './components/common/Toast';
import { ErrorBoundary } from './components/common/ErrorBoundary';
import { DashboardPage } from './pages/DashboardPage';
import { EmergencyQueuePage } from './pages/EmergencyQueuePage';
import { PatientsPage } from './pages/PatientsPage';
import { VisitsPage } from './pages/VisitsPage';
import { HospitalsPage } from './pages/HospitalsPage';
import { ReportsPage } from './pages/ReportsPage';
import { SettingsPage } from './pages/SettingsPage';

export const AppContent: React.FC = () => {
  const { activeTab } = useApp();

  return (
    <div className="smarter-app">
      {/* Dark Navy Sidebar */}
      <Sidebar />

      {/* Main Workspace Area */}
      <div className="main-wrapper">
        <TopNavbar />

        <main className="content-area">
          <ErrorBoundary>
            {activeTab === 'dashboard' && <DashboardPage />}
            {activeTab === 'queue' && <EmergencyQueuePage />}
            {activeTab === 'patients' && <PatientsPage />}
            {activeTab === 'visits' && <VisitsPage />}
            {activeTab === 'hospitals' && <HospitalsPage />}
            {activeTab === 'reports' && <ReportsPage />}
            {activeTab === 'settings' && <SettingsPage />}
          </ErrorBoundary>
        </main>
      </div>

      {/* Global Notifications */}
      <ToastContainer />
    </div>
  );
};
