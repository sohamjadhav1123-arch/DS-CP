#include "smarter/Visit.hpp"
#include <iomanip>

namespace smarter {

Visit::Visit()
    : visitId_(""),
      patientId_("") ,
      hospitalId_(""),
      arrivalTime_(""),
      triageLevel_(TriageLevel::NonUrgent),
      chiefComplaint_(""),
      diagnosis_(""),
      status_(Status::Waiting),
      notes_("") {}

Visit::Visit(std::string visitId,
             std::string patientId,
             std::string hospitalId,
             std::string arrivalTime,
             TriageLevel triageLevel,
             std::string chiefComplaint,
             std::string diagnosis,
             Status status,
             std::string notes,
             int arrivalSequence,
             bool inEmergencyQueue)
    : visitId_(std::move(visitId)),
      patientId_(std::move(patientId)),
      hospitalId_(std::move(hospitalId)),
      arrivalTime_(std::move(arrivalTime)),
      triageLevel_(triageLevel),
      chiefComplaint_(std::move(chiefComplaint)),
      diagnosis_(std::move(diagnosis)),
      status_(status),
      notes_(std::move(notes)),
      arrivalSequence_(arrivalSequence),
      inEmergencyQueue_(inEmergencyQueue) {}

const std::string& Visit::getVisitId() const { return visitId_; }
const std::string& Visit::getPatientId() const { return patientId_; }
const std::string& Visit::getHospitalId() const { return hospitalId_; }
const std::string& Visit::getArrivalTime() const { return arrivalTime_; }
TriageLevel Visit::getTriageLevel() const { return triageLevel_; }
const std::string& Visit::getChiefComplaint() const { return chiefComplaint_; }
const std::string& Visit::getDiagnosis() const { return diagnosis_; }
Visit::Status Visit::getStatus() const { return status_; }
const std::string& Visit::getNotes() const { return notes_; }
int Visit::getArrivalSequence() const { return arrivalSequence_; }
bool Visit::isInEmergencyQueue() const { return inEmergencyQueue_; }

void Visit::setDiagnosis(const std::string& diagnosis) { diagnosis_ = diagnosis; }
void Visit::setStatus(Status status) { status_ = status; }
void Visit::setNotes(const std::string& notes) { notes_ = notes; }
void Visit::setTriageLevel(TriageLevel level) { triageLevel_ = level; }
void Visit::setArrivalSequence(int seq) { arrivalSequence_ = seq; }
void Visit::setInEmergencyQueue(bool inQueue) { inEmergencyQueue_ = inQueue; }

std::string Visit::getStatusString() const {
    switch (status_) {
        case Status::Waiting:     return "Waiting";
        case Status::InTreatment: return "In Treatment";
        case Status::Admitted:    return "Admitted";
        case Status::Discharged:  return "Discharged";
        default:                  return "Unknown";
    }
}

void Visit::printSummary(std::ostream& os) const {
    os << "  [Visit " << visitId_ << " | " << arrivalTime_ << "]\n"
       << "    Patient ID: " << patientId_ << " | Hospital ID: " << hospitalId_ << "\n"
       << "    Triage:     " << triageLevelToString(triageLevel_) << "\n"
       << "    Complaint:  " << chiefComplaint_ << "\n"
       << "    Status:     " << getStatusString() << "\n"
       << "    Diagnosis:  " << diagnosis_ << "\n";
    if (!notes_.empty()) {
        os << "    Notes:      " << notes_ << "\n";
    }
}

} // namespace smarter

