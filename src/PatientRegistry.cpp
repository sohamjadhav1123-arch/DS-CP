#include "smarter/PatientRegistry.h"
#include <iomanip>
#include <utility>

namespace smarter {

PatientRegistry::PatientRegistry(size_t initialBuckets, float maxLoadFactor)
    : bucketCount_(initialBuckets < 3 ? 3 : initialBuckets),
      size_(0),
      maxLoadFactor_(maxLoadFactor <= 0.1f ? 0.75f : maxLoadFactor),
      buckets_(bucketCount_, nullptr) {}

PatientRegistry::~PatientRegistry() {
    clear();
}

PatientRegistry::PatientRegistry(const PatientRegistry& other)
    : bucketCount_(other.bucketCount_),
      size_(0),
      maxLoadFactor_(other.maxLoadFactor_),
      buckets_(other.bucketCount_, nullptr) {
    for (size_t i = 0; i < other.bucketCount_; ++i) {
        HashNode* curr = other.buckets_[i];
        while (curr != nullptr) {
            insertPatient(curr->patient);
            curr = curr->next;
        }
    }
}

PatientRegistry& PatientRegistry::operator=(const PatientRegistry& other) {
    if (this != &other) {
        clear();
        bucketCount_ = other.bucketCount_;
        maxLoadFactor_ = other.maxLoadFactor_;
        buckets_.assign(bucketCount_, nullptr);
        for (size_t i = 0; i < other.bucketCount_; ++i) {
            HashNode* curr = other.buckets_[i];
            while (curr != nullptr) {
                insertPatient(curr->patient);
                curr = curr->next;
            }
        }
    }
    return *this;
}

PatientRegistry::PatientRegistry(PatientRegistry&& other) noexcept
    : bucketCount_(other.bucketCount_),
      size_(other.size_),
      maxLoadFactor_(other.maxLoadFactor_),
      buckets_(std::move(other.buckets_)) {
    other.bucketCount_ = 0;
    other.size_ = 0;
}

PatientRegistry& PatientRegistry::operator=(PatientRegistry&& other) noexcept {
    if (this != &other) {
        clear();
        bucketCount_ = other.bucketCount_;
        size_ = other.size_;
        maxLoadFactor_ = other.maxLoadFactor_;
        buckets_ = std::move(other.buckets_);

        other.bucketCount_ = 0;
        other.size_ = 0;
    }
    return *this;
}

size_t PatientRegistry::hashFunctionWithCapacity(const std::string& hospitalId,
                                                 const std::string& patientId,
                                                 size_t capacity) {
    if (capacity == 0) return 0;

    // Polynomial rolling hash with prime multiplier 31 (Horner's rule)
    // Delimiter ':' separates hospital ID from patient ID to avoid false collisions
    // e.g. ("10", "11001") vs ("101", "1001")
    size_t hashValue = 0;
    const size_t prime = 31;

    for (char c : hospitalId) {
        hashValue = hashValue * prime + static_cast<unsigned char>(c);
    }
    hashValue = hashValue * prime + static_cast<unsigned char>(':');
    for (char c : patientId) {
        hashValue = hashValue * prime + static_cast<unsigned char>(c);
    }

    return hashValue % capacity;
}

size_t PatientRegistry::hashFunction(const std::string& hospitalId, const std::string& patientId) const {
    return hashFunctionWithCapacity(hospitalId, patientId, bucketCount_);
}

bool PatientRegistry::insertPatient(const Patient& patient) {
    const std::string& hospId = patient.getHospitalId();
    const std::string& patId = patient.getPatientId();

    // 1. Check for duplicate composite key (hospitalId, patientId)
    if (containsPatient(hospId, patId)) {
        std::cerr << "[Registry Warning] Duplicate composite key (" << hospId
                  << ", " << patId << ") rejected. Patient already registered.\n";
        return false;
    }

    // 2. Check load factor and trigger resize if needed
    if (static_cast<float>(size_ + 1) / static_cast<float>(bucketCount_) > maxLoadFactor_) {
        resize();
    }

    // 3. Compute bucket index
    size_t index = hashFunction(hospId, patId);

    // 4. Prepend to bucket collision chain (O(1) insertion)
    HashNode* newNode = new HashNode(patient, buckets_[index]);
    buckets_[index] = newNode;
    ++size_;

    return true;
}

Patient* PatientRegistry::findPatient(const std::string& hospitalId, const std::string& patientId) {
    if (bucketCount_ == 0) return nullptr;

    size_t index = hashFunction(hospitalId, patientId);
    HashNode* curr = buckets_[index];

    while (curr != nullptr) {
        if (curr->getHospitalId() == hospitalId && curr->getPatientId() == patientId) {
            return &(curr->patient);
        }
        curr = curr->next;
    }

    return nullptr;
}

const Patient* PatientRegistry::findPatient(const std::string& hospitalId, const std::string& patientId) const {
    if (bucketCount_ == 0) return nullptr;

    size_t index = hashFunction(hospitalId, patientId);
    HashNode* curr = buckets_[index];

    while (curr != nullptr) {
        if (curr->getHospitalId() == hospitalId && curr->getPatientId() == patientId) {
            return &(curr->patient);
        }
        curr = curr->next;
    }

    return nullptr;
}

bool PatientRegistry::updatePatient(const std::string& hospitalId,
                                    const std::string& patientId,
                                    const std::string& newName,
                                    int newAge,
                                    const std::string& newGender,
                                    const std::string& newBloodGroup,
                                    const std::string& newPhone) {
    Patient* target = findPatient(hospitalId, patientId);
    if (!target) {
        std::cerr << "[Registry Warning] Cannot update non-existent patient ("
                  << hospitalId << ", " << patientId << ").\n";
        return false;
    }

    // Update mutable demographic fields while preserving the composite key
    target->setName(newName);
    target->setAge(newAge);
    target->setGender(newGender);
    target->setBloodGroup(newBloodGroup);
    target->setContactPhone(newPhone);

    return true;
}

bool PatientRegistry::deletePatient(const std::string& hospitalId, const std::string& patientId) {
    if (bucketCount_ == 0 || size_ == 0) return false;

    size_t index = hashFunction(hospitalId, patientId);
    HashNode* curr = buckets_[index];
    HashNode* prev = nullptr;

    while (curr != nullptr) {
        if (curr->getHospitalId() == hospitalId && curr->getPatientId() == patientId) {
            // Relink bucket chain
            if (prev == nullptr) {
                // Deleting first node in chain (bucket head)
                buckets_[index] = curr->next;
            } else {
                // Deleting middle or last node in chain
                prev->next = curr->next;
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

bool PatientRegistry::containsPatient(const std::string& hospitalId, const std::string& patientId) const {
    return findPatient(hospitalId, patientId) != nullptr;
}

void PatientRegistry::displayPatients(std::ostream& os) const {
    os << "----------------------------------------------------------------------\n";
    os << "  Canonical Patient Registry (" << size_ << " patients total)\n";
    os << "  [Note: Iteration order corresponds to hash bucket slots, NOT sorted]\n";
    os << "----------------------------------------------------------------------\n";

    if (size_ == 0) {
        os << "  (Registry is currently empty)\n";
        return;
    }

    size_t printed = 0;
    for (size_t b = 0; b < bucketCount_; ++b) {
        HashNode* curr = buckets_[b];
        while (curr != nullptr) {
            os << "  [" << ++printed << "] Bucket #" << std::setw(2) << b << " | Key: ("
               << curr->getHospitalId() << ", " << curr->getPatientId() << ")\n"
               << "      Name: " << curr->patient.getName()
               << " | Age: " << curr->patient.getAge()
               << " | Gender: " << curr->patient.getGender()
               << " | Blood: " << curr->patient.getBloodGroup()
               << " | Phone: " << curr->patient.getContactPhone() << "\n";
            curr = curr->next;
        }
    }
    os << "----------------------------------------------------------------------\n";
}

RegistryStats PatientRegistry::getStatistics() const {
    RegistryStats stats;
    stats.totalBuckets = bucketCount_;
    stats.totalPatients = size_;
    stats.maxLoadFactor = maxLoadFactor_;
    stats.currentLoadFactor = (bucketCount_ > 0)
        ? static_cast<float>(size_) / static_cast<float>(bucketCount_)
        : 0.0f;

    for (size_t i = 0; i < bucketCount_; ++i) {
        size_t chainLen = 0;
        HashNode* curr = buckets_[i];
        while (curr != nullptr) {
            ++chainLen;
            curr = curr->next;
        }

        if (chainLen == 0) {
            ++stats.emptyBuckets;
        } else if (chainLen > 1) {
            ++stats.collisionBuckets;
            stats.totalCollisions += (chainLen - 1);
        }

        if (chainLen > stats.maxChainLength) {
            stats.maxChainLength = chainLen;
        }
    }

    return stats;
}

void PatientRegistry::displayStatistics(std::ostream& os) const {
    RegistryStats stats = getStatistics();

    os << "================== Patient Registry Statistics ==================\n";
    os << "  Total Stored Patients:  " << stats.totalPatients << "\n";
    os << "  Allocated Buckets:      " << stats.totalBuckets << "\n";
    os << "  Empty Buckets:          " << stats.emptyBuckets << "\n";
    os << "  Collision Buckets (>1): " << stats.collisionBuckets << "\n";
    os << "  Total Collisions:       " << stats.totalCollisions << "\n";
    os << "  Maximum Chain Length:   " << stats.maxChainLength << "\n";
    os << std::fixed << std::setprecision(3);
    os << "  Current Load Factor:    " << stats.currentLoadFactor << "\n";
    os << "  Max Load Threshold:     " << stats.maxLoadFactor << "\n";
    os << "=================================================================\n";
}

size_t PatientRegistry::size() const {
    return size_;
}

bool PatientRegistry::isEmpty() const {
    return size_ == 0;
}

size_t PatientRegistry::getBucketCount() const {
    return bucketCount_;
}

void PatientRegistry::clear() {
    for (size_t i = 0; i < bucketCount_; ++i) {
        HashNode* curr = buckets_[i];
        while (curr != nullptr) {
            HashNode* nextNode = curr->next;
            delete curr;
            curr = nextNode;
        }
        buckets_[i] = nullptr;
    }
    size_ = 0;
}

void PatientRegistry::resize() {
    // Double the bucket count plus one (helps keep it odd / avoid powers of 2)
    size_t newCapacity = bucketCount_ * 2 + 1;
    std::vector<HashNode*> newBuckets(newCapacity, nullptr);

    // Rehash existing nodes into new bucket array without reallocating Patient memory
    for (size_t i = 0; i < bucketCount_; ++i) {
        HashNode* curr = buckets_[i];
        while (curr != nullptr) {
            HashNode* nextNode = curr->next;

            // Compute index with new capacity
            size_t newIndex = hashFunctionWithCapacity(curr->getHospitalId(),
                                                       curr->getPatientId(),
                                                       newCapacity);

            // Prepend existing node to new bucket chain
            curr->next = newBuckets[newIndex];
            newBuckets[newIndex] = curr;

            curr = nextNode;
        }
    }

    buckets_ = std::move(newBuckets);
    bucketCount_ = newCapacity;
}

} // namespace smarter

