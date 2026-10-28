#include "smarter/VisitHistory.h"
#include <iomanip>

namespace smarter {

VisitHistory::VisitHistory(std::string hospitalId, std::string patientId)
    : hospitalId_(std::move(hospitalId)),
      patientId_(std::move(patientId)),
      head_(nullptr),
      tail_(nullptr),
      size_(0) {}

VisitHistory::~VisitHistory() {
    clear();
}

VisitHistory::VisitHistory(const VisitHistory& other)
    : hospitalId_(other.hospitalId_),
      patientId_(other.patientId_),
      head_(nullptr),
      tail_(nullptr),
      size_(0) {
    VisitNode* curr = other.head_;
    while (curr != nullptr) {
        addVisit(curr->visit);
        curr = curr->next;
    }
}

VisitHistory& VisitHistory::operator=(const VisitHistory& other) {
    if (this != &other) {
        clear();
        hospitalId_ = other.hospitalId_;
        patientId_ = other.patientId_;
        VisitNode* curr = other.head_;
        while (curr != nullptr) {
            addVisit(curr->visit);
            curr = curr->next;
        }
    }
    return *this;
}

VisitHistory::VisitHistory(VisitHistory&& other) noexcept
    : hospitalId_(std::move(other.hospitalId_)),
      patientId_(std::move(other.patientId_)),
      head_(other.head_),
      tail_(other.tail_),
      size_(other.size_) {
    other.head_ = nullptr;
    other.tail_ = nullptr;
    other.size_ = 0;
}

VisitHistory& VisitHistory::operator=(VisitHistory&& other) noexcept {
    if (this != &other) {
        clear();
        hospitalId_ = std::move(other.hospitalId_);
        patientId_ = std::move(other.patientId_);
        head_ = other.head_;
        tail_ = other.tail_;
        size_ = other.size_;

        other.head_ = nullptr;
        other.tail_ = nullptr;
        other.size_ = 0;
    }
    return *this;
}

const std::string& VisitHistory::getHospitalId() const { return hospitalId_; }
const std::string& VisitHistory::getPatientId() const { return patientId_; }

void VisitHistory::setScope(const std::string& hospitalId, const std::string& patientId) {
    hospitalId_ = hospitalId;
    patientId_ = patientId;
}

bool VisitHistory::addVisit(const Visit& visit) {
    const std::string& hId = visit.getHospitalId();
    const std::string& pId = visit.getPatientId();
    const std::string& vId = visit.getVisitId();

    // 1. Basic validation
    if (hId.empty() || pId.empty() || vId.empty()) {
        std::cerr << "[VisitHistory Error] Visit must contain valid hospitalId, patientId, and visitId.\n";
        return false;
    }

    // 2. Scoped isolation check (if this list is bound to a specific patient)
    if (!hospitalId_.empty() && hId != hospitalId_) {
        std::cerr << "[VisitHistory Error] Hospital isolation violation: Cannot add visit for Hospital '"
                  << hId << "' into history scoped to Hospital '" << hospitalId_ << "'.\n";
        return false;
    }
    if (!patientId_.empty() && pId != patientId_) {
        std::cerr << "[VisitHistory Error] Patient isolation violation: Cannot add visit for Patient '"
                  << pId << "' into history scoped to Patient '" << patientId_ << "'.\n";
        return false;
    }

    // 3. Prevent duplicate visit ID within the same patient's history
    if (findVisit(hId, pId, vId) != nullptr) {
        std::cerr << "[VisitHistory Warning] Duplicate visit ID '" << vId
                  << "' for Patient (" << hId << ", " << pId << "). Rejected.\n";
        return false;
    }

    // 4. Append to tail (FIFO chronological order: oldest at head, newest at tail)
    VisitNode* newNode = new VisitNode(visit, nullptr);
    if (head_ == nullptr) {
        head_ = tail_ = newNode;
    } else {
        tail_->next = newNode;
        tail_ = newNode;
    }
    ++size_;

    return true;
}

void VisitHistory::displayHistory(const std::string& hospitalId,
                                  const std::string& patientId,
                                  std::ostream& os) const {
    os << "--------------------------------------------------------------------------------\n";
    os << "  Visit History for Patient: " << patientId << " (Hospital: " << hospitalId << ")\n";
    os << "--------------------------------------------------------------------------------\n";

    size_t count = 0;
    VisitNode* curr = head_;
    while (curr != nullptr) {
        if (curr->visit.getHospitalId() == hospitalId && curr->visit.getPatientId() == patientId) {
            ++count;
            curr->visit.printSummary(os);
        }
        curr = curr->next;
    }

    if (count == 0) {
        os << "  (No visit history found for this patient)\n";
    }
    os << "--------------------------------------------------------------------------------\n";
}

void VisitHistory::displayHistory(std::ostream& os) const {
    if (!hospitalId_.empty() && !patientId_.empty()) {
        displayHistory(hospitalId_, patientId_, os);
        return;
    }

    os << "--------------------------------------------------------------------------------\n";
    os << "  Complete Visit History Log (" << size_ << " total records)\n";
    os << "--------------------------------------------------------------------------------\n";

    if (head_ == nullptr) {
        os << "  (Visit history is currently empty)\n";
        os << "--------------------------------------------------------------------------------\n";
        return;
    }

    VisitNode* curr = head_;
    while (curr != nullptr) {
        curr->visit.printSummary(os);
        curr = curr->next;
    }
    os << "--------------------------------------------------------------------------------\n";
}

Visit* VisitHistory::findVisit(const std::string& hospitalId,
                               const std::string& patientId,
                               const std::string& visitId) {
    VisitNode* curr = head_;
    while (curr != nullptr) {
        if (curr->visit.getHospitalId() == hospitalId &&
            curr->visit.getPatientId() == patientId &&
            curr->visit.getVisitId() == visitId) {
            return &(curr->visit);
        }
        curr = curr->next;
    }
    return nullptr;
}

const Visit* VisitHistory::findVisit(const std::string& hospitalId,
                                     const std::string& patientId,
                                     const std::string& visitId) const {
    VisitNode* curr = head_;
    while (curr != nullptr) {
        if (curr->visit.getHospitalId() == hospitalId &&
            curr->visit.getPatientId() == patientId &&
            curr->visit.getVisitId() == visitId) {
            return &(curr->visit);
        }
        curr = curr->next;
    }
    return nullptr;
}

Visit* VisitHistory::findVisit(const std::string& visitId) {
    return findVisit(hospitalId_, patientId_, visitId);
}

const Visit* VisitHistory::findVisit(const std::string& visitId) const {
    return findVisit(hospitalId_, patientId_, visitId);
}

bool VisitHistory::updateVisitStatus(const std::string& hospitalId,
                                     const std::string& patientId,
                                     const std::string& visitId,
                                     Visit::Status newStatus) {
    Visit* v = findVisit(hospitalId, patientId, visitId);
    if (!v) {
        std::cerr << "[VisitHistory Warning] Visit '" << visitId
                  << "' for Patient (" << hospitalId << ", " << patientId << ") not found.\n";
        return false;
    }
    v->setStatus(newStatus);
    if (newStatus != Visit::Status::Waiting) {
        v->setInEmergencyQueue(false);
    }
    return true;
}

bool VisitHistory::updateVisitStatus(const std::string& visitId, Visit::Status newStatus) {
    return updateVisitStatus(hospitalId_, patientId_, visitId, newStatus);
}

size_t VisitHistory::getVisitCount(const std::string& hospitalId,
                                   const std::string& patientId) const {
    if (hospitalId == hospitalId_ && patientId == patientId_ && !hospitalId_.empty()) {
        return size_;
    }

    size_t count = 0;
    VisitNode* curr = head_;
    while (curr != nullptr) {
        if (curr->visit.getHospitalId() == hospitalId && curr->visit.getPatientId() == patientId) {
            ++count;
        }
        curr = curr->next;
    }
    return count;
}

size_t VisitHistory::size() const {
    return size_;
}

bool VisitHistory::hasHistory(const std::string& hospitalId,
                              const std::string& patientId) const {
    return getVisitCount(hospitalId, patientId) > 0;
}

bool VisitHistory::isEmpty() const {
    return size_ == 0;
}

bool VisitHistory::removeVisit(const std::string& hospitalId,
                               const std::string& patientId,
                               const std::string& visitId) {
    VisitNode* curr = head_;
    VisitNode* prev = nullptr;

    while (curr != nullptr) {
        if (curr->visit.getHospitalId() == hospitalId &&
            curr->visit.getPatientId() == patientId &&
            curr->visit.getVisitId() == visitId) {
            
            // Unlink curr
            if (prev == nullptr) {
                head_ = curr->next;
            } else {
                prev->next = curr->next;
            }

            if (curr == tail_) {
                tail_ = prev;
            }

            delete curr;
            --size_;
            return true;
        }

        prev = curr;
        curr = curr->next;
    }

    return false;
}

bool VisitHistory::removeVisit(const std::string& visitId) {
    return removeVisit(hospitalId_, patientId_, visitId);
}

void VisitHistory::clear() {
    VisitNode* curr = head_;
    while (curr != nullptr) {
        VisitNode* tmp = curr->next;
        delete curr;
        curr = tmp;
    }
    head_ = nullptr;
    tail_ = nullptr;
    size_ = 0;
}

std::vector<Visit> VisitHistory::getVisits(const std::string& hospitalId,
                                           const std::string& patientId) const {
    std::vector<Visit> result;
    VisitNode* curr = head_;
    while (curr != nullptr) {
        if (curr->visit.getHospitalId() == hospitalId && curr->visit.getPatientId() == patientId) {
            result.push_back(curr->visit);
        }
        curr = curr->next;
    }
    return result;
}

std::vector<Visit> VisitHistory::getAllVisits() const {
    std::vector<Visit> result;
    VisitNode* curr = head_;
    while (curr != nullptr) {
        result.push_back(curr->visit);
        curr = curr->next;
    }
    return result;
}

} // namespace smarter
