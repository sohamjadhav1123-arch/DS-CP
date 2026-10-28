import React from 'react';
import { useApp } from '../../context/AppContext';
import { StatusBadge } from '../common/StatusBadge';
import { Users, ArrowRight, MoreVertical } from 'lucide-react';

interface RecentPatientsTableProps {
  limit?: number;
  showViewAll?: boolean;
}

const avatarColors = ['#8B5CF6', '#0284C7', '#2563EB', '#D97706', '#059669', '#DB2777'];

export const RecentPatientsTable: React.FC<RecentPatientsTableProps> = ({
  limit = 4,
  showViewAll = true,
}) => {
  const { patients, visits, setActiveTab } = useApp();

  const displayedPatients = limit ? patients.slice(0, limit) : patients;

  return (
    <div className="content-card">
      {/* Header */}
      <div className="card-header">
        <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
          <div
            style={{
              width: '32px',
              height: '32px',
              borderRadius: '8px',
              backgroundColor: '#EFF6FF',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
            }}
          >
            <Users size={18} color="#2563EB" />
          </div>
          <div>
            <h2 className="card-title">Recent Patients</h2>
          </div>
        </div>

        {showViewAll && (
          <button
            onClick={() => setActiveTab('patients')}
            className="action-icon-btn"
            style={{
              padding: '6px 12px',
              borderRadius: '8px',
              fontSize: '0.8125rem',
              fontWeight: 600,
              color: '#475569',
              display: 'flex',
              alignItems: 'center',
              gap: '6px',
            }}
          >
            <span>View All</span>
            <ArrowRight size={14} />
          </button>
        )}
      </div>

      {/* Table */}
      <div className="smarter-table-wrapper">
        <table className="smarter-table">
          <thead>
            <tr>
              <th>Name</th>
              <th>Patient ID</th>
              <th>Hospital ID</th>
              <th>Last Visit</th>
              <th>Status</th>
              <th style={{ width: '40px' }}></th>
            </tr>
          </thead>
          <tbody>
            {displayedPatients.map((p, index) => {
              const initial = p.name ? p.name.charAt(0).toUpperCase() : 'P';
              const colorBg = avatarColors[index % avatarColors.length];

              // Find matching patient visit
              const patientVisit = visits.find((v) => v.patientId === p.patientId);
              const visitStatus = patientVisit ? patientVisit.status : 'Waiting';
              const visitTime = patientVisit?.arrivalTime || 'Today, 10:15 AM';

              return (
                <tr key={p.patientId}>
                  <td>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
                      <div
                        className="avatar-circle"
                        style={{
                          backgroundColor: colorBg,
                          width: '32px',
                          height: '32px',
                          fontSize: '0.75rem',
                        }}
                      >
                        {initial}
                      </div>
                      <span style={{ fontWeight: 600, color: '#0F172A' }}>{p.name}</span>
                    </div>
                  </td>
                  <td>
                    <span
                      style={{
                        fontFamily: 'monospace',
                        fontSize: '0.8125rem',
                        color: '#475569',
                        backgroundColor: '#F1F5F9',
                        padding: '2px 6px',
                        borderRadius: '4px',
                      }}
                    >
                      {p.patientId}
                    </span>
                  </td>
                  <td>
                    <span style={{ fontSize: '0.8125rem', color: '#64748B' }}>
                      {p.hospitalId}
                    </span>
                  </td>
                  <td>
                    <span style={{ fontSize: '0.8125rem', color: '#64748B' }}>
                      {visitTime}
                    </span>
                  </td>
                  <td>
                    <StatusBadge status={visitStatus} />
                  </td>
                  <td>
                    <button className="action-icon-btn" style={{ padding: '4px' }}>
                      <MoreVertical size={16} />
                    </button>
                  </td>
                </tr>
              );
            })}

            {displayedPatients.length === 0 && (
              <tr>
                <td colSpan={6} style={{ textAlign: 'center', padding: '28px 16px', color: '#64748B' }}>
                  No patients found in Custom Hash Table registry.
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>
    </div>
  );
};

