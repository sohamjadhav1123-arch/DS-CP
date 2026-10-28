#include "smarter/PatientBST.h"
#include "smarter/PatientRegistry.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <sstream>

using namespace smarter;

static int testsPassed = 0;
static int testsTotal = 0;

#define ASSERT_TEST(condition, testName) \
    do { \
        ++testsTotal; \
        if (condition) { \
            std::cout << "  [PASS] " << testName << "\n"; \
            ++testsPassed; \
        } else { \
            std::cerr << "  [FAIL] " << testName << " (Line " << __LINE__ << ")\n"; \
        } \
    } while (0)

void testInsertAndSearchSingle() {
    std::cout << "\n--- Test 1, 4, 5: Insert One Key & Search Existing/Missing ---\n";
    PatientBST bst;
    ASSERT_TEST(bst.isEmpty(), "BST starts empty");
    ASSERT_TEST(bst.size() == 0, "Initial size is 0");

    bool inserted = bst.insertPatient("101", "1001");
    ASSERT_TEST(inserted, "Inserted (101, 1001)");
    ASSERT_TEST(!bst.isEmpty(), "BST is not empty after insert");
    ASSERT_TEST(bst.size() == 1, "BST size is 1");

    ASSERT_TEST(bst.searchPatient("101", "1001"), "searchPatient finds (101, 1001)");
    ASSERT_TEST(!bst.searchPatient("101", "9999"), "searchPatient returns false for missing patientId");
    ASSERT_TEST(!bst.searchPatient("999", "1001"), "searchPatient returns false for missing hospitalId");
}

void testRequiredCompositeKeysAndInorder() {
    std::cout << "\n--- Test 2, 3, 6, 12, 13: Unsorted Inserts, Duplicates, and Inorder Sort ---\n";
    PatientBST bst;

    // Prompt 4 required fictional composite keys inserted in UNSORTED order:
    // (101, 1003)
    // (101, 1001)
    // (101, 1002)
    // (102, 1001)
    // (102, 1003)
    ASSERT_TEST(bst.insertPatient("101", "1003"), "Inserted (101, 1003)");
    ASSERT_TEST(bst.insertPatient("101", "1001"), "Inserted (101, 1001)");
    ASSERT_TEST(bst.insertPatient("101", "1002"), "Inserted (101, 1002)");
    ASSERT_TEST(bst.insertPatient("102", "1001"), "Inserted (102, 1001) - Identical Patient ID across hospitals");
    ASSERT_TEST(bst.insertPatient("102", "1003"), "Inserted (102, 1003)");

    ASSERT_TEST(bst.size() == 5, "Size is 5 after inserting all 5 keys");

    // Test 3: Reject duplicate composite key
    bool dupResult = bst.insertPatient("101", "1001");
    ASSERT_TEST(!dupResult, "Duplicate key (101, 1001) properly rejected");
    ASSERT_TEST(bst.size() == 5, "Size remains 5 after duplicate rejection");

    // Test 6 & 13: Inorder traversal must return ascending order:
    // Expected:
    // (101, 1001)
    // (101, 1002)
    // (101, 1003)
    // (102, 1001)
    // (102, 1003)
    auto keys = bst.getInorderKeys();
    ASSERT_TEST(keys.size() == 5, "Inorder traversal returned 5 keys");

    std::vector<std::pair<std::string, std::string>> expected = {
        {"101", "1001"},
        {"101", "1002"},
        {"101", "1003"},
        {"102", "1001"},
        {"102", "1003"}
    };

    bool orderCorrect = (keys == expected);
    ASSERT_TEST(orderCorrect, "Inorder traversal strictly matches ascending composite key order");
}

