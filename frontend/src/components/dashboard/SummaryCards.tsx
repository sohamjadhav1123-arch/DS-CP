import React from 'react';
import { useApp } from '../../context/AppContext';
import { Users, AlertTriangle, BedDouble, CheckCircle2 } from 'lucide-react';

export const SummaryCards: React.FC = () => {
  const { stats, queue, patients, visits } = useApp();

  // Prefer backend calculated stats if available, otherwise compute from local state
  const totalPatients = stats?.registeredPatientCount ?? patients.length;
  const waitingER = stats?.activeQueueSize ?? queue.length;
  const inTreatment = stats?.inTreatmentVisits ?? visits.filter((v) => v.status === 'In Treatment').length;
  const dischargedToday = stats?.completedVisits ?? visits.filter((v) => v.status === 'Completed' || v.status === 'Discharged').length;

  return (
    <div className="grid-summary-cards">
      {/* Total Patients */}
      <div className="summary-card">
        <div style={{ display: 'flex', alignItems: 'flex-start', justifyContent: 'space-between' }}>
          <div>
            <div className="card-label">Total Patients</div>
            <div className="card-value">{totalPatients}</div>
            <div className="card-trend trend-up">
              <span style={{ fontSize: '0.8125rem' }}>↑</span>
              <span>{Math.max(1, Math.round(totalPatients * 0.1))} today</span>
            </div>
          </div>
          <div className="card-icon-wrapper icon-blue">
            <Users size={22} color="#2563EB" />
          </div>
        </div>
      </div>

      {/* Waiting in ER */}
      <div className="summary-card" style={{ backgroundColor: '#FEF2F2', borderColor: '#FEE2E2' }}>
        <div style={{ display: 'flex', alignItems: 'flex-start', justifyContent: 'space-between' }}>
          <div>
            <div className="card-label" style={{ color: '#991B1B' }}>
              Waiting in ER
            </div>
            <div className="card-value" style={{ color: '#991B1B' }}>
              {waitingER}
            </div>
            <div className="card-trend trend-down" style={{ color: '#DC2626' }}>
              <span style={{ fontSize: '0.8125rem' }}>↓</span>
              <span>2 since last hour</span>
            </div>
          </div>
          <div className="card-icon-wrapper" style={{ backgroundColor: '#FEE2E2' }}>
            <AlertTriangle size={22} color="#EF4444" />
          </div>
        </div>
      </div>

      {/* In Treatment */}
      <div className="summary-card" style={{ backgroundColor: '#F0FDF4', borderColor: '#DCFCE7' }}>
        <div style={{ display: 'flex', alignItems: 'flex-start', justifyContent: 'space-between' }}>
          <div>
            <div className="card-label" style={{ color: '#166534' }}>
              In Treatment
            </div>
            <div className="card-value" style={{ color: '#166534' }}>
              {inTreatment}
            </div>
            <div className="card-trend trend-up" style={{ color: '#16A34A' }}>
              <span style={{ fontSize: '0.8125rem' }}>↑</span>
              <span>1 since last hour</span>
            </div>
          </div>
          <div className="card-icon-wrapper" style={{ backgroundColor: '#DCFCE7' }}>
            <BedDouble size={22} color="#16A34A" />
          </div>
        </div>
      </div>

      {/* Discharged Today */}
      <div className="summary-card">
        <div style={{ display: 'flex', alignItems: 'flex-start', justifyContent: 'space-between' }}>
          <div>
            <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
              <span className="card-label">Discharged Today</span>
              <span
                style={{
                  fontSize: '0.6875rem',
                  fontWeight: 600,
                  color: '#059669',
                  backgroundColor: '#D1FAE5',
                  padding: '1px 7px',
                  borderRadius: '10px',
                }}
              >
                Online
              </span>
            </div>
            <div className="card-value">{dischargedToday}</div>
            <div className="card-trend trend-up">
              <span style={{ fontSize: '0.8125rem' }}>↑</span>
              <span>1 since yesterday</span>
            </div>
          </div>
          <div className="card-icon-wrapper icon-purple">
            <CheckCircle2 size={22} color="#8B5CF6" />
          </div>
        </div>
      </div>
    </div>
  );
};

