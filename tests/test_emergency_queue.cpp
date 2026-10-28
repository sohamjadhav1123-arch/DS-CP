#include "smarter/EmergencyQueue.h"
#include "smarter/Hospital.hpp"
#include "smarter/Patient.hpp"
#include <iostream>
#include <cassert>

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

void testBasicInsertionAndRemoval() {
    std::cout << "\n--- Test 1: Basic Insertion and Removal ---\n";
    EmergencyQueue q("101");
    ASSERT_TEST(q.isEmpty(), "Queue starts empty");
    ASSERT_TEST(q.size() == 0, "Initial size is 0");

    bool inserted = q.insertPatient("101", "P01", 3, 10);
    ASSERT_TEST(inserted, "Successfully inserted P01");
    ASSERT_TEST(!q.isEmpty(), "Queue is not empty after insert");
    ASSERT_TEST(q.size() == 1, "Size is 1 after insert");

    auto peeked = q.peekNextPatient();
    ASSERT_TEST(peeked.has_value(), "Peek returned a value");
    ASSERT_TEST(peeked->patientId == "P01", "Peek matches inserted patient");
    ASSERT_TEST(q.size() == 1, "Peek did not remove patient");

    auto served = q.serveNextPatient();
    ASSERT_TEST(served.has_value(), "Serve returned a value");
    ASSERT_TEST(served->patientId == "P01", "Served matches inserted patient");
    ASSERT_TEST(q.isEmpty(), "Queue is empty after serving only patient");
    ASSERT_TEST(q.size() == 0, "Size is 0 after serving");
}

void testCorrectPriorityOrdering() {
    std::cout << "\n--- Test 2: Correct Priority Ordering (Min-Heap) ---\n";
    EmergencyQueue q("101");
    q.insertPatient("101", "P_Low", 4, 1);
    q.insertPatient("101", "P_High", 1, 2);
    q.insertPatient("101", "P_Mid", 3, 3);

    auto s1 = q.serveNextPatient();
    auto s2 = q.serveNextPatient();
    auto s3 = q.serveNextPatient();

    ASSERT_TEST(s1 && s1->patientId == "P_High" && s1->triageLevel == 1, "First served is Triage Level 1");
    ASSERT_TEST(s2 && s2->patientId == "P_Mid" && s2->triageLevel == 3, "Second served is Triage Level 3");
    ASSERT_TEST(s3 && s3->patientId == "P_Low" && s3->triageLevel == 4, "Third served is Triage Level 4");
}

void testArrivalOrderTieBreaking() {
    std::cout << "\n--- Test 3: Arrival-Order Tie-Breaking ---\n";
    EmergencyQueue q("101");
    // Same triage level (2), different arrival sequences (5 then 2 then 8)
    q.insertPatient("101", "SecondArrival", 2, 5);
    q.insertPatient("101", "FirstArrival", 2, 2);
    q.insertPatient("101", "ThirdArrival", 2, 8);

    auto s1 = q.serveNextPatient();
    auto s2 = q.serveNextPatient();
    auto s3 = q.serveNextPatient();

    ASSERT_TEST(s1 && s1->patientId == "FirstArrival" && s1->arrivalSequence == 2, "Tie-break served arrival 2 first");
    ASSERT_TEST(s2 && s2->patientId == "SecondArrival" && s2->arrivalSequence == 5, "Tie-break served arrival 5 second");
    ASSERT_TEST(s3 && s3->patientId == "ThirdArrival" && s3->arrivalSequence == 8, "Tie-break served arrival 8 third");
}

void testEmptyQueueHandling() {
    std::cout << "\n--- Test 4: Empty Queue Handling ---\n";
    EmergencyQueue q("101");
    auto peek = q.peekNextPatient();
    ASSERT_TEST(!peek.has_value(), "Peek on empty queue returns nullopt safely");

    auto serve = q.serveNextPatient();
    ASSERT_TEST(!serve.has_value(), "Serve on empty queue returns nullopt safely");
    ASSERT_TEST(q.isEmpty(), "Queue remains empty");
}

void testInvalidTriageLevels() {
    std::cout << "\n--- Test 5: Invalid Triage Levels Validation ---\n";
    EmergencyQueue q("101");
    bool r0 = q.insertPatient("101", "Bad0", 0, 1);
    bool r6 = q.insertPatient("101", "Bad6", 6, 2);
    bool rNeg = q.insertPatient("101", "BadNeg", -1, 3);

    ASSERT_TEST(!r0, "Rejected triage level 0");
    ASSERT_TEST(!r6, "Rejected triage level 6");
    ASSERT_TEST(!rNeg, "Rejected negative triage level");
    ASSERT_TEST(q.isEmpty(), "Queue remains empty after invalid inserts");
}

void testDuplicatePrevention() {
    std::cout << "\n--- Test 6: Duplicate Queue-Entry Prevention ---\n";
    EmergencyQueue q("101");
    bool first = q.insertPatient("101", "P1001", 2, 1);
    ASSERT_TEST(first, "First insertion of P1001 accepted");

    bool dup = q.insertPatient("101", "P1001", 1, 2);
    ASSERT_TEST(!dup, "Duplicate insertion of P1001 rejected");
    ASSERT_TEST(q.size() == 1, "Queue size remains 1");

    auto peek = q.peekNextPatient();
    ASSERT_TEST(peek && peek->triageLevel == 2, "Original entry triage level intact (not overwritten)");
}

