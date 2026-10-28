#include "smarter/SmartERSystem.h"
#include "smarter/DatabaseManager.h"
#include "smarter/Hospital.hpp"
#include "smarter/Patient.hpp"
#include "smarter/Visit.hpp"
#include "smarter/Triage.hpp"

#include <iostream>
#include <string>
#include <vector>
#include <filesystem>
#include <cassert>

namespace {

int totalAssertions = 0;
int passedAssertions = 0;

void logAssert(bool condition, const std::string& message) {
    ++totalAssertions;
    if (condition) {
        ++passedAssertions;
        std::cout << "  [PASS] " << message << "\n";
    } else {
        std::cerr << "  [FAIL] " << message << "\n";
    }
}

#define ASSERT_TRUE(cond, msg) logAssert((cond), (msg))
#define ASSERT_FALSE(cond, msg) logAssert(!(cond), (msg))
#define ASSERT_EQ(a, b, msg) logAssert(((a) == (b)), (msg))

} // anonymous namespace

int main() {
    std::cout << "=========================================================\n"
              << "       SmartER Stage 9 SQLite Persistence Tests          \n"
              << "=========================================================\n\n";

    namespace fs = std::filesystem;
    const std::string testDbPath = "test_persistence_temp.db";

    // Clean up any stale test database
    std::error_code ec;
    fs::remove(testDbPath, ec);

    // ==========================================
    // Test 1: Empty Database Startup & Schema Creation
    // ==========================================
    std::cout << "--- Test 1: Empty Database Startup & Schema Creation ---\n";
    {
        smarter::DatabaseManager db(testDbPath);
        ASSERT_TRUE(db.open(), "Successfully created and opened temporary SQLite database");
        ASSERT_TRUE(db.isOpen(), "Database reports open state");
        ASSERT_TRUE(db.initializeSchema(), "Schema initialized with tables (hospitals, patients, visits, system_metadata)");
        ASSERT_EQ(db.getSchemaVersion(), 1, "Schema version is 1");

        // Verify tables exist by checking empty loads
        auto hospitals = db.loadHospitals();
        auto patients = db.loadPatients();
        auto visits = db.loadVisits();
        ASSERT_EQ(hospitals.size(), size_t(0), "Hospitals table initially empty");
        ASSERT_EQ(patients.size(), size_t(0), "Patients table initially empty");
        ASSERT_EQ(visits.size(), size_t(0), "Visits table initially empty");
    }

    // ==========================================
    // Test 2: Saving and Loading Hospitals
    // ==========================================
    std::cout << "\n--- Test 2: Saving and Loading Hospitals ---\n";
    {
        smarter::DatabaseManager db(testDbPath);
        ASSERT_TRUE(db.open(), "Opened database for hospital tests");

        ASSERT_TRUE(db.saveHospital("101", "CityCare Metro Hospital", "Metro City", 50), "Saved Hospital 101");
        ASSERT_TRUE(db.saveHospital("102", "GreenLife Regional Clinic", "Green Valley", 35), "Saved Hospital 102");

        ASSERT_TRUE(db.hasHospital("101"), "Confirmed Hospital 101 exists in database");
        ASSERT_TRUE(db.hasHospital("102"), "Confirmed Hospital 102 exists in database");
        ASSERT_FALSE(db.hasHospital("999"), "Confirmed non-existent Hospital 999 is false");

        auto loaded = db.loadHospitals();
        ASSERT_EQ(loaded.size(), size_t(2), "Loaded exactly 2 hospitals");
        ASSERT_EQ(loaded[0].getHospitalId(), std::string("101"), "1st Hospital ID is 101");
        ASSERT_EQ(loaded[0].getName(), std::string("CityCare Metro Hospital"), "1st Hospital name matches");
        ASSERT_EQ(loaded[0].getCity(), std::string("Metro City"), "1st Hospital city matches");
        ASSERT_EQ(loaded[0].getTotalEmergencyBeds(), 50, "1st Hospital beds match 50");
        ASSERT_EQ(loaded[1].getHospitalId(), std::string("102"), "2nd Hospital ID is 102");
    }

    // ==========================================
    // Test 3: Duplicate Hospital Prevention & Referential Foreign Keys
    // ==========================================
    std::cout << "\n--- Test 3: Duplicate Hospital & Foreign Key Integrity ---\n";
    {
        smarter::DatabaseManager db(testDbPath);
        ASSERT_TRUE(db.open(), "Opened database for constraint testing");

        // Duplicate primary key
        ASSERT_FALSE(db.saveHospital("101", "Duplicate Name", "Other City", 10),
                     "Rejected duplicate hospital primary key '101'");

        // Foreign key check: Saving patient under non-existent hospital '999'
        smarter::Patient orphan("P99", "999", "Orphan Patient", 30, "Male", "O+", "555-0000");
        ASSERT_FALSE(db.savePatient(orphan), "Foreign key constraint rejected patient under non-existent hospital '999'");
    }

    // ==========================================
    // Test 4: Saving and Loading Patients with Multiword Data
    // ==========================================
    std::cout << "\n--- Test 4: Saving and Loading Patients with Multiword Data ---\n";
    {
        smarter::DatabaseManager db(testDbPath);
        ASSERT_TRUE(db.open(), "Opened database for patient operations");

        smarter::Patient p1("1001", "101", "Rahul Dev Sharma", 34, "Male", "A+", "+91-9876543210");
        smarter::Patient p2("1002", "101", "Dr. Priya Ananya Patel", 28, "Female", "B+", "+91-9876543211");
        smarter::Patient p3("1003", "101", "Amit Vikram Kumar", 45, "Male", "O+", "+91-9876543212");

        ASSERT_TRUE(db.savePatient(p1), "Saved patient Rahul Dev Sharma");
        ASSERT_TRUE(db.savePatient(p2), "Saved patient Dr. Priya Ananya Patel");
        ASSERT_TRUE(db.savePatient(p3), "Saved patient Amit Vikram Kumar");

        auto patients = db.loadPatients();
        ASSERT_EQ(patients.size(), size_t(3), "Loaded 3 patients from database");
        ASSERT_EQ(patients[0].getName(), std::string("Rahul Dev Sharma"), "Preserved multiword patient name");
        ASSERT_EQ(patients[0].getContactPhone(), std::string("+91-9876543210"), "Preserved contact phone");
    }

    // ==========================================
    // Test 5: Cross-Hospital Duplicate Patient ID Coexistence
    // ==========================================
    std::cout << "\n--- Test 5: Cross-Hospital Duplicate Patient ID Coexistence ---\n";
    {
        smarter::DatabaseManager db(testDbPath);
        ASSERT_TRUE(db.open(), "Opened database for composite key check");

        // Patient ID 1001 registered under Hospital 102 (identical ID to Rahul at 101)
        smarter::Patient neha("1001", "102", "Neha Rashmi Gupta", 29, "Female", "AB+", "+91-9876543213");
        ASSERT_TRUE(db.savePatient(neha), "Successfully saved Patient ID '1001' under Hospital '102'");

        // Reject duplicate under same hospital
        smarter::Patient dup("1001", "101", "Duplicate Rahul", 30, "Male", "A+", "000");
        ASSERT_FALSE(db.savePatient(dup), "Rejected duplicate Patient (101, 1001) in same hospital");

        ASSERT_TRUE(db.hasPatient("101", "1001"), "Patient (101, 1001) exists");
        ASSERT_TRUE(db.hasPatient("102", "1001"), "Patient (102, 1001) exists concurrently");
    }

    // ==========================================
    // Test 6: Patient Updates in Database
    // ==========================================
    std::cout << "\n--- Test 6: Patient Updates in Database ---\n";
    {
        smarter::DatabaseManager db(testDbPath);
        ASSERT_TRUE(db.open(), "Opened database for update test");

        ASSERT_TRUE(db.updatePatient("101", "1001", "Rahul D. Sharma", 35, "Male", "A+", "+91-9999999999"),
                    "Updated patient details in SQLite");

        auto patients = db.loadPatients();
        bool foundUpdated = false;
        for (const auto& p : patients) {
            if (p.getHospitalId() == "101" && p.getPatientId() == "1001") {
                ASSERT_EQ(p.getName(), std::string("Rahul D. Sharma"), "Updated name persisted");
                ASSERT_EQ(p.getAge(), 35, "Updated age persisted");
                ASSERT_EQ(p.getContactPhone(), std::string("+91-9999999999"), "Updated phone persisted");
                foundUpdated = true;
                break;
            }
        }
        ASSERT_TRUE(foundUpdated, "Found updated record in loaded patient list");
    }

    // ==========================================
    // Test 7: Saving and Loading Visits with Detailed Clinical Info
    // ==========================================
    std::cout << "\n--- Test 7: Saving and Loading Visits with Clinical Info ---\n";
    {
        smarter::DatabaseManager db(testDbPath);
        ASSERT_TRUE(db.open(), "Opened database for visit persistence");

        smarter::Visit v1("VIS-101-1001-1", "1001", "101", "2026-10-23 09:15",
                          smarter::TriageLevel::Emergent, "High fever and persistent chills",
                          "Awaiting lab test results", smarter::Visit::Status::Waiting,
                          "Patient arrived with guardian", 1, true);

        smarter::Visit v2("VIS-101-1002-2", "1002", "101", "2026-10-23 09:20",
                          smarter::TriageLevel::Immediate, "Acute crushing chest pain",
                          "Cardiology emergency code", smarter::Visit::Status::Waiting,
                          "Oxygen administered immediately", 2, true);

        smarter::Visit v3("VIS-101-1003-3", "1003", "101", "2026-10-23 09:30",
                          smarter::TriageLevel::Emergent, "Compound fracture of right forearm",
                          "Orthopedic consult", smarter::Visit::Status::Waiting,
                          "Splint applied in ER", 3, true);

        smarter::Visit v4("VIS-102-1001-4", "1001", "102", "2026-10-23 09:40",
                          smarter::TriageLevel::Urgent, "Severe migraine attack",
                          "Neurology outpatient referral", smarter::Visit::Status::Waiting,
                          "Dark room observation", 4, true);

        ASSERT_TRUE(db.saveVisit(v1), "Saved ER visit 1 (Rahul, Triage 2, Arrival 1)");
        ASSERT_TRUE(db.saveVisit(v2), "Saved ER visit 2 (Priya, Triage 1, Arrival 2)");
        ASSERT_TRUE(db.saveVisit(v3), "Saved ER visit 3 (Amit, Triage 2, Arrival 3)");
        ASSERT_TRUE(db.saveVisit(v4), "Saved ER visit 4 (Neha, GreenLife, Triage 3, Arrival 4)");

        auto visits = db.loadVisits();
        ASSERT_EQ(visits.size(), size_t(4), "Loaded all 4 visits");
        ASSERT_EQ(visits[0].getChiefComplaint(), std::string("High fever and persistent chills"), "Preserved multiword complaint");
        ASSERT_EQ(visits[1].getTriageLevel(), smarter::TriageLevel::Immediate, "Preserved TriageLevel::Immediate");
        ASSERT_EQ(visits[1].getArrivalSequence(), 2, "Preserved arrival sequence 2");
        ASSERT_TRUE(visits[1].isInEmergencyQueue(), "Preserved inEmergencyQueue flag true");
    }

    // ==========================================
    // Test 8: Transaction Rollback Integrity
    // ==========================================
    std::cout << "\n--- Test 8: Transaction Rollback Integrity ---\n";
    {
        smarter::DatabaseManager db(testDbPath);
        ASSERT_TRUE(db.open(), "Opened database for transaction test");

        {
            smarter::ScopedTransaction tx(db);
            ASSERT_TRUE(tx.isActive(), "Transaction is active");

            smarter::Visit vTemp("VIS-TEMP-FAIL", "1001", "101", "2026-10-23 10:00",
                                 smarter::TriageLevel::NonUrgent, "Test fail",
                                 "N/A", smarter::Visit::Status::Waiting, "", 999, false);
            ASSERT_TRUE(db.saveVisit(vTemp), "Inserted visit inside transaction");

            // Intentionally allow ScopedTransaction to go out of scope without commit()
        }

        // Verify visit was rolled back
        ASSERT_FALSE(db.hasVisit("101", "1001", "VIS-TEMP-FAIL"), "Rolled back visit does NOT exist in database");
    }

    // Clean up temporary database for fresh system hydration tests
    fs::remove(testDbPath, ec);

    // ==========================================
    // Test 9: End-to-End System Integration with SQLite Persistence
    // ==========================================
    std::cout << "\n--- Test 9: End-to-End System Integration with SQLite Persistence ---\n";
    {
        smarter::SmartERSystem sys1;
        ASSERT_TRUE(sys1.initializeDatabase(testDbPath), "Initialized SmartERSystem with test SQLite database");
        ASSERT_TRUE(sys1.isDatabaseEnabled(), "Persistence layer is confirmed active in coordinator");

        // 1. Register Hospitals
        ASSERT_TRUE(sys1.registerHospital("101", "CityCare Hospital", "Metro City", 50), "Registered CityCare (101)");
        ASSERT_TRUE(sys1.registerHospital("102", "GreenLife Hospital", "Green Valley", 40), "Registered GreenLife (102)");

        // 2. Register Patients
        ASSERT_TRUE(sys1.registerPatient("101", "1001", "Rahul Sharma", 30, "Male", "A+", "555-1001"), "Registered Rahul");
        ASSERT_TRUE(sys1.registerPatient("101", "1002", "Priya Patel", 25, "Female", "B+", "555-1002"), "Registered Priya");
        ASSERT_TRUE(sys1.registerPatient("101", "1003", "Amit Kumar", 42, "Male", "O+", "555-1003"), "Registered Amit");
        ASSERT_TRUE(sys1.registerPatient("102", "1001", "Neha Gupta", 28, "Female", "AB+", "555-1004"), "Registered Neha (102, 1001)");

        // 3. Admit to Emergency Priority Queue
        ASSERT_TRUE(sys1.admitPatientToEmergency("101", "1001", 2, "High fever"), "Admitted Rahul (Triage 2)");
        ASSERT_TRUE(sys1.admitPatientToEmergency("101", "1002", 1, "Acute chest pain"), "Admitted Priya (Triage 1)");
        ASSERT_TRUE(sys1.admitPatientToEmergency("101", "1003", 2, "Fracture"), "Admitted Amit (Triage 2)");
        ASSERT_TRUE(sys1.admitPatientToEmergency("102", "1001", 3, "Migraine"), "Admitted Neha to GreenLife (Triage 3)");

        // Verify active queues before restart
        ASSERT_EQ(sys1.getEmergencyQueueSize("101"), size_t(3), "CityCare has 3 waiting patients");
        ASSERT_EQ(sys1.getEmergencyQueueSize("102"), size_t(1), "GreenLife has 1 waiting patient");

        // Peek next patient at CityCare: Priya (Triage 1)
        auto peekNext = sys1.peekNextEmergencyPatient("101");
        ASSERT_TRUE(peekNext.has_value(), "Peek returned value");
        ASSERT_EQ(peekNext->patientId, std::string("1002"), "Top of queue is Priya (1002)");
    }

    // ==========================================
    // Test 10: Application Restart & Complete DSA Hydration
    // ==========================================
    std::cout << "\n--- Test 10: Application Restart & Complete DSA Hydration ---\n";
    {
        // Simulate a complete application restart by instantiating a brand new coordinator
        smarter::SmartERSystem sys2;
        ASSERT_TRUE(sys2.initializeDatabase(testDbPath), "Restarted SmartERSystem reconnected to same database");

        // 1. Verify Hospitals restored
        ASSERT_EQ(sys2.getHospitalCount(), size_t(2), "Restored 2 hospitals");
        ASSERT_TRUE(sys2.hasHospital("101"), "CityCare Hospital restored");
        ASSERT_TRUE(sys2.hasHospital("102"), "GreenLife Hospital restored");

        // 2. Verify Patients restored into canonical Hash Table
        ASSERT_TRUE(sys2.hasPatient("101", "1001"), "Rahul (101, 1001) in Registry");
        ASSERT_TRUE(sys2.hasPatient("101", "1002"), "Priya (101, 1002) in Registry");
        ASSERT_TRUE(sys2.hasPatient("101", "1003"), "Amit (101, 1003) in Registry");
        ASSERT_TRUE(sys2.hasPatient("102", "1001"), "Neha (102, 1001) in Registry");

        // 3. Verify BST correctly reconstructed
        const auto& bst = sys2.getPatientBST();
        ASSERT_EQ(bst.size(), size_t(4), "BST contains exactly 4 patient keys");
        auto bstKeys = bst.getInorderKeys();
        ASSERT_EQ(bstKeys.size(), size_t(4), "BST traversal produced 4 sorted keys");
        ASSERT_EQ(bstKeys[0].first, std::string("101"), "BST Key 0 hospital 101");
        ASSERT_EQ(bstKeys[0].second, std::string("1001"), "BST Key 0 patient 1001 (Rahul)");
        ASSERT_EQ(bstKeys[1].second, std::string("1002"), "BST Key 1 patient 1002 (Priya)");
        ASSERT_EQ(bstKeys[2].second, std::string("1003"), "BST Key 2 patient 1003 (Amit)");
        ASSERT_EQ(bstKeys[3].first, std::string("102"), "BST Key 3 hospital 102");
        ASSERT_EQ(bstKeys[3].second, std::string("1001"), "BST Key 3 patient 1001 (Neha)");

        // 4. Verify Min-Heap Emergency Queue correctly reconstructed
        ASSERT_EQ(sys2.getEmergencyQueueSize("101"), size_t(3), "CityCare ER queue restored with 3 patients");
        ASSERT_EQ(sys2.getEmergencyQueueSize("102"), size_t(1), "GreenLife ER queue restored with 1 patient");

        // 5. Verify Priority Dispatch Order & Arrival-Order Tie Breaking:
        // Priority: Priya (Triage 1) -> Rahul (Triage 2, arrival 1) -> Amit (Triage 2, arrival 3)
        auto served1 = sys2.serveNextEmergencyPatient("101");
        ASSERT_TRUE(served1.has_value(), "1st Serve returned a patient");
        ASSERT_EQ(served1->patientId, std::string("1002"), "1st Served is Priya (Triage 1)");
        ASSERT_EQ(served1->triageLevel, 1, "1st Served has Triage Level 1");

        auto served2 = sys2.serveNextEmergencyPatient("101");
        ASSERT_TRUE(served2.has_value(), "2nd Serve returned a patient");
        ASSERT_EQ(served2->patientId, std::string("1001"), "2nd Served is Rahul (Triage 2, Arrival 1)");

        auto served3 = sys2.serveNextEmergencyPatient("101");
        ASSERT_TRUE(served3.has_value(), "3rd Serve returned a patient");
        ASSERT_EQ(served3->patientId, std::string("1003"), "3rd Served is Amit (Triage 2, Arrival 3)");

        // Empty CityCare queue check
        ASSERT_EQ(sys2.getEmergencyQueueSize("101"), size_t(0), "CityCare emergency queue now empty");
        auto servedEmpty = sys2.serveNextEmergencyPatient("101");
        ASSERT_FALSE(servedEmpty.has_value(), "Serve on empty queue returns nullopt");

        // GreenLife hospital isolation check
        ASSERT_EQ(sys2.getEmergencyQueueSize("102"), size_t(1), "GreenLife ER queue still contains Neha");
        auto servedGreenLife = sys2.serveNextEmergencyPatient("102");
        ASSERT_TRUE(servedGreenLife.has_value(), "Served patient from GreenLife");
        ASSERT_EQ(servedGreenLife->patientId, std::string("1001"), "Served Neha (1001) from GreenLife");

        // 6. Verify Visit Status State Transitions in Database:
        // Priya's visit was served, so status should now be InTreatment
        auto priyaVisits = sys2.getPatientVisits("101", "1002");
        ASSERT_EQ(priyaVisits.size(), size_t(1), "Priya has 1 visit record");
        ASSERT_EQ(priyaVisits[0].getStatus(), smarter::Visit::Status::InTreatment, "Priya visit status is InTreatment");
        ASSERT_FALSE(priyaVisits[0].isInEmergencyQueue(), "Priya visit is no longer in emergency queue");

        // Update Priya to Completed (Discharged)
        ASSERT_TRUE(sys2.updateVisitStatus("101", "1002", priyaVisits[0].getVisitId(), smarter::Visit::Status::Completed),
                    "Updated Priya visit to Completed");

        // 7. Verify arrival counter state restored across restarts:
        // Next admitted patient must receive arrival sequence > 4
        ASSERT_TRUE(sys2.admitPatientToEmergency("101", "1001", 1, "Relapse fever"),
                    "Re-admitted Rahul to CityCare ER");
        auto rahulVisits = sys2.getPatientVisits("101", "1001");
        ASSERT_EQ(rahulVisits.size(), size_t(2), "Rahul now has 2 visits recorded");
        ASSERT_TRUE(rahulVisits[1].getArrivalSequence() > 4, "New arrival sequence strictly increments above previous runs");
    }

    // ==========================================
    // Test 11: Dynamic Triage Level Persistence Across Restart
    // ==========================================
    std::cout << "\n--- Test 11: Dynamic Triage Level Persistence Across Restart ---\n";
    {
        // Reconnect coordinator
        smarter::SmartERSystem sys3;
        ASSERT_TRUE(sys3.initializeDatabase(testDbPath), "Reconnected sys3 to test database");

        // Rahul is currently waiting in CityCare ER with Triage 1
        ASSERT_EQ(sys3.getEmergencyQueueSize("101"), size_t(1), "CityCare has 1 patient (Rahul)");

        // Dynamically adjust triage from 1 to 5 (downgrade)
        ASSERT_TRUE(sys3.updateEmergencyTriage("101", "1001", 5), "Updated Rahul triage to 5");

        // Admit another patient with Triage 3
        ASSERT_TRUE(sys3.admitPatientToEmergency("101", "1003", 3, "New complaint"), "Admitted Amit with Triage 3");

        // Top of queue should now be Amit (3) instead of Rahul (5)
        auto top = sys3.peekNextEmergencyPatient("101");
        ASSERT_TRUE(top.has_value(), "Peek returned patient");
        ASSERT_EQ(top->patientId, std::string("1003"), "Amit is now root of Min-Heap with higher priority (3 < 5)");
    }

    // Clean up temporary database file
    fs::remove(testDbPath, ec);
    fs::remove(testDbPath + "-wal", ec);
    fs::remove(testDbPath + "-shm", ec);

    std::cout << "\n=========================================================\n"
              << "Persistence Test Summary: " << passedAssertions << " / " << totalAssertions << " passed.\n"
              << "=========================================================\n";

    return (passedAssertions == totalAssertions) ? 0 : 1;
}

