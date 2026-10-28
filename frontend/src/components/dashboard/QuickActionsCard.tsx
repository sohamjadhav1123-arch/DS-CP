import React, { useState } from 'react';
import { useApp } from '../../context/AppContext';
import { Modal } from '../common/Modal';
import { UserPlus, PlusCircle, AlertTriangle, FileText, ChevronRight } from 'lucide-react';

export const QuickActionsCard: React.FC = () => {
  const {
    setActiveTab,
    addPatient,
    addVisit,
    selectedHospitalId,
    patients,
  } = useApp();

  // Modals
  const [showRegisterModal, setShowRegisterModal] = useState(false);
  const [showVisitModal, setShowVisitModal] = useState(false);

  // Form states - Patient
  const [pId, setPId] = useState('');
  const [pName, setPName] = useState('');
  const [pAge, setPAge] = useState('32');
  const [pGender, setPGender] = useState('Male');
  const [pBlood, setPBlood] = useState('O+');
  const [pPhone, setPPhone] = useState('+91 98765 43210');
  const [pLoading, setPLoading] = useState(false);

  // Form states - Visit
  const [vPatientId, setVPatientId] = useState('');
  const [vTriage, setVTriage] = useState('2');
  const [vComplaint, setVComplaint] = useState('');
  const [vAdmitQueue, setVAdmitQueue] = useState(true);
  const [vLoading, setVLoading] = useState(false);

  const handleRegisterPatient = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!pId || !pName) return;
    setPLoading(true);
    const res = await addPatient({
      patientId: pId,
      name: pName,
      age: parseInt(pAge, 10) || 25,
      gender: pGender,
      bloodGroup: pBlood,
      contactPhone: pPhone,
    });
    setPLoading(false);
    if (res) {
      setPId('');
      setPName('');
      setShowRegisterModal(false);
    }
  };

  const handleAddVisit = async (e: React.FormEvent) => {
    e.preventDefault();
    const pid = vPatientId || (patients.length > 0 ? patients[0].patientId : '');
    if (!pid || !vComplaint) return;
    setVLoading(true);
    const res = await addVisit({
      patientId: pid,
      triageLevel: parseInt(vTriage, 10) || 3,
      chiefComplaint: vComplaint,
      admitToEmergencyQueue: vAdmitQueue,
      arrivalTime: new Date().toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' }),
    });
    setVLoading(false);
    if (res) {
      setVComplaint('');
      setShowVisitModal(false);
    }
  };

  return (
    <div className="content-card">
      <div className="card-header" style={{ marginBottom: '12px' }}>
        <h2 className="card-title">Quick Actions</h2>
      </div>

      <div style={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
        <button
          onClick={() => setShowRegisterModal(true)}
          className="quick-action-item"
        >
          <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
            <div className="quick-action-icon" style={{ backgroundColor: '#EFF6FF', color: '#2563EB' }}>
              <UserPlus size={16} />
            </div>
            <span style={{ fontWeight: 600, fontSize: '0.875rem', color: '#1E293B' }}>
              Register Patient
            </span>
          </div>
          <ChevronRight size={16} color="#94A3B8" />
        </button>

        <button
          onClick={() => setShowVisitModal(true)}
          className="quick-action-item"
        >
          <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
            <div className="quick-action-icon" style={{ backgroundColor: '#F0FDF4', color: '#16A34A' }}>
              <PlusCircle size={16} />
            </div>
            <span style={{ fontWeight: 600, fontSize: '0.875rem', color: '#1E293B' }}>
              Add Visit
            </span>
          </div>
          <ChevronRight size={16} color="#94A3B8" />
        </button>

        <button
          onClick={() => setActiveTab('queue')}
          className="quick-action-item"
        >
          <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
            <div className="quick-action-icon" style={{ backgroundColor: '#FEF2F2', color: '#EF4444' }}>
              <AlertTriangle size={16} />
            </div>
            <span style={{ fontWeight: 600, fontSize: '0.875rem', color: '#1E293B' }}>
              View Queue
            </span>
          </div>
          <ChevronRight size={16} color="#94A3B8" />
        </button>

        <button
          onClick={() => setActiveTab('patients')}
          className="quick-action-item"
        >
          <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
            <div className="quick-action-icon" style={{ backgroundColor: '#F8FAFC', color: '#64748B' }}>
              <FileText size={16} />
            </div>
            <span style={{ fontWeight: 600, fontSize: '0.875rem', color: '#1E293B' }}>
              View Records
            </span>
          </div>
          <ChevronRight size={16} color="#94A3B8" />
        </button>
      </div>

      {/* Register Patient Modal */}
      <Modal
        isOpen={showRegisterModal}
        onClose={() => setShowRegisterModal(false)}
        title="Register New Patient"
        subtitle={`Inserting into Custom Hash Table registry (Hospital: ${selectedHospitalId})`}
      >
        <form onSubmit={handleRegisterPatient} style={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Patient ID *
            </label>
            <input
              type="text"
              required
              placeholder="e.g. 1009"
              value={pId}
              onChange={(e) => setPId(e.target.value)}
              className="form-input"
            />
          </div>

          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Full Name *
            </label>
            <input
              type="text"
              required
              placeholder="e.g. Ananya Sen"
              value={pName}
              onChange={(e) => setPName(e.target.value)}
              className="form-input"
            />
          </div>

          <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '12px' }}>
            <div>
              <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
                Age
              </label>
              <input
                type="number"
                min="0"
                max="125"
                value={pAge}
                onChange={(e) => setPAge(e.target.value)}
                className="form-input"
              />
            </div>
            <div>
              <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
                Gender
              </label>
              <select
                value={pGender}
                onChange={(e) => setPGender(e.target.value)}
                className="form-input"
              >
                <option value="Male">Male</option>
                <option value="Female">Female</option>
                <option value="Other">Other</option>
              </select>
            </div>
          </div>

          <div style={{ display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '12px' }}>
            <div>
              <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
                Blood Group
              </label>
              <select
                value={pBlood}
                onChange={(e) => setPBlood(e.target.value)}
                className="form-input"
              >
                <option value="A+">A+</option>
                <option value="A-">A-</option>
                <option value="B+">B+</option>
                <option value="B-">B-</option>
                <option value="AB+">AB+</option>
                <option value="AB-">AB-</option>
                <option value="O+">O+</option>
                <option value="O-">O-</option>
              </select>
            </div>
            <div>
              <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
                Contact Phone
              </label>
              <input
                type="text"
                value={pPhone}
                onChange={(e) => setPPhone(e.target.value)}
                className="form-input"
              />
            </div>
          </div>

          <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '10px', marginTop: '14px' }}>
            <button
              type="button"
              onClick={() => setShowRegisterModal(false)}
              className="btn-secondary"
            >
              Cancel
            </button>
            <button
              type="submit"
              disabled={pLoading}
              className="btn-primary"
            >
              {pLoading ? 'Registering...' : 'Register Patient'}
            </button>
          </div>
        </form>
      </Modal>

      {/* Add Visit Modal */}
      <Modal
        isOpen={showVisitModal}
        onClose={() => setShowVisitModal(false)}
        title="Record Patient Visit"
        subtitle="Appends to Singly Linked List visit history & optionally enqueues into Min-Heap"
      >
        <form onSubmit={handleAddVisit} style={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Select Patient *
            </label>
            <select
              value={vPatientId}
              onChange={(e) => setVPatientId(e.target.value)}
              className="form-input"
              required
            >
              <option value="">-- Choose Patient --</option>
              {patients.map((p) => (
                <option key={p.patientId} value={p.patientId}>
                  {p.name} ({p.patientId})
                </option>
              ))}
            </select>
          </div>

          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Triage Severity (1 = Highest / Critical) *
            </label>
            <select
              value={vTriage}
              onChange={(e) => setVTriage(e.target.value)}
              className="form-input"
            >
              <option value="1">Level 1 - Critical (Immediate resuscitation)</option>
              <option value="2">Level 2 - Emergency (Chest pain, acute distress)</option>
              <option value="3">Level 3 - Urgent (Severe pain, high fever)</option>
              <option value="4">Level 4 - Less Urgent (Mild trauma)</option>
              <option value="5">Level 5 - Non-Urgent (Routine follow-up)</option>
            </select>
          </div>

          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Chief Complaint / Symptoms *
            </label>
            <textarea
              required
              rows={3}
              placeholder="e.g. Acute severe chest pain radiating to left arm..."
              value={vComplaint}
              onChange={(e) => setVComplaint(e.target.value)}
              className="form-input"
              style={{ resize: 'vertical' }}
            />
          </div>

          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', marginTop: '4px' }}>
            <input
              type="checkbox"
              id="admitQueue"
              checked={vAdmitQueue}
              onChange={(e) => setVAdmitQueue(e.target.checked)}
              style={{ width: '16px', height: '16px', accentColor: '#2563EB', cursor: 'pointer' }}
            />
            <label htmlFor="admitQueue" style={{ fontSize: '0.8125rem', color: '#1E293B', cursor: 'pointer' }}>
              Admit directly to Emergency Min-Heap Queue
            </label>
          </div>

          <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '10px', marginTop: '14px' }}>
            <button
              type="button"
              onClick={() => setShowVisitModal(false)}
              className="btn-secondary"
            >
              Cancel
            </button>
            <button
              type="submit"
              disabled={vLoading}
              className="btn-primary"
            >
              {vLoading ? 'Recording...' : 'Record Visit'}
            </button>
          </div>
        </form>
      </Modal>
    </div>
  );
};