void testPriorityUpdates() {
    std::cout << "\n--- Test 7: Priority Updates & Heap Restoration ---\n";
    EmergencyQueue q("101");
    q.insertPatient("101", "P_A", 4, 1);
    q.insertPatient("101", "P_B", 3, 2);
    q.insertPatient("101", "P_C", 5, 3);

    // Initial top should be P_B (triage 3)
    ASSERT_TEST(q.peekNextPatient()->patientId == "P_B", "Initial top is P_B (triage 3)");

    // Invalid update
    bool badUpdate = q.updateTriageLevel("P_C", 9);
    ASSERT_TEST(!badUpdate, "Rejected update with invalid triage level 9");

    // Non-existent patient update
    bool ghostUpdate = q.updateTriageLevel("P_Ghost", 1);
    ASSERT_TEST(!ghostUpdate, "Rejected update for non-existent patient");

    // Upgrade P_C from triage 5 to triage 1 (HeapifyUp)
    bool upgraded = q.updateTriageLevel("P_C", 1);
    ASSERT_TEST(upgraded, "P_C upgraded to triage 1");
    ASSERT_TEST(q.peekNextPatient()->patientId == "P_C", "P_C is now the root of the heap");

    // Downgrade P_C from triage 1 to triage 5 (HeapifyDown)
    bool downgraded = q.updateTriageLevel("P_C", 5);
    ASSERT_TEST(downgraded, "P_C downgraded to triage 5");
    ASSERT_TEST(q.peekNextPatient()->patientId == "P_B", "P_B restored to root of the heap");
}

void testHospitalIsolationAndRequiredScenario() {
    std::cout << "\n--- Test 8: Hospital Isolation & Required Prompt Scenario ---\n";
    
    // Create CityCare (101) and GreenLife (102)
    Hospital cityCare("101", "CityCare Hospital", "Metro", 50);
    Hospital greenLife("102", "GreenLife Hospital", "Suburbs", 30);

    EmergencyQueue& qCity = cityCare.getEmergencyQueue();
    EmergencyQueue& qGreen = greenLife.getEmergencyQueue();

    // Verification of isolation guard: Cross-hospital insert must fail
    bool crossHospitalFail = qCity.insertPatient("102", "CrossPatient", 2, 99);
    ASSERT_TEST(!crossHospitalFail, "Hospital isolation rejected patient from 102 into 101's queue");

    // Prompt scenario patients:
    // Rahul: Patient ID 1001, Hospital ID 101, triage level 2, arrival sequence 1
    // Priya: Patient ID 1002, Hospital ID 101, triage level 1, arrival sequence 2
    // Amit:  Patient ID 1003, Hospital ID 101, triage level 2, arrival sequence 3
    // Neha:  Patient ID 1001, Hospital ID 102, triage level 3, arrival sequence 4
    bool insRahul = qCity.insertPatient("101", "1001", 2, 1);
    bool insPriya = qCity.insertPatient("101", "1002", 1, 2);
    bool insAmit  = qCity.insertPatient("101", "1003", 2, 3);
    bool insNeha  = qGreen.insertPatient("102", "1001", 3, 4);

    ASSERT_TEST(insRahul, "Rahul (1001, Hosp 101) inserted into CityCare");
    ASSERT_TEST(insPriya, "Priya (1002, Hosp 101) inserted into CityCare");
    ASSERT_TEST(insAmit,  "Amit (1003, Hosp 101) inserted into CityCare");
    ASSERT_TEST(insNeha,  "Neha (1001, Hosp 102) inserted into GreenLife with identical Patient ID 1001");

    ASSERT_TEST(qCity.size() == 3, "CityCare queue has 3 patients");
    ASSERT_TEST(qGreen.size() == 1, "GreenLife queue has 1 patient (Neha)");

    // Expected serving order for CityCare:
    // 1. Priya (Triage 1, Arrival 2)
    // 2. Rahul (Triage 2, Arrival 1)
    // 3. Amit  (Triage 2, Arrival 3)
    auto serve1 = qCity.serveNextPatient();
    auto serve2 = qCity.serveNextPatient();
    auto serve3 = qCity.serveNextPatient();

    ASSERT_TEST(serve1 && serve1->patientId == "1002", "Serving Order 1: Priya (1002) - Triage Level 1");
    ASSERT_TEST(serve2 && serve2->patientId == "1001", "Serving Order 2: Rahul (1001) - Triage Level 2, Arrival 1");
    ASSERT_TEST(serve3 && serve3->patientId == "1003", "Serving Order 3: Amit (1003) - Triage Level 2, Arrival 3");
    ASSERT_TEST(qCity.isEmpty(), "CityCare queue is now empty");

    // Neha must remain in GreenLife's separate queue:
    ASSERT_TEST(!qGreen.isEmpty(), "GreenLife queue still contains Neha");
    ASSERT_TEST(qGreen.size() == 1, "GreenLife queue size is 1");
    auto greenServe = qGreen.peekNextPatient();
    ASSERT_TEST(greenServe && greenServe->patientId == "1001" && greenServe->hospitalId == "102",
                "Neha remains safely isolated in GreenLife's queue");
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "      SmartER Min-Heap EmergencyQueue Verification       \n";
    std::cout << "=========================================================\n";

    testBasicInsertionAndRemoval();
    testCorrectPriorityOrdering();
    testArrivalOrderTieBreaking();
    testEmptyQueueHandling();
    testInvalidTriageLevels();
    testDuplicatePrevention();
    testPriorityUpdates();
    testHospitalIsolationAndRequiredScenario();

    std::cout << "\n=========================================================\n";
    std::cout << "Test Summary: " << testsPassed << " / " << testsTotal << " passed.\n";
    std::cout << "=========================================================\n";

    return (testsPassed == testsTotal) ? 0 : 1;
}

