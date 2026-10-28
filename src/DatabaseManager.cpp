#include "smarter/DatabaseManager.h"
#include <filesystem>
#include <cstdlib>
#include <sstream>

namespace smarter {

// ==========================================
// Statement RAII Implementation
// ==========================================

Statement::Statement(sqlite3* db, const std::string& sql) {
    if (db) {
        int rc = sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt_, nullptr);
        if (rc != SQLITE_OK) {
            stmt_ = nullptr;
        }
    }
}

Statement::~Statement() {
    if (stmt_) {
        sqlite3_finalize(stmt_);
        stmt_ = nullptr;
    }
}

Statement::Statement(Statement&& other) noexcept : stmt_(other.stmt_) {
    other.stmt_ = nullptr;
}

Statement& Statement::operator=(Statement&& other) noexcept {
    if (this != &other) {
        if (stmt_) {
            sqlite3_finalize(stmt_);
        }
        stmt_ = other.stmt_;
        other.stmt_ = nullptr;
    }
    return *this;
}

bool Statement::isValid() const {
    return stmt_ != nullptr;
}

sqlite3_stmt* Statement::get() const {
    return stmt_;
}

bool Statement::bindText(int colIndex, const std::string& val) {
    if (!stmt_) return false;
    return sqlite3_bind_text(stmt_, colIndex, val.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK;
}

bool Statement::bindInt(int colIndex, int val) {
    if (!stmt_) return false;
    return sqlite3_bind_int(stmt_, colIndex, val) == SQLITE_OK;
}

bool Statement::bindNull(int colIndex) {
    if (!stmt_) return false;
    return sqlite3_bind_null(stmt_, colIndex) == SQLITE_OK;
}

int Statement::step() {
    if (!stmt_) return SQLITE_ERROR;
    return sqlite3_step(stmt_);
}

bool Statement::execute() {
    int rc = step();
    return rc == SQLITE_DONE || rc == SQLITE_ROW;
}

void Statement::reset() {
    if (stmt_) {
        sqlite3_reset(stmt_);
        sqlite3_clear_bindings(stmt_);
    }
}

std::string Statement::getText(int colIndex) const {
    if (!stmt_) return "";
    const unsigned char* text = sqlite3_column_text(stmt_, colIndex);
    return text ? reinterpret_cast<const char*>(text) : "";
}

int Statement::getInt(int colIndex) const {
    if (!stmt_) return 0;
    return sqlite3_column_int(stmt_, colIndex);
}

// ==========================================
// ScopedTransaction Implementation
// ==========================================

ScopedTransaction::ScopedTransaction(DatabaseManager& db)
    : db_(db), active_(db_.beginTransaction()) {}

ScopedTransaction::~ScopedTransaction() {
    if (active_) {
        db_.rollbackTransaction();
    }
}

bool ScopedTransaction::commit() {
    if (!active_) return false;
    bool ok = db_.commitTransaction();
    if (ok) {
        active_ = false;
    }
    return ok;
}

bool ScopedTransaction::isActive() const {
    return active_;
}

// ==========================================
// DatabaseManager Implementation
// ==========================================

DatabaseManager::DatabaseManager(std::string dbPath)
    : dbPath_(std::move(dbPath)), db_(nullptr) {}

DatabaseManager::~DatabaseManager() {
    close();
}

DatabaseManager::DatabaseManager(DatabaseManager&& other) noexcept
    : dbPath_(std::move(other.dbPath_)),
      db_(other.db_),
      lastError_(std::move(other.lastError_)) {
    other.db_ = nullptr;
}

DatabaseManager& DatabaseManager::operator=(DatabaseManager&& other) noexcept {
    if (this != &other) {
        close();
        dbPath_ = std::move(other.dbPath_);
        db_ = other.db_;
        lastError_ = std::move(other.lastError_);
        other.db_ = nullptr;
    }
    return *this;
}

bool DatabaseManager::open() {
    if (db_) return true;

    // Ensure parent directory exists
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path p(dbPath_);
    if (p.has_parent_path()) {
        fs::create_directories(p.parent_path(), ec);
    }

    int flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;
    int rc = sqlite3_open_v2(dbPath_.c_str(), &db_, flags, nullptr);
    if (rc != SQLITE_OK) {
        logSqliteError("sqlite3_open_v2 failed for " + dbPath_);
        close();
        return false;
    }

    // Always enforce foreign key constraints
    char* errmsg = nullptr;
    rc = sqlite3_exec(db_, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) {
            setLastError(errmsg);
            sqlite3_free(errmsg);
        }
        close();
        return false;
    }

    return true;
}

