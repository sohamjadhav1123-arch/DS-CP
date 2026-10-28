import React, { useState } from 'react';
import { useApp } from '../context/AppContext';
import { Modal } from '../components/common/Modal';
import { Building2, Plus, BedDouble, Users, MapPin, CheckCircle2 } from 'lucide-react';

export const HospitalsPage: React.FC = () => {
  const {
    hospitals,
    selectedHospitalId,
    setSelectedHospitalId,
    addHospital,
  } = useApp();

  const [showAddModal, setShowAddModal] = useState(false);
  const [hId, setHId] = useState('');
  const [hName, setHName] = useState('');
  const [hCity, setHCity] = useState('');
  const [hBeds, setHBeds] = useState('50');
  const [isSubmitting, setIsSubmitting] = useState(false);

  const handleCreate = async (e: React.FormEvent) => {
    e.preventDefault();
    if (!hId || !hName || !hCity) return;
    setIsSubmitting(true);
    const res = await addHospital({
      hospitalId: hId,
      name: hName,
      city: hCity,
      totalEmergencyBeds: parseInt(hBeds, 10) || 20,
    });
    setIsSubmitting(false);
    if (res) {
      setHId('');
      setHName('');
      setHCity('');
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
                <Building2 size={20} color="#2563EB" />
              </div>
              <h1 style={{ fontSize: '1.5rem', fontWeight: 800, color: '#0F172A', margin: 0 }}>
                Hospitals Network
              </h1>
            </div>
            <p style={{ color: '#64748B', fontSize: '0.875rem', marginTop: '6px', margin: 0 }}>
              SmartER supports multi-hospital networks, each maintaining isolated Min-Heaps and patient registries.
            </p>
          </div>

          <button
            onClick={() => setShowAddModal(true)}
            className="btn-primary"
            style={{ padding: '10px 18px', fontSize: '0.875rem' }}
          >
            <Plus size={16} />
            <span>Add New Hospital</span>
          </button>
        </div>
      </div>

      {/* Hospital Cards Grid */}
      <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fill, minmax(320px, 1fr))', gap: '18px' }}>
        {hospitals.map((h) => {
          const isSelected = h.hospitalId === selectedHospitalId;
          return (
            <div
              key={h.hospitalId}
              className="content-card"
              style={{
                borderColor: isSelected ? '#2563EB' : '#E2E8F0',
                borderWidth: isSelected ? '2px' : '1px',
                position: 'relative',
                overflow: 'hidden',
              }}
            >
              {isSelected && (
                <div
                  style={{
                    position: 'absolute',
                    top: 0,
                    right: 0,
                    backgroundColor: '#2563EB',
                    color: '#FFFFFF',
                    fontSize: '0.6875rem',
                    fontWeight: 700,
                    padding: '3px 10px',
                    borderBottomLeftRadius: '8px',
                  }}
                >
                  Active Center
                </div>
              )}

              <div style={{ display: 'flex', alignItems: 'flex-start', gap: '14px', marginBottom: '14px' }}>
                <div
                  style={{
                    width: '48px',
                    height: '48px',
                    borderRadius: '10px',
                    backgroundColor: isSelected ? '#EFF6FF' : '#F8FAFC',
                    display: 'flex',
                    alignItems: 'center',
                    justifyContent: 'center',
                    flexShrink: 0,
                  }}
                >
                  <Building2 size={24} color={isSelected ? '#2563EB' : '#64748B'} />
                </div>
                <div>
                  <h3 style={{ fontSize: '1.05rem', fontWeight: 700, color: '#0F172A', margin: 0 }}>
                    {h.name}
                  </h3>
                  <div style={{ display: 'flex', alignItems: 'center', gap: '4px', color: '#64748B', fontSize: '0.8125rem', marginTop: '3px' }}>
                    <MapPin size={13} />
                    <span>{h.city}</span>
                    <span style={{ color: '#CBD5E1' }}>•</span>
                    <span style={{ fontFamily: 'monospace' }}>ID: {h.hospitalId}</span>
                  </div>
                </div>
              </div>

              {/* Stats info */}
              <div
                style={{
                  backgroundColor: '#F8FAFC',
                  borderRadius: '8px',
                  padding: '12px',
                  display: 'grid',
                  gridTemplateColumns: '1fr 1fr',
                  gap: '8px',
                  fontSize: '0.8125rem',
                  marginBottom: '14px',
                }}
              >
                <div>
                  <div style={{ color: '#64748B', fontSize: '0.75rem' }}>Emergency Beds</div>
                  <div style={{ fontWeight: 700, color: '#0F172A', display: 'flex', alignItems: 'center', gap: '5px', marginTop: '2px' }}>
                    <BedDouble size={14} color="#16A34A" />
                    <span>{h.totalEmergencyBeds}</span>
                  </div>
                </div>

                <div>
                  <div style={{ color: '#64748B', fontSize: '0.75rem' }}>Engine Status</div>
                  <div style={{ fontWeight: 700, color: '#059669', display: 'flex', alignItems: 'center', gap: '5px', marginTop: '2px' }}>
                    <CheckCircle2 size={14} color="#059669" />
                    <span>Online</span>
                  </div>
                </div>
              </div>

              {/* Action */}
              <button
                onClick={() => setSelectedHospitalId(h.hospitalId)}
                disabled={isSelected}
                className={isSelected ? 'btn-secondary' : 'btn-primary'}
                style={{ width: '100%', justifyContent: 'center' }}
              >
                {isSelected ? 'Currently Selected' : 'Switch to this Hospital'}
              </button>
            </div>
          );
        })}
      </div>

      {/* Add Hospital Modal */}
      <Modal
        isOpen={showAddModal}
        onClose={() => setShowAddModal(false)}
        title="Add Hospital Facility"
        subtitle="Registers a new healthcare center into SQLite and SmartER coordinator"
      >
        <form onSubmit={handleCreate} style={{ display: 'flex', flexDirection: 'column', gap: '12px' }}>
          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Hospital ID *
            </label>
            <input
              type="text"
              required
              placeholder="e.g. 103"
              value={hId}
              onChange={(e) => setHId(e.target.value)}
              className="form-input"
            />
          </div>

          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Hospital Name *
            </label>
            <input
              type="text"
              required
              placeholder="e.g. Apollo Memorial Hospital"
              value={hName}
              onChange={(e) => setHName(e.target.value)}
              className="form-input"
            />
          </div>

          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              City / Location *
            </label>
            <input
              type="text"
              required
              placeholder="e.g. Pune"
              value={hCity}
              onChange={(e) => setHCity(e.target.value)}
              className="form-input"
            />
          </div>

          <div>
            <label style={{ fontSize: '0.75rem', fontWeight: 600, color: '#475569', display: 'block', marginBottom: '4px' }}>
              Emergency Bed Capacity
            </label>
            <input
              type="number"
              min="1"
              value={hBeds}
              onChange={(e) => setHBeds(e.target.value)}
              className="form-input"
            />
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
              {isSubmitting ? 'Registering...' : 'Register Hospital'}
            </button>
          </div>
        </form>
      </Modal>
    </div>
  );
};

