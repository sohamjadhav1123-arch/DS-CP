#pragma once

#include "smarter/Patient.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <cstddef>

namespace smarter {

/**
 * @brief Node in the separate chaining linked list for the Hash Table.
 * 
 * Memory Ownership & Stability Note:
 * Each HashNode is individually allocated on the free store (heap).
 * During table resizing, the pointers to these nodes are simply relinked
 * into new bucket chains without reallocating or moving the underlying
 * Patient objects. Therefore, raw pointers (Patient*) returned by findPatient()
 * remain strictly valid across hash table resizings, provided the specific
 * patient has not been deleted or the registry cleared.
 */
struct HashNode {
    Patient patient;         ///< The canonical Patient object
    HashNode* next;          ///< Pointer to next node in the bucket collision chain

    explicit HashNode(Patient p, HashNode* nextNode = nullptr)
        : patient(std::move(p)), next(nextNode) {}

    const std::string& getHospitalId() const { return patient.getHospitalId(); }
    const std::string& getPatientId() const { return patient.getPatientId(); }
};

/**
 * @brief Detailed statistics describing the current state of the Hash Table.
 */
struct RegistryStats {
    size_t totalBuckets = 0;       ///< Total number of bucket slots allocated
    size_t totalPatients = 0;      ///< Total number of patient records stored
    size_t emptyBuckets = 0;       ///< Number of buckets with 0 elements
    size_t collisionBuckets = 0;   ///< Number of buckets with more than 1 element
    size_t totalCollisions = 0;    ///< Total extra elements beyond head: sum(max(0, chainLen - 1))
    size_t maxChainLength = 0;     ///< Longest bucket collision chain
    float currentLoadFactor = 0.0f;///< totalPatients / totalBuckets
    float maxLoadFactor = 0.75f;   ///< Resizing threshold
};

/**
 * @brief Custom Hash Table implementation for the canonical Patient Registry.
 * 
 * Collision Resolution: Separate Chaining (Linked List per bucket).
 * Composite Key: (hospitalId, patientId).
 * 
 * Strict Project Rules:
 *  - std::unordered_map and std::map are strictly PROHIBITED.
 *  - Backed by a manually managed vector/array of HashNode* bucket heads.
 *  - Supports dynamic resizing and rehashing when load factor exceeds threshold.
 * 
 * Complexity (Average / Worst Case):
 *  - insertPatient():   O(1) avg / O(N) worst
 *  - findPatient():     O(1) avg / O(N) worst
 *  - updatePatient():   O(1) avg / O(N) worst
 *  - deletePatient():   O(1) avg / O(N) worst
 *  - containsPatient(): O(1) avg / O(N) worst
 */
class PatientRegistry {
public:
    /**
     * @brief Construct a new PatientRegistry with initial bucket capacity and max load factor.
     * @param initialBuckets Initial number of buckets (prime number recommended, default 11).
     * @param maxLoadFactor Threshold ratio of size/buckets to trigger automatic resizing (default 0.75).
     */
    explicit PatientRegistry(size_t initialBuckets = 11, float maxLoadFactor = 0.75f);

    /**
     * @brief Destructor: Frees all allocated HashNode instances across all buckets.
     */
    ~PatientRegistry();

    // Rule of Five for explicit, safe memory ownership
    PatientRegistry(const PatientRegistry& other);
    PatientRegistry& operator=(const PatientRegistry& other);
    PatientRegistry(PatientRegistry&& other) noexcept;
    PatientRegistry& operator=(PatientRegistry&& other) noexcept;

    /**
     * @brief Inserts a new patient into the registry.
     * 
     * Rejects duplicate composite keys (hospitalId, patientId).
     * Permits identical patientId at different hospitals.
     * Triggers dynamic resize if the load factor exceeds maxLoadFactor.
     * 
     * @param patient Patient object to insert
     * @return true if insertion succeeded, false if duplicate composite key found.
     */
    bool insertPatient(const Patient& patient);

    /**
     * @brief Searches for a patient using the composite key.
     * @param hospitalId Hospital identifier
     * @param patientId Patient identifier
     * @return Pointer to the canonical Patient object, or nullptr if not found.
     */
    Patient* findPatient(const std::string& hospitalId, const std::string& patientId);
    const Patient* findPatient(const std::string& hospitalId, const std::string& patientId) const;

    /**
     * @brief Updates an existing patient's mutable demographics.
     * 
     * Note: The composite key (hospitalId, patientId) is preserved to prevent corrupting hash index.
     * 
     * @param hospitalId Hospital identifier
     * @param patientId Patient identifier
     * @param newName Updated name
     * @param newAge Updated age
     * @param newGender Updated gender
     * @param newBloodGroup Updated blood group
     * @param newPhone Updated contact phone
     * @return true if patient was found and updated, false otherwise.
     */
    bool updatePatient(const std::string& hospitalId,
                       const std::string& patientId,
                       const std::string& newName,
                       int newAge,
                       const std::string& newGender,
                       const std::string& newBloodGroup,
                       const std::string& newPhone);

    /**
     * @brief Deletes a patient record by composite key.
     * 
     * Properly relinks pointers whether the node is at the head, middle, or tail
     * of the bucket collision chain, and reclaims memory.
     * 
     * @param hospitalId Hospital identifier
     * @param patientId Patient identifier
     * @return true if patient was found and deleted, false otherwise.
     */
    bool deletePatient(const std::string& hospitalId, const std::string& patientId);

    /**
     * @brief Checks if a patient exists in the registry.
     */
    bool containsPatient(const std::string& hospitalId, const std::string& patientId) const;

    /**
     * @brief Displays all stored patients in the registry.
     * 
     * Explains clearly that iteration order corresponds to bucket distribution
     * and is not sorted.
     */
    void displayPatients(std::ostream& os = std::cout) const;

    /**
     * @brief Prints diagnostic hash table metrics (buckets, load factor, collisions).
     */
    void displayStatistics(std::ostream& os = std::cout) const;

    /**
     * @brief Returns detailed metrics in a programmatic struct.
     */
    RegistryStats getStatistics() const;

    /**
     * @brief Returns the total number of patients stored.
     */
    size_t size() const;

    /**
     * @brief Checks if the registry is empty.
     */
    bool isEmpty() const;

    /**
     * @brief Returns current number of allocated buckets.
     */
    size_t getBucketCount() const;

    /**
     * @brief Clears all patient nodes from the table, freeing memory.
     */
    void clear();

    /**
     * @brief Primary hash function computing bucket index from composite key.
     * 
     * Uses polynomial rolling hash (Horner's rule, base 31) over hospitalId and patientId,
     * bounded by bucketCount.
     * 
     * @param hospitalId Hospital identifier
     * @param patientId Patient identifier
     * @return Valid bucket index in range [0, bucketCount - 1].
     */
    size_t hashFunction(const std::string& hospitalId, const std::string& patientId) const;

    /**
     * @brief Overload computing bucket index with an explicit bucket count (used during rehashing).
     */
    static size_t hashFunctionWithCapacity(const std::string& hospitalId,
                                           const std::string& patientId,
                                           size_t capacity);

private:
    size_t bucketCount_;              ///< Number of bucket slots
    size_t size_;                     ///< Total number of stored patients
    float maxLoadFactor_;             ///< Resizing threshold
    std::vector<HashNode*> buckets_;  ///< Array of bucket head pointers

    /**
     * @brief Automatically doubles capacity and rehashes all existing nodes.
     */
    void resize();
};

} // namespace smarter

