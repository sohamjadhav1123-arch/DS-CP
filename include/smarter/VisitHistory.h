#pragma once

#include "smarter/Visit.hpp"
#include <string>
#include <vector>
#include <iostream>
#include <cstddef>

namespace smarter {

/**
 * @brief Node in the custom singly linked list representing a patient visit.
 * 
 * Each node stores a complete Visit record and a pointer to the next visit node.
 */
struct VisitNode {
    Visit visit;            ///< The visit record
    VisitNode* next;        ///< Pointer to subsequent visit in chronological order

    explicit VisitNode(Visit v, VisitNode* nextNode = nullptr)
        : visit(std::move(v)), next(nextNode) {}
};

/**
 * @brief Custom Singly Linked List for maintaining chronological patient visit histories.
 * 
 * Design Constraints:
 *  - std::list or other container classes are strictly PROHIBITED.
 *  - Manually manages heap-allocated VisitNode instances with head and tail pointers.
 *  - Visits are linked in chronological arrival order (oldest first, newest at tail).
 *  - Maintains hospital and patient scoping: each visit is bound to (hospitalId, patientId).
 * 
 * Complexity Summary:
 *  - addVisit():           O(1) append to tail (O(K) if duplicate check across patient visits)
 *  - findVisit():          O(K) linear traversal over patient visits
 *  - updateVisitStatus():  O(K) linear lookup + O(1) field update
 *  - removeVisit():        O(K) node unlinking and deallocation
 *  - displayHistory():     O(K) sequential traversal
 *  - getVisitCount():      O(1) for scoped list, O(N) for multi-patient list
 *  - isEmpty():            O(1)
 *  - clear():              O(N) deallocating nodes
 */
class VisitHistory {
public:
    /**
     * @brief Constructs a new VisitHistory singly linked list.
     * Can optionally be scoped to a specific (hospitalId, patientId).
     * 
     * @param hospitalId Optional hospital identifier
     * @param patientId Optional patient identifier
     */
    explicit VisitHistory(std::string hospitalId = "", std::string patientId = "");

    /**
     * @brief Destructor: Recursively deallocates all linked list nodes.
     */
    ~VisitHistory();

    // Rule of Five: Deep copying and moving
    VisitHistory(const VisitHistory& other);
    VisitHistory& operator=(const VisitHistory& other);
    VisitHistory(VisitHistory&& other) noexcept;
    VisitHistory& operator=(VisitHistory&& other) noexcept;

    // Getters / Setters for scoped ownership
    const std::string& getHospitalId() const;
    const std::string& getPatientId() const;
    void setScope(const std::string& hospitalId, const std::string& patientId);

    /**
     * @brief Appends a new visit to the linked list.
     * 
     * Enforces:
     * 1. Valid hospitalId, patientId, and visitId.
     * 2. If scoped, ensures visit matches scoped hospital and patient.
     * 3. Prevents duplicate visitId for the same patient.
     * 
     * @param visit The visit object to append
     * @return true if appended successfully, false if validation/duplicate failed.
     */
    bool addVisit(const Visit& visit);

    /**
     * @brief Displays visits for a specified hospital and patient in chronological order.
     * @param hospitalId Hospital identifier
     * @param patientId Patient identifier
     * @param os Output stream
     */
    void displayHistory(const std::string& hospitalId,
                        const std::string& patientId,
                        std::ostream& os = std::cout) const;

    /**
     * @brief Displays all visits stored in this list (or for scoped patient).
     * @param os Output stream
     */
    void displayHistory(std::ostream& os = std::cout) const;

    /**
     * @brief Finds a visit by hospitalId, patientId, and visitId.
     * @return Pointer to Visit object if found, nullptr otherwise.
     */
    Visit* findVisit(const std::string& hospitalId,
                     const std::string& patientId,
                     const std::string& visitId);

    const Visit* findVisit(const std::string& hospitalId,
                           const std::string& patientId,
                           const std::string& visitId) const;

    /**
     * @brief Scoped lookup when list is bound to a specific hospital and patient.
     */
    Visit* findVisit(const std::string& visitId);
    const Visit* findVisit(const std::string& visitId) const;

    /**
     * @brief Updates a visit's clinical status (e.g. Waiting -> InTreatment -> Completed).
     * @return true if found and updated, false otherwise.
     */
    bool updateVisitStatus(const std::string& hospitalId,
                           const std::string& patientId,
                           const std::string& visitId,
                           Visit::Status newStatus);

    bool updateVisitStatus(const std::string& visitId, Visit::Status newStatus);

    /**
     * @brief Counts visits for a specific hospital and patient.
     */
    size_t getVisitCount(const std::string& hospitalId, const std::string& patientId) const;

    /**
     * @brief Returns total number of visit nodes in this linked list.
     */
    size_t size() const;

    /**
     * @brief Checks if list contains any visits for specified patient.
     */
    bool hasHistory(const std::string& hospitalId, const std::string& patientId) const;

    /**
     * @brief Checks if the list has zero visits.
     */
    bool isEmpty() const;

    /**
     * @brief Removes a specific visit from the linked list.
     * @return true if found and removed, false otherwise.
     */
    bool removeVisit(const std::string& hospitalId,
                     const std::string& patientId,
                     const std::string& visitId);

    bool removeVisit(const std::string& visitId);

    /**
     * @brief Clears all visit nodes from the list and reclaims memory.
     */
    void clear();

    /**
     * @brief Retrieves all visits for a patient in a vector (useful for assertions and testing).
     */
    std::vector<Visit> getVisits(const std::string& hospitalId, const std::string& patientId) const;

    /**
     * @brief Retrieves all visits in the list in chronological order.
     */
    std::vector<Visit> getAllVisits() const;

private:
    std::string hospitalId_; ///< Optional scoped hospital ID
    std::string patientId_;  ///< Optional scoped patient ID
    VisitNode* head_;        ///< Head of singly linked list (oldest visit)
    VisitNode* tail_;        ///< Tail of singly linked list (newest visit)
    size_t size_;            ///< Total number of visit nodes in list
};

} // namespace smarter

