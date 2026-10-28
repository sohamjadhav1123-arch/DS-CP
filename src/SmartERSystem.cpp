#include "smarter/SmartERSystem.h"
#include <iomanip>
#include <algorithm>

namespace smarter {

SmartERSystem::SmartERSystem()
    : patientRegistry_(11, 0.75f),
      patientBST_(&patientRegistry_),
      visitHistory_(),
      arrivalCounter_(0),
      visitCounter_(0),
      dbManager_(nullptr) {}

// ==========================================
// 0. Persistence Management (SQLite Stage 9)
// ==========================================

bool SmartERSystem::initializeDatabase(const std::string& dbPath) {
    std::string path = dbPath.empty() ? DatabaseManager::getDefaultDatabasePath() : dbPath;
    dbManager_ = std::make_unique<DatabaseManager>(path);

    if (!dbManager_->open()) {
        std::cerr << "[SmartER Database Error] Failed to open database at " << path << ": "
                  << dbManager_->getLastError() << "\n";
        dbManager_.reset();
        return false;
    }

    if (!dbManager_->initializeSchema()) {
        std::cerr << "[SmartER Database Error] Failed to initialize schema: "
                  << dbManager_->getLastError() << "\n";
        dbManager_.reset();
        return false;
    }

    return loadFromDatabase();
}

bool SmartERSystem::isDatabaseEnabled() const {
    return dbManager_ != nullptr && dbManager_->isOpen();
}

DatabaseManager* SmartERSystem::getDatabaseManager() {
    return dbManager_.get();
}

const DatabaseManager* SmartERSystem::getDatabaseManager() const {
    return dbManager_.get();
}

bool SmartERSystem::loadFromDatabase() {
    if (!isDatabaseEnabled()) return false;

    // 1. Load Hospitals
    auto loadedHospitals = dbManager_->loadHospitals();
    for (const auto& h : loadedHospitals) {
        if (!hasHospital(h.getHospitalId())) {
            registerHospitalMemory(h.getHospitalId(), h.getName(), h.getCity(), h.getTotalEmergencyBeds());
        }
    }

    // 2. Load Patients
    auto loadedPatients = dbManager_->loadPatients();
    for (const auto& p : loadedPatients) {
        if (!patientRegistry_.containsPatient(p.getHospitalId(), p.getPatientId())) {
            patientRegistry_.insertPatient(p);
            patientBST_.insertPatient(p.getHospitalId(), p.getPatientId());
        }
    }

    // 3. Load Visits
    auto loadedVisits = dbManager_->loadVisits();
    int maxSeq = 0;
    for (const auto& v : loadedVisits) {
        if (visitHistory_.findVisit(v.getHospitalId(), v.getPatientId(), v.getVisitId()) == nullptr) {
            visitHistory_.addVisit(v);
        }

        if (v.getArrivalSequence() > maxSeq) {
            maxSeq = v.getArrivalSequence();
        }

        // 4. Reconstruct emergency queue from active waiting visits
        if (v.isInEmergencyQueue() && v.getStatus() == Visit::Status::Waiting) {
            Hospital* hosp = getHospital(v.getHospitalId());
            if (hosp) {
                if (!hosp->getEmergencyQueue().hasPatient(v.getPatientId())) {
                    hosp->getEmergencyQueue().insertPatient(v.getHospitalId(),
                                                            v.getPatientId(),
                                                            static_cast<int>(v.getTriageLevel()),
                                                            v.getArrivalSequence());
                }
            }
        }
    }

    // 5. Restore counters
    arrivalCounter_ = std::max(arrivalCounter_, std::max(maxSeq, dbManager_->getMaxArrivalSequence()));
    visitCounter_ = std::max(visitCounter_, dbManager_->getMaxVisitCounter());

    // Scan visit IDs to ensure visitCounter_ matches or exceeds parsed numeric suffixes
    for (const auto& v : loadedVisits) {
        const std::string& vid = v.getVisitId();
        size_t lastDash = vid.rfind('-');
        if (lastDash != std::string::npos && lastDash + 1 < vid.size()) {
            try {
                int idNum = std::stoi(vid.substr(lastDash + 1));
                if (idNum > visitCounter_) {
                    visitCounter_ = idNum;
                }
            } catch (...) {}
        }
    }

    return true;
}

// ==========================================
// 1. Hospital Management
// ==========================================

bool SmartERSystem::registerHospitalMemory(const std::string& hospitalId,
                                          const std::string& name,
                                          const std::string& city,
                                          int totalEmergencyBeds) {
    if (hospitalId.empty() || name.empty()) return false;
    if (hasHospital(hospitalId)) return false;

    auto hosp = std::make_unique<Hospital>(hospitalId, name, city, totalEmergencyBeds);
    hosp->bindRegistry(&patientRegistry_);
    hospitals_.push_back(std::move(hosp));
    return true;
}

bool SmartERSystem::registerHospital(const std::string& hospitalId,
                                     const std::string& name,
                                     const std::string& city,
                                     int totalEmergencyBeds) {
    if (hospitalId.empty() || name.empty()) {
        std::cerr << "[SmartER Error] Hospital ID and name cannot be empty.\n";
        return false;
    }

    if (hasHospital(hospitalId)) {
        std::cerr << "[SmartER Warning] Hospital '" << hospitalId
                  << "' is already registered. Rejected.\n";
        return false;
    }

    if (isDatabaseEnabled()) {
        if (!dbManager_->saveHospital(hospitalId, name, city, totalEmergencyBeds)) {
            std::cerr << "[SmartER Database Error] Failed to persist hospital '" << hospitalId
                      << "': " << dbManager_->getLastError() << "\n";
            return false;
        }
    }

    return registerHospitalMemory(hospitalId, name, city, totalEmergencyBeds);
}

bool SmartERSystem::registerHospital(const Hospital& hospital) {
    return registerHospital(hospital.getHospitalId(),
                            hospital.getName(),
                            hospital.getCity(),
                            hospital.getTotalEmergencyBeds());
}

bool SmartERSystem::hasHospital(const std::string& hospitalId) const {
    return getHospital(hospitalId) != nullptr;
}

Hospital* SmartERSystem::getHospital(const std::string& hospitalId) {
    for (auto& h : hospitals_) {
        if (h->getHospitalId() == hospitalId) {
            return h.get();
        }
    }
    return nullptr;
}

const Hospital* SmartERSystem::getHospital(const std::string& hospitalId) const {
    for (const auto& h : hospitals_) {
        if (h->getHospitalId() == hospitalId) {
            return h.get();
        }
    }
    return nullptr;
}

size_t SmartERSystem::getHospitalCount() const {
    return hospitals_.size();
}

std::vector<Hospital> SmartERSystem::getAllHospitals() const {
    std::vector<Hospital> result;
    result.reserve(hospitals_.size());
    for (const auto& h : hospitals_) {
        result.push_back(*h);
    }
    return result;
}

std::vector<Patient> SmartERSystem::getPatientsForHospital(const std::string& hospitalId) const {
    std::vector<Patient> result;
    auto allKeys = patientBST_.getInorderKeys();
    for (const auto& key : allKeys) {
        if (key.first == hospitalId) {
            const Patient* p = patientRegistry_.findPatient(key.first, key.second);
            if (p) {
                result.push_back(*p);
            }
        }
    }
    return result;
}

// ==========================================
// 2. Patient Registration (Registry + BST Atomic Sync)
// ==========================================

bool SmartERSystem::registerPatient(const Patient& patient) {
    const std::string& hospId = patient.getHospitalId();
    const std::string& patId = patient.getPatientId();

    // 1. Validate hospital existence
    if (!hasHospital(hospId)) {
        std::cerr << "[SmartER Error] Cannot register patient '" << patId
                  << "': Hospital '" << hospId << "' does not exist in network.\n";
        return false;
    }

    if (patId.empty()) {
        std::cerr << "[SmartER Error] Patient ID cannot be empty.\n";
        return false;
    }

    // 2. Reject duplicate composite key
    if (patientRegistry_.containsPatient(hospId, patId)) {
        std::cerr << "[SmartER Warning] Patient (" << hospId << ", " << patId
                  << ") already exists. Duplicate registration rejected.\n";
        return false;
    }

    // 3. Persist to database first if active
    if (isDatabaseEnabled()) {
        if (!dbManager_->savePatient(patient)) {
            std::cerr << "[SmartER Database Error] Failed to persist patient ("
                      << hospId << ", " << patId << "): " << dbManager_->getLastError() << "\n";
            return false;
        }
    }

    // 4. Insert into canonical Hash Table
    if (!patientRegistry_.insertPatient(patient)) {
        if (isDatabaseEnabled()) {
            dbManager_->deletePatient(hospId, patId);
        }
        return false;
    }

    // 5. Synchronize with sorted BST index (with atomic rollback on failure)
    if (!patientBST_.insertPatient(hospId, patId)) {
        std::cerr << "[SmartER Critical] BST insertion failed for (" << hospId
                  << ", " << patId << "). Performing atomic rollback on Registry.\n";
        patientRegistry_.deletePatient(hospId, patId);
        if (isDatabaseEnabled()) {
            dbManager_->deletePatient(hospId, patId);
        }
        return false;
    }

    return true;
}

bool SmartERSystem::registerPatient(const std::string& hospitalId,
                                    const std::string& patientId,
                                    const std::string& name,
                                    int age,
                                    const std::string& gender,
                                    const std::string& bloodGroup,
                                    const std::string& contactPhone) {
    Patient p(patientId, hospitalId, name, age, gender, bloodGroup, contactPhone);
    return registerPatient(p);
}

Patient* SmartERSystem::findPatient(const std::string& hospitalId, const std::string& patientId) {
    return patientRegistry_.findPatient(hospitalId, patientId);
}

const Patient* SmartERSystem::findPatient(const std::string& hospitalId, const std::string& patientId) const {
    return patientRegistry_.findPatient(hospitalId, patientId);
}

bool SmartERSystem::hasPatient(const std::string& hospitalId, const std::string& patientId) const {
    return patientRegistry_.containsPatient(hospitalId, patientId);
}

bool SmartERSystem::updatePatient(const std::string& hospitalId,
                                  const std::string& patientId,
                                  const std::string& newName,
                                  int newAge,
                                  const std::string& newGender,
                                  const std::string& newBloodGroup,
                                  const std::string& newPhone) {
    if (!hasPatient(hospitalId, patientId)) return false;

    if (isDatabaseEnabled()) {
        if (!dbManager_->updatePatient(hospitalId, patientId, newName, newAge, newGender, newBloodGroup, newPhone)) {
            return false;
        }
    }

    return patientRegistry_.updatePatient(hospitalId, patientId,
                                          newName, newAge, newGender,
                                          newBloodGroup, newPhone);
}

// ==========================================
// 3. Emergency Queue (Min-Heap)
// ==========================================

bool SmartERSystem::admitPatientToEmergency(const std::string& hospitalId,
                                           const std::string& patientId,
                                           int triageLevel,
                                           const std::string& chiefComplaint,
                                           const std::string& arrivalTime,
                                           const std::string& customVisitId) {
    Hospital* hosp = getHospital(hospitalId);
    if (!hosp) {
        std::cerr << "[SmartER Error] Hospital '" << hospitalId << "' not found.\n";
        return false;
    }

    if (!hasPatient(hospitalId, patientId)) {
        std::cerr << "[SmartER Error] Cannot admit un-registered patient ("
                  << hospitalId << ", " << patientId << ") to emergency room.\n";
        return false;
    }

    if (triageLevel < 1 || triageLevel > 5) {
        std::cerr << "[SmartER Error] Invalid triage level: " << triageLevel << ". Must be 1-5.\n";
        return false;
    }

    // Check if patient is already active in this hospital's emergency queue
    if (hosp->getEmergencyQueue().hasPatient(patientId)) {
        std::cerr << "[SmartER Warning] Patient (" << hospitalId << ", " << patientId
                  << ") is already waiting in the emergency queue.\n";
        return false;
    }

    int nextVisitNum = visitCounter_ + 1;
    std::string vId = customVisitId.empty()
        ? ("VIS-" + hospitalId + "-" + patientId + "-" + std::to_string(nextVisitNum))
        : customVisitId;

    std::string timestamp = arrivalTime.empty() ? "2026-10-23 10:00" : arrivalTime;
    int seq = arrivalCounter_ + 1;

    Visit v(vId, patientId, hospitalId, timestamp,
            static_cast<TriageLevel>(triageLevel),
            chiefComplaint, "Awaiting triage physician",
            Visit::Status::Waiting, "", seq, true);

    if (isDatabaseEnabled()) {
        ScopedTransaction tx(*dbManager_);
        if (!tx.isActive()) return false;

        if (!dbManager_->saveVisit(v)) {
            std::cerr << "[SmartER Database Error] Failed to persist ER visit: "
                      << dbManager_->getLastError() << "\n";
            return false;
        }

        dbManager_->setMetadataInt("max_visit_counter", nextVisitNum);
        dbManager_->setMetadataInt("max_arrival_sequence", seq);

        if (!tx.commit()) {
            std::cerr << "[SmartER Database Error] Transaction commit failed: "
                      << dbManager_->getLastError() << "\n";
            return false;
        }
    }

    visitCounter_ = nextVisitNum;
    arrivalCounter_ = seq;

    visitHistory_.addVisit(v);
    return hosp->getEmergencyQueue().insertPatient(hospitalId, patientId, triageLevel, seq);
}

bool SmartERSystem::enqueuePatient(const std::string& hospitalId,
                                   const std::string& patientId,
                                   int triageLevel,
                                   int arrivalSequence) {
    Hospital* hosp = getHospital(hospitalId);
    if (!hosp) return false;
    if (!hasPatient(hospitalId, patientId)) return false;
    return hosp->getEmergencyQueue().insertPatient(hospitalId, patientId, triageLevel, arrivalSequence);
}

std::optional<QueueEntry> SmartERSystem::peekNextEmergencyPatient(const std::string& hospitalId) const {
    const Hospital* hosp = getHospital(hospitalId);
    if (!hosp) return std::nullopt;
    return hosp->getEmergencyQueue().peekNextPatient();
}

std::optional<QueueEntry> SmartERSystem::serveNextEmergencyPatient(const std::string& hospitalId) {
    Hospital* hosp = getHospital(hospitalId);
    if (!hosp) return std::nullopt;

    auto served = hosp->getEmergencyQueue().serveNextPatient();
    if (!served.has_value()) {
        return std::nullopt;
    }

    // Update active 'Waiting' visit to 'InTreatment'
    auto visits = visitHistory_.getVisits(hospitalId, served->patientId);
    for (auto it = visits.rbegin(); it != visits.rend(); ++it) {
        if (it->getStatus() == Visit::Status::Waiting) {
            if (isDatabaseEnabled()) {
                dbManager_->updateVisitStatus(hospitalId, served->patientId,
                                              it->getVisitId(), Visit::Status::InTreatment, false);
            }
            visitHistory_.updateVisitStatus(hospitalId, served->patientId,
                                            it->getVisitId(), Visit::Status::InTreatment);
            break;
        }
    }

    return served;
}

bool SmartERSystem::updateEmergencyTriage(const std::string& hospitalId,
                                         const std::string& patientId,
                                         int newTriageLevel) {
    Hospital* hosp = getHospital(hospitalId);
    if (!hosp) return false;

    if (!hosp->getEmergencyQueue().hasPatient(patientId)) {
        return false;
    }

    if (newTriageLevel < 1 || newTriageLevel > 5) {
        std::cerr << "[Queue Error] Invalid Triage Level: " << newTriageLevel << ". Update rejected.\n";
        return false;
    }

    if (isDatabaseEnabled()) {
        if (!dbManager_->updateEmergencyTriage(hospitalId, patientId, newTriageLevel)) {
            std::cerr << "[SmartER Database Error] Failed to update triage level in DB: "
                      << dbManager_->getLastError() << "\n";
            return false;
        }
    }

    bool updated = hosp->getEmergencyQueue().updateTriageLevel(patientId, newTriageLevel);
    if (updated) {
        auto visits = visitHistory_.getVisits(hospitalId, patientId);
        for (auto it = visits.rbegin(); it != visits.rend(); ++it) {
            if (it->getStatus() == Visit::Status::Waiting) {
                Visit* v = visitHistory_.findVisit(hospitalId, patientId, it->getVisitId());
                if (v) {
                    v->setTriageLevel(static_cast<TriageLevel>(newTriageLevel));
                }
                break;
            }
        }
    }

    return updated;
}

void SmartERSystem::displayEmergencyQueue(const std::string& hospitalId, std::ostream& os) const {
    const Hospital* hosp = getHospital(hospitalId);
    if (!hosp) {
        os << "Hospital [" << hospitalId << "] not found.\n";
        return;
    }
    hosp->getEmergencyQueue().displayQueue(os);
}

size_t SmartERSystem::getEmergencyQueueSize(const std::string& hospitalId) const {
    const Hospital* hosp = getHospital(hospitalId);
    return hosp ? hosp->getEmergencyQueue().size() : 0;
}

std::vector<QueueEntry> SmartERSystem::getEmergencyQueueEntries(const std::string& hospitalId) const {
    const Hospital* hosp = getHospital(hospitalId);
    if (!hosp) return {};
    return hosp->getEmergencyQueue().getSortedEntries();
}

// ==========================================
// 4. Visit History (Singly Linked List)
// ==========================================

bool SmartERSystem::addVisit(const Visit& visit) {
    if (!hasHospital(visit.getHospitalId())) return false;
    if (!hasPatient(visit.getHospitalId(), visit.getPatientId())) return false;

    if (isDatabaseEnabled()) {
        if (!dbManager_->saveVisit(visit)) {
            std::cerr << "[SmartER Database Error] Failed to persist visit: "
                      << dbManager_->getLastError() << "\n";
            return false;
        }
    }

    return visitHistory_.addVisit(visit);
}

Visit* SmartERSystem::findVisit(const std::string& hospitalId,
                                const std::string& patientId,
                                const std::string& visitId) {
    return visitHistory_.findVisit(hospitalId, patientId, visitId);
}

const Visit* SmartERSystem::findVisit(const std::string& hospitalId,
                                      const std::string& patientId,
                                      const std::string& visitId) const {
    return visitHistory_.findVisit(hospitalId, patientId, visitId);
}

bool SmartERSystem::updateVisitStatus(const std::string& hospitalId,
                                     const std::string& patientId,
                                     const std::string& visitId,
                                     Visit::Status newStatus) {
    if (isDatabaseEnabled()) {
        bool inQueue = (newStatus == Visit::Status::Waiting);
        if (!dbManager_->updateVisitStatus(hospitalId, patientId, visitId, newStatus, inQueue)) {
            return false;
        }
    }

    return visitHistory_.updateVisitStatus(hospitalId, patientId, visitId, newStatus);
}

bool SmartERSystem::updateVisitStatus(const std::string& hospitalId,
                                     const std::string& visitId,
                                     Visit::Status newStatus) {
    auto allVisits = visitHistory_.getAllVisits();
    for (const auto& v : allVisits) {
        if (v.getHospitalId() == hospitalId && v.getVisitId() == visitId) {
            return updateVisitStatus(hospitalId, v.getPatientId(), visitId, newStatus);
        }
    }
    return false;
}

void SmartERSystem::displayPatientVisits(const std::string& hospitalId,
                                        const std::string& patientId,
                                        std::ostream& os) const {
    visitHistory_.displayHistory(hospitalId, patientId, os);
}

std::vector<Visit> SmartERSystem::getPatientVisits(const std::string& hospitalId,
                                                   const std::string& patientId) const {
    return visitHistory_.getVisits(hospitalId, patientId);
}

std::vector<Visit> SmartERSystem::getHospitalVisits(const std::string& hospitalId) const {
    std::vector<Visit> result;
    auto all = visitHistory_.getAllVisits();
    for (const auto& v : all) {
        if (v.getHospitalId() == hospitalId) {
            result.push_back(v);
        }
    }
    return result;
}

// ==========================================
// 5. Sorted Patient Directory (BST)
// ==========================================

void SmartERSystem::displaySortedPatients(std::ostream& os) const {
    patientBST_.displaySortedPatients(os);
}

// ==========================================
// 6. Analytics & Statistics
// ==========================================

HospitalStats SmartERSystem::getHospitalStatistics(const std::string& hospitalId) const {
    HospitalStats stats;
    stats.hospitalId = hospitalId;

    const Hospital* hosp = getHospital(hospitalId);
    if (!hosp) return stats;

    stats.hospitalName = hosp->getName();
    stats.city = hosp->getCity();
    stats.totalEmergencyBeds = hosp->getTotalEmergencyBeds();
    stats.activeQueueSize = hosp->getEmergencyQueue().size();

    auto allKeys = patientBST_.getInorderKeys();
    for (const auto& k : allKeys) {
        if (k.first == hospitalId) {
            ++stats.registeredPatientCount;
        }
    }

    auto allVisits = visitHistory_.getAllVisits();
    for (const auto& v : allVisits) {
        if (v.getHospitalId() == hospitalId) {
            ++stats.totalVisitsRecorded;
            if (v.getStatus() == Visit::Status::Waiting) ++stats.waitingVisits;
            else if (v.getStatus() == Visit::Status::InTreatment) ++stats.inTreatmentVisits;
            else if (v.getStatus() == Visit::Status::Discharged) ++stats.completedVisits;
        }
    }

    return stats;
}

void SmartERSystem::displayHospitalStatistics(const std::string& hospitalId, std::ostream& os) const {
    HospitalStats s = getHospitalStatistics(hospitalId);
    os << "======================================================================\n";
    os << " Hospital Analytics: " << s.hospitalName << " [" << s.hospitalId << "]\n";
    os << " Location: " << s.city << " | Bed Capacity: " << s.totalEmergencyBeds << "\n";
    os << "----------------------------------------------------------------------\n";
    os << " Registered Patients:    " << s.registeredPatientCount << "\n";
    os << " Active ER Waiting Queue: " << s.activeQueueSize << "\n";
    os << " Total Visits Logged:    " << s.totalVisitsRecorded << "\n";
    os << "   - Waiting Visits:     " << s.waitingVisits << "\n";
    os << "   - In Treatment:       " << s.inTreatmentVisits << "\n";
    os << "   - Completed:          " << s.completedVisits << "\n";
    os << "======================================================================\n";
}

PatientRegistry& SmartERSystem::getPatientRegistry() { return patientRegistry_; }
const PatientRegistry& SmartERSystem::getPatientRegistry() const { return patientRegistry_; }

PatientBST& SmartERSystem::getPatientBST() { return patientBST_; }
const PatientBST& SmartERSystem::getPatientBST() const { return patientBST_; }

VisitHistory& SmartERSystem::getVisitHistory() { return visitHistory_; }
const VisitHistory& SmartERSystem::getVisitHistory() const { return visitHistory_; }

} // namespace smarter
