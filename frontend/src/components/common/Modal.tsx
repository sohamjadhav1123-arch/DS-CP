import React, { useEffect, ReactNode } from 'react';
import { X } from 'lucide-react';

interface ModalProps {
  isOpen: boolean;
  onClose: () => void;
  title: string;
  subtitle?: string;
  children: ReactNode;
  maxWidth?: string;
}

export const Modal: React.FC<ModalProps> = ({
  isOpen,
  onClose,
  title,
  subtitle,
  children,
  maxWidth = '540px',
}) => {
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.key === 'Escape' && isOpen) {
        onClose();
      }
    };
    window.addEventListener('keydown', handleKeyDown);
    return () => window.removeEventListener('keydown', handleKeyDown);
  }, [isOpen, onClose]);

  if (!isOpen) return null;

  return (
    <div className="modal-backdrop" onClick={onClose}>
      <div
        className="modal-content"
        style={{ maxWidth }}
        onClick={(e) => e.stopPropagation()}
      >
        <div className="modal-header">
          <div>
            <h3 style={{ fontSize: '1.15rem', fontWeight: 700, color: '#0F172A', margin: 0 }}>
              {title}
            </h3>
            {subtitle && (
              <p style={{ fontSize: '0.8125rem', color: '#64748B', marginTop: '3px', margin: 0 }}>
                {subtitle}
              </p>
            )}
          </div>
          <button
            onClick={onClose}
            className="action-icon-btn"
            style={{ borderRadius: '50%', padding: '6px' }}
            aria-label="Close modal"
          >
            <X size={18} />
          </button>
        </div>
        <div style={{ marginTop: '16px' }}>{children}</div>
      </div>
    </div>
  );
};

