import React, { useState } from 'react';
import { useApp } from '../context/AppContext';
import { TriageBadge } from '../components/common/TriageBadge';
import { StatusBadge } from '../components/common/StatusBadge';
import { Modal } from '../components/common/Modal';
import {
  AlertTriangle,
  Play,
  Clock,
  ArrowUpDown,
  Search,
  Filter,
  Layers,
  Sparkles,
} from 'lucide-react';

export const EmergencyQueuePage: React.FC = () => {
  const { queue, serveNext, updateTriage } = useApp();
  const [searchTerm, setSearchTerm] = useState('');
  const [selectedTriageFilter, setSelectedTriageFilter] = useState<string>('all');
  const [isServing, setIsServing] = useState(false);

  // Edit triage modal
  const [editPatient, setEditPatient] = useState<{ id: string; name: string; currentTriage: number } | null>(null);
  const [newTriageLevel, setNewTriageLevel] = useState<number>(1);
  const [isUpdatingTriage, setIsUpdatingTriage] = useState(false);

  const filteredQueue = queue.filter((entry) => {
    const matchesSearch =
      entry.patientName.toLowerCase().includes(searchTerm.toLowerCase()) ||
      entry.patientId.includes(searchTerm);
    const matchesFilter =
      selectedTriageFilter === 'all' || entry.triageLevel.toString() === selectedTriageFilter;
    return matchesSearch && matchesFilter;
  });

  const handleServe = async () => {
    setIsServing(true);
    await serveNext();
    setIsServing(false);
  };

  const handleUpdateTriageSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!editPatient) return;
    setIsUpdatingTriage(true);
    await updateTriage(editPatient.id, newTriageLevel);
    setIsUpdatingTriage(false);
    setEditPatient(null);
  };

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
      {/* Header Banner */}
      <div className="content-card" style={{ padding: '24px' }}>
        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', flexWrap: 'wrap', gap: '16px' }}>
          <div>
            <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
              <div
                style={{
                  width: '36px',
                  height: '36px',
                  borderRadius: '10px',
                  backgroundColor: '#FEE2E2',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                }}
              >
                <AlertTriangle size={20} color="#EF4444" />
              </div>
              <h1 style={{ fontSize: '1.5rem', fontWeight: 800, color: '#0F172A', margin: 0 }}>
                Emergency Priority Queue
              </h1>
            </div>
            <p style={{ color: '#64748B', fontSize: '0.875rem', marginTop: '6px', margin: 0 }}>
              Live C++ Min-Heap prioritizing patients by Triage Level (1 = Critical) and Arrival Sequence with \(O(\log N)\) heap operations.
            </p>
          </div>

          <div style={{ display: 'flex', gap: '12px' }}>
            <button
              onClick={handleServe}
              disabled={isServing || queue.length === 0}
              className="btn-primary"
              style={{
                backgroundColor: '#DC2626',
                borderColor: '#DC2626',
                padding: '10px 20px',
                fontSize: '0.875rem',
              }}
            >
              <Play size={15} fill="#FFFFFF" />
              <span>{isServing ? 'Serving...' : 'Serve Highest Priority Patient'}</span>
            </button>
          </div>
        </div>

        {/* DSA Info Callout */}
        <div
          style={{
            marginTop: '18px',
            padding: '12px 16px',
            backgroundColor: '#F8FAFC',
            borderRadius: '8px',
            border: '1px solid #E2E8F0',
            display: 'flex',
            alignItems: 'center',
            gap: '12px',
            fontSize: '0.8125rem',
            color: '#334155',
          }}
        >
          <Layers size={18} color="#2563EB" style={{ flexShrink: 0 }} />
          <span>
            <strong>Min-Heap Guarantee:</strong> The root patient always has the lowest numerical triage level. Ties are strictly ordered by earliest arrival sequence. Root pop executes in \(O(\log N)\).
          </span>
        </div>
      </div>

      {/* Filter and Table Card */}
      <div className="content-card">
        {/* Controls Bar */}
        <div
          style={{
            padding: '16px 20px',
            borderBottom: '1px solid #E2E8F0',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'space-between',
            flexWrap: 'wrap',
            gap: '12px',
          }}
        >
          {/* Search */}
          <div style={{ position: 'relative', width: '280px' }}>
            <Search
              size={16}
              color="#94A3B8"
              style={{ position: 'absolute', left: '10px', top: '50%', transform: 'translateY(-50%)' }}
            />
            <input
              type="text"
              placeholder="Search by name or ID..."
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
              className="form-input"
              style={{ paddingLeft: '34px', fontSize: '0.8125rem' }}
            />
          </div>

          {/* Filter Pills */}
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
            <Filter size={15} color="#64748B" />
            <span style={{ fontSize: '0.8125rem', color: '#64748B', fontWeight: 500 }}>Triage:</span>
            {['all', '1', '2', '3', '4', '5'].map((lvl) => (
              <button
                key={lvl}
                onClick={() => setSelectedTriageFilter(lvl)}
                className={`action-icon-btn ${selectedTriageFilter === lvl ? 'active' : ''}`}
                style={{
                  padding: '4px 10px',
                  borderRadius: '6px',
                  fontSize: '0.75rem',
                  fontWeight: selectedTriageFilter === lvl ? 700 : 500,
                  backgroundColor: selectedTriageFilter === lvl ? '#2563EB' : '#F1F5F9',
                  color: selectedTriageFilter === lvl ? '#FFFFFF' : '#475569',
                  border: 'none',
                }}
              >
                {lvl === 'all' ? 'All' : `Level ${lvl}`}
              </button>
            ))}
          </div>
        </div>

        {/* Table */}
        <div className="smarter-table-wrapper">
          <table className="smarter-table">
            <thead>
              <tr>
                <th style={{ width: '50px' }}>Rank</th>
                <th>Patient Name</th>
                <th>Patient ID</th>
                <th>Triage Severity</th>
                <th>Arrival Seq</th>
                <th>Status</th>
                <th style={{ textAlign: 'right' }}>Actions</th>
              </tr>
            </thead>
            <tbody>
              {filteredQueue.map((entry) => (
                <tr key={`${entry.patientId}-${entry.rank}`}>
                  <td>
                    <span
                      style={{
                        width: '26px',
                        height: '26px',
                        borderRadius: '50%',
                        backgroundColor: entry.rank === 1 ? '#FEE2E2' : '#F1F5F9',
                        color: entry.rank === 1 ? '#EF4444' : '#475569',
                        fontWeight: 700,
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'center',
                        fontSize: '0.75rem',
                      }}
                    >
                      {entry.rank}
                    </span>
                  </td>
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
                    <TriageBadge level={entry.triageLevel} showSeverity />
                  </td>
                  <td>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '5px', color: '#64748B', fontSize: '0.8125rem' }}>
                      <Clock size={13} />
                      <span>Seq #{entry.arrivalSequence}</span>
                    </div>
                  </td>
                  <td>
                    <StatusBadge status="Waiting" />
                  </td>
                  <td>
                    <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'flex-end', gap: '8px' }}>
                      <button
                        onClick={() =>
                          setEditPatient({
                            id: entry.patientId,
                            name: entry.patientName,
                            currentTriage: entry.triageLevel,
                          })
                        }
                        className="btn-secondary"
                        style={{ padding: '4px 10px', fontSize: '0.75rem' }}
                      >
                        <ArrowUpDown size={12} />
                        <span>Update Triage</span>
                      </button>

                      {entry.rank === 1 && (
                        <button
                          onClick={handleServe}
                          disabled={isServing}
                          className="btn-primary"
                          style={{
                            padding: '4px 12px',
                            fontSize: '0.75rem',
                            backgroundColor: '#16A34A',
                            borderColor: '#16A34A',
                          }}
                        >
                          <Play size={11} fill="#FFFFFF" />
                          <span>Serve</span>
                        </button>
                      )}
                    </div>
                  </td>
                </tr>
              ))}

              {filteredQueue.length === 0 && (
                <tr>
                  <td colSpan={7} style={{ textAlign: 'center', padding: '40px 16px', color: '#64748B' }}>
                    <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', gap: '8px' }}>
                      <Sparkles size={28} color="#94A3B8" />
                      <span style={{ fontWeight: 600 }}>Emergency Queue is currently clear.</span>
                      <span style={{ fontSize: '0.8125rem', color: '#94A3B8' }}>
                        No patients are waiting. New patients added with &quot;Admit to Queue&quot; will appear here.
                      </span>
                    </div>
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      </div>

      {/* Update Triage Modal */}
      <Modal
        isOpen={editPatient !== null}
        onClose={() => setEditPatient(null)}
        title="Update Patient Triage Level"
        subtitle={`Reheapifying Min-Heap in O(log N) for patient: ${editPatient?.name} (${editPatient?.id})`}
      >
        <form onSubmit={handleUpdateTriageSubmit} style={{ display: 'flex', flexDirection: 'column', gap: '14px' }}>
          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '6px' }}>
              Select New Triage Level
            </label>
            <select
              value={newTriageLevel}
              onChange={(e) => setNewTriageLevel(parseInt(e.target.value, 10))}
              className="form-input"
            >
              <option value="1">1 - Critical (Immediate resuscitation)</option>
              <option value="2">2 - Emergency (Chest pain, acute distress)</option>
              <option value="3">3 - Urgent (Severe abdominal pain, high fever)</option>
              <option value="4">4 - Less Urgent (Mild fracture, sprain)</option>
              <option value="5">5 - Non-Urgent (Minor rashes, suture check)</option>
            </select>
          </div>

          <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '10px', marginTop: '12px' }}>
            <button
              type="button"
              onClick={() => setEditPatient(null)}
              className="btn-secondary"
            >
              Cancel
            </button>
            <button
              type="submit"
              disabled={isUpdatingTriage}
              className="btn-primary"
            >
              {isUpdatingTriage ? 'Updating...' : 'Update & Reheapify'}
            </button>
          </div>
        </form>
      </Modal>
    </div>
  );
};

