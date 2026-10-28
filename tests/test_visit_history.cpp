#include "smarter/VisitHistory.h"
#include "smarter/Visit.hpp"
#include "smarter/Triage.hpp"
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

void testAddAndRetrieveSingleVisit() {
    std::cout << "\n--- Test 1: Add Single Visit & Retrieve ---\n";
    VisitHistory history("101", "1001");
    ASSERT_TEST(history.isEmpty(), "History starts empty");
    ASSERT_TEST(history.size() == 0, "Initial size is 0");
    ASSERT_TEST(!history.hasHistory("101", "1001"), "hasHistory returns false initially");

    Visit v1("VIS-101", "1001", "101", "2026-10-23 08:30",
             TriageLevel::Emergent, "Acute chest pain", "Evaluation in progress",
             Visit::Status::InTreatment, "ECG ordered");

    bool added = history.addVisit(v1);
    ASSERT_TEST(added, "Added single visit successfully");
    ASSERT_TEST(!history.isEmpty(), "History is not empty after add");
    ASSERT_TEST(history.size() == 1, "Size is 1");
    ASSERT_TEST(history.hasHistory("101", "1001"), "hasHistory returns true");

    Visit* found = history.findVisit("VIS-101");
    ASSERT_TEST(found != nullptr, "Found visit by ID");
    ASSERT_TEST(found && found->getChiefComplaint() == "Acute chest pain", "Complaint matches");
    ASSERT_TEST(found && found->getStatus() == Visit::Status::InTreatment, "Status matches InTreatment");
}

void testMultipleVisitsAndChronologicalOrder() {
    std::cout << "\n--- Test 2 & 3: Multiple Visits & Chronological FIFO Order ---\n";
    VisitHistory history("101", "1001");

    Visit v1("VIS-101", "1001", "101", "2026-01-10 10:00",
             TriageLevel::Urgent, "Fractured arm", "Cast applied",
             Visit::Status::Discharged, "Healed in 6 weeks");
    Visit v2("VIS-102", "1001", "101", "2026-05-15 14:00",
             TriageLevel::LessUrgent, "Follow-up consultation", "Cast removed",
             Visit::Status::Completed, "Normal range of motion");
    Visit v3("VIS-103", "1001", "101", "2026-10-23 09:30",
             TriageLevel::Emergent, "Chest palpitations", "Monitored",
             Visit::Status::Waiting, "Telemetry ordered");

    history.addVisit(v1);
    history.addVisit(v2);
    history.addVisit(v3);

    ASSERT_TEST(history.size() == 3, "All 3 visits added");

    // Chronological order verification (oldest v1 first, newest v3 last)
    std::vector<Visit> all = history.getAllVisits();
    ASSERT_TEST(all.size() == 3, "Retrieved 3 visits");
    ASSERT_TEST(all[0].getVisitId() == "VIS-101", "First visit is oldest (VIS-101)");
    ASSERT_TEST(all[1].getVisitId() == "VIS-102", "Second visit is middle (VIS-102)");
    ASSERT_TEST(all[2].getVisitId() == "VIS-103", "Third visit is newest (VIS-103)");
}

void testDuplicateVisitRejection() {
    std::cout << "\n--- Test 4: Duplicate Visit ID Rejection ---\n";
    VisitHistory history("101", "1001");

    Visit v1("VIS-101", "1001", "101", "2026-10-23 08:00",
             TriageLevel::Urgent, "Migraine", "Analgesics", Visit::Status::InTreatment);
    Visit v1_dup("VIS-101", "1001", "101", "2026-10-23 08:30",
                 TriageLevel::NonUrgent, "Duplicate ID visit", "None", Visit::Status::Waiting);

    ASSERT_TEST(history.addVisit(v1), "First visit added");
    bool dupAdded = history.addVisit(v1_dup);
    ASSERT_TEST(!dupAdded, "Duplicate visit ID rejected");
    ASSERT_TEST(history.size() == 1, "Size remains 1");
}

