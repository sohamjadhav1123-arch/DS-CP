#include "smarter/SmartERSystem.h"
#include "smarter/DatabaseManager.h"
#include "smarter/ApiServer.h"

#include <iostream>
#include <string>
#include <csignal>

namespace {
smarter::ApiServer* globalServerPtr = nullptr;

void signalHandler(int signum) {
    std::cout << "\n>> Received shutdown signal (" << signum << "). Stopping API server...\n";
    if (globalServerPtr) {
        globalServerPtr->stop();
    }
}
} // anonymous namespace

int main(int argc, char* argv[]) {
    std::string host = "127.0.0.1";
    int port = 8080;
    std::string origin = "http://localhost:5173";

    // Parse optional command line flags: --port 8080 --host 127.0.0.1 --origin http://localhost:5173
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--port" && i + 1 < argc) {
            try {
                port = std::stoi(argv[++i]);
            } catch (...) {
                std::cerr << "Invalid port argument. Using default 8080.\n";
            }
        } else if (arg == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (arg == "--origin" && i + 1 < argc) {
            origin = argv[++i];
        }
    }

    smarter::SmartERSystem system;
    std::string dbPath = smarter::DatabaseManager::getDefaultDatabasePath();
    if (!system.initializeDatabase(dbPath)) {
        std::cerr << ">> [Warning] Could not initialize SQLite database at " << dbPath
                  << ". Operating in memory-only mode.\n";
    } else {
        std::cout << ">> SQLite Persistent Storage: Connected [" << dbPath << "]\n"
                  << ">> Restored " << system.getHospitalCount() << " hospital(s) from persistent storage.\n";
    }

    smarter::ApiServer server(system, host, port, origin);
    globalServerPtr = &server;

    // Register OS signal handlers for clean SIGINT / SIGTERM shutdown
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);

    if (!server.start(false)) {
        std::cerr << ">> Failed to start SmartER REST API server.\n";
        return 1;
    }

    std::cout << ">> SmartER REST API Server cleanly terminated. Goodbye!\n";
    return 0;
}

