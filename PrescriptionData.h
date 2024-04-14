//
// Created by sigsegv on 1/31/24.
//

#ifndef DRWHATSNOT_PRESCRIPTIONDATA_H
#define DRWHATSNOT_PRESCRIPTIONDATA_H

#include <string>
#include "MedicalCodedValue.h"

class FhirMedicationStatement;

struct PrescriptionData {
    std::string reseptdate{}; // I dag, YYYY-MM-DD
    std::string expirationdate{}; // Gyldig til (typ 1 år), YYYY-MM-DD
    std::string festUpdate{"2023-12-20T11:54:48.9287539+00:00"}; // TODO
    bool guardianTransparencyReservation{false};
    bool inDoctorsName{false};
    bool lockedPrescription{false};
    std::string dssn{};
    bool numberOfPackagesSet{false};
    double numberOfPackages{};
    bool amountIsSet{false};
    double amount{};
    MedicalCodedValue amountUnit{};
    int reit{0};
    MedicalCodedValue itemGroup{};
    MedicalCodedValue reimbursementCode{};
    MedicalCodedValue reimbursementParagraph{};
    MedicalCodedValue rfstatus{};
    std::string lastChanged{};
    MedicalCodedValue typeresept{};
    MedicalCodedValue use{};
    std::string applicationArea{};
    bool genericSubstitutionAccepted{true};
    std::string prescribedByDisplay{};
    std::string prescribedByReference{};
    std::string subjectDisplay{};
    std::string subjectReference{};
    FhirMedicationStatement ToFhir();
};

#endif //DRWHATSNOT_PRESCRIPTIONDATA_H
