#pragma once

#include <string>

namespace smarter {

/**
 * @brief Emergency Severity Index (ESI) triage categories.
 * 
 * In a medical emergency room, triage is used to prioritize patient care
 * based on clinical acuity rather than first-come-first-served.
 * 
 * Lower numeric value = higher clinical priority (ideal for Min-Heap implementation):
 *  - Level 1 (Immediate / Resuscitation): Imminent life threat (cardiac arrest, severe trauma).
 *  - Level 2 (Emergent / Very Urgent): High risk, chest pain, stroke symptoms.
 *  - Level 3 (Urgent): Stable vital signs but requiring multiple resources (moderate asthma, fractures).
 *  - Level 4 (Standard / Less Urgent): Low acuity, single resource needed (sprains, simple stitches).
 *  - Level 5 (Non-Urgent): Minor complaints (prescription refills, rash).
 */
enum class TriageLevel {
    Immediate = 1,
    Emergent = 2,
    Urgent = 3,
    LessUrgent = 4,
    NonUrgent = 5
};

inline std::string triageLevelToString(TriageLevel level) {
    switch (level) {
        case TriageLevel::Immediate:  return "LEVEL 1 - IMMEDIATE (Resuscitation)";
        case TriageLevel::Emergent:   return "LEVEL 2 - EMERGENT (High Risk)";
        case TriageLevel::Urgent:     return "LEVEL 3 - URGENT";
        case TriageLevel::LessUrgent: return "LEVEL 4 - LESS URGENT";
        case TriageLevel::NonUrgent:  return "LEVEL 5 - NON-URGENT";
        default:                      return "UNKNOWN";
    }
}

inline int triageLevelToInt(TriageLevel level) {
    return static_cast<int>(level);
}

} // namespace smarter