void DatabaseManager::close() {
    if (db_) {
        sqlite3_close_v2(db_);
        db_ = nullptr;
    }
}

bool DatabaseManager::isOpen() const {
    return db_ != nullptr;
}

const std::string& DatabaseManager::getDbPath() const {
    return dbPath_;
}

std::string DatabaseManager::getDefaultDatabasePath() {
    // 1. Environment variable override
    const char* envPath = std::getenv("SMARTER_DB_PATH");
    if (envPath && envPath[0] != '\0') {
        return envPath;
    }

    // 2. Discover workspace project root by traversing upward for CMakeLists.txt
    namespace fs = std::filesystem;
    std::error_code ec;
    fs::path current = fs::current_path(ec);
    if (!ec) {
        fs::path probe = current;
        for (int i = 0; i < 5; ++i) {
            if (fs::exists(probe / "CMakeLists.txt", ec) &&
                fs::exists(probe / "src" / "SmartERSystem.cpp", ec)) {
                fs::path dataDir = probe / "data";
                fs::create_directories(dataDir, ec);
                return (dataDir / "smarter.db").string();
            }
            if (!probe.has_parent_path() || probe == probe.parent_path()) {
                break;
            }
            probe = probe.parent_path();
        }
    }

    // 3. Fallback: local data/smarter.db
    fs::path fallback = "data";
    fs::create_directories(fallback, ec);
    return "data/smarter.db";
}

bool DatabaseManager::initializeSchema() {
    if (!isOpen() && !open()) return false;

    const char* schemaSql = R"(
        CREATE TABLE IF NOT EXISTS hospitals (
            hospital_id TEXT PRIMARY KEY,
            hospital_name TEXT NOT NULL,
            city TEXT NOT NULL,
            bed_capacity INTEGER NOT NULL
        );

        CREATE TABLE IF NOT EXISTS patients (
            hospital_id TEXT NOT NULL,
            patient_id TEXT NOT NULL,
            name TEXT NOT NULL,
            age INTEGER NOT NULL,
            gender TEXT NOT NULL,
            blood_group TEXT NOT NULL,
            contact_phone TEXT NOT NULL,
            PRIMARY KEY (hospital_id, patient_id),
            FOREIGN KEY (hospital_id) REFERENCES hospitals(hospital_id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS visits (
            visit_id TEXT PRIMARY KEY,
            hospital_id TEXT NOT NULL,
            patient_id TEXT NOT NULL,
            timestamp TEXT NOT NULL,
            triage_level INTEGER NOT NULL,
            chief_complaint TEXT NOT NULL,
            diagnosis TEXT NOT NULL,
            status INTEGER NOT NULL,
            notes TEXT NOT NULL DEFAULT '',
            arrival_sequence INTEGER NOT NULL DEFAULT 0,
            in_emergency_queue INTEGER NOT NULL DEFAULT 0,
            FOREIGN KEY (hospital_id, patient_id) REFERENCES patients(hospital_id, patient_id) ON DELETE CASCADE
        );

        CREATE TABLE IF NOT EXISTS system_metadata (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        );
    )";

    char* errmsg = nullptr;
    int rc = sqlite3_exec(db_, schemaSql, nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) {
            setLastError(errmsg);
            sqlite3_free(errmsg);
        }
        return false;
    }

    // Set schema version if unset
    if (getSchemaVersion() == 0) {
        setSchemaVersion(1);
    }

    return true;
}

int DatabaseManager::getSchemaVersion() {
    return getMetadataInt("schema_version", 0);
}

bool DatabaseManager::setSchemaVersion(int version) {
    return setMetadataInt("schema_version", version);
}

bool DatabaseManager::beginTransaction() {
    if (!isOpen()) return false;
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) {
            setLastError(errmsg);
            sqlite3_free(errmsg);
        }
        return false;
    }
    return true;
}

bool DatabaseManager::commitTransaction() {
    if (!isOpen()) return false;
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) {
            setLastError(errmsg);
            sqlite3_free(errmsg);
        }
        return false;
    }
    return true;
}

bool DatabaseManager::rollbackTransaction() {
    if (!isOpen()) return false;
    char* errmsg = nullptr;
    int rc = sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, &errmsg);
    if (rc != SQLITE_OK) {
        if (errmsg) {
            setLastError(errmsg);
            sqlite3_free(errmsg);
        }
        return false;
    }
    return true;
}

