//
// Created by sigsegv on 4/10/24.
//

#include "SfmMedicamentMapper.h"
#include "FestDb.h"
#include <medfest/Struct/Decoded/LegemiddelMerkevare.h>
#include <medfest/Struct/Decoded/LegemiddelVirkestoff.h>
#include <medfest/Struct/Decoded/Legemiddelpakning.h>
#include <medfest/Struct/Packed/FestUuid.h>
#include <medfest/Struct/Decoded/VirkestoffMedStyrke.h>
#include <sfmbasisapi/fhir/bundleentry.h>
#include "DateOnly.h"
#include "GetLegemiddelRefunds.h"
#include "Uuid.h"
#include <algorithm>

SfmMedicamentDetailsMapper::SfmMedicamentDetailsMapper(const std::shared_ptr<FestDb> &festDb, const std::shared_ptr<LegemiddelCore> &legemiddelCore) : festDb(festDb) {
    medicamentRefunds = GetLegemiddelRefunds::GetMedicamentRefunds(*festDb, GetLegemiddelRefunds(*legemiddelCore));
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
    auto today = DateOnly::Today();
    std::sort(prescriptionValidity.begin(), prescriptionValidity.end(), [today] (auto pv1, auto pv2) {
        return (today + pv1.duration) < (today + pv2.duration);
    });
}

void SfmMedicamentDetailsMapper::Map(const LegemiddelMerkevare &legemiddelMerkevare) {
    {
        FestUuid merkevareId{legemiddelMerkevare.GetId()};
        auto pakninger = festDb->GetLegemiddelpakningForMerkevare(merkevareId);
        for (const auto &pakning : pakninger) {
            auto &mapper = packages.emplace_back(festDb, std::make_shared<Legemiddelpakning>(pakning));
            for (const auto &refund : mapper.GetMedicamentRefunds()) {
                bool found{false};
                for (auto &existingRefund : medicamentRefunds) {
                    if (refund.refund.GetCode() == existingRefund.refund.GetCode()) {
                        for (const auto &refundCode : refund.codes) {
                            bool found{false};
                            for (const auto &existigCode : existingRefund.codes) {
                                if (refundCode.GetCode() == existigCode.GetCode()) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                existingRefund.codes.emplace_back(refundCode);
                            }
                        }
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    medicamentRefunds.emplace_back(refund);
                }
            }
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
                {
                    auto preparatType = legemiddelMerkevare.GetPreparattype();
                    medicamentType.emplace_back("2.16.578.1.12.4.1.1.9101", preparatType.GetValue(), preparatType.GetDistinguishedName(), preparatType.GetDistinguishedName());
                }
            }
        }
        for (const auto reseptgyldighet : legemiddelMerkevare.GetReseptgyldighet()) {
            auto kjonn = reseptgyldighet.GetKjonn();
            PrescriptionValidity pv{.gender = {"", kjonn.GetValue(), kjonn.GetDistinguishedName(), kjonn.GetDistinguishedName()}, .duration = Duration::FromString(reseptgyldighet.GetVarighet())};
            prescriptionValidity.emplace_back(std::move(pv));
        }
    }
    {
        auto administreringLegemiddel = legemiddelMerkevare.GetAdministreringLegemiddel();
        for (const auto &use : administreringLegemiddel.GetBruksomradeEtikett()) {
            medicamentUses.emplace_back(use.GetCodeSet(), use.GetValue(), use.GetDistinguishedName(), use.GetDistinguishedName());
        }
    }
    for (const auto &ref : legemiddelMerkevare.GetSortertVirkestoffUtenStyrke()) {
        substances.emplace_back(ref);
    }
    for (const auto &refWS : legemiddelMerkevare.GetSortertVirkestoffMedStyrke()) {
        auto substance = festDb->GetVirkestoffMedStyrke(FestUuid(refWS));
        auto substanceId = substance.GetRefVirkestoff();
        if (!substanceId.empty() && std::find(substances.cbegin(), substances.cend(), substanceId) == substances.cend()) {
            substances.emplace_back(substanceId);
        }
    }
}

void SfmMedicamentDetailsMapper::Map(const LegemiddelVirkestoff &legemiddelVirkestoff) {
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
        auto administreringLegemiddel = legemiddelVirkestoff.GetAdministreringLegemiddel();
        for (const auto &use : administreringLegemiddel.GetBruksomradeEtikett()) {
            medicamentUses.emplace_back(use.GetCodeSet(), use.GetValue(), use.GetDistinguishedName(), use.GetDistinguishedName());
        }
    }
    {
        auto merkevareIds = legemiddelVirkestoff.GetRefLegemiddelMerkevare();
        for (const auto &merkevareId : merkevareIds) {
            auto merkevare = festDb->GetLegemiddelMerkevare(FestUuid(merkevareId));
            {
                auto preparatType = merkevare.GetPreparattype();
                if (std::find_if(medicamentType.cbegin(), medicamentType.cend(), [&preparatType] (const MedicalCodedValue &mcv) { return mcv.GetCode() == preparatType.GetValue(); }) == medicamentType.cend()) {
                    medicamentType.emplace_back("2.16.578.1.12.4.1.1.9101", preparatType.GetValue(),
                                                preparatType.GetDistinguishedName(),
                                                preparatType.GetDistinguishedName());
                }
                for (const auto reseptgyldighet : merkevare.GetReseptgyldighet()) {
                    auto kjonn = reseptgyldighet.GetKjonn();
                    PrescriptionValidity pv{.gender = {"", kjonn.GetValue(), kjonn.GetDistinguishedName(),
                                                       kjonn.GetDistinguishedName()}, .duration = Duration::FromString(
                            reseptgyldighet.GetVarighet())};
                    if (std::find(prescriptionValidity.cbegin(), prescriptionValidity.cend(), pv) == prescriptionValidity.cend()) {
                        prescriptionValidity.emplace_back(std::move(pv));
                    }
                }
            }
        }
    }
    for (const auto &refWS : legemiddelVirkestoff.GetSortertVirkestoffMedStyrke()) {
        auto substance = festDb->GetVirkestoffMedStyrke(FestUuid(refWS));
        if (!substance.GetRefVirkestoff().empty()) {
            substances.emplace_back(substance.GetRefVirkestoff());
        }
    }
}