void testMultiPatientAndMultiHospitalIsolation() {
    std::cout << "\n--- Test 5 & 6: Cross-Patient and Cross-Hospital Isolation ---\n";
    // General VisitHistory holding visits across different patients and hospitals
    VisitHistory registryHistory;

    // Patient Rahul: (101, 1001)
    Visit r1("VIS-R1", "1001", "101", "2026-10-23 08:00", TriageLevel::Emergent, "Chest pain");
    Visit r2("VIS-R2", "1001", "101", "2026-10-23 11:00", TriageLevel::Urgent, "Abdominal pain");

    // Patient Priya: (101, 1002)
    Visit p1("VIS-P1", "1002", "101", "2026-10-23 09:00", TriageLevel::Immediate, "Cardiac arrest");

    // Patient Neha: (102, 1001) - Same patient ID 1001, but at Hospital 102!
    Visit n1("VIS-N1", "1001", "102", "2026-10-23 09:30", TriageLevel::Urgent, "Sprained ankle");

    registryHistory.addVisit(r1);
    registryHistory.addVisit(r2);
    registryHistory.addVisit(p1);
    registryHistory.addVisit(n1);

    ASSERT_TEST(registryHistory.size() == 4, "Total 4 visits across network");

    // Rahul (101, 1001) must have exactly 2 visits:
    ASSERT_TEST(registryHistory.getVisitCount("101", "1001") == 2, "Rahul (101, 1001) has 2 visits");

    // Priya (101, 1002) must have exactly 1 visit:
    ASSERT_TEST(registryHistory.getVisitCount("101", "1002") == 1, "Priya (101, 1002) has 1 visit");

    // Neha (102, 1001) at Hospital 102 must have exactly 1 visit (completely distinct from Rahul):
    ASSERT_TEST(registryHistory.getVisitCount("102", "1001") == 1, "Neha (102, 1001) has 1 visit");

    // Verify Rahul's history does not contain Neha's visit
    auto rahulVisits = registryHistory.getVisits("101", "1001");
    bool containsNeha = false;
    for (const auto& v : rahulVisits) {
        if (v.getVisitId() == "VIS-N1") containsNeha = true;
    }
    ASSERT_TEST(!containsNeha, "Rahul's history is strictly isolated from Neha's");

    // Verify searching for Neha's visit requires hospital 102
    ASSERT_TEST(registryHistory.findVisit("102", "1001", "VIS-N1") != nullptr, "Found Neha's visit with (102, 1001)");
    ASSERT_TEST(registryHistory.findVisit("101", "1001", "VIS-N1") == nullptr, "Cannot find Neha's visit under Hospital 101");
}

void testUpdateVisitStatus() {
    std::cout << "\n--- Test 7: Update Visit Status ---\n";
    VisitHistory history("101", "1001");
    Visit v1("VIS-101", "1001", "101", "2026-10-23 08:30",
             TriageLevel::Emergent, "Laceration", "Stitches needed", Visit::Status::Waiting);
    history.addVisit(v1);

    ASSERT_TEST(history.findVisit("VIS-101")->getStatus() == Visit::Status::Waiting, "Initial status is Waiting");

    bool upd1 = history.updateVisitStatus("VIS-101", Visit::Status::InTreatment);
    ASSERT_TEST(upd1, "Updated to InTreatment");
    ASSERT_TEST(history.findVisit("VIS-101")->getStatus() == Visit::Status::InTreatment, "Status is InTreatment");

    bool upd2 = history.updateVisitStatus("VIS-101", Visit::Status::Completed);
    ASSERT_TEST(upd2, "Updated to Completed");
    ASSERT_TEST(history.findVisit("VIS-101")->getStatus() == Visit::Status::Completed, "Status is Completed");

    bool updMissing = history.updateVisitStatus("VIS-GHOST", Visit::Status::Completed);
    ASSERT_TEST(!updMissing, "Updating non-existent visit returns false");
}

