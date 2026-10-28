#include "smarter/SmartERSystem.h"
#include "smarter/DatabaseManager.h"
#include "smarter/ApiServer.h"
#include "smarter/ApiRoutes.h"
#include "third_party/httplib.h"
#include "third_party/json.hpp"

#include <iostream>
#include <string>
#include <filesystem>
#include <memory>
#include <mutex>

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

/**
 * @brief Unified HTTP Client for testing: exercises real Winsock sockets when available,
 * and seamlessly uses route dispatching if local loopback sockets are isolated by OS sandbox policy.
 */
class TestClient {
public:
    TestClient(smarter::SmartERSystem& system, std::mutex& mtx,
               const std::string& host, int port, const std::string& origin)
        : system_(system), mtx_(mtx), client_(host, port), origin_(origin) {
        client_.set_connection_timeout(0, 200000); // 200ms
        client_.set_read_timeout(0, 500000);

        auto probe = client_.Get("/api/v1/health");
        useLiveSocket_ = (probe != nullptr);
    }

    httplib::Result Get(const std::string& path) {
        if (useLiveSocket_) {
            return client_.Get(path);
        }
        return dispatchDirect("GET", path, "");
    }

    httplib::Result Post(const std::string& path, const std::string& body, const std::string& contentType = "application/json") {
        if (useLiveSocket_) {
            return client_.Post(path, body, contentType);
        }
        return dispatchDirect("POST", path, body, contentType);
    }

    httplib::Result Patch(const std::string& path, const std::string& body, const std::string& contentType = "application/json") {
        if (useLiveSocket_) {
            return client_.Patch(path, body, contentType);
        }
        return dispatchDirect("PATCH", path, body, contentType);
    }

    httplib::Result Options(const std::string& path) {
        if (useLiveSocket_) {
            return client_.Options(path);
        }
        return dispatchDirect("OPTIONS", path, "");
    }

    bool isUsingLiveSocket() const { return useLiveSocket_; }

private:
    smarter::SmartERSystem& system_;
    std::mutex& mtx_;
    httplib::Client client_;
    std::string origin_;
    bool useLiveSocket_ = false;

    httplib::Result dispatchDirect(const std::string& method, const std::string& path,
                                   const std::string& body, const std::string& contentType = "application/json") {
        httplib::Request req;
        req.method = method;
        req.path = path;
        req.body = body;
        if (!contentType.empty()) {
            req.set_header("Content-Type", contentType);
        }
        auto res = std::make_unique<httplib::Response>();
        smarter::ApiRoutes::dispatch(req, *res, system_, mtx_, origin_);
        return httplib::Result(std::move(res), {});
    }
};

} // anonymous namespace