// ==========================================
// 1. Hospital Operations
// ==========================================

bool DatabaseManager::saveHospital(const Hospital& hospital) {
    return saveHospital(hospital.getHospitalId(),
                        hospital.getName(),
                        hospital.getCity(),
                        hospital.getTotalEmergencyBeds());
}

bool DatabaseManager::saveHospital(const std::string& hospitalId,
                                  const std::string& name,
                                  const std::string& city,
                                  int totalEmergencyBeds) {
    if (!isOpen()) return false;

    Statement stmt(db_, "INSERT INTO hospitals (hospital_id, hospital_name, city, bed_capacity) VALUES (?, ?, ?, ?);");
    if (!stmt.isValid()) {
        logSqliteError("Prepare saveHospital");
        return false;
    }

    stmt.bindText(1, hospitalId);
    stmt.bindText(2, name);
    stmt.bindText(3, city);
    stmt.bindInt(4, totalEmergencyBeds);

    int rc = stmt.step();
    if (rc != SQLITE_DONE) {
        logSqliteError("Execute saveHospital (" + hospitalId + ")");
        return false;
    }
    return true;
}

std::vector<Hospital> DatabaseManager::loadHospitals() {
    std::vector<Hospital> list;
    if (!isOpen()) return list;

    Statement stmt(db_, "SELECT hospital_id, hospital_name, city, bed_capacity FROM hospitals ORDER BY hospital_id ASC;");
    if (!stmt.isValid()) {
        logSqliteError("Prepare loadHospitals");
        return list;
    }

    while (stmt.step() == SQLITE_ROW) {
        std::string id = stmt.getText(0);
        std::string name = stmt.getText(1);
        std::string city = stmt.getText(2);
        int beds = stmt.getInt(3);
        list.emplace_back(std::move(id), std::move(name), std::move(city), beds);
    }
    return list;
}

bool DatabaseManager::hasHospital(const std::string& hospitalId) {
    if (!isOpen()) return false;
    Statement stmt(db_, "SELECT 1 FROM hospitals WHERE hospital_id = ?;");
    if (!stmt.isValid()) return false;
    stmt.bindText(1, hospitalId);
    return stmt.step() == SQLITE_ROW;
}

bool DatabaseManager::deleteHospital(const std::string& hospitalId) {
    if (!isOpen()) return false;
    Statement stmt(db_, "DELETE FROM hospitals WHERE hospital_id = ?;");
    if (!stmt.isValid()) return false;
    stmt.bindText(1, hospitalId);
    return stmt.execute();
}

// ==========================================
// 2. Patient Operations
// ==========================================

bool DatabaseManager::savePatient(const Patient& patient) {
    if (!isOpen()) return false;

    Statement stmt(db_, "INSERT INTO patients (hospital_id, patient_id, name, age, gender, blood_group, contact_phone) VALUES (?, ?, ?, ?, ?, ?, ?);");
    if (!stmt.isValid()) {
        logSqliteError("Prepare savePatient");
        return false;
    }

    stmt.bindText(1, patient.getHospitalId());
    stmt.bindText(2, patient.getPatientId());
    stmt.bindText(3, patient.getName());
    stmt.bindInt(4, patient.getAge());
    stmt.bindText(5, patient.getGender());
    stmt.bindText(6, patient.getBloodGroup());
    stmt.bindText(7, patient.getContactPhone());

    int rc = stmt.step();
    if (rc != SQLITE_DONE) {
        logSqliteError("Execute savePatient (" + patient.getHospitalId() + ", " + patient.getPatientId() + ")");
        return false;
    }
    return true;
}

std::vector<Patient> DatabaseManager::loadPatients() {
    std::vector<Patient> list;
    if (!isOpen()) return list;

    Statement stmt(db_, "SELECT patient_id, hospital_id, name, age, gender, blood_group, contact_phone FROM patients ORDER BY hospital_id ASC, patient_id ASC;");
    if (!stmt.isValid()) {
        logSqliteError("Prepare loadPatients");
        return list;
    }

    while (stmt.step() == SQLITE_ROW) {
        std::string pId = stmt.getText(0);
        std::string hId = stmt.getText(1);
        std::string name = stmt.getText(2);
        int age = stmt.getInt(3);
        std::string gender = stmt.getText(4);
        std::string blood = stmt.getText(5);
        std::string phone = stmt.getText(6);
        list.emplace_back(std::move(pId), std::move(hId), std::move(name), age, std::move(gender), std::move(blood), std::move(phone));
    }
    return list;
}

