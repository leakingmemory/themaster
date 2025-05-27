//
// Created by sigsegv on 1/23/24.
//

#ifndef DRWHATSNOT_MAGISTRALMEDICAMENT_H
#define DRWHATSNOT_MAGISTRALMEDICAMENT_H

#include <string>
#include <vector>
#include <memory>
#include "MedicalCodedValue.h"

enum class DilutionType {
    AD, QS
};

class SfmMedicamentMapper;

struct Dilution {
    std::shared_ptr<SfmMedicamentMapper> medicamentMapper;
    std::string name;
    DilutionType dilution;
};

struct Substance {
    std::shared_ptr<SfmMedicamentMapper> medicamentMapper;
    std::string name;
    double strength;
    std::string strengthUnit;
};

class FhirBundleEntry;
class FhirMedication;

struct FhirMagistral {
    std::vector<FhirBundleEntry> substances;
    std::shared_ptr<FhirMedication> medication;
};

struct MagistralMedicament {
    std::vector<Dilution> dilutions{};
    std::vector<Substance> substances{};
    MedicalCodedValue form{};
    double amount;
    std::string amountUnit;
    std::string instructions;
    FhirMagistral ToFhir();
};

#endif //DRWHATSNOT_MAGISTRALMEDICAMENT_H