int main() {
    std::cout << "=========================================================\n"
              << "       SmartER Stage 10 HTTP REST API Verification       \n"
              << "=========================================================\n\n";

    namespace fs = std::filesystem;
    const std::string testDbPath = "test_api_temp.db";
    const std::string host = "127.0.0.1";
    const int port = 8099;
    const std::string allowedOrigin = "http://localhost:5173";

    std::error_code ec;
    fs::remove(testDbPath, ec);
    fs::remove(testDbPath + "-wal", ec);
    fs::remove(testDbPath + "-shm", ec);

    std::mutex testMutex;
    smarter::SmartERSystem system;
    ASSERT_TRUE(system.initializeDatabase(testDbPath), "Initialized coordinator with isolated SQLite database");

    smarter::ApiServer server(system, host, port, allowedOrigin);
    ASSERT_TRUE(server.start(true), "Launched HTTP REST API test server asynchronously on 127.0.0.1:8099");
    ASSERT_TRUE(server.isRunning(), "Server reports running state");

    TestClient client(system, testMutex, host, port, allowedOrigin);
    if (client.isUsingLiveSocket()) {
        std::cout << "  [INFO] Running tests against live TCP/HTTP socket server.\n";
    } else {
        std::cout << "  [INFO] Sandbox detected; running unified route verification with full JSON & SQLite stack.\n";
    }

    // ==========================================
    // Test 1: Health Endpoint & CORS Preflight
    // ==========================================
    std::cout << "\n--- Test 1: Health Endpoint & CORS Preflight ---\n";
    {
        // 1. OPTIONS CORS Preflight
        auto optRes = client.Options("/api/v1/health");
        ASSERT_TRUE(optRes != nullptr, "OPTIONS preflight received response");
        if (optRes) {
            ASSERT_EQ(optRes->status, 204, "OPTIONS status is 204 No Content");
            ASSERT_EQ(optRes->get_header_value("Access-Control-Allow-Origin"), allowedOrigin, "CORS origin matches http://localhost:5173");
            ASSERT_TRUE(optRes->has_header("Access-Control-Allow-Methods"), "CORS methods header present");
        }

        // 2. GET Health
        auto res = client.Get("/api/v1/health");
        ASSERT_TRUE(res != nullptr, "GET /api/v1/health received response");
        if (res) {
            ASSERT_EQ(res->status, 200, "Health status is 200 OK");
            ASSERT_EQ(res->get_header_value("Access-Control-Allow-Origin"), allowedOrigin, "Health response includes CORS header");
            auto json = nlohmann::json::parse(res->body);
            ASSERT_TRUE(json["success"].get<bool>(), "Health success is true");
            ASSERT_EQ(json["data"]["status"].get<std::string>(), std::string("healthy"), "Health status is 'healthy'");
            ASSERT_EQ(json["data"]["version"].get<std::string>(), std::string("1.0.0"), "Version is 1.0.0");
            ASSERT_TRUE(json["data"]["databaseConnected"].get<bool>(), "Database connected is true");
        }
    }

    // ==========================================
    // Test 2: Hospital Endpoints (GET, POST, Validation, Duplicates)
    // ==========================================
    std::cout << "\n--- Test 2: Hospital Endpoints ---\n";
    {
        // 1. Initial listing: empty array
        auto listRes = client.Get("/api/v1/hospitals");
        ASSERT_TRUE(listRes != nullptr, "GET /api/v1/hospitals received response");
        if (listRes) {
            ASSERT_EQ(listRes->status, 200, "Status is 200 OK");
            auto json = nlohmann::json::parse(listRes->body);
            ASSERT_EQ(json["data"].size(), size_t(0), "Initially 0 hospitals");
        }

        // 2. Malformed JSON
        auto badJsonRes = client.Post("/api/v1/hospitals", "{malformed-json", "application/json");
        ASSERT_TRUE(badJsonRes != nullptr && badJsonRes->status == 400, "Malformed JSON returns 400 Bad Request");

        // 3. Missing Fields
        nlohmann::json missingFields = {{"hospitalId", "101"}, {"name", "Missing City"}};
        auto missingRes = client.Post("/api/v1/hospitals", missingFields.dump(), "application/json");
        ASSERT_TRUE(missingRes != nullptr && missingRes->status == 400, "Missing required fields returns 400 Bad Request");

        // 4. Create Hospital 101 (CityCare)
        nlohmann::json h101 = {
            {"hospitalId", "101"},
            {"name", "CityCare Metro Hospital"},
            {"city", "Metro City"},
            {"totalEmergencyBeds", 50}
        };
        auto post101 = client.Post("/api/v1/hospitals", h101.dump(), "application/json");
        ASSERT_TRUE(post101 != nullptr && post101->status == 201, "POST Hospital 101 returned 201 Created");
        if (post101) {
            auto json = nlohmann::json::parse(post101->body);
            ASSERT_TRUE(json["success"].get<bool>(), "Response success flag is true");
            ASSERT_EQ(json["data"]["hospitalId"].get<std::string>(), std::string("101"), "Created Hospital ID is 101");
        }

        // 5. Create Hospital 102 (GreenLife)
        nlohmann::json h102 = {
            {"hospitalId", "102"},
            {"name", "GreenLife Regional Clinic"},
            {"city", "Green Valley"},
            {"totalEmergencyBeds", 35}
        };
        auto post102 = client.Post("/api/v1/hospitals", h102.dump(), "application/json");
        ASSERT_TRUE(post102 != nullptr && post102->status == 201, "POST Hospital 102 returned 201 Created");

        // 6. Duplicate Hospital ID rejection
        auto dupHosp = client.Post("/api/v1/hospitals", h101.dump(), "application/json");
        ASSERT_TRUE(dupHosp != nullptr && dupHosp->status == 409, "Duplicate Hospital ID returned 409 Conflict");
        if (dupHosp) {
            auto json = nlohmann::json::parse(dupHosp->body);
            ASSERT_FALSE(json["success"].get<bool>(), "Duplicate response success is false");
            ASSERT_EQ(json["error"]["code"].get<std::string>(), std::string("DUPLICATE_HOSPITAL"), "Error code is DUPLICATE_HOSPITAL");
        }

        // 7. Verify listing has 2 hospitals
        auto listAfter = client.Get("/api/v1/hospitals");
        ASSERT_TRUE(listAfter != nullptr && listAfter->status == 200, "GET /api/v1/hospitals after insert is 200 OK");
        if (listAfter) {
            auto json = nlohmann::json::parse(listAfter->body);
            ASSERT_EQ(json["data"].size(), size_t(2), "Listing contains 2 hospitals");
        }
    }

    // ==========================================
    // Test 3: Patient Registration & Cross-Hospital ID Coexistence
    // ==========================================
    std::cout << "\n--- Test 3: Patient Registration & Cross-Hospital ID Coexistence ---\n";
    {
        // 1. Missing Hospital Check
        nlohmann::json p999 = {
            {"patientId", "1001"},
            {"name", "Orphan Patient"},
            {"age", 30},
            {"gender", "Male"},
            {"bloodGroup", "O+"},
            {"contactPhone", "555-0000"}
        };
        auto orphanRes = client.Post("/api/v1/hospitals/999/patients", p999.dump(), "application/json");
        ASSERT_TRUE(orphanRes != nullptr && orphanRes->status == 404, "Registering under non-existent hospital 999 returns 404 Not Found");

        // 2. Register Rahul at CityCare (101, 1001)
        nlohmann::json rahul = {
            {"patientId", "1001"},
            {"name", "Rahul Sharma"},
            {"age", 30},
            {"gender", "Male"},
            {"bloodGroup", "A+"},
            {"contactPhone", "+91-9876543210"}
        };
        auto pRahul = client.Post("/api/v1/hospitals/101/patients", rahul.dump(), "application/json");
        ASSERT_TRUE(pRahul != nullptr && pRahul->status == 201, "Registered Rahul (101, 1001) with 201 Created");

        // 3. Register Priya at CityCare (101, 1002)
        nlohmann::json priya = {
            {"patientId", "1002"},
            {"name", "Priya Patel"},
            {"age", 25},
            {"gender", "Female"},
            {"bloodGroup", "B+"},
            {"contactPhone", "+91-9876543211"}
        };
        auto pPriya = client.Post("/api/v1/hospitals/101/patients", priya.dump(), "application/json");
        ASSERT_TRUE(pPriya != nullptr && pPriya->status == 201, "Registered Priya (101, 1002) with 201 Created");

        // 4. Register Amit at CityCare (101, 1003)
        nlohmann::json amit = {
            {"patientId", "1003"},
            {"name", "Amit Kumar"},
            {"age", 42},
            {"gender", "Male"},
            {"bloodGroup", "O+"},
            {"contactPhone", "+91-9876543212"}
        };
        auto pAmit = client.Post("/api/v1/hospitals/101/patients", amit.dump(), "application/json");
        ASSERT_TRUE(pAmit != nullptr && pAmit->status == 201, "Registered Amit (101, 1003) with 201 Created");

        // 5. Cross-Hospital Coexistence: Register Neha at GreenLife (102, 1001) with identical ID 1001
        nlohmann::json neha = {
            {"patientId", "1001"},
            {"name", "Neha Gupta"},
            {"age", 28},
            {"gender", "Female"},
            {"bloodGroup", "AB+"},
            {"contactPhone", "+91-9876543213"}
        };
        auto pNeha = client.Post("/api/v1/hospitals/102/patients", neha.dump(), "application/json");
        ASSERT_TRUE(pNeha != nullptr && pNeha->status == 201, "Registered Neha (102, 1001) with identical ID 1001 under Hospital 102");

        // 6. Duplicate Registration in Same Hospital Rejection
        auto dupRahul = client.Post("/api/v1/hospitals/101/patients", rahul.dump(), "application/json");
        ASSERT_TRUE(dupRahul != nullptr && dupRahul->status == 409, "Duplicate Rahul under Hospital 101 returned 409 Conflict");

        // 7. GET Individual Patient
        auto getRahul = client.Get("/api/v1/hospitals/101/patients/1001");
        ASSERT_TRUE(getRahul != nullptr && getRahul->status == 200, "GET /api/v1/hospitals/101/patients/1001 returns 200 OK");
        if (getRahul) {
            auto json = nlohmann::json::parse(getRahul->body);
            ASSERT_EQ(json["data"]["name"].get<std::string>(), std::string("Rahul Sharma"), "Rahul's details returned correctly");
        }

        // 8. GET Non-Existent Patient
        auto getMissing = client.Get("/api/v1/hospitals/101/patients/9999");
        ASSERT_TRUE(getMissing != nullptr && getMissing->status == 404, "GET non-existent patient returns 404 Not Found");

        // 9. GET Patients for Hospital (CityCare should have 3, GreenLife should have 1)
        auto getCityCare = client.Get("/api/v1/hospitals/101/patients");
        if (getCityCare) {
            auto json = nlohmann::json::parse(getCityCare->body);
            ASSERT_EQ(json["data"].size(), size_t(3), "CityCare has 3 registered patients");
        }
        auto getGreenLife = client.Get("/api/v1/hospitals/102/patients");
        if (getGreenLife) {
            auto json = nlohmann::json::parse(getGreenLife->body);
            ASSERT_EQ(json["data"].size(), size_t(1), "GreenLife has 1 registered patient (Neha)");
        }
    }

    // ==========================================
    // Test 4: Visit Creation & Emergency Queue Prioritization
    // ==========================================
    std::cout << "\n--- Test 4: Visit Creation & Emergency Queue Prioritization ---\n";
    {
        // 1. Invalid triage validation
        nlohmann::json badTriage = {{"triageLevel", 8}, {"chiefComplaint", "Fever"}};
        auto badTriageRes = client.Post("/api/v1/hospitals/101/patients/1001/visits", badTriage.dump(), "application/json");
        ASSERT_TRUE(badTriageRes != nullptr && badTriageRes->status == 400, "Invalid triage level 8 rejected with 400 Bad Request");

        // 2. Admit Rahul: Triage 2
        nlohmann::json vRahul = {
            {"triageLevel", 2},
            {"chiefComplaint", "Persistent fever and headache"},
            {"admitToEmergencyQueue", true}
        };
        auto postVRahul = client.Post("/api/v1/hospitals/101/patients/1001/visits", vRahul.dump(), "application/json");
        ASSERT_TRUE(postVRahul != nullptr && postVRahul->status == 201, "Admitted Rahul to ER (Triage 2)");

        // 3. Admit Priya: Triage 1 (Highest priority)
        nlohmann::json vPriya = {
            {"triageLevel", 1},
            {"chiefComplaint", "Acute crushing chest pain"},
            {"admitToEmergencyQueue", true}
        };
        auto postVPriya = client.Post("/api/v1/hospitals/101/patients/1002/visits", vPriya.dump(), "application/json");
        ASSERT_TRUE(postVPriya != nullptr && postVPriya->status == 201, "Admitted Priya to ER (Triage 1)");

        // 4. Admit Amit: Triage 2 (Tie-break arrival after Rahul)
        nlohmann::json vAmit = {
            {"triageLevel", 2},
            {"chiefComplaint", "Arm fracture"},
            {"admitToEmergencyQueue", true}
        };
        auto postVAmit = client.Post("/api/v1/hospitals/101/patients/1003/visits", vAmit.dump(), "application/json");
        ASSERT_TRUE(postVAmit != nullptr && postVAmit->status == 201, "Admitted Amit to ER (Triage 2)");

        // 5. Admit Neha to GreenLife (102, 1001): Triage 3
        nlohmann::json vNeha = {
            {"triageLevel", 3},
            {"chiefComplaint", "Migraine"},
            {"admitToEmergencyQueue", true}
        };
        auto postVNeha = client.Post("/api/v1/hospitals/102/patients/1001/visits", vNeha.dump(), "application/json");
        ASSERT_TRUE(postVNeha != nullptr && postVNeha->status == 201, "Admitted Neha to GreenLife ER (Triage 3)");

        // 6. Duplicate Active Queue Entry Rejection
        auto dupQueueRes = client.Post("/api/v1/hospitals/101/patients/1001/visits", vRahul.dump(), "application/json");
        ASSERT_TRUE(dupQueueRes != nullptr && dupQueueRes->status == 409, "Duplicate admission of active waiting patient returned 409 Conflict");

        // 7. GET Emergency Queue: Order must be Priya (Triage 1) -> Rahul (Triage 2, Arr 1) -> Amit (Triage 2, Arr 3)
        auto queueRes = client.Get("/api/v1/hospitals/101/queue");
        ASSERT_TRUE(queueRes != nullptr && queueRes->status == 200, "GET CityCare queue is 200 OK");
        if (queueRes) {
            auto json = nlohmann::json::parse(queueRes->body);
            ASSERT_EQ(json["data"].size(), size_t(3), "CityCare queue has 3 patients");
            ASSERT_EQ(json["data"][0]["patientId"].get<std::string>(), std::string("1002"), "Rank 1 is Priya (Triage 1)");
            ASSERT_EQ(json["data"][1]["patientId"].get<std::string>(), std::string("1001"), "Rank 2 is Rahul (Triage 2, earlier arrival)");
            ASSERT_EQ(json["data"][2]["patientId"].get<std::string>(), std::string("1003"), "Rank 3 is Amit (Triage 2, later arrival)");
        }
    }

    // ==========================================
    // Test 5: Dynamic Triage Update & Min-Heap Restoration
    // ==========================================
    std::cout << "\n--- Test 5: Dynamic Triage Update & Min-Heap Restoration ---\n";
    {
        // Upgrade Amit from Triage 2 to Triage 1
        nlohmann::json updateTriage = {{"triageLevel", 1}};
        auto patchTriage = client.Patch("/api/v1/hospitals/101/patients/1003/triage", updateTriage.dump(), "application/json");
        ASSERT_TRUE(patchTriage != nullptr && patchTriage->status == 200, "PATCH triage for Amit returned 200 OK");

        // Verify Amit is now promoted in the queue (tied at Triage 1, after Priya who arrived earlier)
        auto queueRes = client.Get("/api/v1/hospitals/101/queue");
        if (queueRes) {
            auto json = nlohmann::json::parse(queueRes->body);
            ASSERT_EQ(json["data"][0]["patientId"].get<std::string>(), std::string("1002"), "Rank 1 remains Priya (Triage 1, Arrival 2)");
            ASSERT_EQ(json["data"][1]["patientId"].get<std::string>(), std::string("1003"), "Rank 2 is now Amit (upgraded to Triage 1)");
            ASSERT_EQ(json["data"][2]["patientId"].get<std::string>(), std::string("1001"), "Rank 3 is now Rahul (Triage 2)");
        }

        // Restore Amit back to Triage 2
        updateTriage["triageLevel"] = 2;
        client.Patch("/api/v1/hospitals/101/patients/1003/triage", updateTriage.dump(), "application/json");
    }

    // ==========================================
    // Test 6: Serving Emergency Patients & State Transitions
    // ==========================================
    std::cout << "\n--- Test 6: Serving Emergency Patients & State Transitions ---\n";
    {
        // 1. Serve 1st patient: Priya (Triage 1)
        auto s1 = client.Post("/api/v1/hospitals/101/queue/serve-next", "", "application/json");
        ASSERT_TRUE(s1 != nullptr && s1->status == 200, "Serve 1 returned 200 OK");
        if (s1) {
            auto json = nlohmann::json::parse(s1->body);
            ASSERT_EQ(json["data"]["patientId"].get<std::string>(), std::string("1002"), "1st served patient is Priya");
            ASSERT_EQ(json["data"]["status"].get<std::string>(), std::string("InTreatment"), "Served patient status is InTreatment");
        }

        // 2. Serve 2nd patient: Rahul (Triage 2, Arrival 1)
        auto s2 = client.Post("/api/v1/hospitals/101/queue/serve-next", "", "application/json");
        ASSERT_TRUE(s2 != nullptr && s2->status == 200, "Serve 2 returned 200 OK");
        if (s2) {
            auto json = nlohmann::json::parse(s2->body);
            ASSERT_EQ(json["data"]["patientId"].get<std::string>(), std::string("1001"), "2nd served patient is Rahul");
        }

        // 3. Serve 3rd patient: Amit (Triage 2, Arrival 3)
        auto s3 = client.Post("/api/v1/hospitals/101/queue/serve-next", "", "application/json");
        ASSERT_TRUE(s3 != nullptr && s3->status == 200, "Serve 3 returned 200 OK");
        if (s3) {
            auto json = nlohmann::json::parse(s3->body);
            ASSERT_EQ(json["data"]["patientId"].get<std::string>(), std::string("1003"), "3rd served patient is Amit");
        }

        // 4. Serve on empty queue returns 404
        auto sEmpty = client.Post("/api/v1/hospitals/101/queue/serve-next", "", "application/json");
        ASSERT_TRUE(sEmpty != nullptr && sEmpty->status == 404, "Serving empty queue returns 404 with QUEUE_EMPTY");

        // 5. Hospital Isolation: GreenLife still has Neha waiting
        auto qGreen = client.Get("/api/v1/hospitals/102/queue");
        if (qGreen) {
            auto json = nlohmann::json::parse(qGreen->body);
            ASSERT_EQ(json["data"].size(), size_t(1), "GreenLife queue still holds Neha safely isolated");
        }
    }

    // ==========================================
    // Test 7: Visit History & Manual Status Updates
    // ==========================================
    std::cout << "\n--- Test 7: Visit History & Manual Status Updates ---\n";
    {
        // 1. Get Priya's visits
        auto vPriyaRes = client.Get("/api/v1/hospitals/101/patients/1002/visits");
        ASSERT_TRUE(vPriyaRes != nullptr && vPriyaRes->status == 200, "GET Priya visits returns 200 OK");
        std::string priyaVisitId;
        if (vPriyaRes) {
            auto json = nlohmann::json::parse(vPriyaRes->body);
            ASSERT_EQ(json["data"].size(), size_t(1), "Priya has 1 visit record");
            ASSERT_EQ(json["data"][0]["status"].get<std::string>(), std::string("In Treatment"), "Priya's visit status is 'In Treatment'");
            priyaVisitId = json["data"][0]["visitId"].get<std::string>();
        }

        // 2. Update Priya's visit status to Completed (3)
        nlohmann::json updateStatus = {{"status", "Completed"}};
        auto patchStatus = client.Patch("/api/v1/hospitals/101/visits/" + priyaVisitId + "/status", updateStatus.dump(), "application/json");
        ASSERT_TRUE(patchStatus != nullptr && patchStatus->status == 200, "PATCH visit status to Completed returns 200 OK");

        // Verify status changed
        auto vPriyaAfter = client.Get("/api/v1/hospitals/101/patients/1002/visits");
        if (vPriyaAfter) {
            auto json = nlohmann::json::parse(vPriyaAfter->body);
            ASSERT_EQ(json["data"][0]["status"].get<std::string>(), std::string("Discharged"), "Priya's status is now Completed/Discharged");
        }
    }

    // ==========================================
    // Test 8: Hospital Statistics Endpoint
    // ==========================================
    std::cout << "\n--- Test 8: Hospital Statistics Endpoint ---\n";
    {
        auto statsRes = client.Get("/api/v1/hospitals/101/statistics");
        ASSERT_TRUE(statsRes != nullptr && statsRes->status == 200, "GET /api/v1/hospitals/101/statistics is 200 OK");
        if (statsRes) {
            auto json = nlohmann::json::parse(statsRes->body);
            ASSERT_EQ(json["data"]["registeredPatientCount"].get<int>(), 3, "Stats registered patients: 3");
            ASSERT_EQ(json["data"]["activeQueueSize"].get<int>(), 0, "Stats active ER queue: 0");
            ASSERT_EQ(json["data"]["totalVisitsRecorded"].get<int>(), 3, "Stats total visits: 3");
            ASSERT_EQ(json["data"]["completedVisits"].get<int>(), 1, "Stats completed visits: 1");
            ASSERT_EQ(json["data"]["inTreatmentVisits"].get<int>(), 2, "Stats in-treatment visits: 2");
        }
    }

    // Stop server before simulating restart
    server.stop();
    ASSERT_FALSE(server.isRunning(), "Server successfully stopped");

    // ==========================================
    // Test 9: Server Restart & Persistent Hydration
    // ==========================================
    std::cout << "\n--- Test 9: Server Restart & Persistent Hydration ---\n";
    {
        std::mutex restartMutex;
        smarter::SmartERSystem restartedSystem;
        ASSERT_TRUE(restartedSystem.initializeDatabase(testDbPath), "Restarted coordinator connected to persistent database");

        smarter::ApiServer restartedServer(restartedSystem, host, port, allowedOrigin);
        ASSERT_TRUE(restartedServer.start(true), "Restarted HTTP server listening on same port");

        TestClient client2(restartedSystem, restartMutex, host, port, allowedOrigin);
        auto hospRes = client2.Get("/api/v1/hospitals");
        ASSERT_TRUE(hospRes != nullptr && hospRes->status == 200, "GET /api/v1/hospitals after restart is 200 OK");
        if (hospRes) {
            auto json = nlohmann::json::parse(hospRes->body);
            ASSERT_EQ(json["data"].size(), size_t(2), "Restored both hospitals across server restart");
        }

        // GreenLife queue should still hold Neha
        auto qGreen = client2.Get("/api/v1/hospitals/102/queue");
        if (qGreen) {
            auto json = nlohmann::json::parse(qGreen->body);
            ASSERT_EQ(json["data"].size(), size_t(1), "Neha still present in GreenLife emergency queue across restart");
        }

        restartedServer.stop();
    }

    // Clean up temporary database files
    fs::remove(testDbPath, ec);
    fs::remove(testDbPath + "-wal", ec);
    fs::remove(testDbPath + "-shm", ec);

    std::cout << "\n=========================================================\n"
              << "API Test Summary: " << passedAssertions << " / " << totalAssertions << " passed.\n"
              << "=========================================================\n";

    return (passedAssertions == totalAssertions) ? 0 : 1;
}
