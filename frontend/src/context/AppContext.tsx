import React, { createContext, useContext, useState, useEffect, useCallback, ReactNode } from 'react';
import { Hospital, Patient, QueueEntry, Visit, HospitalStats, HealthStatus } from '../types/api';
import { api } from '../services/api';

export type NavigationTab = 'dashboard' | 'queue' | 'patients' | 'visits' | 'hospitals' | 'reports' | 'settings';

export interface ToastMessage {
  id: string;
  type: 'success' | 'error' | 'info' | 'warning';
  title: string;
  message: string;
}

interface AppContextType {
  // Navigation
  activeTab: NavigationTab;
  setActiveTab: (tab: NavigationTab) => void;

  // Active hospital
  hospitals: Hospital[];
  selectedHospitalId: string;
  selectedHospital: Hospital | null;
  setSelectedHospitalId: (id: string) => void;

  // Live state
  queue: QueueEntry[];
  patients: Patient[];
  visits: Visit[];
  stats: HospitalStats | null;
  health: HealthStatus | null;
  isConnected: boolean;
  isInitialLoading: boolean;
  isRefreshing: boolean;
  lastErrorMessage: string | null;

  // Actions
  refreshAll: () => Promise<void>;
  serveNext: () => Promise<QueueEntry | null>;
  updateTriage: (patientId: string, level: number) => Promise<boolean>;
  updateVisitStatus: (patientId: string, visitId: string, status: string, diagnosis?: string) => Promise<boolean>;
  addPatient: (data: { patientId: string; name: string; age: number; gender: string; bloodGroup: string; contactPhone: string }) => Promise<Patient | null>;
  addVisit: (data: { patientId: string; triageLevel: number; chiefComplaint: string; admitToEmergencyQueue?: boolean; arrivalTime?: string }) => Promise<Visit | null>;
  addHospital: (data: { hospitalId: string; name: string; city: string; totalEmergencyBeds: number }) => Promise<Hospital | null>;

  // Notifications / Toasts
  toasts: ToastMessage[];
  addToast: (type: 'success' | 'error' | 'info' | 'warning', title: string, message: string) => void;
  removeToast: (id: string) => void;
  unreadAlertCount: number;
}

const AppContext = createContext<AppContextType | undefined>(undefined);

