//
// Created by sigsegv on 7/26/24.
//

#include "PrescriptionChangesService.h"
#include "FestDb.h"
#include <sfmbasisapi/fhir/medstatement.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

const char *RenewalFailureException::what() const noexcept {
    return error.c_str();
}

void PrescriptionChangesService::Renew(FhirMedicationStatement &medicationStatement) {
    std::shared_ptr<FhirExtension> reseptAmendment{};
    {
        auto extensions = medicationStatement.GetExtensions();
        for (const auto extension : extensions) {
            auto url = extension->GetUrl();
            if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
                reseptAmendment = extension;
            }
        }
    }
    if (!reseptAmendment) {
        throw RenewalFailureException("The prescription is not valid (no reseptamendment)");
    }
    std::string reseptId{};
    auto identifiers = medicationStatement.GetIdentifiers();
    auto iterator = identifiers.begin();
    while (iterator != identifiers.end()) {
        auto identifier = *iterator;
        auto key = identifier.GetType().GetText();
        std::transform(key.cbegin(), key.cend(), key.begin(), [] (char ch) -> char { return std::tolower(ch); });
        if (key == "reseptid") {
            reseptId = identifier.GetValue();
            boost::uuids::random_generator generator;
            boost::uuids::uuid randomUUID = generator();
            std::string uuidStr = boost::uuids::to_string(randomUUID);
            FhirIdentifier replacement{identifier.GetType(), identifier.GetUse(), identifier.GetSystem(), uuidStr};
            *iterator = replacement;
            ++iterator;
        } else {
            ++iterator;
        }
    }
    medicationStatement.SetIdentifiers(identifiers);
    bool addCreate{true};
    bool addFestUpdate{true};
    std::string createdDate{};
    std::shared_ptr<FhirValueExtension> createdDateExt{};
    std::string expirationDate{};
    std::shared_ptr<FhirValueExtension> expirationDateExt{};

    {
        auto extensions = reseptAmendment->GetExtensions();
        for (const auto &extension : extensions) {
            auto url = extension->GetUrl();
            if (url == "recallinfo") {
                throw RenewalFailureException("The prescription is already recalled");
            }
            if (url == "createeresept") {
                auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                if (valueExt) {
                    auto value = std::dynamic_pointer_cast<FhirBooleanValue>(valueExt->GetValue());
                    value->SetValue(true);
                    addCreate = false;
                } else {
                    throw RenewalFailureException("The prescription has an incompatible value for createeresept");
                    return;
                }
            }
            if (url == "reseptdate") {
                auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                if (valueExt) {
                    auto value = std::dynamic_pointer_cast<FhirDateValue>(valueExt->GetValue());
                    if (value) {
                        createdDateExt = valueExt;
                        createdDate = value->GetRawValue();
                    }
                }
            }
            if (url == "expirationdate") {
                auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                if (valueExt) {
                    auto value = std::dynamic_pointer_cast<FhirDateValue>(valueExt->GetValue());
                    if (value) {
                        expirationDateExt = valueExt;
                        expirationDate = value->GetRawValue();
                    }
                }
            }
            if (url == "festUpdate") {
                addFestUpdate = false;
            }
        }
        if (addFestUpdate) {
            FestDb festDb{};
            auto versions = festDb.GetFestVersions();
            if (!versions.empty()) {
                auto version = versions[0];
                version.append("Z");
                reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>("festUpdate", std::make_shared<FhirDateTimeValue>(version)));
            }
        }
    }
    std::tm createdDateTm{};
    std::tm expirationDateTm{};
    if (createdDateExt) {
        int y, m, d;
        auto n = sscanf(createdDate.c_str(), "%d-%d-%d", &y, &m, &d);
        if (n != 3) {
            throw RenewalFailureException("Incorrect prescription date or expiration format");
        }
        createdDateTm.tm_year = y - 1900;
        createdDateTm.tm_mon = m - 1;
        createdDateTm.tm_mday = d;
    }
    if (expirationDateExt) {
        int y, m, d;
        auto n = sscanf(expirationDate.c_str(), "%d-%d-%d", &y, &m, &d);
        if (n != 3) {
            throw RenewalFailureException("Incorrect prescription date or expiration format");
        }
        expirationDateTm.tm_year = y - 1900;
        expirationDateTm.tm_mon = m - 1;
        expirationDateTm.tm_mday = d;
    }
    auto nowT = time(nullptr);
    if (createdDateExt) {
        if (expirationDateExt) {
            std::tm plusoney{createdDateTm};
            plusoney.tm_year++;
            auto p1 = mktime(&plusoney);
            auto expt = mktime(&expirationDateTm);
            auto diff = p1 - expt;
            if (diff >= 0 && diff <= (24 * 3600)) {
                if (localtime_r(&nowT, &createdDateTm) != &createdDateTm) {
                    throw RenewalFailureException("Failed to get current date");
                }
                std::tm cplusoney{createdDateTm};
                cplusoney.tm_year++;
                auto cp1 = mktime(&cplusoney);
                cp1 -= 24 * 3600;
                if (localtime_r(&cp1, &expirationDateTm) != &expirationDateTm) {
                    throw RenewalFailureException("Failed to get current date");
                }
            } else {
                auto p0 = mktime(&createdDateTm);
                diff = expt - p0;
                if (localtime_r(&nowT, &createdDateTm) != &createdDateTm) {
                    throw RenewalFailureException("Failed to get current date");
                }
                expt = nowT + diff;
                if (localtime_r(&expt, &expirationDateTm) != &expirationDateTm) {
                    throw RenewalFailureException("Failed to get current date");
                }
            }
        } else {
            if (localtime_r(&nowT, &createdDateTm) != &createdDateTm) {
                throw RenewalFailureException("Failed to get current date");
            }
        }
    } else if (expirationDateExt) {
        auto expt = mktime(&expirationDateTm);
        if (expt > nowT) {
            if (localtime_r(&nowT, &createdDateTm) != &createdDateTm) {
                throw RenewalFailureException("Failed to get current date");
            }
        } else {
            throw RenewalFailureException("Missing reseptdate");
        }
    }
    if (createdDateExt) {
        std::stringstream sstr{};
        sstr << (createdDateTm.tm_year + 1900) << "-" << (createdDateTm.tm_mon < 9 ? "0" : "");
        sstr << (createdDateTm.tm_mon + 1) << "-" << (createdDateTm.tm_mday < 10 ? "0" : "");
        sstr << createdDateTm.tm_mday;
        createdDateExt->SetValue(std::make_shared<FhirDateValue>(sstr.str()));
    }
    if (expirationDateExt) {
        std::stringstream sstr{};
        sstr << (expirationDateTm.tm_year + 1900) << "-" << (expirationDateTm.tm_mon < 9 ? "0" : "");
        sstr << (expirationDateTm.tm_mon + 1) << "-" << (expirationDateTm.tm_mday < 10 ? "0" : "");
        sstr << expirationDateTm.tm_mday;
        expirationDateExt->SetValue(std::make_shared<FhirDateValue>(sstr.str()));
    }
    {
        FhirCodeableConcept recallCode{"urn:oid:2.16.578.1.12.4.1.1.7500", "1", "Fornying"};
        auto recallInfoExt = std::make_shared<FhirExtension>("recallinfo");
        if (!reseptId.empty()) {
            recallInfoExt->AddExtension(
                    std::make_shared<FhirValueExtension>("recallId", std::make_shared<FhirString>(reseptId)));
        }
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("recallcode", std::make_shared<FhirCodeableConceptValue>(recallCode)));
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("text", std::make_shared<FhirString>("Forny uten endring")));
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("notsent", std::make_shared<FhirBooleanValue>(true)));
        reseptAmendment->AddExtension(recallInfoExt);
    }
    if (addCreate) {
        reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>("createeresept", std::make_shared<FhirBooleanValue>(true)));
    }
}

