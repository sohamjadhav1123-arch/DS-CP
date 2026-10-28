#include "smarter/PatientRegistry.h"
#include "smarter/Patient.hpp"
#include <iostream>
#include <cassert>
#include <vector>

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

void testBasicInsertAndFind() {
    std::cout << "\n--- Test 1 & 2: Insert One Patient & Find Existing ---\n";
    PatientRegistry reg(11);
    ASSERT_TEST(reg.isEmpty(), "Registry starts empty");
    ASSERT_TEST(reg.size() == 0, "Initial size is 0");

    Patient p1("1001", "101", "Rahul Sharma", 28, "Male", "B+", "98765-01001");
    bool ins = reg.insertPatient(p1);
    ASSERT_TEST(ins, "Inserted Rahul (101, 1001) successfully");
    ASSERT_TEST(reg.size() == 1, "Registry size is 1");
    ASSERT_TEST(!reg.isEmpty(), "Registry is not empty");

    Patient* found = reg.findPatient("101", "1001");
    ASSERT_TEST(found != nullptr, "Found Rahul by composite key (101, 1001)");
    ASSERT_TEST(found && found->getName() == "Rahul Sharma", "Name matches 'Rahul Sharma'");
    ASSERT_TEST(found && found->getAge() == 28, "Age matches 28");
}

void testSearchMissingPatient() {
    std::cout << "\n--- Test 3: Search Missing Patient ---\n";
    PatientRegistry reg(11);
    Patient p1("1001", "101", "Rahul Sharma", 28, "Male", "B+", "98765-01001");
    reg.insertPatient(p1);

    Patient* missing1 = reg.findPatient("101", "9999");
    ASSERT_TEST(missing1 == nullptr, "Missing patientId returns nullptr safely");

    Patient* missing2 = reg.findPatient("999", "1001");
    ASSERT_TEST(missing2 == nullptr, "Missing hospitalId returns nullptr safely");

    ASSERT_TEST(!reg.containsPatient("101", "9999"), "containsPatient returns false for missing patient");
}

void testDuplicateCompositeKeyAndMultiHospital() {
    std::cout << "\n--- Test 4 & 5: Duplicate Composite Key & Multi-Hospital Coexistence ---\n";
    PatientRegistry reg(11);

    // Prompt fictional patients:
    // CityCare Hospital: ID 101 -> Rahul (1001), Priya (1002), Amit (1003)
    // GreenLife Hospital: ID 102 -> Neha (1001)
    Patient rahul("1001", "101", "Rahul Sharma", 28, "Male", "B+", "98765-01001");
    Patient priya("1002", "101", "Priya Patel", 34, "Female", "O+", "98765-01002");
    Patient amit ("1003", "101", "Amit Verma", 45, "Male", "A+", "98765-01003");
    Patient neha ("1001", "102", "Neha Gupta", 31, "Female", "AB+", "98765-01004");

    ASSERT_TEST(reg.insertPatient(rahul), "Inserted Rahul (101, 1001)");
    ASSERT_TEST(reg.insertPatient(priya), "Inserted Priya (101, 1002)");
    ASSERT_TEST(reg.insertPatient(amit),  "Inserted Amit (101, 1003)");
    ASSERT_TEST(reg.insertPatient(neha),  "Inserted Neha (102, 1001) with identical Patient ID 1001");

    ASSERT_TEST(reg.size() == 4, "Registry size is 4");

    // Duplicate insertion of (101, 1001) must be rejected:
    Patient duplicateRahul("1001", "101", "Duplicate Rahul", 30, "Male", "B+", "00000");
    bool dupResult = reg.insertPatient(duplicateRahul);
    ASSERT_TEST(!dupResult, "Duplicate composite key (101, 1001) properly rejected");
    ASSERT_TEST(reg.size() == 4, "Size remains 4 after rejected duplicate");

    // Verify both distinct patients with ID 1001 coexist and return correct records
    Patient* findRahul = reg.findPatient("101", "1001");
    Patient* findNeha  = reg.findPatient("102", "1001");

    ASSERT_TEST(findRahul && findRahul->getName() == "Rahul Sharma", "Search (101, 1001) returns Rahul");
    ASSERT_TEST(findNeha  && findNeha->getName() == "Neha Gupta",    "Search (102, 1001) returns Neha");
    ASSERT_TEST(findRahul != findNeha, "Rahul and Neha point to distinct memory addresses");
}

