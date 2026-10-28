#include "smarter/SmartERSystem.h"
#include <iostream>
#include <cassert>
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

void testHospitalAndPatientRegistration() {
    std::cout << "\n--- Test 1: Hospital and Patient Registration & Isolation ---\n";
    SmartERSystem system;

    // Register two hospitals
    ASSERT_TEST(system.registerHospital("101", "CityCare Hospital", "Metro", 50), "Registered CityCare (101)");
    ASSERT_TEST(system.registerHospital("102", "GreenLife Hospital", "Suburbs", 30), "Registered GreenLife (102)");
    ASSERT_TEST(!system.registerHospital("101", "Duplicate CityCare", "Metro", 50), "Duplicate hospital 101 rejected");
    ASSERT_TEST(system.getHospitalCount() == 2, "Hospital count is 2");

    // Register patients
    // Rahul: 101, 1001
    // Priya: 101, 1002
    // Amit:  101, 1003
    // Neha:  102, 1001 (Identical ID 1001 at Hospital 102)
    bool rRahul = system.registerPatient("101", "1001", "Rahul Sharma", 28, "Male", "B+", "98765-01001");
    bool rPriya = system.registerPatient("101", "1002", "Priya Patel", 34, "Female", "O+", "98765-01002");
    bool rAmit  = system.registerPatient("101", "1003", "Amit Verma", 45, "Male", "A+", "98765-01003");
    bool rNeha  = system.registerPatient("102", "1001", "Neha Gupta", 31, "Female", "AB+", "98765-01004");

    ASSERT_TEST(rRahul, "Registered Rahul (101, 1001)");
    ASSERT_TEST(rPriya, "Registered Priya (101, 1002)");
    ASSERT_TEST(rAmit,  "Registered Amit (101, 1003)");
    ASSERT_TEST(rNeha,  "Registered Neha (102, 1001) with identical ID 1001");

    // Duplicate patient registration in same hospital must fail
    bool rDupRahul = system.registerPatient("101", "1001", "Duplicate Rahul", 30, "Male", "B+", "000");
    ASSERT_TEST(!rDupRahul, "Duplicate registration (101, 1001) rejected");

    // Registration for non-existent hospital must fail
    bool rGhostHosp = system.registerPatient("999", "1001", "Ghost", 20, "Other", "O+", "000");
    ASSERT_TEST(!rGhostHosp, "Registration under non-existent hospital 999 rejected");
}

void testRegistryAndBSTConsistency() {
    std::cout << "\n--- Test 2: Registry and BST Dual-Index Consistency ---\n";
    SmartERSystem system;
    system.registerHospital("101", "CityCare Hospital", "Metro", 50);
    system.registerHospital("102", "GreenLife Hospital", "Suburbs", 30);

    system.registerPatient("101", "1003", "Amit Verma", 45, "Male", "A+", "98765-01003");
    system.registerPatient("101", "1001", "Rahul Sharma", 28, "Male", "B+", "98765-01001");
    system.registerPatient("101", "1002", "Priya Patel", 34, "Female", "O+", "98765-01002");
    system.registerPatient("102", "1001", "Neha Gupta", 31, "Female", "AB+", "98765-01004");

    // Check size in both structures
    ASSERT_TEST(system.getPatientRegistry().size() == 4, "Registry size is 4");
    ASSERT_TEST(system.getPatientBST().size() == 4, "BST size is 4");

    // Inorder keys in BST must be strictly sorted:
    // (101, 1001), (101, 1002), (101, 1003), (102, 1001)
    auto bstKeys = system.getPatientBST().getInorderKeys();
    ASSERT_TEST(bstKeys.size() == 4, "BST has 4 keys");
    ASSERT_TEST((bstKeys[0] == std::pair<std::string, std::string>{"101", "1001"}), "1st key is (101, 1001)");
    ASSERT_TEST((bstKeys[1] == std::pair<std::string, std::string>{"101", "1002"}), "2nd key is (101, 1002)");
    ASSERT_TEST((bstKeys[2] == std::pair<std::string, std::string>{"101", "1003"}), "3rd key is (101, 1003)");
    ASSERT_TEST((bstKeys[3] == std::pair<std::string, std::string>{"102", "1001"}), "4th key is (102, 1001)");

    // Update patient demographic in registry:
    system.updatePatient("101", "1001", "Rahul Sharma Jr.", 29, "Male", "B-", "111-222");

    // BST traversal output must dynamically reflect the updated demographic without duplication
    std::ostringstream oss;
    system.displaySortedPatients(oss);
    ASSERT_TEST(oss.str().find("Rahul Sharma Jr.") != std::string::npos, "BST reflected updated name");
    ASSERT_TEST(oss.str().find("B-") != std::string::npos, "BST reflected updated blood group");
}

