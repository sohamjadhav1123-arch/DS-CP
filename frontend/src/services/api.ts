import {
  Hospital,
  Patient,
  QueueEntry,
  Visit,
  HospitalStats,
  HealthStatus,
  ApiResponse,
} from '../types/api';

// By default connect directly to C++ server at http://127.0.0.1:8080/api/v1
// If running in development with Vite proxy, '/api/v1' can also be used.
const DEFAULT_URL = import.meta.env.VITE_API_BASE_URL || 'http://127.0.0.1:8080/api/v1';

class ApiService {
  public baseUrl: string = DEFAULT_URL;

  private formatUrl(endpoint: string): string {
    const base = this.baseUrl.replace(/\/+$/, '');
    const path = endpoint.startsWith('/') ? endpoint : `/${endpoint}`;
    return `${base}${path}`;
  }

  private async request<T>(endpoint: string, options?: RequestInit): Promise<T> {
    const url = this.formatUrl(endpoint);
    try {
      const res = await fetch(url, {
        ...options,
        headers: {
          'Content-Type': 'application/json',
          Accept: 'application/json',
          ...(options?.headers || {}),
        },
      });

      const json: ApiResponse<T> = await res.json().catch(() => ({
        success: false,
        error: {
          code: 'INVALID_JSON',
          message: `Server returned non-JSON response with HTTP ${res.status}`,
        },
      }));

      if (!res.ok || !json.success) {
        const errorMsg = json.error?.message || `Request failed with HTTP status ${res.status}`;
        const errorCode = json.error?.code || 'HTTP_ERROR';
        const err = new Error(errorMsg);
        (err as any).code = errorCode;
        (err as any).status = res.status;
        throw err;
      }

      // Handle both unwrapped and wrapped payloads safely
      if (json.data !== undefined) {
        return json.data as T;
      }
      return json as unknown as T;
    } catch (err: any) {
      if (err.name === 'TypeError' && (err.message.includes('fetch') || err.message.includes('Failed'))) {
        const netErr = new Error(`Cannot connect to C++ backend at ${url}. Please verify smarter_server is running.`);
        (netErr as any).code = 'CONNECTION_REFUSED';
        (netErr as any).status = 0;
        throw netErr;
      }
      throw err;
    }
  }

  // Health
  public async getHealth(): Promise<HealthStatus> {
    return this.request<HealthStatus>('/health');
  }

  // Hospitals
  public async getHospitals(): Promise<Hospital[]> {
    return this.request<Hospital[]>('/hospitals');
  }

  public async createHospital(data: {
    hospitalId: string;
    name: string;
    city: string;
    totalEmergencyBeds: number;
  }): Promise<Hospital> {
    return this.request<Hospital>('/hospitals', {
      method: 'POST',
      body: JSON.stringify(data),
    });
  }

  // Patients
  public async getPatients(hospitalId: string): Promise<Patient[]> {
    return this.request<Patient[]>(`/hospitals/${encodeURIComponent(hospitalId)}/patients`);
  }

  public async getPatient(hospitalId: string, patientId: string): Promise<Patient> {
    return this.request<Patient>(
      `/hospitals/${encodeURIComponent(hospitalId)}/patients/${encodeURIComponent(patientId)}`
    );
  }

  public async createPatient(
    hospitalId: string,
    data: {
      patientId: string;
      name: string;
      age: number;
      gender: string;
      bloodGroup: string;
      contactPhone: string;
    }
  ): Promise<Patient> {
    return this.request<Patient>(`/hospitals/${encodeURIComponent(hospitalId)}/patients`, {
      method: 'POST',
      body: JSON.stringify(data),
    });
  }

  // Emergency Queue
  public async getQueue(hospitalId: string): Promise<QueueEntry[]> {
    return this.request<QueueEntry[]>(`/hospitals/${encodeURIComponent(hospitalId)}/queue`);
  }

  public async serveNextPatient(hospitalId: string): Promise<QueueEntry & { status: string }> {
    return this.request<QueueEntry & { status: string }>(
      `/hospitals/${encodeURIComponent(hospitalId)}/queue/serve-next`,
      { method: 'POST' }
    );
  }

  public async updateTriage(
    hospitalId: string,
    patientId: string,
    triageLevel: number
  ): Promise<{ patientId: string; hospitalId: string; triageLevel: number }> {
    return this.request<{ patientId: string; hospitalId: string; triageLevel: number }>(
      `/hospitals/${encodeURIComponent(hospitalId)}/patients/${encodeURIComponent(patientId)}/triage`,
      {
        method: 'PATCH',
        body: JSON.stringify({ triageLevel }),
      }
    );
  }

  // Visits
  public async getHospitalVisits(hospitalId: string): Promise<Visit[]> {
    return this.request<Visit[]>(`/hospitals/${encodeURIComponent(hospitalId)}/visits`);
  }

  public async getPatientVisits(hospitalId: string, patientId: string): Promise<Visit[]> {
    return this.request<Visit[]>(
      `/hospitals/${encodeURIComponent(hospitalId)}/patients/${encodeURIComponent(patientId)}/visits`
    );
  }

  public async createVisit(
    hospitalId: string,
    patientId: string,
    data: {
      triageLevel: number;
      chiefComplaint: string;
      admitToEmergencyQueue?: boolean;
      arrivalTime?: string;
    }
  ): Promise<Visit> {
    return this.request<Visit>(
      `/hospitals/${encodeURIComponent(hospitalId)}/patients/${encodeURIComponent(patientId)}/visits`,
      {
        method: 'POST',
        body: JSON.stringify(data),
      }
    );
  }

  public async updateVisitStatus(
    hospitalId: string,
    visitId: string,
    status: 'Waiting' | 'InTreatment' | 'Admitted' | 'Completed' | string
  ): Promise<{ hospitalId: string; visitId: string; status: number }> {
    return this.request<{ hospitalId: string; visitId: string; status: number }>(
      `/hospitals/${encodeURIComponent(hospitalId)}/visits/${encodeURIComponent(visitId)}/status`,
      {
        method: 'PATCH',
        body: JSON.stringify({ status }),
      }
    );
  }

  // Statistics
  public async getHospitalStats(hospitalId: string): Promise<HospitalStats> {
    return this.request<HospitalStats>(`/hospitals/${encodeURIComponent(hospitalId)}/statistics`);
  }

  public async getStatistics(hospitalId: string): Promise<HospitalStats> {
    return this.request<HospitalStats>(`/hospitals/${encodeURIComponent(hospitalId)}/statistics`);
  }
}

export const api = new ApiService();
