//
// Created by sigsegv on 7/26/24.
//

#ifndef THEMASTER_PRESCRIPTIONCHANGESSERVICE_H
#define THEMASTER_PRESCRIPTIONCHANGESSERVICE_H

#include <exception>
#include <string>
#include <vector>
#include <memory>

class FhirMedicationStatement;
class FhirIdentifier;
class FhirExtension;
class FhirBasic;

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
    bool IsRenewedWithChanges{false};
    bool IsRecallNotSent{false};
    bool IsValidPrescription{false};
    bool HasBeenValidPrescription{false};
};

template <class T> concept CanGetPrescriptionStatusInfoFor = requires (const T &fhir) {
    { fhir.GetIdentifiers() } -> std::convertible_to<std::vector<FhirIdentifier>>;
    { fhir.GetExtensions() } -> std::convertible_to<std::vector<std::shared_ptr<FhirExtension>>>;
};

template <class T> concept RenewableFhirObject = requires (T &renewable) {
    { renewable.GetIdentifiers() } -> std::convertible_to<std::vector<FhirIdentifier>>;
    { renewable.SetIdentifiers(std::declval<std::vector<FhirIdentifier>>()) };
    { renewable.GetExtensions() } -> std::convertible_to<std::vector<std::shared_ptr<FhirExtension>>>;
};

class PrescriptionChangesService {
private:
    template <RenewableFhirObject T> static void GenericRenew(T &);
public:
    static void Renew(FhirMedicationStatement &);
    static void Renew(FhirBasic &);
    static void RenewRevokedOrExpiredPll(FhirMedicationStatement &);
    static std::string GetPreviousPrescriptionId(const FhirMedicationStatement &);
    static std::string GetPrescriptionId(const FhirMedicationStatement &);
    static bool IsRenewedWithoutChangesAssumingIsEprescription(const FhirMedicationStatement &);
    static bool IsRenewedWithoutChanges(const FhirMedicationStatement &);
private:
    template <CanGetPrescriptionStatusInfoFor T> static PrescriptionStatusInfo GetPrescriptionStatusInfoImpl(const T &);
public:
    static PrescriptionStatusInfo GetPrescriptionStatusInfo(const FhirMedicationStatement &);
    static PrescriptionStatusInfo GetPrescriptionStatusInfo(const FhirBasic &);
    static std::string GetPrescriptionStatusString(const PrescriptionStatusInfo &);
};


#endif //THEMASTER_PRESCRIPTIONCHANGESSERVICE_H