void testEmergencyQueueOrderAndTieBreaking() {
    std::cout << "\n--- Test 3: Emergency Queue Priority & Tie-Breaking (CityCare Scenario) ---\n";
    SmartERSystem system;
    system.registerHospital("101", "CityCare Hospital", "Metro", 50);
    system.registerHospital("102", "GreenLife Hospital", "Suburbs", 30);

    system.registerPatient("101", "1001", "Rahul Sharma", 28, "Male", "B+", "111");
    system.registerPatient("101", "1002", "Priya Patel", 34, "Female", "O+", "222");
    system.registerPatient("101", "1003", "Amit Verma", 45, "Male", "A+", "333");
    system.registerPatient("102", "1001", "Neha Gupta", 31, "Female", "AB+", "444");

    // Admit patients to emergency room:
    // Rahul: Hosp 101, ID 1001, triage 2
    // Priya: Hosp 101, ID 1002, triage 1
    // Amit:  Hosp 101, ID 1003, triage 2
    // Neha:  Hosp 102, ID 1001, triage 3
    ASSERT_TEST(system.admitPatientToEmergency("101", "1001", 2, "Moderate asthma", "10:00", "VIS-R1"), "Admitted Rahul (Triage 2)");
    ASSERT_TEST(system.admitPatientToEmergency("101", "1002", 1, "Cardiac arrest", "10:05", "VIS-P1"), "Admitted Priya (Triage 1)");
    ASSERT_TEST(system.admitPatientToEmergency("101", "1003", 2, "Fracture", "10:10", "VIS-A1"), "Admitted Amit (Triage 2)");
    ASSERT_TEST(system.admitPatientToEmergency("102", "1001", 3, "Sprain", "10:15", "VIS-N1"), "Admitted Neha to GreenLife (Triage 3)");

    // Duplicate active queue entry must be rejected
    bool dupQueue = system.admitPatientToEmergency("101", "1001", 1, "Another complaint");
    ASSERT_TEST(!dupQueue, "Duplicate active queue entry for Rahul rejected");

    ASSERT_TEST(system.getEmergencyQueueSize("101") == 3, "CityCare queue size is 3");
    ASSERT_TEST(system.getEmergencyQueueSize("102") == 1, "GreenLife queue size is 1");

    // Serving CityCare queue in expected order:
    // 1. Priya (Triage 1)
    // 2. Rahul (Triage 2, earlier arrival)
    // 3. Amit  (Triage 2, later arrival)
    auto serve1 = system.serveNextEmergencyPatient("101");
    auto serve2 = system.serveNextEmergencyPatient("101");
    auto serve3 = system.serveNextEmergencyPatient("101");

    ASSERT_TEST(serve1 && serve1->patientId == "1002" && serve1->triageLevel == 1, "Served 1: Priya (1002, Triage 1)");
    ASSERT_TEST(serve2 && serve2->patientId == "1001" && serve2->triageLevel == 2, "Served 2: Rahul (1001, Triage 2)");
    ASSERT_TEST(serve3 && serve3->patientId == "1003" && serve3->triageLevel == 2, "Served 3: Amit (1003, Triage 2)");
    ASSERT_TEST(system.getEmergencyQueueSize("101") == 0, "CityCare queue is now empty");

    // Neha must still remain in GreenLife queue
    ASSERT_TEST(system.getEmergencyQueueSize("102") == 1, "GreenLife queue still contains Neha");
    auto nehaPeek = system.peekNextEmergencyPatient("102");
    ASSERT_TEST(nehaPeek && nehaPeek->patientId == "1001" && nehaPeek->hospitalId == "102", "Neha isolated in GreenLife");
}

void testServingUpdatesVisitStatus() {
    std::cout << "\n--- Test 4: Serving Patient Updates Visit Status to InTreatment ---\n";
    SmartERSystem system;
    system.registerHospital("101", "CityCare Hospital", "Metro", 50);
    system.registerPatient("101", "1001", "Rahul Sharma", 28, "Male", "B+", "111");

    system.admitPatientToEmergency("101", "1001", 2, "Chest discomfort", "09:00", "VIS-01");

    // Prior to serving, visit status is Waiting
    Visit* vBefore = system.findVisit("101", "1001", "VIS-01");
    ASSERT_TEST(vBefore && vBefore->getStatus() == Visit::Status::Waiting, "Visit status is Waiting prior to service");

    // Serve patient
    auto served = system.serveNextEmergencyPatient("101");
    ASSERT_TEST(served && served->patientId == "1001", "Served Rahul");

    // After serving, visit status is updated to InTreatment (NOT automatically marked Completed)
    Visit* vAfter = system.findVisit("101", "1001", "VIS-01");
    ASSERT_TEST(vAfter && vAfter->getStatus() == Visit::Status::InTreatment, "Visit status updated to InTreatment");
    ASSERT_TEST(vAfter && vAfter->getStatus() != Visit::Status::Completed, "Visit is NOT automatically completed");

    // Later physician completes visit:
    bool completed = system.updateVisitStatus("101", "1001", "VIS-01", Visit::Status::Completed);
    ASSERT_TEST(completed, "Physician marked visit Completed");
    ASSERT_TEST(system.findVisit("101", "1001", "VIS-01")->getStatus() == Visit::Status::Completed, "Status is Completed");
}

