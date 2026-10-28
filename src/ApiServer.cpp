#include "smarter/ApiServer.h"
#include "smarter/ApiRoutes.h"
#include <iostream>
#include <chrono>

namespace smarter {

ApiServer::ApiServer(SmartERSystem& system,
                     std::string host,
                     int port,
                     std::string allowedOrigin)
    : system_(system),
      host_(std::move(host)),
      port_(port),
      allowedOrigin_(std::move(allowedOrigin)),
      server_(nullptr),
      isRunning_(false) {}

ApiServer::~ApiServer() {
    stop();
}

bool ApiServer::start(bool async) {
    if (isRunning_) return true;

    server_ = std::make_unique<httplib::Server>();
    ApiRoutes::registerRoutes(*server_, system_, systemMutex_, allowedOrigin_);

    // Attempt binding to verify port availability
    if (!server_->bind_to_port(host_.c_str(), port_)) {
        std::cerr << "[ApiServer Error] Failed to bind to " << host_ << ":" << port_
                  << " (Port already in use or socket permission denied).\n";
        server_.reset();
        return false;
    }

    isRunning_ = true;

    if (async) {
        workerThread_ = std::thread([this]() {
            server_->listen_after_bind();
            isRunning_ = false;
        });
        // Brief pause to allow socket loop startup
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return true;
    }

    std::cout << "\n=========================================================\n"
              << "       SmartER HTTP REST API Microservice (C++17)        \n"
              << "=========================================================\n"
              << ">> Server listening at: http://" << host_ << ":" << port_ << "\n"
              << ">> Base API prefix:     http://" << host_ << ":" << port_ << "/api/v1\n"
              << ">> CORS Origin Allowed: " << allowedOrigin_ << "\n"
              << ">> Active Data Store:   SQLite [Ready for React Frontend]\n"
              << ">> Press Ctrl+C to stop.\n"
              << "=========================================================\n" << std::endl;

    server_->listen_after_bind();
    isRunning_ = false;
    return true;
}

void ApiServer::stop() {
    if (server_ && isRunning_) {
        server_->stop();
    }
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
    isRunning_ = false;
}

bool ApiServer::isRunning() const {
    return isRunning_;
}

int ApiServer::getPort() const {
    return port_;
}

const std::string& ApiServer::getHost() const {
    return host_;
}

const std::string& ApiServer::getAllowedOrigin() const {
    return allowedOrigin_;
}

} // namespace smarter

