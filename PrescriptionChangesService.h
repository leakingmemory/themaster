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

class PrescriptionChangesService {
public:
    static void Renew(FhirMedicationStatement &);
    static std::string GetPreviousPrescriptionId(const FhirMedicationStatement &);
    static std::string GetPrescriptionId(const FhirMedicationStatement &);
    static bool IsRenewedWithoutChangesAssumingIsEprescription(const FhirMedicationStatement &);
    static bool IsRenewedWithoutChanges(const FhirMedicationStatement &);
};


#endif //THEMASTER_PRESCRIPTIONCHANGESSERVICE_H
