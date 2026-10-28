import React from 'react';

interface StatusBadgeProps {
  status: string;
}

export const StatusBadge: React.FC<StatusBadgeProps> = ({ status }) => {
  const norm = (status || '').toLowerCase();

  let className = 'status-badge status-waiting';
  let label = status || 'Waiting';

  if (norm.includes('treatment')) {
    className = 'status-badge status-treatment';
    label = 'In Treatment';
  } else if (norm.includes('discharge') || norm.includes('complete')) {
    className = 'status-badge status-discharged';
    label = norm.includes('complete') ? 'Completed' : 'Discharged';
  } else if (norm.includes('admit')) {
    className = 'status-badge status-admitted';
    label = 'Admitted';
  }

  return <span className={className}>{label}</span>;
};

