//
// Created by sigsegv on 1/24/25.
//

#ifndef THEMASTER_MERCHDATA_H
#define THEMASTER_MERCHDATA_H

#include "DateOnly.h"
#include "MedicalCodedValue.h"
#include <string>

class FhirBasic;

struct MerchRefundInfo {
    MedicalCodedValue productGroup{};
    MedicalCodedValue paragraph{};
};

struct MerchData {
    MerchRefundInfo refund{};
    MedicalCodedValue itemGroup{};
    MedicalCodedValue rfstatus{};
    MedicalCodedValue typeresept{};
    std::string dssn{};
    DateOnly startDate{};
    DateOnly expirationDate{};
    std::string festUpdate{};
    std::string prescribedByDisplay{};
    std::string prescribedByReference{};
    std::string subjectDisplay{};
    std::string subjectReference{};
    bool guardianTransparencyReservation{false};
    bool inDoctorsName{false};
    bool lockedPrescription{false};
    void SetDefaults();
    static MerchData FromFhir(const FhirBasic &fhir);
    FhirBasic ToFhir() const;
};


#endif //THEMASTER_MERCHDATA_H
