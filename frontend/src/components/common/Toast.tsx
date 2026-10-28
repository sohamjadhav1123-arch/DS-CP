import React from 'react';
import { useApp } from '../../context/AppContext';
import { CheckCircle2, AlertCircle, Info, AlertTriangle, X } from 'lucide-react';

export const ToastContainer: React.FC = () => {
  const { toasts, removeToast } = useApp();

  if (toasts.length === 0) return null;

  return (
    <div className="toast-container">
      {toasts.map((toast) => {
        let icon = <Info size={18} color="#2563EB" />;
        if (toast.type === 'success') icon = <CheckCircle2 size={18} color="#10B981" />;
        if (toast.type === 'error') icon = <AlertCircle size={18} color="#EF4444" />;
        if (toast.type === 'warning') icon = <AlertTriangle size={18} color="#F59E0B" />;

        return (
          <div key={toast.id} className={`toast-card ${toast.type}`}>
            <div style={{ flexShrink: 0, marginTop: '2px' }}>{icon}</div>
            <div style={{ flex: 1, minWidth: 0 }}>
              <div style={{ fontWeight: 600, fontSize: '0.875rem', color: '#0F172A' }}>
                {toast.title}
              </div>
              <div style={{ fontSize: '0.8125rem', color: '#64748B', marginTop: '2px' }}>
                {toast.message}
              </div>
            </div>
            <button
              onClick={() => removeToast(toast.id)}
              className="action-icon-btn"
              style={{ padding: '2px', alignSelf: 'flex-start' }}
            >
              <X size={14} />
            </button>
          </div>
        );
      })}
    </div>
  );
};

