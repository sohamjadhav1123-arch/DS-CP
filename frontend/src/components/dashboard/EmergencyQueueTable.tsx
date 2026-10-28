import React, { useState } from 'react';
import { useApp } from '../../context/AppContext';
import { TriageBadge } from '../common/TriageBadge';
import { StatusBadge } from '../common/StatusBadge';
import { Modal } from '../common/Modal';
import { AlertTriangle, ArrowRight, MoreVertical, Play, Clock } from 'lucide-react';

interface EmergencyQueueTableProps {
  limit?: number;
  showViewAll?: boolean;
}

export const EmergencyQueueTable: React.FC<EmergencyQueueTableProps> = ({
  limit = 5,
  showViewAll = true,
}) => {
  const { queue, serveNext, setActiveTab } = useApp();
  const [isServing, setIsServing] = useState(false);
  const [selectedEntry, setSelectedEntry] = useState<any | null>(null);

  const displayedQueue = limit ? queue.slice(0, limit) : queue;

  const handleServe = async () => {
    setIsServing(true);
    await serveNext();
    setIsServing(false);
  };

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
              backgroundColor: '#FEE2E2',
              display: 'flex',
              alignItems: 'center',
              justifyContent: 'center',
            }}
          >
            <AlertTriangle size={18} color="#EF4444" />
          </div>
          <div>
            <h2 className="card-title">Emergency Queue</h2>
            <p className="card-subtitle">
              Patients are served based on triage level (1 = highest priority) and arrival time.
            </p>
          </div>
        </div>

        {showViewAll && (
          <button
            onClick={() => setActiveTab('queue')}
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
              <th style={{ width: '40px' }}>#</th>
              <th>Patient Name</th>
              <th>Patient ID</th>
              <th>Triage Level</th>
              <th>Arrival Time</th>
              <th>Status</th>
              <th style={{ textAlign: 'right' }}>Actions</th>
            </tr>
          </thead>
          <tbody>
            {displayedQueue.map((entry, index) => {
              // Convert arrival sequence or timestamp to display time
              const displayTime = entry.arrivalSequence
                ? `Seq #${entry.arrivalSequence}`
                : '10:24 AM';

              return (
                <tr key={`${entry.patientId}-${entry.rank}`}>
                  <td style={{ fontWeight: 600, color: '#64748B' }}>{index + 1}</td>
                  <td>
                    <span style={{ fontWeight: 600, color: '#0F172A' }}>
                      {entry.patientName || `Patient ${entry.patientId}`}
                    </span>
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
                      {entry.patientId}
                    </span>
                  </td>
                  <td>
                    <TriageBadge level={entry.triageLevel} />
                  </td>
                  <td>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '5px', color: '#64748B', fontSize: '0.8125rem' }}>
                      <Clock size={13} />
                      <span>{displayTime}</span>
                    </div>
                  </td>
                  <td>
                    <StatusBadge status="Waiting" />
                  </td>
                  <td>
                    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'flex-end', gap: '8px' }}>
                      <button
                        onClick={handleServe}
                        disabled={isServing || index !== 0}
                        className="btn-primary"
                        style={{
                          padding: '5px 14px',
                          fontSize: '0.8125rem',
                          borderRadius: '6px',
                          opacity: index !== 0 ? 0.45 : 1,
                          cursor: index !== 0 ? 'not-allowed' : 'pointer',
                        }}
                        title={index !== 0 ? 'Min-Heap serves highest priority patient (#1) first' : 'Serve Patient'}
                      >
                        <Play size={12} fill="#FFFFFF" />
                        <span>Serve</span>
                      </button>
                      <button
                        onClick={() => setSelectedEntry(entry)}
                        className="action-icon-btn"
                        style={{ padding: '4px' }}
                      >
                        <MoreVertical size={16} />
                      </button>
                    </div>
                  </td>
                </tr>
              );
            })}

            {displayedQueue.length === 0 && (
              <tr>
                <td colSpan={7} style={{ textAlign: 'center', padding: '36px 16px', color: '#64748B' }}>
                  <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '8px' }}>
                    <AlertTriangle size={32} color="#CBD5E1" />
                    <span style={{ fontWeight: 500, fontSize: '0.9375rem' }}>No patients currently waiting in emergency queue.</span>
                    <span style={{ fontSize: '0.8125rem', color: '#94A3B8' }}>Use &quot;Register Patient&quot; or &quot;Add Visit&quot; to enqueue patients into the Min-Heap.</span>
                  </div>
                </td>
              </tr>
            )}
          </tbody>
        </table>
      </div>

      {/* Footer Link */}
      {showViewAll && queue.length > limit && (
        <div style={{ padding: '12px 18px', textAlign: 'right', borderTop: '1px solid #F1F5F9' }}>
          <button
            onClick={() => setActiveTab('queue')}
            style={{
              background: 'none',
              border: 'none',
              color: '#2563EB',
              fontWeight: 600,
              fontSize: '0.8125rem',
              cursor: 'pointer',
              display: 'inline-flex',
              alignItems: 'center',
              gap: '4px',
            }}
          >
            <span>View Full Queue ({queue.length})</span>
            <ArrowRight size={14} />
          </button>
        </div>
      )}

      {/* Entry Details Modal */}
      <Modal
        isOpen={selectedEntry !== null}
        onClose={() => setSelectedEntry(null)}
        title="Emergency Queue Entry Details"
        subtitle={`Min-Heap Priority Record for Patient ${selectedEntry?.patientId}`}
      >
        {selectedEntry && (
          <div style={{ display: 'flex', flexDirection: 'column', gap: '14px', fontSize: '0.875rem' }}>
            <div style={{ display: 'flex', justifyContent: 'space-between', padding: '8px 0', borderBottom: '1px solid #F1F5F9' }}>
              <span style={{ color: '#64748B' }}>Patient Name</span>
              <span style={{ fontWeight: 600 }}>{selectedEntry.patientName || 'N/A'}</span>
            </div>
            <div style={{ display: 'flex', justifyContent: 'space-between', padding: '8px 0', borderBottom: '1px solid #F1F5F9' }}>
              <span style={{ color: '#64748B' }}>Patient ID</span>
              <span style={{ fontWeight: 600, fontFamily: 'monospace' }}>{selectedEntry.patientId}</span>
            </div>
            <div style={{ display: 'flex', justifyContent: 'space-between', padding: '8px 0', borderBottom: '1px solid #F1F5F9' }}>
              <span style={{ color: '#64748B' }}>Triage Level</span>
              <TriageBadge level={selectedEntry.triageLevel} showSeverity />
            </div>
            <div style={{ display: 'flex', justifyContent: 'space-between', padding: '8px 0', borderBottom: '1px solid #F1F5F9' }}>
              <span style={{ color: '#64748B' }}>Min-Heap Rank</span>
              <span style={{ fontWeight: 700, color: '#2563EB' }}>#{selectedEntry.rank}</span>
            </div>
            <div style={{ display: 'flex', justifyContent: 'space-between', padding: '8px 0' }}>
              <span style={{ color: '#64748B' }}>Arrival Sequence</span>
              <span style={{ fontWeight: 500 }}>{selectedEntry.arrivalSequence}</span>
            </div>
            <div style={{ display: 'flex', justifyContent: 'flex-end', marginTop: '12px' }}>
              <button
                className="btn-secondary"
                onClick={() => setSelectedEntry(null)}
                style={{ padding: '8px 18px' }}
              >
                Close
              </button>
            </div>
          </div>
        )}
      </Modal>
    </div>
  );
};

