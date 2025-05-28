//
// Created by sigsegv on 7/26/24.
//

#include "PrescriptionChangesService.h"
#include "FestDb.h"
#include <sfmbasisapi/fhir/medstatement.h>
#include <sfmbasisapi/fhir/fhirbasic.h>

#include "DateOnly.h"
#include "Uuid.h"

const char *RenewalFailureException::what() const noexcept {
    return error.c_str();
}

template<RenewableFhirObject T> void PrescriptionChangesService::GenericRenew(T &medicationStatement) {
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
            FhirIdentifier replacement{identifier.GetType(), identifier.GetUse(), identifier.GetSystem(), Uuid::RandomUuidString()};
            *iterator = replacement;
            ++iterator;
        } else {
            ++iterator;
        }
    }
    if (reseptId.empty()) {
        throw RenewalFailureException("The prescription is not valid (no prescription id)");
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
    DateOnly createdDateTm{};
    DateOnly expirationDateTm{};
    auto nowT = DateOnly::Today();
    if (createdDateExt) {
        createdDateTm = DateOnly(createdDate);
        if (expirationDateExt) {
            expirationDateTm = DateOnly(expirationDate);
            DateOnly plusoney{createdDateTm};
            plusoney.AddYears(1);
            if (expirationDateTm == plusoney) {
                createdDateTm = nowT;
                expirationDateTm = createdDateTm;
                expirationDateTm.AddYears(1);
                expirationDateTm.AddDays(-1);
            } else {
                expirationDateTm.AddDays(1);
                auto diff = expirationDateTm - createdDateTm;
                createdDateTm = nowT;
                expirationDateTm = createdDateTm + diff;
                expirationDateTm.AddDays(-1);
            }
        } else {
            createdDateTm = nowT;
        }
    } else if (expirationDateExt) {
        expirationDateTm = DateOnly(expirationDate);
        if (expirationDateTm > nowT) {
            createdDateTm = nowT;
        } else {
            throw RenewalFailureException("Missing reseptdate");
        }
    }
    if (createdDateExt) {
        createdDateExt->SetValue(std::make_shared<FhirDateValue>(createdDateTm.ToString()));
    }
    if (expirationDateExt) {

        expirationDateExt->SetValue(std::make_shared<FhirDateValue>(expirationDateTm.ToString()));
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

void PrescriptionChangesService::Renew(FhirMedicationStatement &medicationStatement) {
    GenericRenew(medicationStatement);
}

void PrescriptionChangesService::Renew(FhirBasic &fhirBasic) {
    GenericRenew(fhirBasic);
}

void PrescriptionChangesService::RenewRevokedOrExpiredPll(FhirMedicationStatement &medicationStatement) {
    auto medicationStatementExtensions = medicationStatement.GetExtensions();
    std::shared_ptr<FhirExtension> reseptAmendment{};
    std::shared_ptr<FhirExtension> recallInfo{};
    std::shared_ptr<FhirExtension> rfstatus{};
    std::shared_ptr<FhirExtension> createereseptExt{};
    std::shared_ptr<FhirExtension> festUpdateExt{};
    for (const auto &medicationStatementExtension : medicationStatementExtensions) {
        auto url = medicationStatementExtension->GetUrl();
        std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
        if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
            reseptAmendment = medicationStatementExtension;
            auto reseptAmendmentExtensions = reseptAmendment->GetExtensions();
            auto reseptAmendmentExtensionsIterator = reseptAmendmentExtensions.begin();
            while (reseptAmendmentExtensionsIterator != reseptAmendmentExtensions.end()) {
                auto &reseptAmendmentExtension = *reseptAmendmentExtensionsIterator;
                auto url = reseptAmendmentExtension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                if (url == "recallinfo") {
                    recallInfo = reseptAmendmentExtension;
                    ++reseptAmendmentExtensionsIterator;
                } else if (url == "rfstatus") {
                    rfstatus = reseptAmendmentExtension;
                    ++reseptAmendmentExtensionsIterator;
                } else if (url == "createeresept") {
                    createereseptExt = reseptAmendmentExtension;
                    ++reseptAmendmentExtensionsIterator;
                } else if (url == "festupdate") {
                    festUpdateExt = reseptAmendmentExtension;
                    ++reseptAmendmentExtensionsIterator;
                } else {
                    ++reseptAmendmentExtensionsIterator;
                }
            }
            reseptAmendment->SetExtensions(reseptAmendmentExtensions);
        }
    }
    if (!reseptAmendment) {
        throw RenewalFailureException("Renew without amendment");
    }
    if (!rfstatus) {
        rfstatus = std::make_shared<FhirExtension>("rfstatus");
        reseptAmendment->AddExtension(rfstatus);
    }
    {
        std::vector<std::shared_ptr<FhirExtension>> exts{};
        {
            std::shared_ptr<FhirExtension> status = std::make_shared<FhirValueExtension>("status",
                                                                                         std::make_shared<FhirCodeableConceptValue>(
                                                                                                 FhirCodeableConcept(
                                                                                                         "urn:oid:2.16.578.1.12.4.1.1.7408",
                                                                                                         "E",
                                                                                                         "Ekspederbar")));
            exts.emplace_back(status);
        }
        rfstatus->SetExtensions(exts);
    }
    std::shared_ptr<FhirValueExtension> createeresept{};
    if (createereseptExt) {
        createeresept = std::dynamic_pointer_cast<FhirValueExtension>(createereseptExt);
        if (!createeresept) {
            throw RenewalFailureException("Ext createeresept is not a value extension");
        }
    }
    if (createeresept) {
        createeresept->SetValue(std::make_shared<FhirBooleanValue>(true));
    } else {
        createereseptExt = std::make_shared<FhirValueExtension>("createeresept", std::make_shared<FhirBooleanValue>(true));
        reseptAmendment->AddExtension(createereseptExt);
    }
    if (!festUpdateExt) {
        festUpdateExt = std::make_shared<FhirValueExtension>("festupdate", std::make_shared<FhirString>("2023-12-20T11:54:48.9287539+00:00")); // TODO
        reseptAmendment->AddExtension(festUpdateExt);
    }
    std::string originalReseptId{};
    std::string reseptId = Uuid::RandomUuidString();
    std::vector<FhirIdentifier> identifiers{};
    {
        bool hasReseptId{false};
        auto originalIdentifiers = medicationStatement.GetIdentifiers();
        for (const auto &identifier: originalIdentifiers) {
            auto typeObj = identifier.GetType();
            auto type = typeObj.GetText();
            std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) { return std::tolower(ch); });
            if (type == "reseptid") {
                hasReseptId = true;
                originalReseptId = identifier.GetValue();
                identifiers.emplace_back(typeObj, identifier.GetUse(), identifier.GetSystem(), reseptId);
            } else {
                identifiers.emplace_back(typeObj, identifier.GetUse(), identifier.GetSystem(), identifier.GetValue());
            }
        }
        if (hasReseptId) {
            if (recallInfo) {
                bool hasRecallId{false};
                auto recallExtensions = recallInfo->GetExtensions();
                for (const auto &recallExtension : recallExtensions) {
                    auto url = recallExtension->GetUrl();
                    std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                    if (url == "recallId") {
                        auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(recallExtension);
                        if (!valueExt) {
                            throw RenewalFailureException("RecallInfos recallId is not a value extension");
                        }
                        valueExt->SetValue(std::make_shared<FhirString>(originalReseptId));
                        hasRecallId = true;
                    }
                }
                if (!hasRecallId) {
                    recallExtensions.emplace_back(std::make_shared<FhirValueExtension>("recallId", std::make_shared<FhirString>(originalReseptId)));
                    recallInfo->SetExtensions(recallExtensions);
                }
            }
        } else {
            identifiers.emplace_back(FhirCodeableConcept({}, "ReseptId"), "usual", reseptId);
        }
    }
    medicationStatement.SetIdentifiers(identifiers);
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