void testUpdatePatient() {
    std::cout << "\n--- Test 6: Update Existing Patient Details ---\n";
    PatientRegistry reg(11);
    Patient rahul("1001", "101", "Rahul Sharma", 28, "Male", "B+", "98765-01001");
    reg.insertPatient(rahul);

    bool updSuccess = reg.updatePatient("101", "1001", "Rahul Sharma Jr.", 29, "Male", "B-", "98765-99999");
    ASSERT_TEST(updSuccess, "updatePatient succeeded for existing patient");

    Patient* p = reg.findPatient("101", "1001");
    ASSERT_TEST(p && p->getName() == "Rahul Sharma Jr.", "Updated name is 'Rahul Sharma Jr.'");
    ASSERT_TEST(p && p->getAge() == 29, "Updated age is 29");
    ASSERT_TEST(p && p->getBloodGroup() == "B-", "Updated blood group is B-");
    ASSERT_TEST(p && p->getContactPhone() == "98765-99999", "Updated phone is 98765-99999");
    ASSERT_TEST(p && p->getHospitalId() == "101" && p->getPatientId() == "1001", "Composite key preserved");

    bool updMissing = reg.updatePatient("101", "9999", "Ghost", 50, "Male", "O+", "000");
    ASSERT_TEST(!updMissing, "Updating non-existent patient returns false");
}

void testDeletePatientBasic() {
    std::cout << "\n--- Test 7 & 8: Delete Patient & Delete Nonexistent ---\n";
    PatientRegistry reg(11);
    Patient p1("1001", "101", "Rahul Sharma", 28, "Male", "B+", "98765-01001");
    Patient p2("1002", "101", "Priya Patel", 34, "Female", "O+", "98765-01002");
    reg.insertPatient(p1);
    reg.insertPatient(p2);

    ASSERT_TEST(reg.size() == 2, "Size is 2 before deletion");

    bool delMissing = reg.deletePatient("101", "9999");
    ASSERT_TEST(!delMissing, "Deleting non-existent patient returns false");
    ASSERT_TEST(reg.size() == 2, "Size remains 2 after failed delete");

    bool delSuccess = reg.deletePatient("101", "1001");
    ASSERT_TEST(delSuccess, "Deleted Rahul (101, 1001)");
    ASSERT_TEST(reg.size() == 1, "Size is now 1");
    ASSERT_TEST(reg.findPatient("101", "1001") == nullptr, "Rahul is no longer found");
    ASSERT_TEST(reg.findPatient("101", "1002") != nullptr, "Priya is still found");
}