std::string PrescriptionChangesService::GetPreviousPrescriptionId(const FhirMedicationStatement &medicationStatement) {
    std::string previousPrescriptionId{};
    auto extensions = medicationStatement.GetExtensions();
    for (const auto &extension: extensions) {
        auto url = extension->GetUrl();
        std::transform(url.cbegin(), url.cend(), url.begin(), [](char ch) { return std::tolower(ch); });
        if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
            auto extensions = extension->GetExtensions();
            for (const auto &extension: extensions) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [](char ch) { return std::tolower(ch); });
                if (url == "recallinfo") {
                    auto extensions = extension->GetExtensions();
                    for (const auto &extension: extensions) {
                        auto url = extension->GetUrl();
                        std::transform(url.cbegin(), url.cend(), url.begin(),
                                       [](char ch) { return std::tolower(ch); });
                        if (url == "recallid") {
                            auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExt) {
                                auto value = std::dynamic_pointer_cast<FhirString>(valueExt->GetValue());
                                if (value) {
                                    previousPrescriptionId = value->GetValue();
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    return previousPrescriptionId;
}

std::string PrescriptionChangesService::GetPrescriptionId(const FhirMedicationStatement &medicationStatement) {
    std::string reseptId{};
    {
        auto identifiers = medicationStatement.GetIdentifiers();
        for (const auto &identifier : identifiers) {
            auto type = identifier.GetType().GetText();
            std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) { return std::tolower(ch); });
            if (type == "reseptid") {
                reseptId = identifier.GetValue();
            }
        }
    }
    return reseptId;
}

bool PrescriptionChangesService::IsRenewedWithoutChangesAssumingIsEprescription(const FhirMedicationStatement &medicationStatement) {
    std::string previousPrescriptionId{};
    std::string recallCode{};
    bool recallInfoNotSent{false};
    bool createResept{false};
    auto extensions = medicationStatement.GetExtensions();
    for (const auto &extension: extensions) {
        auto url = extension->GetUrl();
        std::transform(url.cbegin(), url.cend(), url.begin(), [](char ch) { return std::tolower(ch); });
        if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
            auto extensions = extension->GetExtensions();
            for (const auto &extension: extensions) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [](char ch) { return std::tolower(ch); });
                if (url == "recallinfo") {
                    auto extensions = extension->GetExtensions();
                    for (const auto &extension: extensions) {
                        auto url = extension->GetUrl();
                        std::transform(url.cbegin(), url.cend(), url.begin(),
                                       [](char ch) { return std::tolower(ch); });
                        if (url == "recallid") {
                            auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExt) {
                                auto value = std::dynamic_pointer_cast<FhirString>(valueExt->GetValue());
                                if (value) {
                                    previousPrescriptionId = value->GetValue();
                                }
                            }
                        } else if (url == "recallcode") {
                            auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExt) {
                                auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(
                                        valueExt->GetValue());
                                if (value) {
                                    auto codings = value->GetCoding();
                                    if (!codings.empty()) {
                                        recallCode = codings[0].GetCode();
                                    }
                                }
                            }
                        } else if (url == "notsent") {
                            recallInfoNotSent = true;
                        }
                    }
                } else if (url == "createeresept") {
                    createResept = true;
                }
            }
        }
    }
    if (createResept && recallInfoNotSent && recallCode == "1" && !previousPrescriptionId.empty()) {
        return true;
    }
    return false;
}