template<CanGetPrescriptionStatusInfoFor T> PrescriptionStatusInfo PrescriptionChangesService::GetPrescriptionStatusInfoImpl(const T &medicationStatement) {
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
    bool ceased{false};
    std::string recallCode{};
    FhirCoding rfstatus{};
    auto statementExtensions = medicationStatement.GetExtensions();
    for (const auto &statementExtension : statementExtensions) {
        auto url = statementExtension->GetUrl();
        std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
        if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
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
        } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-discontinuation") {
            auto discontinuationExtensions = statementExtension->GetExtensions();
            for (const auto &discontinuationExtension : discontinuationExtensions) {
                auto url = discontinuationExtension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) {return std::tolower(ch);});
                if (url == "reason") {
                    auto reasonExt = std::dynamic_pointer_cast<FhirValueExtension>(discontinuationExtension);
                    if (reasonExt) {
                        auto reason = std::dynamic_pointer_cast<FhirCodeableConceptValue>(reasonExt->GetValue());
                        if (reason && reason->IsSet()) {
                            ceased = true;
                        }
                    }
                }
            }
        }
    }
    if (rfstatus.GetCode() == "T") {
        recalled = true;
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
        } else if (recallCode == "3") {
            prescriptionStatusInfo.IsValidPrescription = true;
            prescriptionStatusInfo.IsRenewedWithChanges = true;
        }
    }
    if (ceased) {
        prescriptionStatusInfo.IsValidPrescription = false;
        prescriptionStatusInfo.IsCeased = true;
    }
    if (prescriptionStatusInfo.IsValidPrescription && rfstatus.GetCode().empty()) {
        prescriptionStatusInfo.IsValidPrescription = false;
    }
    return prescriptionStatusInfo;
}

PrescriptionStatusInfo PrescriptionChangesService::GetPrescriptionStatusInfo(const FhirMedicationStatement &statement) {
    return GetPrescriptionStatusInfoImpl(statement);
}

PrescriptionStatusInfo PrescriptionChangesService::GetPrescriptionStatusInfo(const FhirBasic &basic) {
    return GetPrescriptionStatusInfoImpl(basic);
}

std::string PrescriptionChangesService::GetPrescriptionStatusString(const PrescriptionStatusInfo &info) {
    if (info.IsCreate) {
        if (!info.IsRecalled) {
            return "Draft";
        } else if (info.IsCeased) {
            return "Ceased";
        } else {
            if (info.IsRenewedWithoutChanges) {
                return "To be renewed";
            } else if (info.IsRenewedWithChanges) {
                return "To be renewed";
            } else {
                return "Ambiguous";
            }
        }
    } else if (info.IsCeased) {
        return "Ceased";
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