#pragma once

#include "smarter/Hospital.hpp"
#include "smarter/Patient.hpp"
#include "smarter/Visit.hpp"
#include "smarter/Triage.hpp"
#include "smarter/EmergencyQueue.h"
#include "smarter/PatientRegistry.h"
#include "smarter/PatientBST.h"
#include "smarter/VisitHistory.h"
#include "smarter/DatabaseManager.h"

#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <iostream>

namespace smarter {

/**
 * @brief Diagnostic snapshot containing hospital-level metrics.
 */
struct HospitalStats {
    std::string hospitalId;
    std::string hospitalName;
    std::string city;
    int totalEmergencyBeds = 0;
    size_t registeredPatientCount = 0;
    size_t activeQueueSize = 0;
    size_t totalVisitsRecorded = 0;
    size_t waitingVisits = 0;
    size_t inTreatmentVisits = 0;
    size_t completedVisits = 0;
};

/**
 * @brief Master System Coordinator integrating all 4 custom data structures:
 *  1. PatientRegistry (Custom Separate Chaining Hash Table) - Canonical Patient Store
 *  2. PatientBST (Custom Binary Search Tree) - Sorted Directory Index
 *  3. EmergencyQueue (Custom Min-Heap) - Clinical Triage Priority Dispatch
 *  4. VisitHistory (Custom Singly Linked List) - Chronological Audit Trail
 * 
 * Enforces:
 *  - Strict hospital isolation: Patient IDs are unique per hospital, but identical IDs
 *    can coexist across different hospitals.
 *  - Atomic synchronization: Registering a patient updates both Hash Table and BST;
 *    if one fails, an automatic rollback ensures consistent state.
 *  - State consistency: Serving emergency patients updates their active visit status
 *    to 'InTreatment' in the visit history.
 */
class SmartERSystem {
public:
    SmartERSystem();
    ~SmartERSystem() = default;

    // Move-only semantics to prevent accidental coordinator slicing/cloning
    SmartERSystem(const SmartERSystem&) = delete;
    SmartERSystem& operator=(const SmartERSystem&) = delete;
    SmartERSystem(SmartERSystem&&) noexcept = default;
    SmartERSystem& operator=(SmartERSystem&&) noexcept = default;

    // ==========================================
    // 0. Persistence Management (SQLite Stage 9)
    // ==========================================
    /**
     * @brief Connects to SQLite persistent storage, initializes schema, and hydrates in-memory DSA state.
     * @param dbPath Optional custom database file path. If empty, uses DatabaseManager::getDefaultDatabasePath().
     * @return true if database initialized and loaded successfully.
     */
    bool initializeDatabase(const std::string& dbPath = "");
    bool loadFromDatabase();
    bool isDatabaseEnabled() const;
    DatabaseManager* getDatabaseManager();
    const DatabaseManager* getDatabaseManager() const;

    // ==========================================
    // 1. Hospital Management
    // ==========================================
    bool registerHospital(const std::string& hospitalId,
                          const std::string& name,
                          const std::string& city,
                          int totalEmergencyBeds);
    bool registerHospital(const Hospital& hospital);
    bool hasHospital(const std::string& hospitalId) const;
    Hospital* getHospital(const std::string& hospitalId);
    const Hospital* getHospital(const std::string& hospitalId) const;
    size_t getHospitalCount() const;
    std::vector<Hospital> getAllHospitals() const;

    // ==========================================
    // 2. Patient Registration (Registry + BST)
    // ==========================================
    /**
     * @brief Registers a new patient across the system.
     * Enforces atomic consistency between the canonical Hash Table and BST.
     * If BST insertion fails, rolls back the Hash Table entry.
     */
    bool registerPatient(const Patient& patient);
    bool registerPatient(const std::string& hospitalId,
                         const std::string& patientId,
                         const std::string& name,
                         int age,
                         const std::string& gender,
                         const std::string& bloodGroup,
                         const std::string& contactPhone);

    Patient* findPatient(const std::string& hospitalId, const std::string& patientId);
    const Patient* findPatient(const std::string& hospitalId, const std::string& patientId) const;
    bool hasPatient(const std::string& hospitalId, const std::string& patientId) const;
    std::vector<Patient> getPatientsForHospital(const std::string& hospitalId) const;

