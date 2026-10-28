#pragma once

#include "smarter/SmartERSystem.h"
#include "third_party/httplib.h"

#include <string>
#include <memory>
#include <mutex>
#include <thread>
#include <atomic>

namespace smarter {

/**
 * @brief HTTP REST API Server for the SmartER Hospital System.
 * 
 * Provides an HTTP microservice layer exposing hospital emergency queues,
 * patient records, and visit histories to future web frontend clients.
 * 
 * Features:
 *  - Configurable host binding (defaults to 127.0.0.1 for local isolation).
 *  - Configurable port (defaults to 8080).
 *  - CORS support targeting React/Vite development environments (http://localhost:5173).
 *  - Thread-safe synchronization wrapping canonical SmartERSystem.
 *  - Graceful start, stop, and teardown with port binding error detection.
 */
class ApiServer {
public:
    explicit ApiServer(SmartERSystem& system,
                       std::string host = "127.0.0.1",
                       int port = 8080,
                       std::string allowedOrigin = "http://localhost:5173");
    ~ApiServer();

    // Move-only semantics
    ApiServer(const ApiServer&) = delete;
    ApiServer& operator=(const ApiServer&) = delete;

    /**
     * @brief Launches the HTTP server.
     * @param async If true, runs on a background worker thread and returns immediately.
     *              If false, blocks the calling thread until stop() or SIGINT.
     * @return true if successfully bound and listening; false if port conflict or failure.
     */
    bool start(bool async = false);

    /**
     * @brief Gracefully terminates the running server and joins background threads.
     */
    void stop();

    bool isRunning() const;
    int getPort() const;
    const std::string& getHost() const;
    const std::string& getAllowedOrigin() const;

private:
    SmartERSystem& system_;
    std::string host_;
    int port_;
    std::string allowedOrigin_;

    std::unique_ptr<httplib::Server> server_;
    std::mutex systemMutex_;
    std::atomic<bool> isRunning_{false};
    std::thread workerThread_;
};

} // namespace smarter

