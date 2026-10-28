#include "smarter/SmartERSystem.h"
#include <iostream>
#include <string>
#include <limits>
#include <algorithm>

namespace {

// Safely clear remaining characters in standard input buffer
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Reads a trimmed line of text from standard input
std::string readString(const std::string& prompt, bool allowEmpty = false) {
    std::string value;
    while (true) {
        std::cout << prompt;
        if (!std::getline(std::cin, value)) {
            clearInputBuffer();
            return "";
        }

        // Trim leading and trailing whitespace
        size_t start = value.find_first_not_of(" \t\r\n");
        if (start == std::string::npos) {
            if (allowEmpty) return "";
            std::cout << "  [Input Error] Value cannot be blank. Please try again.\n";
            continue;
        }
        size_t end = value.find_last_not_of(" \t\r\n");
        return value.substr(start, end - start + 1);
    }
}

// Reads an integer with strict validation and optional range checking
int readInt(const std::string& prompt,
            int minVal = std::numeric_limits<int>::min(),
            int maxVal = std::numeric_limits<int>::max()) {
    int value = 0;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value) {
            clearInputBuffer(); // Consume remaining newline
            if (value >= minVal && value <= maxVal) {
                return value;
            }
            std::cout << "  [Input Error] Value must be between "
                      << minVal << " and " << maxVal << ". Please try again.\n";
        } else {
            std::cout << "  [Input Error] Invalid input: integer expected. Please try again.\n";
            clearInputBuffer();
        }
    }
}

void printMenu() {
    std::cout << "\n========================================\n"
              << "SmartER HOSPITAL SYSTEM\n"
              << "=======================\n"
              << "1.  Register Hospital\n"
              << "2.  Register Patient\n"
              << "3.  View Emergency Queue\n"
              << "4.  Serve Next Patient\n"
              << "5.  Search Patient\n"
              << "6.  Update Triage Level\n"
              << "7.  Add Patient Visit\n"
              << "8.  View Patient Visit History\n"
              << "9.  Update Visit Status\n"
              << "10. Display Sorted Patient Records\n"
              << "11. View Hospital Statistics\n"
              << "12. Exit\n"
              << "----------------------------------------\n";
}

} // anonymous namespace

