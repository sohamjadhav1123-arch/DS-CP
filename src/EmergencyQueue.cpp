#include "smarter/EmergencyQueue.h"
#include <iomanip>
#include <utility>

namespace smarter {

EmergencyQueue::EmergencyQueue(std::string hospitalId)
    : hospitalId_(std::move(hospitalId)) {}

const std::string& EmergencyQueue::getHospitalId() const {
    return hospitalId_;
}

void EmergencyQueue::setHospitalId(const std::string& hospitalId) {
    hospitalId_ = hospitalId;
}

bool EmergencyQueue::insertPatient(const std::string& hospitalId,
                                   const std::string& patientId,
                                   int triageLevel,
                                   int arrivalSequence) {
    // 1. Hospital Isolation Validation
    if (!hospitalId_.empty() && hospitalId != hospitalId_) {
        std::cerr << "[Queue Error] Hospital Isolation Violation: Cannot insert patient '"
                  << patientId << "' from Hospital '" << hospitalId
                  << "' into queue for Hospital '" << hospitalId_ << "'.\n";
        return false;
    }

    // 2. Triage Level Validation (must be 1 to 5)
    if (triageLevel < 1 || triageLevel > 5) {
        std::cerr << "[Queue Error] Invalid Triage Level: " << triageLevel
                  << ". Triage level must be between 1 (Immediate) and 5 (Non-Urgent).\n";
        return false;
    }

    // 3. Duplicate Prevention (Patient already in queue)
    if (hasPatient(patientId)) {
        std::cerr << "[Queue Warning] Duplicate Queue Entry: Patient '" << patientId
                  << "' is already waiting in the emergency queue for Hospital '"
                  << hospitalId_ << "'.\n";
        return false;
    }

    // If queue hospitalId wasn't previously set, associate it with this entry
    if (hospitalId_.empty()) {
        hospitalId_ = hospitalId;
    }

    // 4. Insert at the end of the heap and bubble up
    QueueEntry newEntry{hospitalId, patientId, triageLevel, arrivalSequence};
    heap_.push_back(newEntry);
    heapifyUp(heap_.size() - 1);
    return true;
}

bool EmergencyQueue::insertPatient(const QueueEntry& entry) {
    return insertPatient(entry.hospitalId, entry.patientId, entry.triageLevel, entry.arrivalSequence);
}

std::optional<QueueEntry> EmergencyQueue::peekNextPatient() const {
    if (heap_.empty()) {
        return std::nullopt;
    }
    return heap_.front();
}

std::optional<QueueEntry> EmergencyQueue::serveNextPatient() {
    if (heap_.empty()) {
        return std::nullopt;
    }

    QueueEntry served = heap_.front();

    if (heap_.size() == 1) {
        heap_.pop_back();
    } else {
        // Move last element to root and bubble down
        heap_.front() = std::move(heap_.back());
        heap_.pop_back();
        heapifyDown(0);
    }

    return served;
}

void EmergencyQueue::displayQueue(std::ostream& os) const {
    if (heap_.empty()) {
        os << "Emergency Queue for Hospital [" << hospitalId_ << "] is currently empty.\n";
        return;
    }

    // Clone the current queue to extract entries in priority order without mutating original heap
    EmergencyQueue tempQueue = *this;

    os << "----------------------------------------------------------------------\n";
    os << " Emergency Queue for Hospital: " << hospitalId_ << " (" << tempQueue.size() << " patients waiting)\n";
    os << "----------------------------------------------------------------------\n";
    os << std::left
       << std::setw(6)  << "Rank"
       << std::setw(14) << "Hospital ID"
       << std::setw(14) << "Patient ID"
       << std::setw(14) << "Triage Level"
       << std::setw(18) << "Arrival Sequence" << "\n";
    os << "----------------------------------------------------------------------\n";

    int rank = 1;
    while (!tempQueue.isEmpty()) {
        auto entryOpt = tempQueue.serveNextPatient();
        if (entryOpt.has_value()) {
            const auto& e = *entryOpt;
            os << std::left
               << std::setw(6)  << rank++
               << std::setw(14) << e.hospitalId
               << std::setw(14) << e.patientId
               << std::setw(14) << e.triageLevel
               << std::setw(18) << e.arrivalSequence << "\n";
        }
    }
    os << "----------------------------------------------------------------------\n";
}

std::vector<QueueEntry> EmergencyQueue::getSortedEntries() const {
    EmergencyQueue tempQueue = *this;
    std::vector<QueueEntry> sorted;
    sorted.reserve(tempQueue.size());
    while (!tempQueue.isEmpty()) {
        auto entryOpt = tempQueue.serveNextPatient();
        if (entryOpt.has_value()) {
            sorted.push_back(*entryOpt);
        }
    }
    return sorted;
}

bool EmergencyQueue::updateTriageLevel(const std::string& patientId, int newTriageLevel) {
    // 1. Validate new triage level
    if (newTriageLevel < 1 || newTriageLevel > 5) {
        std::cerr << "[Queue Error] Invalid Triage Level: " << newTriageLevel
                  << ". Update rejected.\n";
        return false;
    }

    // 2. Locate patient in heap
    size_t targetIdx = heap_.size();
    for (size_t i = 0; i < heap_.size(); ++i) {
        if (heap_[i].patientId == patientId) {
            targetIdx = i;
            break;
        }
    }

    if (targetIdx == heap_.size()) {
        std::cerr << "[Queue Warning] Patient '" << patientId
                  << "' not found in Emergency Queue for Hospital '" << hospitalId_ << "'.\n";
        return false;
    }

    int oldLevel = heap_[targetIdx].triageLevel;
    heap_[targetIdx].triageLevel = newTriageLevel;

    // 3. Restore heap property based on priority shift
    if (newTriageLevel < oldLevel) {
        // Higher clinical priority (smaller number) -> bubble UP
        heapifyUp(targetIdx);
    } else if (newTriageLevel > oldLevel) {
        // Lower clinical priority (larger number) -> bubble DOWN
        heapifyDown(targetIdx);
    }

    return true;
}

bool EmergencyQueue::isEmpty() const {
    return heap_.empty();
}

size_t EmergencyQueue::size() const {
    return heap_.size();
}

bool EmergencyQueue::hasPatient(const std::string& patientId) const {
    for (const auto& entry : heap_) {
        if (entry.patientId == patientId) {
            return true;
        }
    }
    return false;
}

void EmergencyQueue::heapifyUp(size_t index) {
    while (index > 0) {
        size_t parentIdx = parent(index);
        if (heap_[index].hasHigherPriorityThan(heap_[parentIdx])) {
            std::swap(heap_[index], heap_[parentIdx]);
            index = parentIdx;
        } else {
            break;
        }
    }
}

void EmergencyQueue::heapifyDown(size_t index) {
    const size_t n = heap_.size();
    while (true) {
        size_t smallest = index;
        size_t left = leftChild(index);
        size_t right = rightChild(index);

        if (left < n && heap_[left].hasHigherPriorityThan(heap_[smallest])) {
            smallest = left;
        }
        if (right < n && heap_[right].hasHigherPriorityThan(heap_[smallest])) {
            smallest = right;
        }

        if (smallest != index) {
            std::swap(heap_[index], heap_[smallest]);
            index = smallest;
        } else {
            break;
        }
    }
}

} // namespace smarter