void testCollisionChainDeletions() {
    std::cout << "\n--- Test 9, 10, 11, 12: Collision Chain Deletions (First, Middle, Last) ---\n";
    // Force collisions into bucket count 3 with high load factor threshold
    PatientRegistry reg(3, 100.0f);

    // Insert 6 patients into 3 buckets; by pigeonhole principle, collisions will occur
    reg.insertPatient(Patient("P1", "101", "Patient 1", 20, "Other", "O+", "111"));
    reg.insertPatient(Patient("P2", "101", "Patient 2", 21, "Other", "O+", "222"));
    reg.insertPatient(Patient("P3", "101", "Patient 3", 22, "Other", "O+", "333"));
    reg.insertPatient(Patient("P4", "101", "Patient 4", 23, "Other", "O+", "444"));
    reg.insertPatient(Patient("P5", "101", "Patient 5", 24, "Other", "O+", "555"));
    reg.insertPatient(Patient("P6", "101", "Patient 6", 25, "Other", "O+", "666"));

    auto stats = reg.getStatistics();
    ASSERT_TEST(stats.totalCollisions > 0, "Forced collisions generated successfully");
    ASSERT_TEST(stats.maxChainLength >= 2, "At least one bucket has a collision chain");

    // All inserted patients must be searchable despite collisions
    bool allFound = true;
    for (int i = 1; i <= 6; ++i) {
        if (!reg.containsPatient("101", "P" + std::to_string(i))) {
            allFound = false;
        }
    }
    ASSERT_TEST(allFound, "All 6 patients searchable across collision chains");

    // Now construct a dedicated 3-node collision chain in a new registry with 1 bucket:
    PatientRegistry singleBucketReg(1, 100.0f);
    // Keys in this registry all hash to bucket 0!
    // Insertion prepends:
    // 1. insert Node C -> Bucket: [Node C]
    // 2. insert Node B -> Bucket: [Node B -> Node C]
    // 3. insert Node A -> Bucket: [Node A -> Node B -> Node C]
    singleBucketReg.insertPatient(Patient("Tail_C", "H1", "Tail C", 30, "M", "A+", "1"));
    singleBucketReg.insertPatient(Patient("Mid_B", "H1", "Mid B", 31, "M", "A+", "2"));
    singleBucketReg.insertPatient(Patient("Head_A", "H1", "Head A", 32, "M", "A+", "3"));

    ASSERT_TEST(singleBucketReg.size() == 3, "3 nodes in single collision chain");

    // Scenario 10: Delete Middle Node ("Mid_B")
    bool delMid = singleBucketReg.deletePatient("H1", "Mid_B");
    ASSERT_TEST(delMid, "Deleted middle node 'Mid_B'");
    ASSERT_TEST(singleBucketReg.size() == 2, "Size is now 2");
    ASSERT_TEST(singleBucketReg.findPatient("H1", "Mid_B") == nullptr, "Mid_B no longer in registry");
    ASSERT_TEST(singleBucketReg.findPatient("H1", "Head_A") != nullptr, "Head_A still intact");
    ASSERT_TEST(singleBucketReg.findPatient("H1", "Tail_C") != nullptr, "Tail_C still intact");

    // Scenario 9: Delete First Node / Head ("Head_A")
    bool delHead = singleBucketReg.deletePatient("H1", "Head_A");
    ASSERT_TEST(delHead, "Deleted first node (head) 'Head_A'");
    ASSERT_TEST(singleBucketReg.size() == 1, "Size is now 1");
    ASSERT_TEST(singleBucketReg.findPatient("H1", "Head_A") == nullptr, "Head_A no longer in registry");
    ASSERT_TEST(singleBucketReg.findPatient("H1", "Tail_C") != nullptr, "Tail_C still intact after head deletion");

    // Scenario 11: Delete Last Node / Tail ("Tail_C")
    bool delTail = singleBucketReg.deletePatient("H1", "Tail_C");
    ASSERT_TEST(delTail, "Deleted last node (tail) 'Tail_C'");
    ASSERT_TEST(singleBucketReg.isEmpty(), "Registry is now empty after deleting all chain nodes");
    ASSERT_TEST(singleBucketReg.size() == 0, "Size is 0");
    ASSERT_TEST(singleBucketReg.findPatient("H1", "Tail_C") == nullptr, "Tail_C no longer in registry");
}

void testResizingAndRehashing() {
    std::cout << "\n--- Test 13 & 14: Dynamic Resizing & Rehashing ---\n";
    // Initial buckets = 3, max load factor = 0.75
    // Threshold is 3 * 0.75 = 2.25. On inserting 3rd patient, load factor is 3/3 = 1.0 > 0.75 -> triggers resize!
    PatientRegistry reg(3, 0.75f);
    size_t initialCap = reg.getBucketCount();
    ASSERT_TEST(initialCap == 3, "Initial capacity is 3");

    // Insert 2 patients (within threshold)
    reg.insertPatient(Patient("P1", "101", "Patient 1", 20, "M", "O+", "111"));
    reg.insertPatient(Patient("P2", "101", "Patient 2", 21, "F", "A+", "222"));
    ASSERT_TEST(reg.getBucketCount() == 3, "Capacity remains 3 with 2 patients");

    // Capture memory pointer to Patient 1 before resize:
    Patient* p1Before = reg.findPatient("101", "P1");

    // Insert 3rd patient -> triggers resize!
    reg.insertPatient(Patient("P3", "101", "Patient 3", 22, "M", "B+", "333"));
    size_t newCap = reg.getBucketCount();
    ASSERT_TEST(newCap > initialCap, "Capacity resized (now " + std::to_string(newCap) + ")");
    ASSERT_TEST(reg.size() == 3, "Size is 3 after resizing");

    // Verify pointer stability across resize (nodes relinked, not reallocated):
    Patient* p1After = reg.findPatient("101", "P1");
    ASSERT_TEST(p1Before == p1After, "Patient* pointer remained stable across table resize");

    // Insert more patients to trigger another resize
    for (int i = 4; i <= 15; ++i) {
        reg.insertPatient(Patient("P" + std::to_string(i), "101", "Patient " + std::to_string(i),
                                  20 + i, "Other", "O+", "phone"));
    }
    ASSERT_TEST(reg.size() == 15, "Size is 15 after multiple insertions");
    ASSERT_TEST(reg.getBucketCount() >= 15, "Bucket count expanded proportionally");

    // Verify every single patient is accessible after resizing
    bool allAccessible = true;
    for (int i = 1; i <= 15; ++i) {
        if (!reg.containsPatient("101", "P" + std::to_string(i))) {
            allAccessible = false;
        }
    }
    ASSERT_TEST(allAccessible, "All 15 patients remain accessible after multiple resizings");
}