    bool updatePatient(const std::string& hospitalId,
                       const std::string& patientId,
                       const std::string& newName,
                       int newAge,
                       const std::string& newGender,
                       const std::string& newBloodGroup,
                       const std::string& newPhone);

    // ==========================================
    // 3. Emergency Queue (Min-Heap)
    // ==========================================
    /**
     * @brief Admits a registered patient into the hospital's emergency room.
     * Creates a new 'Waiting' visit record and enqueues the patient into the Min-Heap.
     */
    bool admitPatientToEmergency(const std::string& hospitalId,
                                 const std::string& patientId,
                                 int triageLevel,
                                 const std::string& chiefComplaint,
                                 const std::string& arrivalTime = "",
                                 const std::string& customVisitId = "");

    /**
     * @brief Direct enqueueing into the hospital's emergency Min-Heap.
     */
    bool enqueuePatient(const std::string& hospitalId,
                        const std::string& patientId,
                        int triageLevel,
                        int arrivalSequence);

    std::optional<QueueEntry> peekNextEmergencyPatient(const std::string& hospitalId) const;

    /**
     * @brief Dispatches the highest-priority patient from the emergency queue.
     * Automatically transitions the patient's active 'Waiting' visit to 'InTreatment'.
     */
    std::optional<QueueEntry> serveNextEmergencyPatient(const std::string& hospitalId);

    /**
     * @brief Dynamically adjusts a waiting patient's triage severity and restores heap property.
     */
    bool updateEmergencyTriage(const std::string& hospitalId,
                               const std::string& patientId,
                               int newTriageLevel);

    void displayEmergencyQueue(const std::string& hospitalId, std::ostream& os = std::cout) const;
    size_t getEmergencyQueueSize(const std::string& hospitalId) const;
    std::vector<QueueEntry> getEmergencyQueueEntries(const std::string& hospitalId) const;

    // ==========================================
    // 4. Visit History (Singly Linked List)
    // ==========================================
    bool addVisit(const Visit& visit);

    Visit* findVisit(const std::string& hospitalId,
                     const std::string& patientId,
                     const std::string& visitId);

    const Visit* findVisit(const std::string& hospitalId,
                           const std::string& patientId,
                           const std::string& visitId) const;

    bool updateVisitStatus(const std::string& hospitalId,
                           const std::string& patientId,
                           const std::string& visitId,
                           Visit::Status newStatus);

    bool updateVisitStatus(const std::string& hospitalId,
                           const std::string& visitId,
                           Visit::Status newStatus);

    void displayPatientVisits(const std::string& hospitalId,
                              const std::string& patientId,
                              std::ostream& os = std::cout) const;

    std::vector<Visit> getPatientVisits(const std::string& hospitalId,
                                        const std::string& patientId) const;

    std::vector<Visit> getHospitalVisits(const std::string& hospitalId) const;

    // ==========================================
    // 5. Sorted Patient Directory (BST)
    // ==========================================
    void displaySortedPatients(std::ostream& os = std::cout) const;

    // ==========================================
    // 6. Analytics & Statistics
    // ==========================================
    HospitalStats getHospitalStatistics(const std::string& hospitalId) const;
    void displayHospitalStatistics(const std::string& hospitalId, std::ostream& os = std::cout) const;

    // Core Data Structure References
    PatientRegistry& getPatientRegistry();
    const PatientRegistry& getPatientRegistry() const;

    PatientBST& getPatientBST();
    const PatientBST& getPatientBST() const;

    VisitHistory& getVisitHistory();
    const VisitHistory& getVisitHistory() const;

private:
    bool registerHospitalMemory(const std::string& hospitalId,
                                const std::string& name,
                                const std::string& city,
                                int totalEmergencyBeds);

    std::vector<std::unique_ptr<Hospital>> hospitals_; ///< Owned hospitals (stable addresses)
    PatientRegistry patientRegistry_;                  ///< Canonical patient hash table
    PatientBST patientBST_;                            ///< Sorted patient BST index
    VisitHistory visitHistory_;                        ///< Central network visit ledger
    int arrivalCounter_;                               ///< Monotonically increasing arrival counter
    int visitCounter_;                                 ///< Monotonically increasing visit counter
    std::unique_ptr<DatabaseManager> dbManager_;       ///< SQLite persistence manager (optional)
};

} // namespace smarter

