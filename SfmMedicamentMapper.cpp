//
// Created by sigsegv on 4/10/24.
//

#include "SfmMedicamentMapper.h"
#include "FestDb.h"
#include <medfest/Struct/Decoded/LegemiddelMerkevare.h>
#include <medfest/Struct/Decoded/LegemiddelVirkestoff.h>
#include <medfest/Struct/Decoded/Legemiddelpakning.h>
#include <medfest/Struct/Packed/FestUuid.h>
#include <boost/uuid/uuid_generators.hpp> // for random_generator
#include <boost/uuid/uuid_io.hpp> // for to_string

SfmMedicamentMapper::SfmMedicamentMapper(const std::shared_ptr<FestDb> &festDb, const std::shared_ptr<LegemiddelCore> &legemiddelCore) : festDb(festDb) {
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        medication.SetId(uuidStr);
    }
    {
        auto reseptgruppe = legemiddelCore->GetReseptgruppe();
        auto reseptgruppeCode = reseptgruppe.GetValue();
        if (!reseptgruppeCode.empty()) {
            auto ext = std::make_shared<FhirExtension>("http://hl7.no/fhir/StructureDefinition/no-basis-prescriptiongroup");
            ext->AddExtension(std::make_shared<FhirValueExtension>(
                    "prescriptiongroup",
                    std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept("urn:oid:2.16.578.1.12.4.1.1.7421", reseptgruppeCode, reseptgruppe.GetDistinguishedName()))
            ));
            medication.AddExtension(ext);
        }
    }
    {
        std::shared_ptr<LegemiddelMerkevare> merkevare = std::dynamic_pointer_cast<LegemiddelMerkevare>(legemiddelCore);
        if (merkevare) {
            Map(*merkevare);
        }
        std::shared_ptr<LegemiddelVirkestoff> virkestoff = std::dynamic_pointer_cast<LegemiddelVirkestoff>(
                legemiddelCore);
        if (virkestoff) {
            Map(*virkestoff);
        }
        std::shared_ptr<Legemiddelpakning> pakning = std::dynamic_pointer_cast<Legemiddelpakning>(legemiddelCore);
        if (pakning) {
            Map(*pakning);
        }
    }
    {
        auto atc = legemiddelCore->GetAtc();
        auto atcValue = atc.GetValue();
        if (!atcValue.empty()) {
            auto code = medication.GetCode();
            auto coding= code.GetCoding();
            auto text = code.GetText();
            coding.emplace_back("http://www.whocc.no/atc", atcValue, atc.GetDistinguishedName());
            FhirCodeableConcept newCode{std::move(coding), std::move(text)};
            medication.SetCode(newCode);
        }
    }
    {
        auto form = legemiddelCore->GetLegemiddelformKort();
        auto formCode = form.GetValue();
        if (!formCode.empty()) {
            auto formSystem = form.GetCodeSet();
            if (!formSystem.empty()) {
                std::string system{"urn:oid:"};
                system.append(formSystem);
                formSystem = system;
            } else {
                formSystem = "urn:oid:2.16.578.1.12.4.1.1.7448";
            }
            medication.SetForm(FhirCodeableConcept(formSystem, formCode, form.GetDistinguishedName()));
        }
        medication.SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Medication");
        medication.SetStatus(FhirStatus::ACTIVE);
        medication.SetName(legemiddelCore->GetNavnFormStyrke());
    }
}