void testSearchMissingAndRemoveVisit() {
    std::cout << "\n--- Test 8 & 9: Search Missing & Remove Visit ---\n";
    VisitHistory history("101", "1001");
    history.addVisit(Visit("V1", "1001", "101", "10:00", TriageLevel::Urgent, "Complaint 1"));
    history.addVisit(Visit("V2", "1001", "101", "11:00", TriageLevel::Urgent, "Complaint 2"));
    history.addVisit(Visit("V3", "1001", "101", "12:00", TriageLevel::Urgent, "Complaint 3"));

    ASSERT_TEST(history.size() == 3, "Size is 3");

    // Missing visit search
    ASSERT_TEST(history.findVisit("V999") == nullptr, "Searching missing visit returns nullptr safely");

    // Remove middle visit V2
    bool remMid = history.removeVisit("V2");
    ASSERT_TEST(remMid, "Removed middle visit V2");
    ASSERT_TEST(history.size() == 2, "Size is now 2");
    ASSERT_TEST(history.findVisit("V2") == nullptr, "V2 no longer found");
    ASSERT_TEST(history.findVisit("V1") != nullptr, "V1 still intact");
    ASSERT_TEST(history.findVisit("V3") != nullptr, "V3 still intact");

    // Remove head visit V1
    bool remHead = history.removeVisit("V1");
    ASSERT_TEST(remHead, "Removed head visit V1");
    ASSERT_TEST(history.size() == 1, "Size is now 1");
    ASSERT_TEST(history.findVisit("V3") != nullptr, "V3 still intact");

    // Remove tail visit V3
    bool remTail = history.removeVisit("V3");
    ASSERT_TEST(remTail, "Removed tail visit V3");
    ASSERT_TEST(history.isEmpty(), "History is now empty");
    ASSERT_TEST(history.size() == 0, "Size is 0");
}

void testEmptyHistoryAndDisplaySafety() {
    std::cout << "\n--- Test 10: Empty History and Display Safety ---\n";
    VisitHistory history("101", "1001");
    ASSERT_TEST(history.isEmpty(), "Empty check returns true");
    ASSERT_TEST(!history.removeVisit("V1"), "Remove on empty returns false safely");
    ASSERT_TEST(history.findVisit("V1") == nullptr, "Find on empty returns nullptr safely");

    std::ostringstream oss;
    history.displayHistory(oss);
    ASSERT_TEST(oss.str().find("No visit history found") != std::string::npos ||
                oss.str().find("empty") != std::string::npos, "Empty display outputs safe message without crashing");
}

void testClearAndMemoryOwnership() {
    std::cout << "\n--- Test 11 & 12: Clear & Rule of Five Memory Ownership ---\n";
    VisitHistory original("101", "1001");
    original.addVisit(Visit("V1", "1001", "101", "08:00", TriageLevel::Urgent, "A"));
    original.addVisit(Visit("V2", "1001", "101", "09:00", TriageLevel::Urgent, "B"));

    // Deep copy constructor
    VisitHistory copy(original);
    ASSERT_TEST(copy.size() == 2, "Copy constructor created size 2");
    ASSERT_TEST(copy.findVisit("V1") != nullptr, "Copy contains V1");

    // Mutating original must not affect copy
    original.removeVisit("V1");
    ASSERT_TEST(original.size() == 1, "Original size is 1 after removal");
    ASSERT_TEST(copy.size() == 2, "Copy size remains 2 (deep copy verified)");
    ASSERT_TEST(copy.findVisit("V1") != nullptr, "Copy still retains V1");

    // Copy assignment
    VisitHistory assigned;
    assigned = copy;
    ASSERT_TEST(assigned.size() == 2, "Copy assignment created size 2");

    // Move constructor
    VisitHistory moved(std::move(assigned));
    ASSERT_TEST(moved.size() == 2, "Moved history took ownership of 2 nodes");
    ASSERT_TEST(assigned.isEmpty(), "Moved-from history is safely empty");

    // Clear
    moved.clear();
    ASSERT_TEST(moved.isEmpty(), "History is empty after clear()");
    ASSERT_TEST(moved.size() == 0, "Size is 0 after clear()");
}

int main() {
    std::cout << "=========================================================\n";
    std::cout << "        SmartER Custom VisitHistory Verification         \n";
    std::cout << "=========================================================\n";

    testAddAndRetrieveSingleVisit();
    testMultipleVisitsAndChronologicalOrder();
    testDuplicateVisitRejection();
    testMultiPatientAndMultiHospitalIsolation();
    testUpdateVisitStatus();
    testSearchMissingAndRemoveVisit();
    testEmptyHistoryAndDisplaySafety();
    testClearAndMemoryOwnership();

    std::cout << "\n=========================================================\n";
    std::cout << "Test Summary: " << testsPassed << " / " << testsTotal << " passed.\n";
    std::cout << "=========================================================\n";

    return (testsPassed == testsTotal) ? 0 : 1;
}