bool DatabaseManager::updatePatient(const std::string& hospitalId,
                                   const std::string& patientId,
                                   const std::string& newName,
                                   int newAge,
                                   const std::string& newGender,
                                   const std::string& newBloodGroup,
                                   const std::string& newPhone) {
    if (!isOpen()) return false;

    Statement stmt(db_, "UPDATE patients SET name = ?, age = ?, gender = ?, blood_group = ?, contact_phone = ? WHERE hospital_id = ? AND patient_id = ?;");
    if (!stmt.isValid()) return false;

    stmt.bindText(1, newName);
    stmt.bindInt(2, newAge);
    stmt.bindText(3, newGender);
    stmt.bindText(4, newBloodGroup);
    stmt.bindText(5, newPhone);
    stmt.bindText(6, hospitalId);
    stmt.bindText(7, patientId);

    int rc = stmt.step();
    return rc == SQLITE_DONE;
}

bool DatabaseManager::hasPatient(const std::string& hospitalId, const std::string& patientId) {
    if (!isOpen()) return false;
    Statement stmt(db_, "SELECT 1 FROM patients WHERE hospital_id = ? AND patient_id = ?;");
    if (!stmt.isValid()) return false;
    stmt.bindText(1, hospitalId);
    stmt.bindText(2, patientId);
    return stmt.step() == SQLITE_ROW;
}

bool DatabaseManager::deletePatient(const std::string& hospitalId, const std::string& patientId) {
    if (!isOpen()) return false;
    Statement stmt(db_, "DELETE FROM patients WHERE hospital_id = ? AND patient_id = ?;");
    if (!stmt.isValid()) return false;
    stmt.bindText(1, hospitalId);
    stmt.bindText(2, patientId);
    return stmt.execute();
}

// ==========================================
// 3. Visit Operations
// ==========================================

bool DatabaseManager::saveVisit(const Visit& visit) {
    if (!isOpen()) return false;

    Statement stmt(db_, "INSERT INTO visits (visit_id, hospital_id, patient_id, timestamp, triage_level, chief_complaint, diagnosis, status, notes, arrival_sequence, in_emergency_queue) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);");
    if (!stmt.isValid()) {
        logSqliteError("Prepare saveVisit");
        return false;
    }

    stmt.bindText(1, visit.getVisitId());
    stmt.bindText(2, visit.getHospitalId());
    stmt.bindText(3, visit.getPatientId());
    stmt.bindText(4, visit.getArrivalTime());
    stmt.bindInt(5, static_cast<int>(visit.getTriageLevel()));
    stmt.bindText(6, visit.getChiefComplaint());
    stmt.bindText(7, visit.getDiagnosis());
    stmt.bindInt(8, static_cast<int>(visit.getStatus()));
    stmt.bindText(9, visit.getNotes());
    stmt.bindInt(10, visit.getArrivalSequence());
    stmt.bindInt(11, visit.isInEmergencyQueue() ? 1 : 0);

    int rc = stmt.step();
    if (rc != SQLITE_DONE) {
        logSqliteError("Execute saveVisit (" + visit.getVisitId() + ")");
        return false;
    }
    return true;
}

std::vector<Visit> DatabaseManager::loadVisits() {
    std::vector<Visit> list;
    if (!isOpen()) return list;

    Statement stmt(db_, "SELECT visit_id, patient_id, hospital_id, timestamp, triage_level, chief_complaint, diagnosis, status, notes, arrival_sequence, in_emergency_queue FROM visits ORDER BY arrival_sequence ASC, timestamp ASC;");
    if (!stmt.isValid()) {
        logSqliteError("Prepare loadVisits");
        return list;
    }

    while (stmt.step() == SQLITE_ROW) {
        std::string vId = stmt.getText(0);
        std::string pId = stmt.getText(1);
        std::string hId = stmt.getText(2);
        std::string time = stmt.getText(3);
        int triage = stmt.getInt(4);
        std::string complaint = stmt.getText(5);
        std::string diag = stmt.getText(6);
        int status = stmt.getInt(7);
        std::string notes = stmt.getText(8);
        int arrivalSeq = stmt.getInt(9);
        int inQueue = stmt.getInt(10);

        list.emplace_back(std::move(vId),
                          std::move(pId),
                          std::move(hId),
                          std::move(time),
                          static_cast<TriageLevel>(triage),
                          std::move(complaint),
                          std::move(diag),
                          static_cast<Visit::Status>(status),
                          std::move(notes),
                          arrivalSeq,
                          inQueue == 1);
    }
    return list;
}

