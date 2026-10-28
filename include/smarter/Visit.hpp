#pragma once

#include "smarter/Triage.hpp"
#include <string>
#include <iostream>

namespace smarter {

/**
 * @brief Represents a single emergency room or clinical visit for a patient.
 * 
 * Each visit captures the admission timestamp, clinical complaint,
 * triage severity, current status, and physician's diagnosis.
 * 
 * Visits will later be indexed in:
 *  1. Min-Heap (Emergency Queue, sorted by TriageLevel and arrival order).
 *  2. Patient Visit History (Custom Linked List or Binary Search Tree).
 */
class Visit {
public:
    enum class Status {
        Waiting,
        InTreatment,
        Admitted,
        Discharged,
        Completed = Discharged
    };

    Visit();

    Visit(std::string visitId,
          std::string patientId,
          std::string hospitalId,
          std::string arrivalTime,
          TriageLevel triageLevel,
          std::string chiefComplaint,
          std::string diagnosis = "Pending evaluation",
          Status status = Status::Waiting,
          std::string notes = "",
          int arrivalSequence = 0,
          bool inEmergencyQueue = false);

    // Getters
    const std::string& getVisitId() const;
    const std::string& getPatientId() const;
    const std::string& getHospitalId() const;
    const std::string& getArrivalTime() const;
    TriageLevel getTriageLevel() const;
    const std::string& getChiefComplaint() const;
    const std::string& getDiagnosis() const;
    Status getStatus() const;
    const std::string& getNotes() const;
    int getArrivalSequence() const;
    bool isInEmergencyQueue() const;

    // Setters
    void setDiagnosis(const std::string& diagnosis);
    void setStatus(Status status);
    void setNotes(const std::string& notes);
    void setTriageLevel(TriageLevel level);
    void setArrivalSequence(int seq);
    void setInEmergencyQueue(bool inQueue);

    // Helpers
    std::string getStatusString() const;
    void printSummary(std::ostream& os = std::cout) const;

private:
    std::string visitId_;
    std::string patientId_;
    std::string hospitalId_;
    std::string arrivalTime_;
    TriageLevel triageLevel_;
    std::string chiefComplaint_;
    std::string diagnosis_;
    Status status_;
    std::string notes_;
    int arrivalSequence_ = 0;
    bool inEmergencyQueue_ = false;
};

} // namespace smarter