void testDeletionCases() {
    std::cout << "\n--- Test 7, 8, 9, 10, 11, 14: BST Deletion Cases (Leaf, 1-Child, 2-Children, Root) ---\n";
    // Construct a predictable tree structure:
    // Insert order:
    // Root: (101, 50)
    // Left: (101, 20), Right: (101, 80)
    // (101, 10), (101, 30), (101, 70), (101, 90)
    // (101, 25) -> child of 30 (making 30 have 1 child)
    PatientBST bst;
    bst.insertPatient("101", "50");
    bst.insertPatient("101", "20");
    bst.insertPatient("101", "80");
    bst.insertPatient("101", "10");
    bst.insertPatient("101", "30");
    bst.insertPatient("101", "70");
    bst.insertPatient("101", "90");
    bst.insertPatient("101", "25"); // 30 has 1 child (25)

    ASSERT_TEST(bst.size() == 8, "Tree initialized with 8 nodes");

    // Test 11: Delete nonexistent key
    bool delMissing = bst.removePatient("101", "999");
    ASSERT_TEST(!delMissing, "Deleting nonexistent key returns false");
    ASSERT_TEST(bst.size() == 8, "Size remains 8 after failed deletion");

    // Test 7: Delete a leaf node: (101, 10)
    bool delLeaf = bst.removePatient("101", "10");
    ASSERT_TEST(delLeaf, "Case 1: Leaf node (101, 10) deleted successfully");
    ASSERT_TEST(bst.size() == 7, "Size decremented to 7");
    ASSERT_TEST(!bst.searchPatient("101", "10"), "(101, 10) is no longer in tree");

    // Test 8: Delete node with 1 child: (101, 30) has single child (101, 25)
    bool delOneChild = bst.removePatient("101", "30");
    ASSERT_TEST(delOneChild, "Case 2: Single-child node (101, 30) deleted successfully");
    ASSERT_TEST(bst.size() == 6, "Size decremented to 6");
    ASSERT_TEST(!bst.searchPatient("101", "30"), "(101, 30) is no longer in tree");
    ASSERT_TEST(bst.searchPatient("101", "25"), "Child (101, 25) adopted properly");

    // Test 9: Delete node with 2 children: (101, 80) has children (101, 70) and (101, 90)
    bool delTwoChildren = bst.removePatient("101", "80");
    ASSERT_TEST(delTwoChildren, "Case 3: Two-child node (101, 80) deleted successfully");
    ASSERT_TEST(bst.size() == 5, "Size decremented to 5");
    ASSERT_TEST(!bst.searchPatient("101", "80"), "(101, 80) is no longer in tree");
    ASSERT_TEST(bst.searchPatient("101", "70"), "Child (101, 70) still intact");
    ASSERT_TEST(bst.searchPatient("101", "90"), "Child (101, 90) still intact");

    // Verify ordering property preserved after 3 deletions
    auto keysAfterDeletions = bst.getInorderKeys();
    bool isSorted = true;
    for (size_t i = 1; i < keysAfterDeletions.size(); ++i) {
        if (PatientBST::compareKeys(keysAfterDeletions[i-1].first, keysAfterDeletions[i-1].second,
                                    keysAfterDeletions[i].first, keysAfterDeletions[i].second) >= 0) {
            isSorted = false;
        }
    }
    ASSERT_TEST(isSorted, "In-order ordering property strictly preserved after deletions");

    // Test 10: Delete root node: (101, 50)
    bool delRoot = bst.removePatient("101", "50");
    ASSERT_TEST(delRoot, "Root node (101, 50) deleted successfully");
    ASSERT_TEST(bst.size() == 4, "Size decremented to 4");
    ASSERT_TEST(!bst.searchPatient("101", "50"), "Old root no longer in tree");
}

void testClearAndEmptyTree() {
    std::cout << "\n--- Test 15 & 16: Clear Tree & Empty Tree Safety ---\n";
    PatientBST bst;
    ASSERT_TEST(bst.isEmpty(), "Fresh BST is empty");
    ASSERT_TEST(!bst.removePatient("101", "P1"), "remove on empty BST returns false safely");
    ASSERT_TEST(!bst.searchPatient("101", "P1"), "search on empty BST returns false safely");

    std::ostringstream oss;
    bst.displaySortedPatients(oss);
    ASSERT_TEST(oss.str().find("empty") != std::string::npos, "displaySortedPatients on empty BST handled safely");

    // Insert and clear
    bst.insertPatient("101", "P1");
    bst.insertPatient("101", "P2");
    ASSERT_TEST(bst.size() == 2, "Size is 2 before clear");

    bst.clear();
    ASSERT_TEST(bst.isEmpty(), "BST is empty after clear()");
    ASSERT_TEST(bst.size() == 0, "BST size is 0 after clear()");
    ASSERT_TEST(!bst.searchPatient("101", "P1"), "Cleared nodes cannot be found");
}