bool DatabaseManager::updateVisitStatus(const std::string& hospitalId,
                                       const std::string& patientId,
                                       const std::string& visitId,
                                       Visit::Status newStatus,
                                       bool inEmergencyQueue) {
    if (!isOpen()) return false;

    Statement stmt(db_, "UPDATE visits SET status = ?, in_emergency_queue = ? WHERE hospital_id = ? AND patient_id = ? AND visit_id = ?;");
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, static_cast<int>(newStatus));
    stmt.bindInt(2, inEmergencyQueue ? 1 : 0);
    stmt.bindText(3, hospitalId);
    stmt.bindText(4, patientId);
    stmt.bindText(5, visitId);

    int rc = stmt.step();
    return rc == SQLITE_DONE;
}

bool DatabaseManager::updateEmergencyTriage(const std::string& hospitalId,
                                           const std::string& patientId,
                                           int newTriageLevel) {
    if (!isOpen()) return false;

    Statement stmt(db_, "UPDATE visits SET triage_level = ? WHERE hospital_id = ? AND patient_id = ? AND in_emergency_queue = 1 AND status = 0;");
    if (!stmt.isValid()) return false;

    stmt.bindInt(1, newTriageLevel);
    stmt.bindText(2, hospitalId);
    stmt.bindText(3, patientId);

    int rc = stmt.step();
    return rc == SQLITE_DONE;
}

bool DatabaseManager::hasVisit(const std::string& hospitalId,
                              const std::string& patientId,
                              const std::string& visitId) {
    if (!isOpen()) return false;
    Statement stmt(db_, "SELECT 1 FROM visits WHERE hospital_id = ? AND patient_id = ? AND visit_id = ?;");
    if (!stmt.isValid()) return false;
    stmt.bindText(1, hospitalId);
    stmt.bindText(2, patientId);
    stmt.bindText(3, visitId);
    return stmt.step() == SQLITE_ROW;
}

// ==========================================
// 4. System Metadata & Counters
// ==========================================

bool DatabaseManager::setMetadata(const std::string& key, const std::string& value) {
    if (!isOpen()) return false;

    Statement stmt(db_, "INSERT OR REPLACE INTO system_metadata (key, value) VALUES (?, ?);");
    if (!stmt.isValid()) return false;

    stmt.bindText(1, key);
    stmt.bindText(2, value);
    return stmt.step() == SQLITE_DONE;
}

std::string DatabaseManager::getMetadata(const std::string& key, const std::string& defaultValue) {
    if (!isOpen()) return defaultValue;

    Statement stmt(db_, "SELECT value FROM system_metadata WHERE key = ?;");
    if (!stmt.isValid()) return defaultValue;

    stmt.bindText(1, key);
    if (stmt.step() == SQLITE_ROW) {
        return stmt.getText(0);
    }
    return defaultValue;
}

int DatabaseManager::getMetadataInt(const std::string& key, int defaultValue) {
    std::string val = getMetadata(key, "");
    if (val.empty()) return defaultValue;
    try {
        return std::stoi(val);
    } catch (...) {
        return defaultValue;
    }
}

bool DatabaseManager::setMetadataInt(const std::string& key, int value) {
    return setMetadata(key, std::to_string(value));
}

int DatabaseManager::getMaxArrivalSequence() {
    if (!isOpen()) return 0;
    Statement stmt(db_, "SELECT COALESCE(MAX(arrival_sequence), 0) FROM visits;");
    if (!stmt.isValid()) return 0;
    if (stmt.step() == SQLITE_ROW) {
        return stmt.getInt(0);
    }
    return 0;
}

int DatabaseManager::getMaxVisitCounter() {
    return getMetadataInt("max_visit_counter", 0);
}

// ==========================================
// Diagnostics & Error Logging
// ==========================================

std::string DatabaseManager::getLastError() const {
    return lastError_;
}

void DatabaseManager::setLastError(const std::string& err) const {
    lastError_ = err;
}

void DatabaseManager::logSqliteError(const std::string& context) const {
    if (db_) {
        std::string msg = context + ": " + sqlite3_errmsg(db_);
        setLastError(msg);
    } else {
        setLastError(context + ": database is closed");
    }
}

} // namespace smarter

