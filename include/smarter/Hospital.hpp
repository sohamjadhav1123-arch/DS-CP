#pragma once

#include "smarter/Patient.hpp"
#include "smarter/EmergencyQueue.h"
#include <string>
#include <vector>
#include <iostream>

namespace smarter {

/**
 * @brief Represents an individual Hospital entity within the SmartER network.
 * 
 * Multi-Hospital Architecture:
 * - SmartER supports multiple hospital branches (e.g. Metro Central, City South).
 * - Each Hospital operates an independent emergency department with its own:
 *     1. Patient Registry: Scoped to this hospital (Patient IDs unique per hospital).
 *     2. Emergency Triage Queue: Prioritized emergency triage.
 * 
 * Data Structure Roadmap for Hospital:
 * - Stage 1 (Current): Model definition, architectural scaffolding, vector-based registry.
 * - Stage 2: Custom Hash Table replacing linear search for O(1) patient lookup by ID.
 * - Stage 3: Custom Min-Heap replacing queue for O(log N) triage-based emergency dispatch.
 * - Stage 4: Custom Binary Search Tree (BST) for fast patient sorting/range querying.
 * - Stage 5: Custom Singly/Doubly Linked List for chronological visit audit trails.
 * 
 * Note: Per project requirements, standard library associative containers
 * (std::map, std::unordered_map, std::priority_queue, std::list) are avoided.
 */
class Hospital {
public:
    Hospital();

    Hospital(std::string hospitalId,
             std::string name,
             std::string city,
             int totalEmergencyBeds);

    // Getters
    const std::string& getHospitalId() const;
    const std::string& getName() const;
    const std::string& getCity() const;
    int getTotalEmergencyBeds() const;
    size_t getPatientCount() const;

    // Setters
    void setName(const std::string& name);
    void setCity(const std::string& city);
    void setTotalEmergencyBeds(int beds);

    // Initial Patient Record Management (Scattered across per-hospital scope)
    bool registerPatient(const Patient& patient);
    const Patient* findPatient(const std::string& patientId) const;
    Patient* findPatient(const std::string& patientId);
    bool hasPatient(const std::string& patientId) const;

    // Emergency Queue Management
    EmergencyQueue& getEmergencyQueue();
    const EmergencyQueue& getEmergencyQueue() const;

    // Canonical Registry Binding (Stage 6 Integration)
    void bindRegistry(class PatientRegistry* registry);
    class PatientRegistry* getRegistry();
    const class PatientRegistry* getRegistry() const;

    // Display helpers
    void printDetails(std::ostream& os = std::cout) const;
    void printAllPatients(std::ostream& os = std::cout) const;

private:
    std::string hospitalId_;
    std::string name_;
    std::string city_;
    int totalEmergencyBeds_;

    // Scoped Emergency Queue (Min-Heap based triage queue)
    EmergencyQueue emergencyQueue_;

    // Non-owning pointer to canonical system PatientRegistry
    class PatientRegistry* registry_ = nullptr;

    // Fallback storage for registered patients when used standalone without coordinator
    std::vector<Patient> patients_;
};

} // namespace smarter

