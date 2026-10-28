import React from 'react';

interface TriageBadgeProps {
  level: number;
  showSeverity?: boolean;
  size?: 'sm' | 'md' | 'lg';
}

const severityLabels: Record<number, string> = {
  1: 'Critical',
  2: 'Emergency',
  3: 'Urgent',
  4: 'Less Urgent',
  5: 'Non-Urgent',
};

export const TriageBadge: React.FC<TriageBadgeProps> = ({ level, showSeverity = false, size = 'md' }) => {
  const label = severityLabels[level] || `Level ${level}`;

  return (
    <div style={{ display: 'inline-flex', alignItems: 'center', gap: '6px' }}>
      <span className={`triage-badge triage-${level} ${size === 'sm' ? 'text-xs' : ''}`}>
        {level}
      </span>
      {showSeverity && (
        <span style={{ fontSize: '0.8125rem', fontWeight: 500, color: '#475569' }}>
          {label}
        </span>
      )}
    </div>
  );
};

