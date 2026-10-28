#pragma once

#include "smarter/PatientRegistry.h"
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <cstddef>

namespace smarter {

/**
 * @brief Node in the Binary Search Tree.
 * 
 * Each node stores the composite patient key (hospitalId, patientId).
 * Memory Ownership & Architecture Decision:
 * Rather than holding independent copies of Patient objects (which would risk
 * data desynchronization) or holding raw pointers that might dangle, each node
 * stores the immutable composite key. The BST optionally queries the canonical
 * PatientRegistry at display/retrieval time to obtain the current demographic details.
 */
struct BSTNode {
    std::string hospitalId;      ///< Hospital identifier (primary sorting key)
    std::string patientId;       ///< Patient identifier (secondary sorting key)
    BSTNode* left;               ///< Left child pointer (smaller composite keys)
    BSTNode* right;              ///< Right child pointer (greater composite keys)

    BSTNode(std::string hospId, std::string patId)
        : hospitalId(std::move(hospId)),
          patientId(std::move(patId)),
          left(nullptr),
          right(nullptr) {}
};

/**
 * @brief Custom Binary Search Tree for storing and displaying patient keys in sorted order.
 * 
 * Strict Project Constraints:
 *  - Standard ordered containers (std::map, std::set) are strictly PROHIBITED.
 *  - Implemented manually using pointers and dynamically allocated BSTNode instances.
 *  - Maintains standard BST ordering invariants based on composite key (hospitalId, patientId).
 * 
 * Complexity Summary:
 *  - Search:    O(log N) average, O(N) worst case (unbalanced tree)
 *  - Insertion: O(log N) average, O(N) worst case
 *  - Deletion:  O(log N) average, O(N) worst case
 *  - Inorder:   O(N) time, O(H) auxiliary call-stack space (H = tree height)
 *  - Space:     O(N) for tree nodes
 */
class PatientBST {
public:
    /**
     * @brief Constructs an empty BST, optionally linked to the canonical PatientRegistry.
     * @param registry Pointer to canonical PatientRegistry (optional, for enriched display).
     */
    explicit PatientBST(const PatientRegistry* registry = nullptr);

    /**
     * @brief Destructor: Recursively deallocates all BST nodes via post-order traversal.
     */
    ~PatientBST();

    // Rule of Five: Safe deep-copying and moving
    PatientBST(const PatientBST& other);
    PatientBST& operator=(const PatientBST& other);
    PatientBST(PatientBST&& other) noexcept;
    PatientBST& operator=(PatientBST&& other) noexcept;

    /**
     * @brief Inserts a patient composite key into the BST.
     * 
     * Comparison Rule:
     * 1. Compare hospitalId first.
     * 2. If hospitalIds match, compare patientId.
     * Rejects duplicate composite keys.
     * 
     * @param hospitalId Hospital identifier
     * @param patientId Patient identifier
     * @return true if inserted successfully; false if duplicate composite key.
     */
    bool insertPatient(const std::string& hospitalId, const std::string& patientId);

    /**
     * @brief Searches for a patient composite key in the BST.
     * @param hospitalId Hospital identifier
     * @param patientId Patient identifier
     * @return true if found, false otherwise.
     */
    bool searchPatient(const std::string& hospitalId, const std::string& patientId) const;

    /**
     * @brief Removes a composite key from the BST, handling all 3 deletion cases:
     *  Case 1: Leaf node (no children).
     *  Case 2: Node with 1 child.
     *  Case 3: Node with 2 children (replaced by in-order successor).
     * 
     * @param hospitalId Hospital identifier
     * @param patientId Patient identifier
     * @return true if key was found and removed; false if not found.
     */
    bool removePatient(const std::string& hospitalId, const std::string& patientId);

    /**
     * @brief Performs an in-order traversal (Left, Root, Right) to display patient records in ascending order.
     * If linked with a canonical PatientRegistry, displays full patient records; otherwise displays composite keys.
     * 
     * @param os Output stream (defaults to std::cout)
     */
    void displaySortedPatients(std::ostream& os = std::cout) const;

    /**
     * @brief Returns the total number of keys stored in the BST.
     */
    size_t size() const;

    /**
     * @brief Checks if the BST is empty.
     */
    bool isEmpty() const;

    /**
     * @brief Releases all nodes from the tree and resets size to 0.
     */
    void clear();

    /**
     * @brief Associates this BST with the canonical PatientRegistry for live demographic lookups.
     */
    void setRegistry(const PatientRegistry* registry);

    /**
     * @brief Gets the associated PatientRegistry pointer.
     */
    const PatientRegistry* getRegistry() const;

    /**
     * @brief Returns all composite keys in in-order sequence (useful for testing and verification).
     */
    std::vector<std::pair<std::string, std::string>> getInorderKeys() const;

    /**
     * @brief Static key comparison function:
     *  - Returns -1 if (h1, p1) < (h2, p2)
     *  - Returns  0 if (h1, p1) == (h2, p2)
     *  - Returns  1 if (h1, p1) > (h2, p2)
     */
    static int compareKeys(const std::string& h1, const std::string& p1,
                           const std::string& h2, const std::string& p2);

private:
    BSTNode* root_;                          ///< Root node pointer
    size_t size_;                            ///< Number of nodes in BST
    const PatientRegistry* registry_;        ///< Non-owning pointer to canonical PatientRegistry

    // Recursive helper functions
    BSTNode* insertHelper(BSTNode* node, const std::string& hospId, const std::string& patId, bool& success);
    BSTNode* searchHelper(BSTNode* node, const std::string& hospId, const std::string& patId) const;
    BSTNode* removeHelper(BSTNode* node, const std::string& hospId, const std::string& patId, bool& success);
    BSTNode* findMin(BSTNode* node) const;
    BSTNode* cloneTree(BSTNode* node);
    void destroyTree(BSTNode* node);
    void inorderTraversal(BSTNode* node, std::ostream& os, size_t& rank) const;
    void collectInorderKeys(BSTNode* node, std::vector<std::pair<std::string, std::string>>& out) const;
};

} // namespace smarter

