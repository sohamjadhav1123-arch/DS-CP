import React from 'react';
import { useApp } from '../../context/AppContext';
import { Building2, Users, AlertTriangle, BedDouble, History, CheckCircle2 } from 'lucide-react';

export const HospitalOverviewCard: React.FC = () => {
  const { selectedHospital, stats, patients, queue, visits } = useApp();

  const registeredPatients = stats?.registeredPatientCount ?? patients.length;
  const activeQueue = stats?.activeQueueSize ?? queue.length;
  const inTreatment = stats?.inTreatmentVisits ?? visits.filter((v) => v.status === 'In Treatment').length;
  const totalVisits = stats?.totalVisitsRecorded ?? visits.length;

  const hospitalName = selectedHospital?.name || 'CityCare Hospital';
  const hospitalId = selectedHospital?.hospitalId || '101';
  const hospitalCity = selectedHospital?.city || 'Pune';

  return (
    <div className="content-card">
      {/* Header */}
      <div className="card-header" style={{ marginBottom: '14px' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <Building2 size={18} color="#2563EB" />
          <h2 className="card-title">Hospital Overview</h2>
        </div>
        <span
          style={{
            fontSize: '0.6875rem',
            fontWeight: 600,
            color: '#059669',
            backgroundColor: '#D1FAE5',
            padding: '2px 8px',
            borderRadius: '10px',
          }}
        >
          Online
        </span>
      </div>

      {/* Hospital Banner Info */}
      <div style={{ display: 'flex', gap: '14px', alignItems: 'center', marginBottom: '18px' }}>
        <div
          style={{
            width: '68px',
            height: '68px',
            borderRadius: '10px',
            backgroundColor: '#0F172A',
            backgroundImage:
              'url("https://images.unsplash.com/photo-1519494026892-80bbd2d6fd0d?auto=format&fit=crop&w=160&q=80")',
            backgroundSize: 'cover',
            backgroundPosition: 'center',
            boxShadow: '0 2px 8px rgba(0,0,0,0.1)',
            flexShrink: 0,
          }}
        />
        <div style={{ minWidth: 0, flex: 1 }}>
          <div style={{ fontWeight: 700, fontSize: '0.9375rem', color: '#0F172A', lineHeight: 1.2 }}>
            {hospitalName}
          </div>
          <div style={{ fontSize: '0.75rem', color: '#64748B', marginTop: '2px' }}>
            ID: {hospitalId}
          </div>
          <div style={{ fontSize: '0.75rem', color: '#94A3B8', marginTop: '2px' }}>
            123 Health Lane, {hospitalCity}
          </div>
        </div>
      </div>

      {/* Metric List */}
      <div style={{ display: 'flex', flexDirection: 'column', gap: '10px' }}>
        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', fontSize: '0.8125rem' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#475569' }}>
            <Users size={15} color="#64748B" />
            <span>Registered Patients</span>
          </div>
          <span style={{ fontWeight: 700, color: '#0F172A' }}>{registeredPatients}</span>
        </div>

        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', fontSize: '0.8125rem' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#475569' }}>
            <AlertTriangle size={15} color="#EF4444" />
            <span>Active Queue</span>
          </div>
          <span style={{ fontWeight: 700, color: '#EF4444' }}>{activeQueue}</span>
        </div>

        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', fontSize: '0.8125rem' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#475569' }}>
            <BedDouble size={15} color="#16A34A" />
            <span>In Treatment</span>
          </div>
          <span style={{ fontWeight: 700, color: '#16A34A' }}>{inTreatment}</span>
        </div>

        <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', fontSize: '0.8125rem' }}>
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#475569' }}>
            <History size={15} color="#64748B" />
            <span>Total Visits</span>
          </div>
          <span style={{ fontWeight: 700, color: '#0F172A' }}>{totalVisits}</span>
        </div>
      </div>

      {/* System Status Banner */}
      <div
        style={{
          marginTop: '18px',
          padding: '12px 14px',
          backgroundColor: '#F0FDF4',
          borderRadius: '10px',
          border: '1px solid #DCFCE7',
          display: 'flex',
          alignItems: 'center',
          gap: '10px',
        }}
      >
        <div
          style={{
            width: '28px',
            height: '28px',
            borderRadius: '50%',
            backgroundColor: '#DCFCE7',
            display: 'flex',
            alignItems: 'center',
            justifyContent: 'center',
            flexShrink: 0,
          }}
        >
          <CheckCircle2 size={16} color="#16A34A" />
        </div>
        <div>
          <div style={{ fontSize: '0.8125rem', fontWeight: 700, color: '#166534' }}>
            System Status
          </div>
          <div style={{ fontSize: '0.75rem', color: '#15803D', marginTop: '1px' }}>
            • All systems operational
          </div>
        </div>
      </div>
    </div>
  );
};