int main() {
    smarter::SmartERSystem system;

    std::cout << "=========================================================\n"
              << "  Welcome to SmartER: Hospital Management & ER Triage    \n"
              << "    Data Structures and Algorithms System in C++17       \n"
              << "=========================================================\n";

    std::string dbPath = smarter::DatabaseManager::getDefaultDatabasePath();
    if (system.initializeDatabase(dbPath)) {
        std::cout << ">> SQLite Persistent Storage: Connected [" << dbPath << "]\n";
        std::cout << ">> Restored " << system.getHospitalCount() << " hospital(s) from persistent storage.\n";
    } else {
        std::cerr << ">> [Notice] Operating in memory-only mode (database initialization failed or disabled).\n";
    }

    bool running = true;
    while (running) {
        printMenu();
        int choice = readInt("Enter your choice (1-12): ", 1, 12);

        switch (choice) {
            case 1: { // Register Hospital
                std::cout << "\n--- [1] Register Hospital ---\n";
                std::string hospId = readString("Enter Hospital ID (e.g. 101): ");
                std::string name   = readString("Enter Hospital Name: ");
                std::string city   = readString("Enter Location/City: ");
                int beds           = readInt("Enter ER Bed Capacity: ", 1, 10000);

                if (system.registerHospital(hospId, name, city, beds)) {
                    std::cout << ">> SUCCESS: Hospital '" << name
                              << "' [" << hospId << "] registered successfully.\n";
                } else {
                    std::cout << ">> FAILED: Could not register hospital.\n";
                }
                break;
            }

            case 2: { // Register Patient
                std::cout << "\n--- [2] Register Patient ---\n";
                std::string hospId = readString("Enter Hospital ID: ");
                if (!system.hasHospital(hospId)) {
                    std::cout << ">> FAILED: Hospital '" << hospId << "' does not exist. Register it first.\n";
                    break;
                }

                std::string patId  = readString("Enter Patient ID (e.g. 1001): ");
                std::string name   = readString("Enter Patient Name: ");
                int age            = readInt("Enter Age: ", 0, 130);
                std::string gender = readString("Enter Gender (Male/Female/Other): ");
                std::string blood  = readString("Enter Blood Group (e.g. O+, A-, B+): ");
                std::string phone  = readString("Enter Contact Phone: ");

                if (system.registerPatient(hospId, patId, name, age, gender, blood, phone)) {
                    std::cout << ">> SUCCESS: Patient '" << name
                              << "' (" << hospId << ", " << patId << ") registered.\n";
                } else {
                    std::cout << ">> FAILED: Could not register patient.\n";
                }
                break;
            }

            case 3: { // View Emergency Queue
                std::cout << "\n--- [3] View Emergency Queue ---\n";
                std::string hospId = readString("Enter Hospital ID: ");
                if (!system.hasHospital(hospId)) {
                    std::cout << ">> FAILED: Hospital '" << hospId << "' not found.\n";
                    break;
                }
                system.displayEmergencyQueue(hospId);
                break;
            }

            case 4: { // Serve Next Patient
                std::cout << "\n--- [4] Serve Next Patient ---\n";
                std::string hospId = readString("Enter Hospital ID: ");
                if (!system.hasHospital(hospId)) {
                    std::cout << ">> FAILED: Hospital '" << hospId << "' not found.\n";
                    break;
                }

                auto servedOpt = system.serveNextEmergencyPatient(hospId);
                if (servedOpt.has_value()) {
                    const auto& s = *servedOpt;
                    const auto* p = system.findPatient(hospId, s.patientId);
                    std::string pName = p ? p->getName() : "Unknown";

                    std::cout << ">> DISPATCHED PATIENT FROM ER QUEUE:\n"
                              << "   Patient:         " << pName << " [" << s.patientId << "]\n"
                              << "   Hospital:        " << hospId << "\n"
                              << "   Triage Level:    " << s.triageLevel << " ("
                              << smarter::triageLevelToString(static_cast<smarter::TriageLevel>(s.triageLevel)) << ")\n"
                              << "   Arrival Sequence:" << s.arrivalSequence << "\n"
                              << "   Status:          Active visit updated to 'In Treatment'.\n";
                } else {
                    std::cout << ">> Notice: Emergency queue for Hospital '" << hospId << "' is currently empty.\n";
                }
                break;
            }

            case 5: { // Search Patient
                std::cout << "\n--- [5] Search Patient ---\n";
                std::string hospId = readString("Enter Hospital ID: ");
                std::string patId  = readString("Enter Patient ID: ");

                const auto* p = system.findPatient(hospId, patId);
                if (p != nullptr) {
                    std::cout << "\n>> Patient Record Found:\n";
                    p->printDetails();
                } else {
                    std::cout << ">> Notice: Patient (" << hospId << ", " << patId << ") not found in registry.\n";
                }
                break;
            }

            case 6: { // Update Triage Level
                std::cout << "\n--- [6] Update Triage Level ---\n";
                std::string hospId = readString("Enter Hospital ID: ");
                std::string patId  = readString("Enter Patient ID: ");
                std::cout << "Triage Levels: 1=Immediate, 2=Emergent, 3=Urgent, 4=LessUrgent, 5=NonUrgent\n";
                int newTriage = readInt("Enter New Triage Level (1-5): ", 1, 5);

                if (system.updateEmergencyTriage(hospId, patId, newTriage)) {
                    std::cout << ">> SUCCESS: Triage level updated to " << newTriage
                              << ". Min-Heap order restored.\n";
                } else {
                    std::cout << ">> FAILED: Patient (" << hospId << ", " << patId
                              << ") not found waiting in emergency queue.\n";
                }
                break;
            }

            case 7: { // Add Patient Visit
                std::cout << "\n--- [7] Add Patient Visit ---\n";
                std::string hospId = readString("Enter Hospital ID: ");
                std::string patId  = readString("Enter Patient ID: ");

                if (!system.hasPatient(hospId, patId)) {
                    std::cout << ">> FAILED: Patient (" << hospId << ", " << patId
                              << ") is not registered. Please register patient first.\n";
                    break;
                }

                std::cout << "Triage Levels: 1=Immediate, 2=Emergent, 3=Urgent, 4=LessUrgent, 5=NonUrgent\n";
                int triageLevel = readInt("Enter Triage Acuity (1-5): ", 1, 5);
                std::string complaint = readString("Enter Reason for Visit / Chief Complaint: ");
                std::string admitER   = readString("Admit to Hospital Emergency Room Queue? (Y/N): ");

                if (admitER == "Y" || admitER == "y") {
                    if (system.admitPatientToEmergency(hospId, patId, triageLevel, complaint)) {
                        std::cout << ">> SUCCESS: Visit logged and patient admitted to ER Priority Queue.\n";
                    } else {
                        std::cout << ">> FAILED: Could not admit patient to emergency queue.\n";
                    }
                } else {
                    std::string vId = readString("Enter Visit ID (e.g. VIS-101): ");
                    std::string time = readString("Enter Timestamp (e.g. 2026-10-23 10:30): ");
                    std::cout << "Visit Status: 0=Waiting, 1=In Treatment, 2=Admitted, 3=Completed\n";
                    int statInt = readInt("Enter Visit Status (0-3): ", 0, 3);

                    smarter::Visit v(vId, patId, hospId, time,
                                     static_cast<smarter::TriageLevel>(triageLevel),
                                     complaint, "Consultation logged",
                                     static_cast<smarter::Visit::Status>(statInt));
                    if (system.addVisit(v)) {
                        std::cout << ">> SUCCESS: Visit '" << vId << "' recorded in patient history.\n";
                    } else {
                        std::cout << ">> FAILED: Could not record visit.\n";
                    }
                }
                break;
            }

            case 8: { // View Patient Visit History
                std::cout << "\n--- [8] View Patient Visit History ---\n";
                std::string hospId = readString("Enter Hospital ID: ");
                std::string patId  = readString("Enter Patient ID: ");

                system.displayPatientVisits(hospId, patId);
                break;
            }

            case 9: { // Update Visit Status
                std::cout << "\n--- [9] Update Visit Status ---\n";
                std::string hospId = readString("Enter Hospital ID: ");
                std::string patId  = readString("Enter Patient ID: ");
                std::string vId    = readString("Enter Visit ID: ");

                std::cout << "Select New Status:\n"
                          << "  0. Waiting\n"
                          << "  1. In Treatment\n"
                          << "  2. Admitted\n"
                          << "  3. Completed (Discharged)\n";
                int statusChoice = readInt("Enter Choice (0-3): ", 0, 3);
                auto newStatus = static_cast<smarter::Visit::Status>(statusChoice);

                if (system.updateVisitStatus(hospId, patId, vId, newStatus)) {
                    std::cout << ">> SUCCESS: Visit '" << vId << "' updated.\n";
                } else {
                    std::cout << ">> FAILED: Visit '" << vId << "' not found for Patient ("
                              << hospId << ", " << patId << ").\n";
                }
                break;
            }

            case 10: { // Display Sorted Patient Records
                std::cout << "\n--- [10] Display Sorted Patient Records (BST Directory) ---\n";
                system.displaySortedPatients();
                break;
            }

            case 11: { // View Hospital Statistics
                std::cout << "\n--- [11] View Hospital Statistics ---\n";
                std::string hospId = readString("Enter Hospital ID: ");
                if (!system.hasHospital(hospId)) {
                    std::cout << ">> FAILED: Hospital '" << hospId << "' does not exist.\n";
                    break;
                }
                system.displayHospitalStatistics(hospId);
                break;
            }

            case 12: { // Exit
                std::cout << "\n=========================================================\n"
                          << "  Exiting SmartER Hospital System. Goodbye!\n"
                          << "=========================================================\n";
                running = false;
                break;
            }

            default:
                std::cout << ">> Invalid choice. Please select an option between 1 and 12.\n";
                break;
        }
    }

    return 0;
}
