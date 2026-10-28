import React, { useState } from 'react';
import { useApp } from '../context/AppContext';
import { Modal } from '../components/common/Modal';
import { Users, UserPlus, Search, Phone, Heart, Calendar } from 'lucide-react';

export const PatientsPage: React.FC = () => {
  const { patients, selectedHospitalId, addPatient } = useApp();
  const [searchTerm, setSearchTerm] = useState('');
  const [showAddModal, setShowAddModal] = useState(false);

  // Form states
  const [pId, setPId] = useState('');
  const [pName, setPName] = useState('');
  const [pAge, setPAge] = useState('28');
  const [pGender, setPGender] = useState('Female');
  const [pBlood, setPBlood] = useState('A+');
  const [pPhone, setPPhone] = useState('+91 98234 56789');
  const [isSubmitting, setIsSubmitting] = useState(false);

  const filteredPatients = patients.filter(
    (p) =>
      p.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
      p.patientId.includes(searchTerm) ||
      p.bloodGroup.toLowerCase().includes(searchTerm.toLowerCase())
  );

  const handleRegister = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!pId || !pName) return;
    setIsSubmitting(true);
    const res = await addPatient({
      patientId: pId,
      name: pName,
      age: parseInt(pAge, 10) || 20,
      gender: pGender,
      bloodGroup: pBlood,
      contactPhone: pPhone,
    });
    setIsSubmitting(false);
    if (res) {
      setPId('');
      setPName('');
      setShowAddModal(false);
    }
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
                  backgroundColor: '#EFF6FF',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                }}
              >
                <Users size={20} color="#2563EB" />
              </div>
              <h1 style={{ fontSize: '1.5rem', fontWeight: 800, color: '#0F172A', margin: 0 }}>
                Patient Registry (Hash Table &amp; BST)
              </h1>
            </div>
            <p style={{ color: '#64748B', fontSize: '0.875rem', marginTop: '6px', margin: 0 }}>
              Patients are stored in a Custom Separate-Chaining Hash Table with \(O(1)\) lookup and a Binary Search Tree for sorted traversal.
            </p>
          </div>

          <button
            onClick={() => setShowAddModal(true)}
            className="btn-primary"
            style={{ padding: '10px 18px', fontSize: '0.875rem' }}
          >
            <UserPlus size={16} />
            <span>Register New Patient</span>
          </button>
        </div>
      </div>

      {/* Main Table Card */}
      <div className="content-card">
        {/* Search Bar */}
        <div style={{ padding: '16px 20px', borderBottom: '1px solid #E2E8F0', display: 'flex', alignItems: 'center', gap: '12px' }}>
          <div style={{ position: 'relative', maxWidth: '320px', width: '100%' }}>
            <Search
              size={16}
              color="#94A3B8"
              style={{ position: 'absolute', left: '10px', top: '50%', transform: 'translateY(-50%)' }}
            />
            <input
              type="text"
              placeholder="Search by name, ID, or blood group..."
              value={searchTerm}
              onChange={(e) => setSearchTerm(e.target.value)}
              className="form-input"
              style={{ paddingLeft: '34px', fontSize: '0.8125rem' }}
            />
          </div>
          <span style={{ fontSize: '0.8125rem', color: '#64748B' }}>
            Showing {filteredPatients.length} of {patients.length} registered patient(s)
          </span>
        </div>

        {/* Patients Table */}
        <div className="smarter-table-wrapper">
          <table className="smarter-table">
            <thead>
              <tr>
                <th>Patient ID</th>
                <th>Full Name</th>
                <th>Age / Gender</th>
                <th>Blood Group</th>
                <th>Contact</th>
                <th>Hospital ID</th>
              </tr>
            </thead>
            <tbody>
              {filteredPatients.map((p, index) => {
                const colors = ['#8B5CF6', '#0284C7', '#2563EB', '#D97706', '#059669', '#DB2777'];
                const bg = colors[index % colors.length];

                return (
                  <tr key={p.patientId}>
                    <td>
                      <span
                        style={{
                          fontFamily: 'monospace',
                          fontSize: '0.8125rem',
                          fontWeight: 700,
                          color: '#2563EB',
                          backgroundColor: '#EFF6FF',
                          padding: '3px 8px',
                          borderRadius: '5px',
                        }}
                      >
                        {p.patientId}
                      </span>
                    </td>
                    <td>
                      <div style={{ display: 'flex', alignItems: 'center', gap: '10px' }}>
                        <div
                          className="avatar-circle"
                          style={{
                            backgroundColor: bg,
                            width: '32px',
                            height: '32px',
                            fontSize: '0.75rem',
                          }}
                        >
                          {p.name.charAt(0).toUpperCase()}
                        </div>
                        <span style={{ fontWeight: 600, color: '#0F172A' }}>{p.name}</span>
                      </div>
                    </td>
                    <td>
                      <div style={{ display: 'flex', alignItems: 'center', gap: '6px', color: '#475569', fontSize: '0.8125rem' }}>
                        <Calendar size={13} color="#94A3B8" />
                        <span>{p.age} yrs</span>
                        <span style={{ color: '#CBD5E1' }}>•</span>
                        <span>{p.gender}</span>
                      </div>
                    </td>
                    <td>
                      <div style={{ display: 'inline-flex', alignItems: 'center', gap: '4px', backgroundColor: '#FEF2F2', padding: '2px 8px', borderRadius: '4px' }}>
                        <Heart size={12} color="#EF4444" fill="#EF4444" />
                        <span style={{ fontSize: '0.8125rem', fontWeight: 700, color: '#991B1B' }}>
                          {p.bloodGroup || 'O+'}
                        </span>
                      </div>
                    </td>
                    <td>
                      <div style={{ display: 'flex', alignItems: 'center', gap: '6px', color: '#64748B', fontSize: '0.8125rem' }}>
                        <Phone size={13} />
                        <span>{p.contactPhone || 'N/A'}</span>
                      </div>
                    </td>
                    <td>
                      <span style={{ fontSize: '0.8125rem', color: '#64748B' }}>
                        {p.hospitalId}
                      </span>
                    </td>
                  </tr>
                );
              })}

              {filteredPatients.length === 0 && (
                <tr>
                  <td colSpan={6} style={{ textAlign: 'center', padding: '36px 16px', color: '#64748B' }}>
                    No patients match your search.
                  </td>
                </tr>
              )}
            </tbody>
          </table>
        </div>
      </div>

      {/* Register Modal */}
      <Modal
        isOpen={showAddModal}
        onClose={() => setShowAddModal(false)}
        title="Register Patient in Custom Hash Table"
        subtitle={`Adding patient for Hospital ${selectedHospitalId}`}
      >
        <form onSubmit={handleRegister} style={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Patient ID *
            </label>
            <input
              type="text"
              required
              placeholder="e.g. 1010"
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
              placeholder="e.g. Meera Raman"
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
                <option value="Female">Female</option>
                <option value="Male">Male</option>
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

          <div style={{ display: 'flex', justifyContent: 'flex-end', gap: '10px', marginTop: '12px' }}>
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
              {isSubmitting ? 'Registering...' : 'Save Patient'}
            </button>
          </div>
        </form>
      </Modal>
    </div>
  );
};

