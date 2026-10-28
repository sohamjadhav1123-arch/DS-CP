#include "smarter/Hospital.hpp"
#include "smarter/PatientRegistry.h"
#include <iostream>

namespace smarter {

Hospital::Hospital()
    : hospitalId_(""),
      name_(""),
      city_(""),
      totalEmergencyBeds_(0),
      emergencyQueue_("") {}

Hospital::Hospital(std::string hospitalId,
                   std::string name,
                   std::string city,
                   int totalEmergencyBeds)
    : hospitalId_(hospitalId),
      name_(std::move(name)),
      city_(std::move(city)),
      totalEmergencyBeds_(totalEmergencyBeds),
      emergencyQueue_(hospitalId) {}

const std::string& Hospital::getHospitalId() const { return hospitalId_; }
const std::string& Hospital::getName() const { return name_; }
const std::string& Hospital::getCity() const { return city_; }
int Hospital::getTotalEmergencyBeds() const { return totalEmergencyBeds_; }
size_t Hospital::getPatientCount() const { return patients_.size(); }

void Hospital::setName(const std::string& name) { name_ = name; }
void Hospital::setCity(const std::string& city) { city_ = city; }
void Hospital::setTotalEmergencyBeds(int beds) { totalEmergencyBeds_ = beds; }

EmergencyQueue& Hospital::getEmergencyQueue() {
    return emergencyQueue_;
}

const EmergencyQueue& Hospital::getEmergencyQueue() const {
    return emergencyQueue_;
}

void Hospital::bindRegistry(PatientRegistry* registry) {
    registry_ = registry;
}

PatientRegistry* Hospital::getRegistry() {
    return registry_;
}

const PatientRegistry* Hospital::getRegistry() const {
    return registry_;
}

bool Hospital::registerPatient(const Patient& patient) {
    if (registry_ != nullptr) {
        return registry_->insertPatient(patient);
    }

    // Fallback if standalone:
    if (hasPatient(patient.getPatientId())) {
        std::cerr << "[Warning] Patient with ID '" << patient.getPatientId()
                  << "' is already registered at hospital '" << name_
                  << "' (" << hospitalId_ << "). Registration rejected.\n";
        return false;
    }

    patients_.push_back(patient);
    return true;
}

const Patient* Hospital::findPatient(const std::string& patientId) const {
    if (registry_ != nullptr) {
        return registry_->findPatient(hospitalId_, patientId);
    }

    for (const auto& p : patients_) {
        if (p.getPatientId() == patientId) {
            return &p;
        }
    }
    return nullptr;
}

Patient* Hospital::findPatient(const std::string& patientId) {
    if (registry_ != nullptr) {
        return registry_->findPatient(hospitalId_, patientId);
    }

    for (auto& p : patients_) {
        if (p.getPatientId() == patientId) {
            return &p;
        }
    }
    return nullptr;
}

bool Hospital::hasPatient(const std::string& patientId) const {
    if (registry_ != nullptr) {
        return registry_->containsPatient(hospitalId_, patientId);
    }
    return findPatient(patientId) != nullptr;
}

void Hospital::printDetails(std::ostream& os) const {
    os << "==================================================\n"
       << "Hospital: " << name_ << " [" << hospitalId_ << "]\n"
       << "Location: " << city_ << "\n"
       << "ER Bed Capacity: " << totalEmergencyBeds_ << "\n"
       << "Registered Patients: " << patients_.size() << "\n"
       << "==================================================\n";
}

void Hospital::printAllPatients(std::ostream& os) const {
    os << "Patients registered at " << name_ << " (" << hospitalId_ << "):\n";
    if (patients_.empty()) {
        os << "  (No patients registered yet)\n";
        return;
    }
    for (const auto& p : patients_) {
        p.printDetails(os);
    }
}

} // namespace smarter

