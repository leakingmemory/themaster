//
// Created by sigsegv on 1/23/24.
//

#include "MagistralMedicament.h"
#include <sfmbasisapi/fhir/medication.h>
#include <sfmbasisapi/fhir/bundleentry.h>
#include "Uuid.h"
#include <sstream>
#include "SfmMedicamentMapper.h"

FhirMagistral MagistralMedicament::ToFhir() {
    std::vector<FhirBundleEntry> substanceFhirs{};
    auto fhir = std::make_shared<FhirMedication>();
    fhir->SetStatus(FhirStatus::ACTIVE);
    fhir->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Magistrell-Medication");
    fhir->SetId(Uuid::RandomUuidString());
    {
        FhirQuantity quantity{amount, amountUnit};
        fhir->SetAmount(FhirRatio(quantity, {}));
    }
    fhir->SetCode(FhirCodeableConcept("urn:oid:2.16.578.1.12.4.1.1.7424", "10", "Magistrell"));
    std::vector<FhirIngredient> ingredients{};
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
                if (dilution.medicamentMapper && !dilution.medicamentMapper->GetMedications().empty()) {
                    auto substanceFhir = dilution.medicamentMapper->GetMedications()[0];
                    substanceFhirs.emplace_back(substanceFhir);
                    FhirIngredient ingredient{};
                    ingredient.SetActive(false);
                    ingredient.SetItemReference({substanceFhir.GetFullUrl(), substanceFhir.GetResource()->GetResourceType(), substanceFhir.GetResource()->GetDisplay()});
                    auto adqs = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-adqs");
                    adqs->AddExtension(std::make_shared<FhirValueExtension>(dilution.dilution == DilutionType::AD ? "ad" : "qs", std::make_shared<FhirBooleanValue>(true)));
                    ingredient.AddExtension(adqs);
                    ingredients.emplace_back(std::move(ingredient));
                }
            }
            for (const auto &substance: substances) {
                if (comma) {
                    sstr << ", ";
                }
                comma = true;
                sstr << substance.name;
                sstr << " " << substance.strength << " " << substance.strengthUnit;
                if (substance.medicamentMapper && !substance.medicamentMapper->GetMedications().empty()) {
                    auto substanceFhir = substance.medicamentMapper->GetMedications()[0];
                    substanceFhirs.emplace_back(substanceFhir);
                    FhirIngredient ingredient{};
                    ingredient.SetActive(true);
                    ingredient.SetItemReference({substanceFhir.GetFullUrl(), substanceFhir.GetResource()->GetResourceType(), substanceFhir.GetResource()->GetDisplay()});
                    ingredient.SetStrength(FhirRatio(FhirQuantity{substance.strength, substance.strengthUnit}, {}));
                    ingredients.emplace_back(std::move(ingredient));
                }
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
        fhir->SetName(name);
        std::shared_ptr<FhirString> fhirName = std::make_shared<FhirString>(name);
        std::shared_ptr<FhirValueExtension> fhirExtName = std::make_shared<FhirValueExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-name", fhirName);
        fhir->AddExtension(fhirExtName);
    }
    {
        std::shared_ptr<FhirString> fhirRecipe = std::make_shared<FhirString>(this->instructions);
        std::shared_ptr<FhirValueExtension> fhirExtRecipe = std::make_shared<FhirValueExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-recipe", fhirRecipe);
        fhir->AddExtension(fhirExtRecipe);
    }
    fhir->SetForm(form.ToCodeableConcept());
    fhir->SetIngredients(ingredients);
    return {.substances = substanceFhirs, .medication = fhir};
}