export const AppProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [activeTab, setActiveTab] = useState<NavigationTab>('dashboard');
  const [hospitals, setHospitals] = useState<Hospital[]>([]);
  const [selectedHospitalId, setSelectedHospitalId] = useState<string>('');
  const [queue, setQueue] = useState<QueueEntry[]>([]);
  const [patients, setPatients] = useState<Patient[]>([]);
  const [visits, setVisits] = useState<Visit[]>([]);
  const [stats, setStats] = useState<HospitalStats | null>(null);
  const [health, setHealth] = useState<HealthStatus | null>(null);
  const [isConnected, setIsConnected] = useState<boolean>(false);
  const [isInitialLoading, setIsInitialLoading] = useState<boolean>(true);
  const [isRefreshing, setIsRefreshing] = useState<boolean>(false);
  const [lastErrorMessage, setLastErrorMessage] = useState<string | null>(null);
  const [toasts, setToasts] = useState<ToastMessage[]>([]);

  const addToast = useCallback((type: 'success' | 'error' | 'info' | 'warning', title: string, message: string) => {
    const id = Date.now().toString() + Math.random().toString(36).substring(2, 6);
    setToasts((prev) => [...prev, { id, type, title, message }]);
    setTimeout(() => {
      setToasts((prev) => prev.filter((t) => t.id !== id));
    }, 4500);
  }, []);

  const removeToast = useCallback((id: string) => {
    setToasts((prev) => prev.filter((t) => t.id !== id));
  }, []);

  // Poll backend health & fetch hospitals
  const checkHealthAndLoadHospitals = useCallback(async () => {
    try {
      const h = await api.getHealth();
      setHealth(h);
      setIsConnected(true);
      setLastErrorMessage(null);

      const list = await api.getHospitals();
      if (Array.isArray(list)) {
        setHospitals(list);
        if (list.length > 0) {
          setSelectedHospitalId((curr) => {
            if (!curr || !list.some((item) => item.hospitalId === curr)) {
              return list[0].hospitalId;
            }
            return curr;
          });
        }
      }
    } catch (err: any) {
      console.warn('[SmartER Frontend] Health check failed:', err.message);
      setIsConnected(false);
      setLastErrorMessage(err.message || 'Cannot connect to C++ backend at http://127.0.0.1:8080/api/v1');
    }
  }, []);

  // Refresh current hospital's data (Queue, Patients, Visits, Stats)
  const refreshHospitalData = useCallback(async (hId: string) => {
    if (!hId) return;
    try {
      const [q, pts, vsts, st] = await Promise.all([
        api.getQueue(hId).catch((e) => {
          console.warn('[SmartER Frontend] Failed to load queue:', e.message);
          return [];
        }),
        api.getPatients(hId).catch((e) => {
          console.warn('[SmartER Frontend] Failed to load patients:', e.message);
          return [];
        }),
        api.getHospitalVisits(hId).catch((e) => {
          console.warn('[SmartER Frontend] Failed to load visits:', e.message);
          return [];
        }),
        api.getHospitalStats(hId).catch((e) => {
          console.warn('[SmartER Frontend] Failed to load stats:', e.message);
          return null;
        }),
      ]);

      if (Array.isArray(q)) setQueue(q);
      if (Array.isArray(pts)) setPatients(pts);
      if (Array.isArray(vsts)) setVisits(vsts);
      if (st) setStats(st);
    } catch (err: any) {
      console.warn('[SmartER Frontend] Error refreshing hospital data:', err);
    }
  }, []);

  const refreshAll = useCallback(async () => {
    setIsRefreshing(true);
    try {
      await checkHealthAndLoadHospitals();
      if (selectedHospitalId) {
        await refreshHospitalData(selectedHospitalId);
      }
    } finally {
      setIsRefreshing(false);
    }
  }, [checkHealthAndLoadHospitals, refreshHospitalData, selectedHospitalId]);

  // Initial mount
  useEffect(() => {
    let mounted = true;
    (async () => {
      try {
        await checkHealthAndLoadHospitals();
      } finally {
        if (mounted) setIsInitialLoading(false);
      }
    })();
    return () => {
      mounted = false;
    };
  }, [checkHealthAndLoadHospitals]);

  // When selectedHospitalId changes, refresh hospital-specific data
  useEffect(() => {
    if (selectedHospitalId) {
      refreshHospitalData(selectedHospitalId);
    }
  }, [selectedHospitalId, refreshHospitalData]);

  // Periodic poll every 5 seconds for live queue updates
  useEffect(() => {
    const interval = setInterval(() => {
      if (selectedHospitalId && isConnected) {
        refreshHospitalData(selectedHospitalId);
      } else {
        checkHealthAndLoadHospitals();
      }
    }, 5000);
    return () => clearInterval(interval);
  }, [selectedHospitalId, isConnected, refreshHospitalData, checkHealthAndLoadHospitals]);

  // Actions
  const serveNext = async (): Promise<QueueEntry | null> => {
    if (!selectedHospitalId) return null;
    try {
      const res = await api.serveNextPatient(selectedHospitalId);
      addToast('success', 'Patient Served', `${res.patientName || res.patientId} has been called for treatment.`);
      await refreshHospitalData(selectedHospitalId);
      return res;
    } catch (err: any) {
      addToast('error', 'Cannot Serve Patient', err.message);
      return null;
    }
  };

  const updateTriage = async (patientId: string, level: number): Promise<boolean> => {
    if (!selectedHospitalId) return false;
    try {
      await api.updateTriage(selectedHospitalId, patientId, level);
      addToast('success', 'Triage Updated', `Patient ${patientId} triage adjusted to level ${level}.`);
      await refreshHospitalData(selectedHospitalId);
      return true;
    } catch (err: any) {
      addToast('error', 'Triage Update Failed', err.message);
      return false;
    }
  };

  const updateVisitStatus = async (
    _patientId: string,
    visitId: string,
    status: string,
    _diagnosis?: string
  ): Promise<boolean> => {
    if (!selectedHospitalId) return false;
    try {
      await api.updateVisitStatus(selectedHospitalId, visitId, status);
      addToast('success', 'Visit Status Updated', `Status changed to ${status}.`);
      await refreshHospitalData(selectedHospitalId);
      return true;
    } catch (err: any) {
      addToast('error', 'Update Failed', err.message);
      return false;
    }
  };

  const addPatient = async (data: {
    patientId: string;
    name: string;
    age: number;
    gender: string;
    bloodGroup: string;
    contactPhone: string;
  }): Promise<Patient | null> => {
    if (!selectedHospitalId) return null;
    try {
      const p = await api.createPatient(selectedHospitalId, data);
      addToast('success', 'Patient Registered', `${p.name} (${p.patientId}) successfully registered.`);
      await refreshHospitalData(selectedHospitalId);
      return p;
    } catch (err: any) {
      addToast('error', 'Registration Failed', err.message);
      return null;
    }
  };

  const addVisit = async (data: {
    patientId: string;
    triageLevel: number;
    chiefComplaint: string;
    admitToEmergencyQueue?: boolean;
    arrivalTime?: string;
  }): Promise<Visit | null> => {
    if (!selectedHospitalId) return null;
    try {
      const v = await api.createVisit(selectedHospitalId, data.patientId, data);
      addToast('success', 'Visit Logged', `Visit ${v.visitId} recorded for patient ${data.patientId}.`);
      await refreshHospitalData(selectedHospitalId);
      return v;
    } catch (err: any) {
      addToast('error', 'Failed to Log Visit', err.message);
      return null;
    }
  };

  const addHospital = async (data: {
    hospitalId: string;
    name: string;
    city: string;
    totalEmergencyBeds: number;
  }): Promise<Hospital | null> => {
    try {
      const h = await api.createHospital(data);
      addToast('success', 'Hospital Registered', `${h.name} (${h.hospitalId}) added.`);
      await checkHealthAndLoadHospitals();
      setSelectedHospitalId(h.hospitalId);
      return h;
    } catch (err: any) {
      addToast('error', 'Hospital Creation Failed', err.message);
      return null;
    }
  };

  const selectedHospital = hospitals.find((h) => h.hospitalId === selectedHospitalId) || null;
  const unreadAlertCount = queue.filter((q) => q.triageLevel <= 2).length;

  return (
    <AppContext.Provider
      value={{
        activeTab,
        setActiveTab,
        hospitals,
        selectedHospitalId,
        selectedHospital,
        setSelectedHospitalId,
        queue,
        patients,
        visits,
        stats,
        health,
        isConnected,
        isInitialLoading,
        isRefreshing,
        lastErrorMessage,
        refreshAll,
        serveNext,
        updateTriage,
        updateVisitStatus,
        addPatient,
        addVisit,
        addHospital,
        toasts,
        addToast,
        removeToast,
        unreadAlertCount,
      }}
    >
      {children}
    </AppContext.Provider>
  );
};

export const useApp = () => {
  const context = useContext(AppContext);
  if (!context) {
    throw new Error('useApp must be used within an AppProvider');
  }
  return context;
};
