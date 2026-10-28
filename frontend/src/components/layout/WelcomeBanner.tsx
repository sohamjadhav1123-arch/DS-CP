import React from 'react';
import { useApp } from '../../context/AppContext';

export const WelcomeBanner: React.FC = () => {
  const { selectedHospital } = useApp();

  const hospitalName = selectedHospital?.name || 'CityCare Hospital';

  return (
    <div className="welcome-banner">
      {/* Decorative architectural background svg overlay */}
      <div className="banner-bg-graphic" />

      {/* Left Text */}
      <div className="welcome-banner-content">
        <h1 className="welcome-title">Welcome back, Soham!</h1>
        <p className="welcome-sub">
          Here&apos;s what&apos;s happening at {hospitalName} today.
        </p>
      </div>

      {/* Right Cursive Calligraphy Motto */}
      <div className="motto-container">
        <div className="motto-text">Every second matters</div>
        <div className="motto-pulse">
          <svg width="64" height="20" viewBox="0 0 64 20" fill="none">
            <path
              d="M0 10 H22 L26 2 L31 18 L36 6 L40 14 L44 10 H64"
              stroke="#93C5FD"
              strokeWidth="2"
              strokeLinecap="round"
              strokeLinejoin="round"
            />
          </svg>
        </div>
      </div>
    </div>
  );
};

