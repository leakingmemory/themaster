//
// Created by sigsegv on 1/31/24.
//

#include "PrescriptionData.h"
#include "AdvancedDosingPeriod.h"
#include <sfmbasisapi/fhir/medstatement.h>
#include <boost/uuid/uuid_generators.hpp> // for random_generator
#include <boost/uuid/uuid_io.hpp> // for to_string

FhirMedicationStatement PrescriptionData::ToFhir() {
    FhirMedicationStatement fhir{};
    fhir.SetStatus(FhirStatus::ACTIVE);
    fhir.SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement");
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        fhir.SetId(uuidStr);
    }
    {
        FhirDosage dosage{dosingText.empty() ? dssn : dosingText, 1};
        if (!kortdose.GetCode().empty()) {
            std::shared_ptr<FhirValueExtension> kortdoseExt = std::make_shared<FhirValueExtension>(
                    "http://ehelse.no/fhir/StructureDefinition/sfm-shortdosage",
                    std::make_shared<FhirCodeableConceptValue>(kortdose.ToCodeableConcept())
            );
            dosage.AddExtension(kortdoseExt);
        }
        {
            std::shared_ptr<FhirValueExtension> useExt = std::make_shared<FhirValueExtension>(
                    "http://ehelse.no/fhir/StructureDefinition/sfm-use",
                    std::make_shared<FhirCodeableConceptValue>(use.ToCodeableConcept())
            );
            dosage.AddExtension(useExt);
        }
        {
            std::shared_ptr<FhirValueExtension> applicationAreaTextExt = std::make_shared<FhirValueExtension>(
                    "text",
                    std::make_shared<FhirString>(applicationArea)
            );
            std::shared_ptr<FhirExtension> applicationAreaExt = std::make_shared<FhirExtension>(
                    "http://ehelse.no/fhir/StructureDefinition/sfm-application-area"
            );
            applicationAreaExt->AddExtension(applicationAreaTextExt);
            dosage.AddExtension(applicationAreaExt);
        }
        fhir.AddDosage(dosage);
    }
    {
        std::shared_ptr<FhirExtension> reseptAmendment = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment");
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                "reseptdate",
                std::make_shared<FhirDateValue>(reseptdate.ToString())
        ));
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                "expirationdate",
                std::make_shared<FhirDateValue>(expirationdate.ToString())
        ));
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                "festUpdate",
                std::make_shared<FhirDateTimeValue>(festUpdate)
        ));
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                "dssn",
                std::make_shared<FhirString>(dssn)
        ));
        if (numberOfPackagesSet) {
            reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                    "numberofpackages",
                    std::make_shared<FhirDecimalValue>(numberOfPackages)
            ));
        } else if (amountIsSet) {
            reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                    "amount",
                    std::make_shared<FhirQuantityValue>(FhirQuantity(amount, amountUnit.GetDisplay()))
            ));
        }
        {
            std::string reitStr{};
            {
                std::stringstream reitStrS{};
                reitStrS << reit;
                reitStr = reitStrS.str();
            }
            reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                    "reit",
                    std::make_shared<FhirString>(reitStr)
            ));
        }
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                "itemgroup",
                std::make_shared<FhirCodeableConceptValue>(itemGroup.ToCodeableConcept())
        ));
        {
            auto dpStart = reseptdate;
            auto iterator = dosingPeriods.begin();
            while (iterator != dosingPeriods.end()) {
                const auto &dosingPeriod = *iterator;
                dosingPeriod->SetDosingUnit(dosingUnit);
                ++iterator;
                auto isNotLast = iterator != dosingPeriods.end();
                auto ereseptdosing = std::make_shared<FhirExtension>("ereseptdosing");
                ereseptdosing->AddExtension(std::make_shared<FhirValueExtension>("starttime",
                                                                                 std::make_shared<FhirDateValue>(
                                                                                         dpStart.ToString())));
                auto days = dosingPeriod->GetDays();
                if (days <= 0 && isNotLast) {
                    days = 1;
                }
                if (days > 0) {
                    dpStart.AddDays(days);
                    ereseptdosing->AddExtension(std::make_shared<FhirValueExtension>("endtime",
                                                                                     std::make_shared<FhirDateValue>(
                                                                                             dpStart.ToString())));
                }
                dosingPeriod->Apply(*ereseptdosing);
                reseptAmendment->AddExtension(ereseptdosing);
            }
        }
        {
            std::shared_ptr<FhirExtension> rfstatusExt = std::make_shared<FhirExtension>("rfstatus");
            rfstatusExt->AddExtension(std::make_shared<FhirValueExtension>(
                    "status",
                    std::make_shared<FhirCodeableConceptValue>(this->rfstatus.ToCodeableConcept())
            ));
            reseptAmendment->AddExtension(rfstatusExt);
        }
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                "lastchanged",
                std::make_shared<FhirDateTimeValue>(lastChanged)
        ));
        auto typeresept = std::make_shared<FhirCodeableConceptValue>(this->typeresept.ToCodeableConcept());
        if (typeresept->IsSet()) {
            reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                "typeresept",
                typeresept
            ));
        }
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
            "createeresept",
            std::make_shared<FhirBooleanValue>(true)
        ));
        fhir.AddExtension(reseptAmendment);
    }
    {
        std::shared_ptr<FhirExtension> regInfo = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-regInfo");
        regInfo->AddExtension(std::make_shared<FhirValueExtension>(
            "status",
            std::make_shared<FhirCodeableConceptValue>(
                FhirCodeableConcept(
                    "http://ehelse.no/fhir/CodeSystem/sfm-medicationstatement-registration-status",
                    "3",
                    "Godkjent"
                )
            )
        ));
        regInfo->AddExtension(std::make_shared<FhirValueExtension>(
            "type",
            std::make_shared<FhirCodeableConceptValue>(
                FhirCodeableConcept(
                    "http://ehelse.no/fhir/CodeSystem/sfm-performer-roles",
                    "1",
                    "Forskrevet av"
                )
            )
        ));
        regInfo->AddExtension(std::make_shared<FhirValueExtension>(
            "provider",
            std::make_shared<FhirReference>(
                prescribedByReference,
                "http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner",
                prescribedByDisplay
            )
        ));
        {
            std::time_t now = std::time(nullptr);
            std::tm tm{};
            localtime_r(&now, &tm);

            std::ostringstream nowStream;
            nowStream << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

            auto tzone = localtime(&now);
            if(tzone->tm_gmtoff >= 0)
                nowStream << "+";
            else
                nowStream << "-";
            nowStream << std::setfill('0') << std::setw(2) << abs(tzone->tm_gmtoff / 3600) << ":" << std::setw(2) << abs((tzone->tm_gmtoff / 60) % 60);

            std::string nowString = nowStream.str();

            regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                "timestamp",
                std::make_shared<FhirDateTimeValue>(nowString)
            ));
        }
        fhir.AddExtension(regInfo);
    }
    {
        std::shared_ptr<FhirExtension> genSubst = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-generic-substitution");
        genSubst->AddExtension(std::make_shared<FhirValueExtension>(
            "genericSubstitutionAccepted",
            std::make_shared<FhirBooleanValue>(genericSubstitutionAccepted)
        ));
        fhir.AddExtension(genSubst);
    }
    {
        FhirReference ref{subjectReference, "http://ehelse.no/fhir/StructureDefinition/sfm-Patient", subjectDisplay};
        fhir.SetSubject(ref);
    }
    {
        FhirCodeableConcept identifierType{"ReseptId"};
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        FhirIdentifier identifier{identifierType, "usual", uuidStr};
        fhir.AddIdentifier(identifier);
    }
    return fhir;
}
