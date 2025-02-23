//
// Created by sigsegv on 1/24/25.
//

#include "MerchData.h"
#include <sfmbasisapi/nhnfhir/SfmBandaPrescription.h>
#include <boost/uuid/uuid_generators.hpp> // for random_generator
#include <boost/uuid/uuid_io.hpp> // for to_string
#include <functional>

void MerchData::SetDefaults() {
    itemGroup = {"urn:oid:2.16.578.1.12.4.1.1.7402", "A", "Annet", "Annet"};
    rfstatus = {"urn:oid:2.16.578.1.12.4.1.1.7408", "E", "Ekspederbar", "Ekspederbar"};
    typeresept = {"urn:oid:2.16.578.1.12.4.1.1.7491", "E", "Eresept", "Eresept"};
    festUpdate = "2022-09-08T13:34:11+00:00";
}

MerchData MerchData::FromFhir(const FhirBasic &fhir) {
    MerchData data{};
    for (const auto &extension : fhir.GetExtensions()) {
        auto url = extension->GetUrl();
        std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
        if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptdocbanda") {
            for (const auto &extension : extension->GetExtensions()) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                if (url == "productgroup") {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExtension) {
                        auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                        if (value) {
                            for (const auto &coding : value->GetCoding()) {
                                data.refund.productGroup = {coding.GetSystem(), coding.GetCode(), coding.GetDisplay(), coding.GetDisplay()};
                            }
                        }
                    }
                }
            }
        } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
            for (const auto &extension : extension->GetExtensions()) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(),
                               [](char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                std::function<std::shared_ptr<FhirValueExtension> ()> valueExtension{[&extension] () {
                    return std::dynamic_pointer_cast<FhirValueExtension>(extension);
                }};
                std::function<DateOnly ()> dateValue{[&valueExtension] () -> DateOnly {
                    auto ve = valueExtension();
                    if (ve) {
                        auto dv = std::dynamic_pointer_cast<FhirDateValue>(ve->GetValue());
                        if (dv) {
                            try {
                                DateOnly dOnly{dv->GetRawValue()};
                                return dOnly;
                            } catch (const DateOnlyInvalidException &) {
                                return {};
                            }
                        }
                    }
                    return {};
                }};
                std::function<std::shared_ptr<FhirBooleanValue> ()> booleanValue{[&valueExtension] () -> std::shared_ptr<FhirBooleanValue> {
                    auto ve = valueExtension();
                    if (ve) {
                        return std::dynamic_pointer_cast<FhirBooleanValue>(ve->GetValue());
                    }
                    return {};
                }};
                std::function<std::string ()> dateTimeRawValue{[&valueExtension] () -> std::string {
                    auto ve = valueExtension();
                    if (ve) {
                        auto dtv = std::dynamic_pointer_cast<FhirDateTimeValue>(ve->GetValue());
                        if (dtv) {
                            return dtv->GetDateTime();
                        }
                    }
                    return {};
                }};
                std::function<std::string ()> stringValue{[&valueExtension] () -> std::string {
                    auto ve = valueExtension();
                    if (ve) {
                        auto strv = std::dynamic_pointer_cast<FhirString>(ve->GetValue());
                        if (strv) {
                            return strv->GetValue();
                        }
                    }
                    return {};
                }};
                std::function<FhirCodeableConcept ()> codeableConceptValue{[&valueExtension] () -> FhirCodeableConcept {
                    auto ve = valueExtension();
                    if (ve) {
                        auto cvcv = std::dynamic_pointer_cast<FhirCodeableConceptValue>(ve->GetValue());
                        if (cvcv) {
                            return *cvcv;
                        }
                    }
                    return {};
                }};
                std::function<MedicalCodedValue ()> codeableConceptAsMedicalCodedValue{[&codeableConceptValue] () -> MedicalCodedValue {
                    auto cc = codeableConceptValue();
                    if (cc.IsSet()) {
                        auto codings = cc.GetCoding();
                        if (!codings.empty()) {
                            return {codings[0].GetSystem(), codings[0].GetCode(), codings[0].GetDisplay(), codings[0].GetDisplay()};
                        }
                        if (!cc.GetText().empty()) {
                            return {"", "", cc.GetText(), cc.GetText()};
                        }
                    }
                    return {};
                }};
                if (url == "reseptdate") {
                    auto dOnly = dateValue();
                    if (dOnly.operator bool()) {
                        data.startDate = dOnly;
                    }
                } else if (url == "expirationdate") {
                    auto dOnly = dateValue();
                    if (dOnly.operator bool()) {
                        data.expirationDate = dOnly;
                    }
                } else if (url == "festupdate") {
                    auto fu = dateTimeRawValue();
                    if (!fu.empty()) {
                        data.festUpdate = fu;
                    }
                } else if (url == "guardiantransparencyreservation") {
                    auto bv = booleanValue();
                    if (bv) {
                        data.guardianTransparencyReservation = bv->IsTrue();
                    }
                } else if (url == "indoctorsname") {
                    auto bv = booleanValue();
                    if (bv) {
                        data.inDoctorsName = bv->IsTrue();
                    }
                } else if (url == "dssn") {
                    auto str = stringValue();
                    if (!str.empty()) {
                        data.dssn = str;
                    }
                } else if (url == "lockedresept") {
                    auto bv = booleanValue();
                    if (bv) {
                        data.lockedPrescription = bv->IsTrue();
                    }
                } else if (url == "itemgroup") {
                    auto vcv = codeableConceptAsMedicalCodedValue();
                    if (!vcv.GetCode().empty() || !vcv.GetDisplay().empty()) {
                        data.itemGroup = vcv;
                    }
                } else if (url == "reimbursementparagraph") {
                    auto vcv = codeableConceptAsMedicalCodedValue();
                    if (!vcv.GetCode().empty() || !vcv.GetDisplay().empty()) {
                        data.refund.paragraph = vcv;
                    }
                } else if (url == "rfstatus") {
                    for (const auto &extension : extension->GetExtensions()) {
                        auto url = extension->GetUrl();
                        std::transform(url.cbegin(), url.cend(), url.begin(),
                                       [](char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                        if (url == "status") {
                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExtension) {
                                auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                                if (value) {
                                    auto codings = value->GetCoding();
                                    if (!codings.empty()) {
                                        data.rfstatus = {codings[0].GetSystem(), codings[0].GetCode(), codings[0].GetDisplay(), codings[0].GetDisplay()};
                                    }
                                }
                            }
                        }
                    }
                } else if (url == "typeresept") {
                    auto vcv = codeableConceptAsMedicalCodedValue();
                    if (!vcv.GetCode().empty() || !vcv.GetDisplay().empty()) {
                        data.typeresept = vcv;
                    }
                }
            }
        }
    }
    if (fhir.GetAuthor().IsSet()) {
        data.prescribedByReference = fhir.GetAuthor().GetReference();
        data.prescribedByDisplay = fhir.GetAuthor().GetDisplay();
    }
    if (fhir.GetSubject().IsSet()) {
        data.subjectReference = fhir.GetSubject().GetReference();
        data.subjectDisplay = fhir.GetSubject().GetDisplay();
    }
    return data;
}