void testEmptyRegistryAndStatistics() {
    std::cout << "\n--- Test 15 & 16: Empty Registry Safety & Statistics ---\n";
    PatientRegistry reg(7, 0.75f);
    ASSERT_TEST(reg.isEmpty(), "Registry reports empty");
    ASSERT_TEST(reg.size() == 0, "Registry reports size 0");
    ASSERT_TEST(!reg.deletePatient("101", "P1"), "delete on empty returns false safely");
    ASSERT_TEST(reg.findPatient("101", "P1") == nullptr, "find on empty returns nullptr safely");

    auto statsEmpty = reg.getStatistics();
    ASSERT_TEST(statsEmpty.totalPatients == 0, "Stats total patients is 0");
    ASSERT_TEST(statsEmpty.totalBuckets == 7, "Stats total buckets is 7");
    ASSERT_TEST(statsEmpty.emptyBuckets == 7, "All 7 buckets are empty");
    ASSERT_TEST(statsEmpty.currentLoadFactor == 0.0f, "Load factor is 0.0");
    ASSERT_TEST(statsEmpty.totalCollisions == 0, "Zero collisions on empty");

    // Add patients and verify sensible stats
    reg.insertPatient(Patient("1001", "101", "Rahul", 28, "M", "B+", "1"));
    reg.insertPatient(Patient("1002", "101", "Priya", 34, "F", "O+", "2"));
    reg.insertPatient(Patient("1003", "101", "Amit",  45, "M", "A+", "3"));

    auto statsFilled = reg.getStatistics();
    ASSERT_TEST(statsFilled.totalPatients == 3, "Stats total patients is 3");
    ASSERT_TEST(statsFilled.currentLoadFactor > 0.0f && statsFilled.currentLoadFactor <= 0.75f,
                "Load factor is within valid threshold");
}

void testIsolationCrossHospitalDeletion() {
    std::cout << "\n--- Additional Test: Cross-Hospital Deletion Isolation ---\n";
    PatientRegistry reg(11);
    // Same patient ID 1001 in Hospital 101 (Rahul) and Hospital 102 (Neha)
    reg.insertPatient(Patient("1001", "101", "Rahul Sharma", 28, "M", "B+", "111"));
    reg.insertPatient(Patient("1001", "102", "Neha Gupta",   31, "F", "AB+", "222"));

    // Delete Rahul from Hospital 101
    bool delRahul = reg.deletePatient("101", "1001");
    ASSERT_TEST(delRahul, "Deleted Rahul (101, 1001)");

    // Neha at Hospital 102 must NOT be deleted!
    Patient* neha = reg.findPatient("102", "1001");
    ASSERT_TEST(neha != nullptr, "Neha (102, 1001) remains untouched after Rahul deleted");
    ASSERT_TEST(neha && neha->getName() == "Neha Gupta", "Neha's record is completely intact");
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "     SmartER Custom HashTable PatientRegistry Tests      \n";
    std::cout << "=========================================================\n";

    testBasicInsertAndFind();
    testSearchMissingPatient();
    testDuplicateCompositeKeyAndMultiHospital();
    testUpdatePatient();
    testDeletePatientBasic();
    testCollisionChainDeletions();
    testResizingAndRehashing();
    testEmptyRegistryAndStatistics();
    testIsolationCrossHospitalDeletion();

    std::cout << "\n=========================================================\n";
    std::cout << "Test Summary: " << testsPassed << " / " << testsTotal << " passed.\n";
    std::cout << "=========================================================\n";

    return (testsPassed == testsTotal) ? 0 : 1;
}

