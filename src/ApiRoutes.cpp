#include "smarter/ApiRoutes.h"
#include <iostream>
#include <regex>

namespace smarter {

nlohmann::json ApiRoutes::makeSuccess(const nlohmann::json& data) {
    return {
        {"success", true},
        {"data", data}
    };
}

nlohmann::json ApiRoutes::makeError(const std::string& code, const std::string& message) {
    return {
        {"success", false},
        {"error", {
            {"code", code},
            {"message", message}
        }}
    };
}

void ApiRoutes::applyCors(httplib::Response& res, const std::string& allowedOrigin) {
    res.set_header("Access-Control-Allow-Origin", allowedOrigin);
    res.set_header("Access-Control-Allow-Methods", "GET, POST, PATCH, DELETE, OPTIONS");
    res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization, X-Requested-With");
    res.set_header("Access-Control-Max-Age", "86400");
}

void ApiRoutes::sendJson(httplib::Response& res,
                         int statusCode,
                         const nlohmann::json& body,
                         const std::string& allowedOrigin) {
    applyCors(res, allowedOrigin);
    res.status = statusCode;
    res.set_content(body.dump(2), "application/json");
}

void ApiRoutes::registerRoutes(httplib::Server& server,
                               SmartERSystem& system,
                               std::mutex& systemMutex,
                               const std::string& allowedOrigin) {
    server.Get(R"(/api/.*)", [&system, &systemMutex, allowedOrigin](const httplib::Request& req, httplib::Response& res) {
        dispatch(req, res, system, systemMutex, allowedOrigin);
    });

    server.Post(R"(/api/.*)", [&system, &systemMutex, allowedOrigin](const httplib::Request& req, httplib::Response& res) {
        dispatch(req, res, system, systemMutex, allowedOrigin);
    });

    server.Patch(R"(/api/.*)", [&system, &systemMutex, allowedOrigin](const httplib::Request& req, httplib::Response& res) {
        dispatch(req, res, system, systemMutex, allowedOrigin);
    });

    server.Options(R"(/api/.*)", [allowedOrigin](const httplib::Request&, httplib::Response& res) {
        applyCors(res, allowedOrigin);
        res.status = 204;
    });
}

void ApiRoutes::dispatch(const httplib::Request& req,
                         httplib::Response& res,
                         SmartERSystem& system,
                         std::mutex& systemMutex,
                         const std::string& allowedOrigin) {

    // 1. Handle CORS Preflight OPTIONS
    if (req.method == "OPTIONS") {
        applyCors(res, allowedOrigin);
        res.status = 204;
        return;
    }

    std::smatch match;

    // ==========================================
    // GET Endpoints
    // ==========================================
    if (req.method == "GET") {
        // GET /api/v1/health
        if (req.path == "/api/v1/health") {
            std::lock_guard<std::mutex> lock(systemMutex);
            nlohmann::json data = {
                {"status", "healthy"},
                {"version", "1.0.0"},
                {"databaseConnected", system.isDatabaseEnabled()},
                {"hospitalCount", system.getHospitalCount()}
            };
            sendJson(res, 200, makeSuccess(data), allowedOrigin);
            return;
        }

        // GET /api/v1/hospitals
        if (req.path == "/api/v1/hospitals") {
            std::lock_guard<std::mutex> lock(systemMutex);
            auto hospitals = system.getAllHospitals();
            nlohmann::json list = nlohmann::json::array();
            for (const auto& h : hospitals) {
                auto stats = system.getHospitalStatistics(h.getHospitalId());
                list.push_back({
                    {"hospitalId", h.getHospitalId()},
                    {"name", h.getName()},
                    {"city", h.getCity()},
                    {"totalEmergencyBeds", h.getTotalEmergencyBeds()},
                    {"registeredPatientCount", stats.registeredPatientCount},
                    {"activeQueueSize", stats.activeQueueSize}
                });
            }
            sendJson(res, 200, makeSuccess(list), allowedOrigin);
            return;
        }

        // GET /api/v1/hospitals/{hospitalId}/patients
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/patients)"))) {
            std::string hospId = match[1].str();
            std::lock_guard<std::mutex> lock(systemMutex);

            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' was not found."), allowedOrigin);
                return;
            }