bool PrescriptionChangesService::IsRenewedWithoutChanges(const FhirMedicationStatement &medicationStatement) {
    auto prescriptionId = GetPrescriptionId(medicationStatement);
    return !prescriptionId.empty() && IsRenewedWithoutChangesAssumingIsEprescription(medicationStatement);
}

PrescriptionStatusInfo PrescriptionChangesService::GetPrescriptionStatusInfo(const FhirMedicationStatement &medicationStatement) {
    PrescriptionStatusInfo prescriptionStatusInfo{};
    auto identifiers = medicationStatement.GetIdentifiers();
    for (const auto &identifier : identifiers) {
        auto tp = identifier.GetType().GetText();
        std::transform(tp.cbegin(), tp.cend(), tp.begin(), [] (char ch) -> char {return std::tolower(ch);});
        if (tp == "reseptid") {
            prescriptionStatusInfo.IsValidPrescription = true;
            prescriptionStatusInfo.HasBeenValidPrescription = true;
        } else if (tp == "pll") {
            prescriptionStatusInfo.IsPll = true;
            prescriptionStatusInfo.HasBeenPll = true;
        }
    }
    bool createeresept{false};
    bool recalled{false};
    bool recallNotSent{false};
    std::string recallCode{};
    FhirCoding rfstatus{};
    auto statementExtensions = medicationStatement.GetExtensions();
    for (const auto &statementExtension : statementExtensions) {
        auto url = statementExtension->GetUrl();
        if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
            auto reseptAmendmentExtensions = statementExtension->GetExtensions();
            for (const auto &reseptAmendmentExtension : reseptAmendmentExtensions) {
                auto url = reseptAmendmentExtension->GetUrl();
                if (url == "createeresept") {
                    auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(reseptAmendmentExtension);
                    if (valueExt) {
                        auto value = std::dynamic_pointer_cast<FhirBooleanValue>(valueExt->GetValue());
                        if (value && value->IsTrue()) {
                            createeresept = true;
                        }
                    }
                }
                if (url == "recallinfo") {
                    auto extensions = reseptAmendmentExtension->GetExtensions();
                    for (const auto &extension : extensions) {
                        auto url = extension->GetUrl();
                        if (url == "recallcode") {
                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExtension) {
                                auto codeableConceptValue = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                                if (codeableConceptValue && codeableConceptValue->IsSet()) {
                                    auto coding = codeableConceptValue->GetCoding();
                                    if (!coding.empty()) {
                                        recallCode = coding[0].GetCode();
                                    }
                                    recalled = true;
                                }
                            }
                        }
                        if (url == "notsent") {
                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExtension) {
                                auto booleanExtension = std::dynamic_pointer_cast<FhirBooleanValue>(valueExtension->GetValue());
                                if (booleanExtension) {
                                    recallNotSent = booleanExtension->IsTrue();
                                }
                            }
                        }
                    }
                }
                if (url == "rfstatus") {
                    auto extensions = reseptAmendmentExtension->GetExtensions();
                    for (const auto &extension : extensions) {
                        auto url = extension->GetUrl();
                        if (url == "status") {
                            auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExt) {
                                auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExt->GetValue());
                                if (value) {
                                    auto codings = value->GetCoding();
                                    if (!codings.empty()) {
                                        rfstatus = codings[0];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (createeresept) {
        prescriptionStatusInfo.IsCreate = true;
    }
    if (recalled) {
        prescriptionStatusInfo.IsValidPrescription = false;
        prescriptionStatusInfo.IsRecalled = true;
        prescriptionStatusInfo.IsRecallNotSent = recallNotSent;
        if (recallCode == "1") {
            prescriptionStatusInfo.IsValidPrescription = true;
            prescriptionStatusInfo.IsRenewedWithoutChanges = true;
        }
    }
    if (prescriptionStatusInfo.IsValidPrescription && rfstatus.GetCode().empty()) {
        prescriptionStatusInfo.IsValidPrescription = false;
    }
    return prescriptionStatusInfo;
}

std::string PrescriptionChangesService::GetPrescriptionStatusString(const PrescriptionStatusInfo &info) {
    if (info.IsCreate) {
        if (!info.IsRecalled) {
            return "Draft";
        } else {
            if (info.IsRenewedWithoutChanges) {
                return "To be renewed";
            } else {
                return "Ambiguous";
            }
        }
    } else if (info.IsRecalled) {
        if (info.IsRecallNotSent) {
            return "To be recalled";
        } else {
            return "Recalled";
        }
    } else {
        if (info.IsValidPrescription) {
            return "Prescription";
        } else if (info.HasBeenValidPrescription) {
            return "Expired prescription";
        } else {
            return "Without prescription";
        }
    }
}