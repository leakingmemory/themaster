//
// Created by sigsegv on 1/23/24.
//

#include "MagistralMedicament.h"
#include <sfmbasisapi/fhir/medication.h>
#include <boost/uuid/uuid_generators.hpp> // for random_generator
#include <boost/uuid/uuid_io.hpp> // for to_string

FhirMedication MagistralMedicament::ToFhir() {
    FhirMedication fhir{};
    fhir.SetStatus(FhirStatus::ACTIVE);
    fhir.SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Magistrell-Medication");
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        fhir.SetId(uuidStr);
    }
    {
        FhirQuantity quantity{amount, amountUnit};
        fhir.SetAmount(FhirRatio(quantity, {}));
    }
    fhir.SetCode(FhirCodeableConcept("urn:oid:2.16.578.1.12.4.1.1.7424", "10", "Magistrell"));
    {
        std::string name{};
        {
            bool comma{false};
            std::stringstream sstr{};
            for (const auto &dilution: dilutions) {
                if (comma) {
                    sstr << ", ";
                }
                comma = true;
                sstr << dilution.name;
                sstr << (dilution.dilution == DilutionType::AD ? " ad" : " qs");
            }
            for (const auto &substance: substances) {
                if (comma) {
                    sstr << ", ";
                }
                comma = true;
                sstr << substance.name;
                sstr << " " << substance.strength << " " << substance.strengthUnit;
            }
            {
                std::string formDisplay{this->form.GetShortDisplay()};
                if (formDisplay.empty()) {
                    formDisplay = this->form.GetDisplay();
                }
                sstr << " " << formDisplay;
            }
            sstr << " " << amount << " " << amountUnit;
            name = sstr.str();
        }
        fhir.SetName(name);
        std::shared_ptr<FhirString> fhirName = std::make_shared<FhirString>(name);
        std::shared_ptr<FhirValueExtension> fhirExtName = std::make_shared<FhirValueExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-name", fhirName);
        fhir.AddExtension(fhirExtName);
    }
    {
        std::shared_ptr<FhirString> fhirRecipe = std::make_shared<FhirString>(this->instructions);
        std::shared_ptr<FhirValueExtension> fhirExtRecipe = std::make_shared<FhirValueExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-recipe", fhirRecipe);
        fhir.AddExtension(fhirExtRecipe);
    }
    fhir.SetForm(form.ToCodeableConcept());
    return fhir;
}