            auto patients = system.getPatientsForHospital(hospId);
            nlohmann::json list = nlohmann::json::array();
            for (const auto& p : patients) {
                list.push_back({
                    {"patientId", p.getPatientId()},
                    {"hospitalId", p.getHospitalId()},
                    {"name", p.getName()},
                    {"age", p.getAge()},
                    {"gender", p.getGender()},
                    {"bloodGroup", p.getBloodGroup()},
                    {"contactPhone", p.getContactPhone()}
                });
            }
            sendJson(res, 200, makeSuccess(list), allowedOrigin);
            return;
        }

        // GET /api/v1/hospitals/{hospitalId}/patients/{patientId}
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/patients/([^/]+))"))) {
            std::string hospId = match[1].str();
            std::string patId  = match[2].str();

            std::lock_guard<std::mutex> lock(systemMutex);
            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' was not found."), allowedOrigin);
                return;
            }

            const auto* p = system.findPatient(hospId, patId);
            if (!p) {
                sendJson(res, 404, makeError("PATIENT_NOT_FOUND", "Patient (" + hospId + ", " + patId + ") was not found."), allowedOrigin);
                return;
            }

            nlohmann::json data = {
                {"patientId", p->getPatientId()},
                {"hospitalId", p->getHospitalId()},
                {"name", p->getName()},
                {"age", p->getAge()},
                {"gender", p->getGender()},
                {"bloodGroup", p->getBloodGroup()},
                {"contactPhone", p->getContactPhone()}
            };
            sendJson(res, 200, makeSuccess(data), allowedOrigin);
            return;
        }

        // GET /api/v1/hospitals/{hospitalId}/queue
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/queue)"))) {
            std::string hospId = match[1].str();
            std::lock_guard<std::mutex> lock(systemMutex);

            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' was not found."), allowedOrigin);
                return;
            }

            auto entries = system.getEmergencyQueueEntries(hospId);
            nlohmann::json list = nlohmann::json::array();
            int rank = 1;
            for (const auto& e : entries) {
                const auto* p = system.findPatient(hospId, e.patientId);
                std::string pName = p ? p->getName() : "Unknown";
                list.push_back({
                    {"rank", rank++},
                    {"hospitalId", e.hospitalId},
                    {"patientId", e.patientId},
                    {"patientName", pName},
                    {"triageLevel", e.triageLevel},
                    {"triageSeverity", triageLevelToString(static_cast<TriageLevel>(e.triageLevel))},
                    {"arrivalSequence", e.arrivalSequence}
                });
            }
            sendJson(res, 200, makeSuccess(list), allowedOrigin);
            return;
        }

        // GET /api/v1/hospitals/{hospitalId}/patients/{patientId}/visits
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/patients/([^/]+)/visits)"))) {
            std::string hospId = match[1].str();
            std::string patId  = match[2].str();

            std::lock_guard<std::mutex> lock(systemMutex);
            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' was not found."), allowedOrigin);
                return;
            }

            if (!system.hasPatient(hospId, patId)) {
                sendJson(res, 404, makeError("PATIENT_NOT_FOUND", "Patient (" + hospId + ", " + patId + ") was not found."), allowedOrigin);
                return;
            }

            auto visits = system.getPatientVisits(hospId, patId);
            nlohmann::json list = nlohmann::json::array();
            for (const auto& v : visits) {
                list.push_back({
                    {"visitId", v.getVisitId()},
                    {"hospitalId", v.getHospitalId()},
                    {"patientId", v.getPatientId()},
                    {"arrivalTime", v.getArrivalTime()},
                    {"triageLevel", static_cast<int>(v.getTriageLevel())},
                    {"triageSeverity", triageLevelToString(v.getTriageLevel())},
                    {"chiefComplaint", v.getChiefComplaint()},
                    {"diagnosis", v.getDiagnosis()},
                    {"status", v.getStatusString()},
                    {"notes", v.getNotes()},
                    {"arrivalSequence", v.getArrivalSequence()}
                });
            }
            sendJson(res, 200, makeSuccess(list), allowedOrigin);
            return;
        }

        // GET /api/v1/hospitals/{hospitalId}/visits
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/visits)"))) {
            std::string hospId = match[1].str();
            std::lock_guard<std::mutex> lock(systemMutex);

            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' was not found."), allowedOrigin);
                return;
            }

            auto visits = system.getHospitalVisits(hospId);
            nlohmann::json list = nlohmann::json::array();
            for (const auto& v : visits) {
                const auto* p = system.findPatient(hospId, v.getPatientId());
                std::string pName = p ? p->getName() : "Unknown";
                list.push_back({
                    {"visitId", v.getVisitId()},
                    {"hospitalId", v.getHospitalId()},
                    {"patientId", v.getPatientId()},
                    {"patientName", pName},
                    {"arrivalTime", v.getArrivalTime()},
                    {"triageLevel", static_cast<int>(v.getTriageLevel())},
                    {"triageSeverity", triageLevelToString(v.getTriageLevel())},
                    {"chiefComplaint", v.getChiefComplaint()},
                    {"diagnosis", v.getDiagnosis()},
                    {"status", v.getStatusString()},
                    {"notes", v.getNotes()},
                    {"arrivalSequence", v.getArrivalSequence()}
                });
            }
            sendJson(res, 200, makeSuccess(list), allowedOrigin);
            return;
        }

        // GET /api/v1/hospitals/{hospitalId}/statistics
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/statistics)"))) {
            std::string hospId = match[1].str();
            std::lock_guard<std::mutex> lock(systemMutex);

            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' was not found."), allowedOrigin);
                return;
            }

            HospitalStats stats = system.getHospitalStatistics(hospId);
            nlohmann::json data = {
                {"hospitalId", stats.hospitalId},
                {"hospitalName", stats.hospitalName},
                {"city", stats.city},
                {"totalEmergencyBeds", stats.totalEmergencyBeds},
                {"registeredPatientCount", stats.registeredPatientCount},
                {"activeQueueSize", stats.activeQueueSize},
                {"totalVisitsRecorded", stats.totalVisitsRecorded},
                {"waitingVisits", stats.waitingVisits},
                {"inTreatmentVisits", stats.inTreatmentVisits},
                {"completedVisits", stats.completedVisits}
            };
            sendJson(res, 200, makeSuccess(data), allowedOrigin);
            return;
        }
    }

    // ==========================================
    // POST Endpoints
    // ==========================================
    if (req.method == "POST") {
        // POST /api/v1/hospitals
        if (req.path == "/api/v1/hospitals") {
            nlohmann::json body;
            try {
                body = nlohmann::json::parse(req.body);
            } catch (...) {
                sendJson(res, 400, makeError("MALFORMED_JSON", "Request body must be valid JSON."), allowedOrigin);
                return;
            }

            if (!body.contains("hospitalId") || !body.contains("name") ||
                !body.contains("city") || !body.contains("totalEmergencyBeds")) {
                sendJson(res, 400, makeError("MISSING_FIELDS", "Required fields: hospitalId, name, city, totalEmergencyBeds."), allowedOrigin);
                return;
            }

            if (!body["hospitalId"].is_string() || !body["name"].is_string() ||
                !body["city"].is_string() || !body["totalEmergencyBeds"].is_number_integer()) {
                sendJson(res, 400, makeError("INVALID_TYPES", "Invalid data types for hospital fields."), allowedOrigin);
                return;
            }

            std::string hospId = body["hospitalId"].get<std::string>();
            std::string name   = body["name"].get<std::string>();
            std::string city   = body["city"].get<std::string>();
            int beds           = body["totalEmergencyBeds"].get<int>();

            if (hospId.empty() || name.empty() || city.empty() || beds <= 0) {
                sendJson(res, 400, makeError("INVALID_VALUES", "Fields cannot be empty and totalEmergencyBeds must be > 0."), allowedOrigin);
                return;
            }

            std::lock_guard<std::mutex> lock(systemMutex);
            if (system.hasHospital(hospId)) {
                sendJson(res, 409, makeError("DUPLICATE_HOSPITAL", "A hospital with ID '" + hospId + "' is already registered."), allowedOrigin);
                return;
            }

            if (!system.registerHospital(hospId, name, city, beds)) {
                sendJson(res, 500, makeError("REGISTRATION_FAILED", "Failed to register hospital in system storage."), allowedOrigin);
                return;
            }

            nlohmann::json respData = {
                {"hospitalId", hospId},
                {"name", name},
                {"city", city},
                {"totalEmergencyBeds", beds}
            };
            sendJson(res, 201, makeSuccess(respData), allowedOrigin);
            return;
        }

        // POST /api/v1/hospitals/{hospitalId}/patients
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/patients)"))) {
            std::string hospId = match[1].str();
            nlohmann::json body;
            try {
                body = nlohmann::json::parse(req.body);
            } catch (...) {
                sendJson(res, 400, makeError("MALFORMED_JSON", "Request body must be valid JSON."), allowedOrigin);
                return;
            }

            if (!body.contains("patientId") || !body.contains("name") ||
                !body.contains("age") || !body.contains("gender") ||
                !body.contains("bloodGroup") || !body.contains("contactPhone")) {
                sendJson(res, 400, makeError("MISSING_FIELDS", "Required fields: patientId, name, age, gender, bloodGroup, contactPhone."), allowedOrigin);
                return;
            }

            if (!body["patientId"].is_string() || !body["name"].is_string() ||
                !body["age"].is_number_integer() || !body["gender"].is_string() ||
                !body["bloodGroup"].is_string() || !body["contactPhone"].is_string()) {
                sendJson(res, 400, makeError("INVALID_TYPES", "Invalid data types supplied for patient fields."), allowedOrigin);
                return;
            }

            std::string patId  = body["patientId"].get<std::string>();
            std::string name   = body["name"].get<std::string>();
            int age            = body["age"].get<int>();
            std::string gender = body["gender"].get<std::string>();
            std::string blood  = body["bloodGroup"].get<std::string>();
            std::string phone  = body["contactPhone"].get<std::string>();

            if (patId.empty() || name.empty() || age < 0 || age > 130) {
                sendJson(res, 400, makeError("INVALID_VALUES", "Invalid patient demographic values."), allowedOrigin);
                return;
            }

            std::lock_guard<std::mutex> lock(systemMutex);
            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' does not exist."), allowedOrigin);
                return;
            }

            if (system.hasPatient(hospId, patId)) {
                sendJson(res, 409, makeError("DUPLICATE_PATIENT", "Patient with ID '" + patId + "' already exists in Hospital '" + hospId + "'."), allowedOrigin);
                return;
            }

            if (!system.registerPatient(hospId, patId, name, age, gender, blood, phone)) {
                sendJson(res, 500, makeError("REGISTRATION_FAILED", "Failed to register patient in system storage."), allowedOrigin);
                return;
            }

            nlohmann::json respData = {
                {"patientId", patId},
                {"hospitalId", hospId},
                {"name", name},
                {"age", age},
                {"gender", gender},
                {"bloodGroup", blood},
                {"contactPhone", phone}
            };
            sendJson(res, 201, makeSuccess(respData), allowedOrigin);
            return;
        }

        // POST /api/v1/hospitals/{hospitalId}/queue/serve-next
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/queue/serve-next)"))) {
            std::string hospId = match[1].str();
            std::lock_guard<std::mutex> lock(systemMutex);

            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' was not found."), allowedOrigin);
                return;
            }

            auto servedOpt = system.serveNextEmergencyPatient(hospId);
            if (!servedOpt.has_value()) {
                sendJson(res, 404, makeError("QUEUE_EMPTY", "Emergency queue for Hospital '" + hospId + "' is currently empty."), allowedOrigin);
                return;
            }

            const auto& s = *servedOpt;
            const auto* p = system.findPatient(hospId, s.patientId);
            std::string pName = p ? p->getName() : "Unknown";

            nlohmann::json data = {
                {"patientId", s.patientId},
                {"hospitalId", s.hospitalId},
                {"patientName", pName},
                {"triageLevel", s.triageLevel},
                {"triageSeverity", triageLevelToString(static_cast<TriageLevel>(s.triageLevel))},
                {"arrivalSequence", s.arrivalSequence},
                {"status", "InTreatment"}
            };
            sendJson(res, 200, makeSuccess(data), allowedOrigin);
            return;
        }

        // POST /api/v1/hospitals/{hospitalId}/patients/{patientId}/visits
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/patients/([^/]+)/visits)"))) {
            std::string hospId = match[1].str();
            std::string patId  = match[2].str();

            nlohmann::json body;
            try {
                body = nlohmann::json::parse(req.body);
            } catch (...) {
                sendJson(res, 400, makeError("MALFORMED_JSON", "Request body must be valid JSON."), allowedOrigin);
                return;
            }

            if (!body.contains("triageLevel") || !body.contains("chiefComplaint")) {
                sendJson(res, 400, makeError("MISSING_FIELDS", "Required fields: triageLevel (1-5), chiefComplaint."), allowedOrigin);
                return;
            }

            if (!body["triageLevel"].is_number_integer() || !body["chiefComplaint"].is_string()) {
                sendJson(res, 400, makeError("INVALID_TYPES", "triageLevel must be an integer, chiefComplaint must be a string."), allowedOrigin);
                return;
            }

            int triage = body["triageLevel"].get<int>();
            if (triage < 1 || triage > 5) {
                sendJson(res, 400, makeError("INVALID_TRIAGE_LEVEL", "Triage level must be between 1 and 5."), allowedOrigin);
                return;
            }

            std::string complaint = body["chiefComplaint"].get<std::string>();
            bool admitER = body.value("admitToEmergencyQueue", true);
            std::string arrivalTime = body.value("arrivalTime", "");

            std::lock_guard<std::mutex> lock(systemMutex);
            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' was not found."), allowedOrigin);
                return;
            }

            if (!system.hasPatient(hospId, patId)) {
                sendJson(res, 404, makeError("PATIENT_NOT_FOUND", "Patient (" + hospId + ", " + patId + ") is not registered."), allowedOrigin);
                return;
            }

            if (admitER) {
                Hospital* hosp = system.getHospital(hospId);
                if (hosp && hosp->getEmergencyQueue().hasPatient(patId)) {
                    sendJson(res, 409, makeError("PATIENT_ALREADY_IN_QUEUE", "Patient is already waiting in the emergency queue for this hospital."), allowedOrigin);
                    return;
                }

                if (!system.admitPatientToEmergency(hospId, patId, triage, complaint, arrivalTime)) {
                    sendJson(res, 500, makeError("ADMISSION_FAILED", "Failed to admit patient to emergency room."), allowedOrigin);
                    return;
                }
            } else {
                std::string vId = "VIS-" + hospId + "-" + patId + "-" + std::to_string(std::rand());
                std::string time = arrivalTime.empty() ? "2026-10-23 10:00" : arrivalTime;
                Visit v(vId, patId, hospId, time, static_cast<TriageLevel>(triage), complaint, "Consultation logged", Visit::Status::Waiting);
                if (!system.addVisit(v)) {
                    sendJson(res, 500, makeError("VISIT_RECORD_FAILED", "Failed to record non-ER visit."), allowedOrigin);
                    return;
                }
            }

            auto visits = system.getPatientVisits(hospId, patId);
            nlohmann::json latestVisitData = nlohmann::json::object();
            if (!visits.empty()) {
                const auto& lv = visits.back();
                latestVisitData = {
                    {"visitId", lv.getVisitId()},
                    {"patientId", lv.getPatientId()},
                    {"hospitalId", lv.getHospitalId()},
                    {"arrivalTime", lv.getArrivalTime()},
                    {"triageLevel", static_cast<int>(lv.getTriageLevel())},
                    {"chiefComplaint", lv.getChiefComplaint()},
                    {"status", lv.getStatusString()},
                    {"admittedToEmergencyQueue", admitER}
                };
            }

            sendJson(res, 201, makeSuccess(latestVisitData), allowedOrigin);
            return;
        }
    }

    // ==========================================
    // PATCH Endpoints
    // ==========================================
    if (req.method == "PATCH") {
        // PATCH /api/v1/hospitals/{hospitalId}/patients/{patientId}/triage
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/patients/([^/]+)/triage)"))) {
            std::string hospId = match[1].str();
            std::string patId  = match[2].str();

            nlohmann::json body;
            try {
                body = nlohmann::json::parse(req.body);
            } catch (...) {
                sendJson(res, 400, makeError("MALFORMED_JSON", "Request body must be valid JSON."), allowedOrigin);
                return;
            }

            if (!body.contains("triageLevel") || !body["triageLevel"].is_number_integer()) {
                sendJson(res, 400, makeError("MISSING_FIELDS", "Field 'triageLevel' (integer 1-5) is required."), allowedOrigin);
                return;
            }

            int newTriage = body["triageLevel"].get<int>();
            if (newTriage < 1 || newTriage > 5) {
                sendJson(res, 400, makeError("INVALID_TRIAGE_LEVEL", "Triage level must be between 1 and 5."), allowedOrigin);
                return;
            }

            std::lock_guard<std::mutex> lock(systemMutex);
            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' was not found."), allowedOrigin);
                return;
            }

            if (!system.updateEmergencyTriage(hospId, patId, newTriage)) {
                sendJson(res, 404, makeError("PATIENT_NOT_IN_QUEUE", "Patient (" + hospId + ", " + patId + ") is not waiting in the emergency queue."), allowedOrigin);
                return;
            }

            nlohmann::json data = {
                {"patientId", patId},
                {"hospitalId", hospId},
                {"triageLevel", newTriage},
                {"triageSeverity", triageLevelToString(static_cast<TriageLevel>(newTriage))}
            };
            sendJson(res, 200, makeSuccess(data), allowedOrigin);
            return;
        }

        // PATCH /api/v1/hospitals/{hospitalId}/visits/{visitId}/status
        if (std::regex_match(req.path, match, std::regex(R"(/api/v1/hospitals/([^/]+)/visits/([^/]+)/status)"))) {
            std::string hospId  = match[1].str();
            std::string visitId = match[2].str();

            nlohmann::json body;
            try {
                body = nlohmann::json::parse(req.body);
            } catch (...) {
                sendJson(res, 400, makeError("MALFORMED_JSON", "Request body must be valid JSON."), allowedOrigin);
                return;
            }

            if (!body.contains("status")) {
                sendJson(res, 400, makeError("MISSING_FIELDS", "Field 'status' is required."), allowedOrigin);
                return;
            }

            Visit::Status newStatus;
            if (body["status"].is_number_integer()) {
                int statInt = body["status"].get<int>();
                if (statInt < 0 || statInt > 3) {
                    sendJson(res, 400, makeError("INVALID_STATUS", "Status integer must be between 0 and 3."), allowedOrigin);
                    return;
                }
                newStatus = static_cast<Visit::Status>(statInt);
            } else if (body["status"].is_string()) {
                std::string statStr = body["status"].get<std::string>();
                if (statStr == "Waiting") newStatus = Visit::Status::Waiting;
                else if (statStr == "InTreatment" || statStr == "In Treatment") newStatus = Visit::Status::InTreatment;
                else if (statStr == "Admitted") newStatus = Visit::Status::Admitted;
                else if (statStr == "Completed" || statStr == "Discharged") newStatus = Visit::Status::Completed;
                else {
                    sendJson(res, 400, makeError("INVALID_STATUS", "Unknown status string: " + statStr), allowedOrigin);
                    return;
                }
            } else {
                sendJson(res, 400, makeError("INVALID_TYPES", "Status must be integer or string."), allowedOrigin);
                return;
            }

            std::lock_guard<std::mutex> lock(systemMutex);
            if (!system.hasHospital(hospId)) {
                sendJson(res, 404, makeError("HOSPITAL_NOT_FOUND", "Hospital '" + hospId + "' was not found."), allowedOrigin);
                return;
            }

            if (!system.updateVisitStatus(hospId, visitId, newStatus)) {
                sendJson(res, 404, makeError("VISIT_NOT_FOUND", "Visit '" + visitId + "' was not found in Hospital '" + hospId + "'."), allowedOrigin);
                return;
            }

            nlohmann::json data = {
                {"hospitalId", hospId},
                {"visitId", visitId},
                {"status", static_cast<int>(newStatus)}
            };
            sendJson(res, 200, makeSuccess(data), allowedOrigin);
            return;
        }
    }

    // Route not found
    sendJson(res, 404, makeError("ROUTE_NOT_FOUND", "No matching API route for " + req.method + " " + req.path), allowedOrigin);
}

} // namespace smarter