void SfmMedicamentDetailsMapper::Map(const Legemiddelpakning &legemiddelpakning) {
    auto pakningsinfoList = legemiddelpakning.GetPakningsinfo();
    for (const auto &pakningsinfo : pakningsinfoList) {
        auto packageUnit = pakningsinfo.GetEnhetPakning();
        this->prescriptionUnit.emplace_back(packageUnit.GetCodeSet(), packageUnit.GetValue(),
                                       packageUnit.GetDistinguishedName(), "");
    }
    {
        for (const auto &pi : pakningsinfoList) {
            FestUuid merkevareId{pi.GetMerkevareId()};
            auto merkevare = festDb->GetLegemiddelMerkevare(merkevareId);
            if (!packageDescription.empty()) {
                packageDescription.append(", ");
            }
            packageDescription.append(pi.GetPakningsstr());
            {
                auto preparatType = merkevare.GetPreparattype();
                if (std::find_if(medicamentType.cbegin(), medicamentType.cend(), [&preparatType] (const MedicalCodedValue &mcv) { return mcv.GetCode() == preparatType.GetValue(); }) == medicamentType.cend()) {
                    medicamentType.emplace_back("2.16.578.1.12.4.1.1.9101", preparatType.GetValue(),
                                                preparatType.GetDistinguishedName(),
                                                preparatType.GetDistinguishedName());
                }
            }
            for (const auto reseptgyldighet : merkevare.GetReseptgyldighet()) {
                auto kjonn = reseptgyldighet.GetKjonn();
                PrescriptionValidity pv{.gender = {"", kjonn.GetValue(), kjonn.GetDistinguishedName(),
                                                   kjonn.GetDistinguishedName()}, .duration = Duration::FromString(
                        reseptgyldighet.GetVarighet())};
                if (std::find(prescriptionValidity.cbegin(), prescriptionValidity.cend(), pv) == prescriptionValidity.cend()) {
                    prescriptionValidity.emplace_back(std::move(pv));
                }
            }
            {
                auto administreringLegemiddel = merkevare.GetAdministreringLegemiddel();
                for (const auto &use : administreringLegemiddel.GetBruksomradeEtikett()) {
                    auto code = use.GetValue();
                    if (std::find_if(medicamentUses.cbegin(), medicamentUses.cend(), [code] (const MedicalCodedValue &cv) { return cv.GetCode() == code; }) == medicamentUses.cend()) {
                        medicamentUses.emplace_back(use.GetCodeSet(), code, use.GetDistinguishedName(), use.GetDistinguishedName());
                    }
                }
            }
            for (const auto &ref : merkevare.GetSortertVirkestoffUtenStyrke()) {
                if (std::find(substances.cbegin(), substances.cend(), ref) == substances.cend()) {
                    substances.emplace_back(ref);
                }
            }
            for (const auto &refWS : merkevare.GetSortertVirkestoffMedStyrke()) {
                auto substance = festDb->GetVirkestoffMedStyrke(FestUuid(refWS));
                auto substanceId = substance.GetRefVirkestoff();
                if (!substanceId.empty() && std::find(substances.cbegin(), substances.cend(), substanceId) == substances.cend()) {
                    substances.emplace_back(substanceId);
                }
            }
        }
    }
    isPackage = true;
}

std::vector<SfmMedicamentMapper> SfmMedicamentDetailsMapper::GetPackages() const {
    return packages;
}

SfmMedicamentMapper::SfmMedicamentMapper(const std::shared_ptr<FestDb> &festDb,
                                         const std::shared_ptr<LegemiddelCore> &legemiddelCore) : detailsMapper(std::make_shared<std::unique_ptr<SfmMedicamentDetailsMapper>>()), festDb(festDb), legemiddelCore(legemiddelCore) {
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
    medication.SetId(Uuid::RandomUuidString());
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
        auto pakningsinfoList = legemiddelpakning.GetPakningsinfo();
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
}

std::vector <FhirBundleEntry> SfmMedicamentMapper::GetMedications() const {
    std::string fullUrl{"urn:uuid:"};
    fullUrl.append(Uuid::RandomUuidString());
    FhirBundleEntry bundleEntry{fullUrl, std::make_shared<FhirMedication>(medication)};
    return {bundleEntry};
}