FhirBasic MerchData::ToFhir() const {
    SfmBandaPrescription basic{};
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        basic.SetId(uuidStr);
    }
    if (!prescribedByReference.empty()) {
        basic.SetAuthor({prescribedByReference, "http://ehelse.no/fhir/StructureDefinition/sfm-PractitionerRole", prescribedByDisplay});
    }
    if (!refund.productGroup.GetCode().empty()) {
        std::string system{refund.productGroup.GetSystem()};
        if (!system.starts_with("urn:oid:")) {
            std::string sys{system};
            system = "urn:oid:";
            system.append(sys);
        }
        FhirCodeableConcept codeable{system, refund.productGroup.GetCode(), refund.productGroup.GetDisplay()};
        basic.SetProductGroup(codeable);
    }
    {
        auto amendment = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment");
        if (startDate.operator bool()) {
            amendment->AddExtension(std::make_shared<FhirValueExtension>("reseptdate", std::make_shared<FhirDateValue>(startDate.ToString())));
        }
        if (expirationDate.operator bool()) {
            amendment->AddExtension(std::make_shared<FhirValueExtension>("expirationdate", std::make_shared<FhirDateValue>(expirationDate.ToString())));
        }
        if (!festUpdate.empty()) {
            amendment->AddExtension(std::make_shared<FhirValueExtension>("festUpdate", std::make_shared<FhirDateTimeValue>(festUpdate)));
        }
        amendment->AddExtension(std::make_shared<FhirValueExtension>("guardiantransparencyreservation", std::make_shared<FhirBooleanValue>(guardianTransparencyReservation)));
        amendment->AddExtension(std::make_shared<FhirValueExtension>("indoctorsname", std::make_shared<FhirBooleanValue>(inDoctorsName)));
        if (!dssn.empty()) {
            amendment->AddExtension(std::make_shared<FhirValueExtension>("dssn", std::make_shared<FhirString>(dssn)));
        }
        amendment->AddExtension(std::make_shared<FhirValueExtension>("lockedresept", std::make_shared<FhirBooleanValue>(lockedPrescription)));
        if (!itemGroup.GetCode().empty()) {
            FhirCodeableConcept codeable{itemGroup.GetSystem(), itemGroup.GetCode(), itemGroup.GetDisplay()};
            amendment->AddExtension(std::make_shared<FhirValueExtension>("itemgroup", std::make_shared<FhirCodeableConceptValue>(codeable)));
        }
        if (!refund.paragraph.GetCode().empty()) {
            std::string system{refund.paragraph.GetSystem()};
            if (!system.starts_with("urn:oid:")) {
                std::string sys{system};
                system = "urn:oid:";
                system.append(sys);
            }
            FhirCodeableConcept codeable{system, refund.paragraph.GetCode(), refund.paragraph.GetDisplay()};
            amendment->AddExtension(std::make_shared<FhirValueExtension>("reimbursementparagraph", std::make_shared<FhirCodeableConceptValue>(codeable)));
        }
        if (!rfstatus.GetCode().empty()) {
            auto extension = std::make_shared<FhirExtension>("rfstatus");
            FhirCodeableConcept codeable{rfstatus.GetSystem(), rfstatus.GetCode(), rfstatus.GetDisplay()};
            extension->AddExtension(std::make_shared<FhirValueExtension>("status", std::make_shared<FhirCodeableConceptValue>(codeable)));
            amendment->AddExtension(extension);
        }
        if (!typeresept.GetCode().empty()) {
            FhirCodeableConcept codeable{typeresept.GetSystem(), typeresept.GetCode(), typeresept.GetDisplay()};
            amendment->AddExtension(std::make_shared<FhirValueExtension>("typeresept", std::make_shared<FhirCodeableConceptValue>(codeable)));
        }
        amendment->AddExtension(std::make_shared<FhirValueExtension>(
                "createeresept",
                std::make_shared<FhirBooleanValue>(true)
        ));
        basic.AddExtension(amendment);
    }
    if (!subjectReference.empty()) {
        basic.SetSubject({subjectReference, "http://ehelse.no/fhir/StructureDefinition/sfm-Patient", subjectDisplay});
    }
    {
        FhirCodeableConcept identifierType{"ReseptId"};
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        FhirIdentifier identifier{identifierType, "usual", uuidStr};
        basic.SetIdentifiers({identifier});
    }
    return basic;
}