void SfmMedicamentMapper::Map(const LegemiddelMerkevare &legemiddelMerkevare) {
    {
        FestUuid merkevareId{legemiddelMerkevare.GetId()};
        auto pakninger = festDb->GetLegemiddelpakningForMerkevare(merkevareId);
        for (const auto &pakning : pakninger) {
            auto &mapper = packages.emplace_back(festDb, std::make_shared<Legemiddelpakning>(pakning));
            auto pInfos = pakning.GetPakningsinfo();
            for (const auto pInfo : pInfos) {
                auto packingUnit = pInfo.GetEnhetPakning();
                auto packingUnitCode = packingUnit.GetValue();
                bool found{false};
                for (const auto &pu: prescriptionUnit) {
                    if (pu.GetCode() == packingUnit.GetValue()) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    prescriptionUnit.emplace_back(packingUnit.GetCodeSet(), packingUnitCode, packingUnit.GetDistinguishedName(), "");
                }
            }
        }
    }
    {
        auto code = medication.GetCode();
        auto coding = code.GetCoding();
        coding.emplace_back("http://ehelse.no/fhir/CodeSystem/FEST", legemiddelMerkevare.GetId(),
                            legemiddelMerkevare.GetNavnFormStyrke());
        auto text = code.GetText();
        medication.SetCode(FhirCodeableConcept(std::move(coding), std::move(text)));
    }
    {
        auto medicationDetails = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-medicationdetails");
        medicationDetails->AddExtension(std::make_shared<FhirValueExtension>("registreringstype", std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-festregistrationtype", "2", "Legemiddelmerkevare"))));
        medicationDetails->AddExtension(std::make_shared<FhirValueExtension>("name", std::make_shared<FhirString>(legemiddelMerkevare.GetNavnFormStyrke())));
        medication.AddExtension(medicationDetails);
    }
}

void SfmMedicamentMapper::Map(const LegemiddelVirkestoff &legemiddelVirkestoff) {
    {
        auto dosingUnit = legemiddelVirkestoff.GetForskrivningsenhetResept();
        auto dosingUnitSystem = dosingUnit.GetCodeSet();
        if (!dosingUnitSystem.empty()) {
            std::string system{"urn:oid:"};
            system.append(dosingUnitSystem);
            dosingUnitSystem = system;
        } else {
            dosingUnitSystem = "urn:oid:2.16.578.1.12.4.1.1.7448";
        }
        this->prescriptionUnit.emplace_back(dosingUnitSystem, dosingUnit.GetValue(), dosingUnit.GetDistinguishedName(), "");
    }
    {
        auto code = medication.GetCode();
        auto coding = code.GetCoding();
        coding.emplace_back("http://ehelse.no/fhir/CodeSystem/FEST", legemiddelVirkestoff.GetId(),
                            legemiddelVirkestoff.GetNavnFormStyrke());
        auto text = code.GetText();
        medication.SetCode(FhirCodeableConcept(std::move(coding), std::move(text)));
    }
    {
        auto medicationDetails = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-medicationdetails");
        medicationDetails->AddExtension(std::make_shared<FhirValueExtension>("registreringstype", std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-festregistrationtype", "1", "Legemiddelvirkestoff"))));
        medication.AddExtension(medicationDetails);
    }
}

void SfmMedicamentMapper::Map(const Legemiddelpakning &legemiddelpakning) {
    auto pakningsinfoList = legemiddelpakning.GetPakningsinfo();
    for (const auto &pakningsinfo : pakningsinfoList) {
        auto packageUnit = pakningsinfo.GetEnhetPakning();
        this->prescriptionUnit.emplace_back(packageUnit.GetCodeSet(), packageUnit.GetValue(),
                                       packageUnit.GetDistinguishedName(), "");
    }
    {
        auto code = medication.GetCode();
        auto coding = code.GetCoding();
        coding.emplace_back("http://ehelse.no/fhir/CodeSystem/FEST", legemiddelpakning.GetVarenr(),
                            legemiddelpakning.GetNavnFormStyrke());
        auto text = code.GetText();
        medication.SetCode(FhirCodeableConcept(std::move(coding), std::move(text)));
    }
    {
        auto medicationDetails = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-medicationdetails");
        medicationDetails->AddExtension(std::make_shared<FhirValueExtension>("registreringstype", std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-festregistrationtype", "3", "Legemiddelpakning"))));
        for (const auto &pi : pakningsinfoList) {
            FestUuid merkevareId{pi.GetMerkevareId()};
            auto merkevare = festDb->GetLegemiddelMerkevare(merkevareId);
            auto packinginfo = std::make_shared<FhirExtension>("packinginfoprescription");
            packinginfo->AddExtension(std::make_shared<FhirValueExtension>("name", std::make_shared<FhirString>(merkevare.GetVarenavn())));
            packinginfo->AddExtension(std::make_shared<FhirValueExtension>("packingsize", std::make_shared<FhirString>(pi.GetPakningsstr())));
            {
                auto enhetPakning = pi.GetEnhetPakning();
                std::string codeSystem{"urn:oid:"};
                codeSystem.append(enhetPakning.GetCodeSet());
                packinginfo->AddExtension(std::make_shared<FhirValueExtension>("packingunit",
                                                                               std::make_shared<FhirCodeableConceptValue>(
                                                                                       FhirCodeableConcept(
                                                                                               codeSystem,
                                                                                               enhetPakning.GetValue(),
                                                                                               enhetPakning.GetDistinguishedName()))));
            }
            medicationDetails->AddExtension(packinginfo);
        }
        medication.AddExtension(medicationDetails);
    }
    isPackage = true;
}
