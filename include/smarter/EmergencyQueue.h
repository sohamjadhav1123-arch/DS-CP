#pragma once

#include <string>
#include <vector>
#include <optional>
#include <iostream>
#include <cstddef>

namespace smarter {

/**
 * @brief Represents a single triage entry within the Emergency Queue.
 * 
 * In a Min-Heap:
 *  - Primary Key: Triage Level (1 = Immediate/Highest Priority, 5 = Non-Urgent/Lowest Priority)
 *  - Secondary Key: Arrival Sequence (Smaller number = Arrived earlier, used as tie-breaker)
 */
struct QueueEntry {
    std::string hospitalId;      ///< Scoped hospital identifier
    std::string patientId;       ///< Unique patient identifier within this hospital
    int triageLevel;             ///< Acuity level (1 to 5)
    int arrivalSequence;         ///< Monotonically increasing arrival counter

    /**
     * @brief Determines if this entry has strictly higher priority than another.
     * 
     * Rule:
     * 1. Lower triage numerical value has higher clinical urgency.
     * 2. If triage values are equal, lower arrival sequence number (earlier arrival) takes precedence.
     */
    bool hasHigherPriorityThan(const QueueEntry& other) const {
        if (triageLevel != other.triageLevel) {
            return triageLevel < other.triageLevel;
        }
        return arrivalSequence < other.arrivalSequence;
    }
};

/**
 * @brief Custom Min-Heap implementation for a hospital emergency triage queue.
 * 
 * Design Constraints:
 * - Implemented manually using std::vector as the underlying contiguous array.
 * - std::priority_queue and external libraries are strictly NOT used.
 * - Strictly isolated to a single hospital to prevent cross-facility patient mixing.
 * 
 * Complexity Summary:
 * - insertPatient():     O(N) due to duplicate check, O(log N) heap insertion
 * - peekNextPatient():   O(1)
 * - serveNextPatient():  O(log N)
 * - displayQueue():      O(N log N) without modifying the original heap
 * - updateTriageLevel(): O(N) lookup + O(log N) heap adjustment
 * - isEmpty():           O(1)
 * - size():              O(1)
 */
class EmergencyQueue {
public:
    /**
     * @brief Construct a new EmergencyQueue for a specific hospital.
     * @param hospitalId Unique ID of the hospital owning this queue.
     */
    explicit EmergencyQueue(std::string hospitalId = "");

    // Getters / Setters
    const std::string& getHospitalId() const;
    void setHospitalId(const std::string& hospitalId);

    /**
     * @brief Inserts a patient into the emergency queue.
     * 
     * Validation checks:
     *  - hospitalId must match this queue's hospitalId.
     *  - triageLevel must be within [1, 5].
     *  - patientId must not already exist in this queue.
     * 
     * @param hospitalId Hospital ID of the patient
     * @param patientId Patient identifier
     * @param triageLevel Triage priority (1-5)
     * @param arrivalSequence Order of arrival
     * @return true if insertion succeeded; false if validation failed.
     */
    bool insertPatient(const std::string& hospitalId,
                       const std::string& patientId,
                       int triageLevel,
                       int arrivalSequence);

    /**
     * @brief Overload to insert a pre-constructed QueueEntry.
     */
    bool insertPatient(const QueueEntry& entry);

    /**
     * @brief Inspects the next highest-priority patient without removing them.
     * @return std::optional containing the highest-priority QueueEntry, or std::nullopt if empty.
     */
    std::optional<QueueEntry> peekNextPatient() const;

    /**
     * @brief Removes and returns the highest-priority patient from the queue.
     * @return std::optional containing the served QueueEntry, or std::nullopt if empty.
     */
    std::optional<QueueEntry> serveNextPatient();

    /**
     * @brief Displays the current queue in priority order without destroying the heap.
     * @param os Output stream (defaults to std::cout)
     */
    void displayQueue(std::ostream& os = std::cout) const;

    /**
     * @brief Updates the triage level of an existing patient and restores the heap property.
     * 
     * If the new level has higher priority (smaller number), heapifyUp is performed.
     * If the new level has lower priority (larger number), heapifyDown is performed.
     * 
     * @param patientId Patient ID to update
     * @param newTriageLevel New triage level (1 to 5)
     * @return true if updated; false if patient not found or triage level invalid.
     */
    bool updateTriageLevel(const std::string& patientId, int newTriageLevel);

    /**
     * @brief Returns a copy of all queue entries sorted in strict priority order.
     */
    std::vector<QueueEntry> getSortedEntries() const;

    /**
     * @brief Checks if the queue is empty.
     */
    bool isEmpty() const;

    /**
     * @brief Returns the number of patients currently waiting in the queue.
     */
    size_t size() const;

    /**
     * @brief Checks if a patient ID is currently present in this queue.
     */
    bool hasPatient(const std::string& patientId) const;

private:
    std::string hospitalId_;          ///< Owner hospital ID
    std::vector<QueueEntry> heap_;    ///< Contiguous binary min-heap storage

    // Helper functions for binary heap indexing (0-based)
    static size_t parent(size_t index) { return (index - 1) / 2; }
    static size_t leftChild(size_t index) { return 2 * index + 1; }
    static size_t rightChild(size_t index) { return 2 * index + 2; }

    /**
     * @brief Moves an element at 'index' up the heap until heap invariant holds.
     */
    void heapifyUp(size_t index);

    /**
     * @brief Moves an element at 'index' down the heap until heap invariant holds.
     */
    void heapifyDown(size_t index);
};

} // namespace smarter