void testMemoryAndRuleOfFive() {
    std::cout << "\n--- Test 17: Deep Copy, Move, and Ownership Safety ---\n";
    PatientBST original;
    original.insertPatient("101", "1001");
    original.insertPatient("101", "1002");
    original.insertPatient("102", "1001");

    // Deep copy constructor
    PatientBST copy(original);
    ASSERT_TEST(copy.size() == 3, "Copy constructor created tree with size 3");
    ASSERT_TEST(copy.searchPatient("101", "1001"), "Copy contains (101, 1001)");

    // Mutating original must not affect copy
    original.removePatient("101", "1001");
    ASSERT_TEST(original.size() == 2, "Original size is 2 after deletion");
    ASSERT_TEST(copy.size() == 3, "Copy size remains 3 (deep copy verified)");
    ASSERT_TEST(copy.searchPatient("101", "1001"), "Copy still retains (101, 1001)");

    // Copy assignment
    PatientBST assigned;
    assigned = copy;
    ASSERT_TEST(assigned.size() == 3, "Copy assignment created tree with size 3");
    ASSERT_TEST(assigned.searchPatient("102", "1001"), "Assigned tree contains (102, 1001)");

    // Move constructor
    PatientBST moved(std::move(assigned));
    ASSERT_TEST(moved.size() == 3, "Moved tree took ownership of 3 nodes");
    ASSERT_TEST(assigned.isEmpty(), "Moved-from tree is safely reset to empty");
}

void testRegistryIntegration() {
    std::cout << "\n--- Canonical PatientRegistry Integration Verification ---\n";
    PatientRegistry registry(7);

    // Register patients
    registry.insertPatient(Patient("1001", "101", "Rahul Sharma", 28, "Male", "B+", "98765-01001"));
    registry.insertPatient(Patient("1002", "101", "Priya Patel", 34, "Female", "O+", "98765-01002"));
    registry.insertPatient(Patient("1003", "101", "Amit Verma", 45, "Male", "A+", "98765-01003"));
    registry.insertPatient(Patient("1001", "102", "Neha Gupta", 31, "Female", "AB+", "98765-01004"));

    // Populate BST with corresponding keys
    PatientBST bst(&registry);
    bst.insertPatient("101", "1003");
    bst.insertPatient("101", "1001");
    bst.insertPatient("101", "1002");
    bst.insertPatient("102", "1001");

    ASSERT_TEST(bst.size() == 4, "BST populated with 4 index keys");

    // Update Rahul in PatientRegistry
    registry.updatePatient("101", "1001", "Rahul Sharma Jr.", 29, "Male", "B-", "99999-00000");

    // Verify BST dynamically reflects the updated canonical data without stale duplication
    std::ostringstream oss;
    bst.displaySortedPatients(oss);
    std::string output = oss.str();

    ASSERT_TEST(output.find("Rahul Sharma Jr.") != std::string::npos,
                "BST display retrieved updated name 'Rahul Sharma Jr.' from canonical registry");
    ASSERT_TEST(output.find("B-") != std::string::npos,
                "BST display retrieved updated blood group 'B-' from canonical registry");
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "          SmartER Custom PatientBST Verification         \n";
    std::cout << "=========================================================\n";

    testInsertAndSearchSingle();
    testRequiredCompositeKeysAndInorder();
    testDeletionCases();
    testClearAndEmptyTree();
    testMemoryAndRuleOfFive();
    testRegistryIntegration();

    std::cout << "\n=========================================================\n";
    std::cout << "Test Summary: " << testsPassed << " / " << testsTotal << " passed.\n";
    std::cout << "=========================================================\n";

    return (testsPassed == testsTotal) ? 0 : 1;
}

