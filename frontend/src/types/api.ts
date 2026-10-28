export interface Hospital {
  hospitalId: string;
  name: string;
  city: string;
  totalEmergencyBeds: number;
  registeredPatientCount?: number;
  activeQueueSize?: number;
}

export interface Patient {
  patientId: string;
  hospitalId: string;
  name: string;
  age: number;
  gender: string;
  bloodGroup: string;
  contactPhone: string;
}

export interface QueueEntry {
  rank: number;
  hospitalId: string;
  patientId: string;
  patientName: string;
  triageLevel: number;
  triageSeverity: string;
  arrivalSequence: number;
}

export interface Visit {
  visitId: string;
  hospitalId: string;
  patientId: string;
  patientName?: string;
  arrivalTime: string;
  triageLevel: number;
  triageSeverity?: string;
  chiefComplaint: string;
  diagnosis: string;
  status: 'Waiting' | 'In Treatment' | 'Admitted' | 'Completed' | 'Discharged';
  notes?: string;
  arrivalSequence?: number;
}

export interface HospitalStats {
  hospitalId: string;
  hospitalName: string;
  city: string;
  totalEmergencyBeds: number;
  registeredPatientCount: number;
  activeQueueSize: number;
  totalVisitsRecorded: number;
  waitingVisits: number;
  inTreatmentVisits: number;
  completedVisits: number;
}

export interface HealthStatus {
  status: string;
  version: string;
  databaseConnected: boolean;
  hospitalCount: number;
}

export interface ApiResponse<T> {
  success: boolean;
  data?: T;
  error?: {
    code: string;
    message: string;
  };
}

