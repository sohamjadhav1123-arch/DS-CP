#include "smarter/Patient.hpp"
#include <iomanip>

namespace smarter {

Patient::Patient()
    : patientId_(""),
      hospitalId_(""),
      name_(""),
      age_(0),
      gender_("Unknown"),
      bloodGroup_("Unknown"),
      contactPhone_("") {}

Patient::Patient(std::string patientId,
                 std::string hospitalId,
                 std::string name,
                 int age,
                 std::string gender,
                 std::string bloodGroup,
                 std::string contactPhone)
    : patientId_(std::move(patientId)),
      hospitalId_(std::move(hospitalId)),
      name_(std::move(name)),
      age_(age),
      gender_(std::move(gender)),
      bloodGroup_(std::move(bloodGroup)),
      contactPhone_(std::move(contactPhone)) {}

const std::string& Patient::getPatientId() const { return patientId_; }
const std::string& Patient::getHospitalId() const { return hospitalId_; }
const std::string& Patient::getName() const { return name_; }
int Patient::getAge() const { return age_; }
const std::string& Patient::getGender() const { return gender_; }
const std::string& Patient::getBloodGroup() const { return bloodGroup_; }
const std::string& Patient::getContactPhone() const { return contactPhone_; }

std::string Patient::getCompositeKey() const {
    return hospitalId_ + ":" + patientId_;
}

void Patient::setName(const std::string& name) { name_ = name; }
void Patient::setAge(int age) { age_ = age; }
void Patient::setGender(const std::string& gender) { gender_ = gender; }
void Patient::setBloodGroup(const std::string& bloodGroup) { bloodGroup_ = bloodGroup; }
void Patient::setContactPhone(const std::string& contactPhone) { contactPhone_ = contactPhone; }

void Patient::addVisit(const Visit& visit) {
    visits_.push_back(visit);
}

const std::vector<Visit>& Patient::getVisits() const {
    return visits_;
}

size_t Patient::getVisitCount() const {
    return visits_.size();
}

void Patient::printDetails(std::ostream& os) const {
    os << "--------------------------------------------------\n"
       << "Patient ID:      " << patientId_ << "\n"
       << "Hospital ID:     " << hospitalId_ << "\n"
       << "Global Ref:      " << getCompositeKey() << "\n"
       << "Name:            " << name_ << "\n"
       << "Age / Gender:    " << age_ << " / " << gender_ << "\n"
       << "Blood Group:     " << bloodGroup_ << "\n"
       << "Contact:         " << contactPhone_ << "\n"
       << "Total Visits:    " << visits_.size() << "\n";
    if (!visits_.empty()) {
        os << "Visit History:\n";
        for (const auto& v : visits_) {
            v.printSummary(os);
        }
    }
    os << "--------------------------------------------------\n";
}

} // namespace smarter

