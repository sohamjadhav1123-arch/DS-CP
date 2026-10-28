import React, { useState } from 'react';
import { useApp } from '../context/AppContext';
import { TriageBadge } from '../components/common/TriageBadge';
import { StatusBadge } from '../components/common/StatusBadge';
import { Modal } from '../components/common/Modal';
import {
  History,
  PlusCircle,
  Search,
  Clock,
  Activity,
  CheckCircle,
  FileText,
  User,
} from 'lucide-react';

export const VisitsPage: React.FC = () => {
  const { visits, patients, addVisit, updateVisitStatus } = useApp();
  const [searchTerm, setSearchTerm] = useState('');
  const [showAddModal, setShowAddModal] = useState(false);

  // Add visit form states
  const [vPatientId, setVPatientId] = useState('');
  const [vTriage, setVTriage] = useState('2');
  const [vComplaint, setVComplaint] = useState('');
  const [vAdmitQueue, setVAdmitQueue] = useState(true);
  const [isSubmitting, setIsSubmitting] = useState(false);

  // Status update modal
  const [selectedVisit, setSelectedVisit] = useState<any | null>(null);
  const [newStatus, setNewStatus] = useState<string>('In Treatment');
  const [diagnosisText, setDiagnosisText] = useState<string>('');
  const [isUpdatingStatus, setIsUpdatingStatus] = useState(false);

  const filteredVisits = visits.filter(
    (v) =>
      v.patientId.includes(searchTerm) ||
      (v.patientName && v.patientName.toLowerCase().includes(searchTerm.toLowerCase())) ||
      v.chiefComplaint.toLowerCase().includes(searchTerm.toLowerCase()) ||
      v.diagnosis.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const handleAddSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!vPatientId || !vComplaint) return;
    setIsSubmitting(true);
    const res = await addVisit({
      patientId: vPatientId,
      triageLevel: parseInt(vTriage, 10) || 3,
      chiefComplaint: vComplaint,
      admitToEmergencyQueue: vAdmitQueue,
      arrivalTime: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }),
    });
    setIsSubmitting(false);
    if (res) {
      setVComplaint('');
      setShowAddModal(false);
    }
  };

  const handleUpdateStatusSubmit = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!selectedVisit) return;
    setIsUpdatingStatus(true);
    await updateVisitStatus(selectedVisit.patientId, selectedVisit.visitId, newStatus, diagnosisText);
    setIsUpdatingStatus(false);
    setSelectedVisit(null);
  };

  return (
    <div style={{ display: 'flex', flexDirection: 'column', gap: '20px' }}>
      {/* Page Header */}
      <div className="content-card" style={{ padding: '24px' }}>
        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', flexWrap: 'wrap', gap: '16px' }}>
          <div>
            <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
              <div
                style={{
                  width: '36px',
                  height: '36px',
                  borderRadius: '10px',
                  backgroundColor: '#F3E8FF',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                }}
              >
                <History size={20} color="#8B5CF6" />
              </div>
              <h1 style={{ fontSize: '1.5rem', fontWeight: 800, color: '#0F172A', margin: 0 }}>
                Patient Visit History
              </h1>
            </div>
            <p style={{ color: '#64748B', fontSize: '0.875rem', marginTop: '6px', margin: 0 }}>
              Each patient has a dedicated Singly Linked List preserving chronological visits, complaints, triage, and diagnoses.
            </p>
          </div>

          <button
            onClick={() => setShowAddModal(true)}
            className="btn-primary"
            style={{ padding: '10px 18px', fontSize: '0.875rem' }}
          >
            <PlusCircle size={16} />
            <span>Record New Visit</span>
          </button>
        </div>
      </div>

      {/* Visits Table Card */}
      <div className="content-card">
        {/* Search */}
        <div style={{ padding: '16px 20px', borderBottom: '1px solid #E2E8F0', display: 'flex', alignItems: 'center', gap: '12px' }}>
          <div style={{ position: 'relative', maxWidth: '340px', width: '100%' }}>
            <Search
              size={16}
              color="#94A3B8"
              style={{ position: 'absolute', left: '10px', top: '50%', transform: 'translateY(-50%)' }}
            />
            <input
              type="text"
              placeholder="Search by patient ID, name, or diagnosis..."
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
              className="form-input"
              style={{ paddingLeft: '34px', fontSize: '0.8125rem' }}
            />
          </div>
          <span style={{ fontSize: '0.8125rem', color: '#64748B' }}>
            Showing {filteredVisits.length} recorded visit(s)
          </span>
        </div>

        {/* Table */}
        <div className="smarter-table-wrapper">
          <table className="smarter-table">
            <thead>
              <tr>
                <th>Visit ID</th>
                <th>Patient</th>
                <th>Arrival Time</th>
                <th>Triage</th>
                <th>Chief Complaint</th>
                <th>Diagnosis</th>
                <th>Status</th>
                <th style={{ textAlign: 'right' }}>Actions</th>
              </tr>
            </thead>
            <tbody>
              {filteredVisits.map((v) => (
                <tr key={v.visitId}>
                  <td>
                    <span
                      style={{
                        fontFamily: 'monospace',
                        fontSize: '0.8125rem',
                        fontWeight: 600,
                        color: '#64748B',
                      }}
                    >
                      {v.visitId}
                    </span>
                  </td>
                  <td>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                      <User size={14} color="#94A3B8" />
                      <div>
                        <div style={{ fontWeight: 600, color: '#0F172A', fontSize: '0.875rem' }}>
                          {v.patientName || `Patient ${v.patientId}`}
                        </div>
                        <div style={{ fontSize: '0.6875rem', color: '#64748B', fontFamily: 'monospace' }}>
                          ID: {v.patientId}
                        </div>
                      </div>
                    </div>
                  </td>
                  <td>
                    <div style={{ display: 'flex', alignItems: 'center', gap: '4px', fontSize: '0.8125rem', color: '#64748B' }}>
                      <Clock size={13} />
                      <span>{v.arrivalTime || 'N/A'}</span>
                    </div>
                  </td>
                  <td>
                    <TriageBadge level={v.triageLevel} showSeverity={false} />
                  </td>
                  <td>
                    <span style={{ fontSize: '0.8125rem', color: '#334155', maxWidth: '200px', display: 'inline-block' }}>
                      {v.chiefComplaint}
                    </span>
                  </td>
                  <td>
                    <span style={{ fontSize: '0.8125rem', color: '#64748B', fontStyle: v.diagnosis ? 'normal' : 'italic' }}>
                      {v.diagnosis || 'Pending examination'}
                    </span>
                  </td>
                  <td>
                    <StatusBadge status={v.status} />
                  </td>
                  <td>
                    <div style={{ display: 'flex', justifyContent: 'flex-end' }}>
                      <button
                        onClick={() => {
                          setSelectedVisit(v);
                          setNewStatus(v.status);
                          setDiagnosisText(v.diagnosis || '');
                        }}
                        className="btn-secondary"
                        style={{ padding: '4px 10px', fontSize: '0.75rem' }}
                      >
                        <Activity size={12} />
                        <span>Update Status</span>
                      </button>
                    </div>
                  </td>
                </tr>
              ))}

              {filteredVisits.length === 0 && (
                <tr>
                  <td colSpan={8} style={{ textAlign: 'center', padding: '36px 16px', color: '#64748B' }}>
                    No visit records found in patient linked lists.
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      </div>

      {/* Record Visit Modal */}
      <Modal
        isOpen={showAddModal}
        onClose={() => setShowAddModal(false)}
        title="Record New Patient Visit"
        subtitle="Appends to the patient's custom singly linked list"
      >
        <form onSubmit={handleAddSubmit} style={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Patient *
            </label>
            <select
              value={vPatientId}
              onChange={(e) => setVPatientId(e.target.value)}
              className="form-input"
              required
            >
              <option value="">-- Select Patient --</option>
              {patients.map((p) => (
                <option key={p.patientId} value={p.patientId}>
                  {p.name} ({p.patientId})
                </option>
              ))}
            </select>
          </div>

          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Triage Level *
            </label>
            <select
              value={vTriage}
              onChange={(e) => setVTriage(e.target.value)}
              className="form-input"
            >
              <option value="1">Level 1 - Critical (Immediate resuscitation)</option>
              <option value="2">Level 2 - Emergency (Severe trauma/pain)</option>
              <option value="3">Level 3 - Urgent (Acute symptoms)</option>
              <option value="4">Level 4 - Less Urgent (Mild injury)</option>
              <option value="5">Level 5 - Non-Urgent (Routine clinic visit)</option>
            </select>
          </div>

          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Chief Complaint *
            </label>
            <textarea
              required
              rows={3}
              placeholder="e.g. High fever, shortness of breath..."
              value={vComplaint}
              onChange={(e) => setVComplaint(e.target.value)}
              className="form-input"
            />
          </div>

          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', marginTop: '4px' }}>
            <input
              type="checkbox"
              id="visitAdmitQueue"
              checked={vAdmitQueue}
              onChange={(e) => setVAdmitQueue(e.target.checked)}
              style={{ width: '16px', height: '16px', accentColor: '#2563EB', cursor: 'pointer' }}
            />
            <label htmlFor="visitAdmitQueue" style={{ fontSize: '0.8125rem', color: '#1E293B', cursor: 'pointer' }}>
              Enqueue into Min-Heap Emergency Queue
            </label>
          </div>

          <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '10px', marginTop: '14px' }}>
            <button
              type="button"
              onClick={() => setShowAddModal(false)}
              className="btn-secondary"
            >
              Cancel
            </button>
            <button
              type="submit"
              disabled={isSubmitting}
              className="btn-primary"
            >
              {isSubmitting ? 'Recording...' : 'Record Visit'}
            </button>
          </div>
        </form>
      </Modal>

      {/* Update Status Modal */}
      <Modal
        isOpen={selectedVisit !== null}
        onClose={() => setSelectedVisit(null)}
        title="Update Visit Status & Diagnosis"
        subtitle={`Visit ${selectedVisit?.visitId} (Patient: ${selectedVisit?.patientId})`}
      >
        <form onSubmit={handleUpdateStatusSubmit} style={{ display: 'flex', flexDirection: 'column', gap: '14px' }}>
          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Clinical Status
            </label>
            <select
              value={newStatus}
              onChange={(e) => setNewStatus(e.target.value)}
              className="form-input"
            >
              <option value="Waiting">Waiting</option>
              <option value="In Treatment">In Treatment</option>
              <option value="Admitted">Admitted</option>
              <option value="Completed">Completed</option>
              <option value="Discharged">Discharged</option>
            </select>
          </div>

          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Physician Diagnosis / Notes
            </label>
            <textarea
              rows={3}
              placeholder="e.g. Acute bronchitis, prescribed antibiotics and inhaler."
              value={diagnosisText}
              onChange={(e) => setDiagnosisText(e.target.value)}
              className="form-input"
            />
          </div>

          <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '10px', marginTop: '12px' }}>
            <button
              type="button"
              onClick={() => setSelectedVisit(null)}
              className="btn-secondary"
            >
              Cancel
            </button>
            <button
              type="submit"
              disabled={isUpdatingStatus}
              className="btn-primary"
            >
              {isUpdatingStatus ? 'Saving...' : 'Save Updates'}
            </button>
          </div>
        </form>
      </Modal>
    </div>
  );
};

