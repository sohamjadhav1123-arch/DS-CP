#pragma once

#include "smarter/SmartERSystem.h"
#include "third_party/httplib.h"
#include "third_party/json.hpp"

#include <mutex>
#include <string>

namespace smarter {

/**
 * @brief Registers and handles HTTP REST API routes for SmartER.
 * 
 * Maps HTTP requests to canonical SmartERSystem coordinator actions,
 * enforcing JSON validation, HTTP status semantics, CORS policies,
 * and error payload formatting.
 */
class ApiRoutes {
public:
    static void registerRoutes(httplib::Server& server,
                               SmartERSystem& system,
                               std::mutex& systemMutex,
                               const std::string& allowedOrigin = "http://localhost:5173");

    /**
     * @brief Core request router and dispatcher for both live HTTP connections
     * and in-process testing.
     */
    static void dispatch(const httplib::Request& req,
                         httplib::Response& res,
                         SmartERSystem& system,
                         std::mutex& systemMutex,
                         const std::string& allowedOrigin = "http://localhost:5173");

    // JSON Helper Utilities
    static nlohmann::json makeSuccess(const nlohmann::json& data = nlohmann::json::object());
    static nlohmann::json makeError(const std::string& code, const std::string& message);
    static void sendJson(httplib::Response& res,
                         int statusCode,
                         const nlohmann::json& body,
                         const std::string& allowedOrigin = "http://localhost:5173");
    static void applyCors(httplib::Response& res,
                          const std::string& allowedOrigin = "http://localhost:5173");
};

} // namespace smarter
