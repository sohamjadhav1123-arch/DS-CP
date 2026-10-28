#pragma once

#include "smarter/Visit.hpp"
#include <string>
#include <vector>
#include <iostream>

namespace smarter {

/**
 * @brief Represents a patient registered at a specific hospital.
 * 
 * Scoping Rule:
 * Patient IDs (e.g. "P101") are unique WITHIN a given hospital,
 * but identical IDs may exist at another hospital. Therefore, a patient
 * is strictly identified within a hospital scope by patientId, or globally
 * by the composite pair (hospitalId, patientId).
 * 
 * Data Structure Roadmap:
 * In future stages:
 *  - Fast O(1) patient lookup within a hospital will use a custom Hash Table.
 *  - Historical visits for a patient will be tracked via a custom Linked List.
 */
class Patient {
public:
    Patient();

    Patient(std::string patientId,
            std::string hospitalId,
            std::string name,
            int age,
            std::string gender,
            std::string bloodGroup,
            std::string contactPhone);

    // Getters
    const std::string& getPatientId() const;
    const std::string& getHospitalId() const;
    const std::string& getName() const;
    int getAge() const;
    const std::string& getGender() const;
    const std::string& getBloodGroup() const;
    const std::string& getContactPhone() const;

    // Composite key representation: e.g. "HOSP-01:P101"
    std::string getCompositeKey() const;

    // Setters
    void setName(const std::string& name);
    void setAge(int age);
    void setGender(const std::string& gender);
    void setBloodGroup(const std::string& bloodGroup);
    void setContactPhone(const std::string& contactPhone);

    // Visit history references (in initial stage, vector of visits for demonstration;
    // to be replaced by custom Linked List in future stages)
    void addVisit(const Visit& visit);
    const std::vector<Visit>& getVisits() const;
    size_t getVisitCount() const;

    // Helpers
    void printDetails(std::ostream& os = std::cout) const;

private:
    std::string patientId_;
    std::string hospitalId_;
    std::string name_;
    int age_;
    std::string gender_;
    std::string bloodGroup_;
    std::string contactPhone_;

    // Temporary storage for visits during initial phase;
    // will be migrated to custom Linked List data structure in Stage 2/3.
    std::vector<Visit> visits_;
};

} // namespace smarter