void testTriageUpdateAndHeapRestoration() {
    std::cout << "\n--- Test 5: Dynamic Triage Severity Update ---\n";
    SmartERSystem system;
    system.registerHospital("101", "CityCare Hospital", "Metro", 50);
    system.registerPatient("101", "1001", "Patient A", 30, "M", "O+", "111");
    system.registerPatient("101", "1002", "Patient B", 40, "F", "A+", "222");

    system.admitPatientToEmergency("101", "1001", 4, "Low acuity complaint");
    system.admitPatientToEmergency("101", "1002", 3, "Moderate complaint");

    // Initial top should be Patient B (Triage 3)
    ASSERT_TEST(system.peekNextEmergencyPatient("101")->patientId == "1002", "Initial top is Patient B (Triage 3)");

    // Patient A condition deteriorates: update triage from 4 to 1
    bool updated = system.updateEmergencyTriage("101", "1001", 1);
    ASSERT_TEST(updated, "Updated Patient A triage to 1");

    // Heap restored: Patient A is now top of the queue
    ASSERT_TEST(system.peekNextEmergencyPatient("101")->patientId == "1001", "Patient A is now top after priority increase");

    // Invalid triage level update rejected
    ASSERT_TEST(!system.updateEmergencyTriage("101", "1001", 8), "Invalid triage 8 rejected");
}

void testHospitalStatisticsAndEmptyHandling() {
    std::cout << "\n--- Test 6: Hospital Statistics & Safe Empty Handling ---\n";
    SmartERSystem system;
    system.registerHospital("101", "CityCare Hospital", "Metro", 50);

    // Empty checks
    ASSERT_TEST(system.getEmergencyQueueSize("101") == 0, "Queue size 0");
    ASSERT_TEST(!system.serveNextEmergencyPatient("101").has_value(), "Serve on empty queue returns nullopt");
    ASSERT_TEST(!system.peekNextEmergencyPatient("101").has_value(), "Peek on empty queue returns nullopt");
    ASSERT_TEST(system.findPatient("101", "999") == nullptr, "Find non-existent patient returns nullptr");

    // Add patients and visits
    system.registerPatient("101", "P1", "Pat 1", 20, "M", "O+", "1");
    system.registerPatient("101", "P2", "Pat 2", 30, "F", "A+", "2");
    system.admitPatientToEmergency("101", "P1", 2, "Complaint 1");
    system.admitPatientToEmergency("101", "P2", 3, "Complaint 2");
    system.serveNextEmergencyPatient("101"); // P1 served -> InTreatment

    HospitalStats stats = system.getHospitalStatistics("101");
    ASSERT_TEST(stats.registeredPatientCount == 2, "Stats registered patients is 2");
    ASSERT_TEST(stats.activeQueueSize == 1, "Stats active queue size is 1");
    ASSERT_TEST(stats.totalVisitsRecorded == 2, "Stats total visits is 2");
    ASSERT_TEST(stats.waitingVisits == 1, "Stats waiting visits is 1");
    ASSERT_TEST(stats.inTreatmentVisits == 1, "Stats inTreatment visits is 1");
    ASSERT_TEST(stats.totalEmergencyBeds == 50, "Stats bed capacity is 50");
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "      SmartER Stage 6 System Integration Verification    \n";
    std::cout << "=========================================================\n";

    testHospitalAndPatientRegistration();
    testRegistryAndBSTConsistency();
    testEmergencyQueueOrderAndTieBreaking();
    testServingUpdatesVisitStatus();
    testTriageUpdateAndHeapRestoration();
    testHospitalStatisticsAndEmptyHandling();

    std::cout << "\n=========================================================\n";
    std::cout << "Test Summary: " << testsPassed << " / " << testsTotal << " passed.\n";
    std::cout << "=========================================================\n";

    return (testsPassed == testsTotal) ? 0 : 1;
}

