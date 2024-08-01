//
// Created by sigsegv on 7/26/24.
//

#ifndef THEMASTER_PRESCRIPTIONCHANGESSERVICE_H
#define THEMASTER_PRESCRIPTIONCHANGESSERVICE_H

#include <exception>
#include <string>

class FhirMedicationStatement;

class RenewalFailureException : public std::exception {
private:
    std::string error;
public:
    RenewalFailureException(const std::string &error) : error(error) {}
    const char * what() const noexcept override;
};

struct PrescriptionStatusInfo {
    bool IsPll{false};
    bool HasBeenPll{false};
    bool IsCreate{false};
    bool IsRecalled{false};
    bool IsCeased{false};
    bool IsRenewedWithoutChanges{false};
    bool IsRecallNotSent{false};
    bool IsValidPrescription{false};
    bool HasBeenValidPrescription{false};
};

class PrescriptionChangesService {
public:
    static void Renew(FhirMedicationStatement &);
    static std::string GetPreviousPrescriptionId(const FhirMedicationStatement &);
    static std::string GetPrescriptionId(const FhirMedicationStatement &);
    static bool IsRenewedWithoutChangesAssumingIsEprescription(const FhirMedicationStatement &);
    static bool IsRenewedWithoutChanges(const FhirMedicationStatement &);
    static PrescriptionStatusInfo GetPrescriptionStatusInfo(const FhirMedicationStatement &);
    static std::string GetPrescriptionStatusString(const PrescriptionStatusInfo &);
};


#endif //THEMASTER_PRESCRIPTIONCHANGESSERVICE_H
