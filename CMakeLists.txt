cmake_minimum_required(VERSION 3.15)
project(SmartER
    VERSION 1.0.0
    DESCRIPTION "Hospital Emergency Queue and Patient History Management System"
    LANGUAGES CXX
)

# Standard C++17 configuration
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warning flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Locate SQLite3 official library
find_package(SQLite3 REQUIRED)

# Network socket libraries on Windows (for cpp-httplib)
set(PLATFORM_NETWORK_LIBS "")
if(WIN32)
    set(PLATFORM_NETWORK_LIBS ws2_32 crypt32)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core source files
set(SMARTER_SOURCES
    src/Patient.cpp
    src/Visit.cpp
    src/Hospital.cpp
    src/EmergencyQueue.cpp
    src/PatientRegistry.cpp
    src/PatientBST.cpp
    src/VisitHistory.cpp
    src/DatabaseManager.cpp
    src/SmartERSystem.cpp
    src/ApiRoutes.cpp
    src/ApiServer.cpp
)

# Main console demonstration application
add_executable(smarter_app
    src/main.cpp
    ${SMARTER_SOURCES}
)
target_link_libraries(smarter_app PRIVATE SQLite3::SQLite3 ${PLATFORM_NETWORK_LIBS})

# HTTP REST API Microservice executable
add_executable(smarter_server
    src/server_main.cpp
    ${SMARTER_SOURCES}
)
target_link_libraries(smarter_server PRIVATE SQLite3::SQLite3 ${PLATFORM_NETWORK_LIBS})

# Test executables
add_executable(test_emergency_queue
    tests/test_emergency_queue.cpp
    ${SMARTER_SOURCES}
)
target_link_libraries(test_emergency_queue PRIVATE SQLite3::SQLite3 ${PLATFORM_NETWORK_LIBS})

add_executable(test_patient_registry
    tests/test_patient_registry.cpp
    ${SMARTER_SOURCES}
)
target_link_libraries(test_patient_registry PRIVATE SQLite3::SQLite3 ${PLATFORM_NETWORK_LIBS})

add_executable(test_patient_bst
    tests/test_patient_bst.cpp
    ${SMARTER_SOURCES}
)
target_link_libraries(test_patient_bst PRIVATE SQLite3::SQLite3 ${PLATFORM_NETWORK_LIBS})

add_executable(test_visit_history
    tests/test_visit_history.cpp
    ${SMARTER_SOURCES}
)
target_link_libraries(test_visit_history PRIVATE SQLite3::SQLite3 ${PLATFORM_NETWORK_LIBS})

add_executable(test_system_integration
    tests/test_system_integration.cpp
    ${SMARTER_SOURCES}
)
target_link_libraries(test_system_integration PRIVATE SQLite3::SQLite3 ${PLATFORM_NETWORK_LIBS})

add_executable(test_database_persistence
    tests/test_database_persistence.cpp
    ${SMARTER_SOURCES}
)
target_link_libraries(test_database_persistence PRIVATE SQLite3::SQLite3 ${PLATFORM_NETWORK_LIBS})

add_executable(test_api_server
    tests/test_api_server.cpp
    ${SMARTER_SOURCES}
)
target_link_libraries(test_api_server PRIVATE SQLite3::SQLite3 ${PLATFORM_NETWORK_LIBS})

# Set output directory
set_target_properties(
    smarter_app
    smarter_server
    test_emergency_queue
    test_patient_registry
    test_patient_bst
    test_visit_history
    test_system_integration
    test_database_persistence
    test_api_server
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

# Enable CTest
enable_testing()
add_test(NAME EmergencyQueueTests COMMAND test_emergency_queue)
add_test(NAME PatientRegistryTests COMMAND test_patient_registry)
add_test(NAME PatientBSTTests COMMAND test_patient_bst)
add_test(NAME VisitHistoryTests COMMAND test_visit_history)
add_test(NAME SystemIntegrationTests COMMAND test_system_integration)
add_test(NAME DatabasePersistenceTests COMMAND test_database_persistence)
add_test(NAME ApiServerTests COMMAND test_api_server)
