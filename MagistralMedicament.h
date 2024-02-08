//
// Created by sigsegv on 1/23/24.
//

#ifndef DRWHATSNOT_MAGISTRALMEDICAMENT_H
#define DRWHATSNOT_MAGISTRALMEDICAMENT_H

#include <string>
#include <vector>
#include "MedicalCodedValue.h"

enum class DilutionType {
    AD, QS
};

struct Dilution {
    std::string name;
    DilutionType dilution;
};

struct Substance {
    std::string name;
    double strength;
    std::string strengthUnit;
};

class FhirMedication;

struct MagistralMedicament {
    std::vector<Dilution> dilutions{};
    std::vector<Substance> substances{};
    MedicalCodedValue form{};
    double amount;
    std::string amountUnit;
    std::string instructions;
    FhirMedication ToFhir();
};

#endif //DRWHATSNOT_MAGISTRALMEDICAMENT_H
