//
// Created by sigsegv on 4/10/24.
//

#ifndef DRWHATSNOT_SFMMEDICAMENTMAPPER_H
#define DRWHATSNOT_SFMMEDICAMENTMAPPER_H

#include <memory>
#include <sfmbasisapi/fhir/medication.h>
#include "MedicalCodedValue.h"
#include "Duration.h"

class FestDb;
class LegemiddelCore;
class LegemiddelMerkevare;
class LegemiddelVirkestoff;
class Legemiddelpakning;

struct PrescriptionValidity {
    MedicalCodedValue gender;
    Duration duration;
    constexpr bool operator == (const PrescriptionValidity &other) const {
        return gender == other.gender && duration == other.duration;
    }
};

class SfmMedicamentMapper {
private:
    FhirMedication medication{};
    std::vector<SfmMedicamentMapper> packages{};
    std::vector<MedicalCodedValue> prescriptionUnit{};
    std::vector<MedicalCodedValue> medicamentType{};
    std::vector<MedicalCodedValue> medicamentUses{};
    std::vector<PrescriptionValidity> prescriptionValidity{};
    std::string packageDescription{};
    std::shared_ptr<FestDb> festDb;
    bool isPackage{false};
public:
    SfmMedicamentMapper(const std::shared_ptr<FestDb> &festDb, const std::shared_ptr<LegemiddelCore> &legemiddelCore);
private:
    void Map(const LegemiddelMerkevare &legemiddelMerkevare);
    void Map(const LegemiddelVirkestoff &legemiddelVirkestoff);
    void Map(const Legemiddelpakning &legemiddelpakning);
public:
    [[nodiscard]] FhirMedication GetMedication() const {
        return medication;
    }
    [[nodiscard]] std::vector<MedicalCodedValue> GetPrescriptionUnit() const {
        return prescriptionUnit;
    }
    [[nodiscard]] std::vector<MedicalCodedValue> GetMedicamentType() const {
        return medicamentType;
    }
    [[nodiscard]] std::vector<MedicalCodedValue> GetMedicamentUses() const {
        return medicamentUses;
    }
    [[nodiscard]] std::vector<PrescriptionValidity> GetPrescriptionValidity() const {
        return prescriptionValidity;
    }
    [[nodiscard]] bool IsPackage() const {
        return isPackage;
    }
    [[nodiscard]] std::vector<SfmMedicamentMapper> GetPackages() const {
        return packages;
    }
    [[nodiscard]] std::string GetPackageDescription() const {
        return packageDescription;
    }
};


#endif //DRWHATSNOT_SFMMEDICAMENTMAPPER_H
