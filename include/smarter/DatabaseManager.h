#pragma once

#include "smarter/Hospital.hpp"
#include "smarter/Patient.hpp"
#include "smarter/Visit.hpp"
#include "smarter/Triage.hpp"

#include <sqlite3.h>
#include <string>
#include <vector>
#include <memory>
#include <optional>
#include <iostream>

namespace smarter {

/**
 * @brief RAII Prepared Statement wrapper for SQLite.
 * Manages sqlite3_stmt lifecycle and handles typed binding and extraction safely.
 */
class Statement {
public:
    Statement(sqlite3* db, const std::string& sql);
    ~Statement();

    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;
    Statement(Statement&& other) noexcept;
    Statement& operator=(Statement&& other) noexcept;

    bool isValid() const;
    sqlite3_stmt* get() const;

    bool bindText(int colIndex, const std::string& val);
    bool bindInt(int colIndex, int val);
    bool bindNull(int colIndex);

    int step();
    bool execute();
    void reset();

    std::string getText(int colIndex) const;
    int getInt(int colIndex) const;

private:
    sqlite3_stmt* stmt_ = nullptr;
};

/**
 * @brief Scoped RAII transaction helper for SQLite.
 * Rolls back automatically if not explicitly committed.
 */
class ScopedTransaction {
public:
    explicit ScopedTransaction(class DatabaseManager& db);
    ~ScopedTransaction();

    ScopedTransaction(const ScopedTransaction&) = delete;
    ScopedTransaction& operator=(const ScopedTransaction&) = delete;

    bool commit();
    bool isActive() const;

private:
    class DatabaseManager& db_;
    bool active_ = false;
};

/**
 * @brief Persistence Manager for SmartER using the SQLite C API.
 * 
 * Persists Hospital branches, registered Patients, clinical Visits, and
 * Emergency Queue arrival sequences to disk.
 * 
 * Invariants:
 *  - Enforces foreign key constraints (PRAGMA foreign_keys = ON).
 *  - Composite primary key on (hospital_id, patient_id) enables cross-facility ID reuse.
 *  - All queries use parameterized prepared statements to prevent injection.
 *  - Atomic operations utilize ACID transactions.
 *  - Predictable database path resolution prevents accidental database duplication.
 */
class DatabaseManager {
public:
    explicit DatabaseManager(std::string dbPath);
    ~DatabaseManager();

    // Move-only semantics
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;
    DatabaseManager(DatabaseManager&& other) noexcept;
    DatabaseManager& operator=(DatabaseManager&& other) noexcept;

    // Database Lifecycle
    bool open();
    void close();
    bool isOpen() const;
    const std::string& getDbPath() const;
    static std::string getDefaultDatabasePath();

    // Schema Management
    bool initializeSchema();
    int getSchemaVersion();
    bool setSchemaVersion(int version);

    // Transaction Management
    bool beginTransaction();
    bool commitTransaction();
    bool rollbackTransaction();

    // 1. Hospital Operations
    bool saveHospital(const Hospital& hospital);
    bool saveHospital(const std::string& hospitalId,
                      const std::string& name,
                      const std::string& city,
                      int totalEmergencyBeds);
    std::vector<Hospital> loadHospitals();
    bool hasHospital(const std::string& hospitalId);
    bool deleteHospital(const std::string& hospitalId);

    // 2. Patient Operations
    bool savePatient(const Patient& patient);
    std::vector<Patient> loadPatients();
    bool updatePatient(const std::string& hospitalId,
                       const std::string& patientId,
                       const std::string& newName,
                       int newAge,
                       const std::string& newGender,
                       const std::string& newBloodGroup,
                       const std::string& newPhone);
    bool hasPatient(const std::string& hospitalId, const std::string& patientId);
    bool deletePatient(const std::string& hospitalId, const std::string& patientId);

    // 3. Visit Operations
    bool saveVisit(const Visit& visit);
    std::vector<Visit> loadVisits();
    bool updateVisitStatus(const std::string& hospitalId,
                           const std::string& patientId,
                           const std::string& visitId,
                           Visit::Status newStatus,
                           bool inEmergencyQueue);
    bool updateEmergencyTriage(const std::string& hospitalId,
                               const std::string& patientId,
                               int newTriageLevel);
    bool hasVisit(const std::string& hospitalId,
                  const std::string& patientId,
                  const std::string& visitId);

    // 4. System Metadata & Counters
    bool setMetadata(const std::string& key, const std::string& value);
    std::string getMetadata(const std::string& key, const std::string& defaultValue = "");
    int getMetadataInt(const std::string& key, int defaultValue = 0);
    bool setMetadataInt(const std::string& key, int value);

    int getMaxArrivalSequence();
    int getMaxVisitCounter();

    // Diagnostics
    std::string getLastError() const;
    sqlite3* getRawHandle() { return db_; }

private:
    std::string dbPath_;
    sqlite3* db_ = nullptr;
    mutable std::string lastError_;

    void setLastError(const std::string& err) const;
    void logSqliteError(const std::string& context) const;
};

} // namespace smarter

