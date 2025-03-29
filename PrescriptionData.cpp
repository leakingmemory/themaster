//
// Created by sigsegv on 1/31/24.
//

#include "PrescriptionData.h"
#include "AdvancedDosingPeriod.h"
#include "DateTime.h"
#include "Lazy.h"
#include <sfmbasisapi/fhir/medstatement.h>
#include <boost/uuid/uuid_generators.hpp> // for random_generator
#include <boost/uuid/uuid_io.hpp> // for to_string
#include <functional>
#include <sstream>

struct DosingPeriodData {
    DateOnly startDate{};
    DateOnly endDate{};
    std::string amountUnit{};
    float morgen{0}, formiddag{0}, middag{0}, ettermiddag{0}, kveld{0}, natt{0};
};

static MedicalCodedValue GetMedicalCodedValue(const FhirCodeableConcept &codeableConcept) {
    auto codings = codeableConcept.GetCoding();
    if (!codings.empty()) {
        return {codings[0].GetSystem(), codings[0].GetCode(), codings[0].GetDisplay(), codings[0].GetDisplay()};
    }
    return {"", "", codeableConcept.GetText(), codeableConcept.GetText()};
}

void PrescriptionData::FromFhir(const FhirMedicationStatement &medicationStatement) {
    {
        auto dosages = medicationStatement.GetDosage();
        for (const auto &dosage : dosages) {
            dosingText = dosage.GetText();
            for (const auto &extension : dosage.GetExtensions()) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return std::tolower(ch); });
                if (url == "http://ehelse.no/fhir/structuredefinition/sfm-shortdosage") {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExtension) {
                        auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                        if (value) {
                            auto codings = value->GetCoding();
                            if (!codings.empty()) {
                                kortdose = {codings[0].GetSystem(), codings[0].GetCode(), codings[0].GetDisplay(), codings[0].GetDisplay()};
                            }
                        }
                    }
                } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-use") {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExtension) {
                        auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                        if (value) {
                            auto codings = value->GetCoding();
                            if (!codings.empty()) {
                                use = {codings[0].GetSystem(), codings[0].GetCode(), codings[0].GetDisplay(), codings[0].GetDisplay()};
                            }
                        }
                    }
                } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-application-area") {
                    for (const auto &extension : extension->GetExtensions()) {
                        auto url = extension->GetUrl();
                        std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                        if (url == "text") {
                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExtension) {
                                auto value = std::dynamic_pointer_cast<FhirString>(valueExtension->GetValue());
                                if (value) {
                                    applicationArea = value->GetValue();
                                }
                            }
                        }
                        if (url == "coded") {
                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExtension) {
                                auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                                if (value) {
                                    auto codings = value->GetCoding();
                                    if (!codings.empty()) {
                                        applicationArea = codings[0].GetDisplay();
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    for (const auto &extension : medicationStatement.GetExtensions()) {
        auto url = extension->GetUrl();
        std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
        if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
            std::vector<DosingPeriodData> dosingPeriodDataList{};
            bool invalidStructuredDosing{false};
            for (const auto &extension : extension->GetExtensions()) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                SingleThreadedLazy<std::function<std::shared_ptr<FhirValue> ()>> value{[&extension] () -> std::shared_ptr<FhirValue> {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExtension) {
                        return valueExtension->GetValue();
                    } else {
                        return {};
                    }
                }};
                SingleThreadedLazy<std::function<std::string ()>> stringValue{[&value] () -> std::string {
                    auto val = std::dynamic_pointer_cast<FhirString>(value.operator std::shared_ptr<FhirValue> &());
                    if (val) {
                        return val->GetValue();
                    } else {
                        return {};
                    }
                }};
                SingleThreadedLazy<std::function<double ()>> decimalValue([&value] () -> double {
                    auto val = std::dynamic_pointer_cast<FhirDecimalValue>(value.operator std::shared_ptr<FhirValue> &());
                    if (val) {
                        return val->GetValue();
                    } else {
                        return static_cast<double>(0.0f);
                    }
                });
                SingleThreadedLazy<std::function<int ()>> stringInterpretedAsInt([&stringValue] () -> int {
                    std::string str = stringValue.operator std::string();
                    if (str.empty()) {
                        return 0;
                    }
                    std::size_t err{0};
                    auto res = std::stoi(str, &err);
                    if (err != str.size()) {
                        return 0;
                    }
                    return res;
                });
                SingleThreadedLazy<std::function<FhirQuantity ()>> quantity([&value] () -> FhirQuantity {
                    auto quantityValue = std::dynamic_pointer_cast<FhirQuantityValue>(value.operator std::shared_ptr<FhirValue> &());
                    if (!quantityValue) {
                        return {};
                    }
                    return *quantityValue;
                });
                SingleThreadedLazy<std::function<FhirCodeableConcept ()>> codeableConcept([&value] () -> FhirCodeableConcept {
                    auto codeableConceptValue = std::dynamic_pointer_cast<FhirCodeableConceptValue>(value.operator std::shared_ptr<FhirValue> &());
                    if (!codeableConceptValue) {
                        return {};
                    }
                    return *codeableConceptValue;
                });
                if (url == "dssn") {
                    dssn = stringValue.operator std::string();
                } else if (url == "numberofpackages") {
                    numberOfPackages = decimalValue.operator double();
                    numberOfPackagesSet = (numberOfPackages > 0.001);
                } else if (url == "amount") {
                    amount = quantity.operator FhirQuantity &().GetValue();
                    amountUnit = {"", quantity.operator FhirQuantity &().GetUnit(), "", ""};
                    amountIsSet = (amount > 0.001);
                } else if (url == "reimbursementcode") {
                    reimbursementCode = GetMedicalCodedValue(codeableConcept.operator FhirCodeableConcept &());
                } else if (url == "reimbursementparagraph") {
                    reimbursementParagraph = GetMedicalCodedValue(codeableConcept.operator FhirCodeableConcept &());
                } else if (url == "reit") {
                    reit = stringInterpretedAsInt.operator int();
                } else if (url == "ereseptdosing") {
                    DosingPeriodData dosingPeriodData{};
                    bool fixed{true};
                    for (const auto &extension : extension->GetExtensions()) {
                        auto url = extension->GetUrl();
                        std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                        if (url == "starttime") {
                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExtension) {
                                auto value = std::dynamic_pointer_cast<FhirDateValue>(valueExtension->GetValue());
                                if (value) {
                                    try {
                                        dosingPeriodData.startDate = DateOnly(value->GetRawValue());
                                    } catch (...) {
                                        dosingPeriodData.startDate = {};
                                    }
                                }
                            }
                        } else if (url == "endtime") {
                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExtension) {
                                auto value = std::dynamic_pointer_cast<FhirDateValue>(valueExtension->GetValue());
                                if (value) {
                                    try {
                                        dosingPeriodData.endDate = DateOnly(value->GetRawValue());
                                    } catch (...) {
                                        dosingPeriodData.endDate = {};
                                    }
                                }
                            }
                        } else if (url == "repeatingdosage") {
                            double amount{0};
                            std::string timerangeCode{};
                            for (const auto &extension : extension->GetExtensions()) {
                                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                if (valueExtension) {
                                    auto url = valueExtension->GetUrl();
                                    std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                                    if (url == "amount") {
                                        auto quantity = std::dynamic_pointer_cast<FhirQuantityValue>(valueExtension->GetValue());
                                        if (quantity) {
                                           if (dosingPeriodData.amountUnit.empty() || dosingPeriodData.amountUnit == quantity->GetUnit()) {
                                               dosingPeriodData.amountUnit = quantity->GetUnit();
                                               amount = quantity->GetValue();
                                           } else {
                                               fixed = false;
                                           }
                                        }
                                    } else if (url == "interval") {
                                        auto quantity = std::dynamic_pointer_cast<FhirQuantityValue>(valueExtension->GetValue());
                                        if (quantity) {
                                            auto unit = quantity->GetUnit();
                                            std::transform(unit.cbegin(), unit.cend(), unit.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                                            if (fabs(quantity->GetValue() - 1.0) > 0.01 || unit != "døgn") {
                                                fixed = false;
                                            }
                                        }
                                    } else if (url == "accurate") {
                                        // TODO
                                    } else if (url == "timerange") {
                                        auto codeableValue = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                                        if (codeableValue) {
                                            auto codings = codeableValue->GetCoding();
                                            auto coding = std::find_if(codings.cbegin(), codings.cend(), [] (const FhirCoding &coding) {
                                                return coding.GetSystem().ends_with(".8325");
                                            });
                                            if (coding != codings.cend()) {
                                                timerangeCode = coding->GetCode();
                                            }
                                        }
                                    }
                                }
                            }
                            if (timerangeCode == "1") {
                                dosingPeriodData.morgen = static_cast<float>(amount);
                            } else if (timerangeCode == "2") {
                                dosingPeriodData.formiddag = static_cast<float>(amount);
                            } else if (timerangeCode == "3") {
                                dosingPeriodData.middag = static_cast<float>(amount);
                            } else if (timerangeCode == "4") {
                                dosingPeriodData.ettermiddag = static_cast<float>(amount);
                            } else if (timerangeCode == "5") {
                                dosingPeriodData.kveld = static_cast<float>(amount);
                            } else if (timerangeCode == "6") {
                                dosingPeriodData.natt = static_cast<float>(amount);
                            } else {
                                fixed = false;
                            }
                        }
                    }
                    if (fixed && dosingPeriodData.startDate) {
                        if (!dosingPeriodDataList.empty() && !((dosingPeriodDataList.end() - 1)->endDate)) {
                            invalidStructuredDosing = true;
                        }
                        dosingPeriodDataList.emplace_back(dosingPeriodData);
                    } else {
                        invalidStructuredDosing = true;
                    }
                }
            }
            if (!invalidStructuredDosing && !dosingPeriodDataList.empty()) {
                std::sort(dosingPeriodDataList.begin(), dosingPeriodDataList.end(), [] (const DosingPeriodData &p1, const DosingPeriodData &p2) -> bool {
                    return p1.startDate < p2.startDate;
                });
                auto startDate = dosingPeriodDataList.begin()->startDate;
                for (const auto &dosingPeriodData : dosingPeriodDataList) {
                    if (!startDate || dosingPeriodData.startDate < startDate) {
                        invalidStructuredDosing = true;
                        break;
                    }
                    if (dosingPeriodData.startDate > startDate) {
                        auto days = (dosingPeriodData.startDate - startDate).GetDays();
                        dosingPeriods.emplace_back(std::make_shared<FixedTimeAdvancedDosingPeriod>(0,0,0,0,0,0,days));
                    }
                    auto days = dosingPeriodData.endDate ? (dosingPeriodData.endDate - dosingPeriodData.startDate).GetDays() : 0;
                    dosingPeriods.emplace_back(std::make_shared<FixedTimeAdvancedDosingPeriod>(dosingPeriodData.morgen, dosingPeriodData.formiddag, dosingPeriodData.middag, dosingPeriodData.ettermiddag, dosingPeriodData.kveld, dosingPeriodData.natt, days));
                    startDate = dosingPeriodData.endDate;
                }
                if (invalidStructuredDosing) {
                    dosingPeriods.clear();
                }
            }
        } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-generic-substitution") {
            for (const auto &extension : extension->GetExtensions()) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                if (url == "genericsubstitutionaccepted") {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExtension) {
                        auto value = std::dynamic_pointer_cast<FhirBooleanValue>(valueExtension->GetValue());
                        if (value) {
                            genericSubstitutionAccepted = value->IsTrue();
                        }
                    }
                }
            }
        } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-discontinuation") {
            for (const auto &extension : extension->GetExtensions()) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                if (url == "timedate") {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExtension) {
                        auto value = std::dynamic_pointer_cast<FhirDateTimeValue>(valueExtension->GetValue());
                        if (value) {
                            try {
                                ceaseDate = DateOnly::FromDateTimeOffsetString(value->GetDateTime());
                            } catch (...) {
                            }
                        }
                    }
                }
            }
        }
    }
}

FhirMedicationStatement PrescriptionData::ToFhir() const {
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
            std::shared_ptr<FhirValueExtension> applicationAreaExt;
            if (applicationAreaCoded.GetCode().empty()) {
                applicationAreaExt = std::make_shared<FhirValueExtension>(
                        "text",
                        std::make_shared<FhirString>(applicationArea)
                );
            } else {
                applicationAreaExt = std::make_shared<FhirValueExtension>(
                        "coded",
                        std::make_shared<FhirCodeableConceptValue>(applicationAreaCoded.ToCodeableConcept())
                );
            }
            std::shared_ptr<FhirExtension> applicationAreaOuterExt = std::make_shared<FhirExtension>(
                    "http://ehelse.no/fhir/StructureDefinition/sfm-application-area"
            );
            applicationAreaOuterExt->AddExtension(applicationAreaExt);
            dosage.AddExtension(applicationAreaOuterExt);
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
        if (!reimbursementCode.GetCode().empty()) {
            auto system = reimbursementCode.GetSystem();
            if (!system.starts_with("urn:oid:")) {
                std::string sys{"urn:oid:"};
                sys.append(system);
                system = sys;
            }
            FhirCodeableConcept codeable{system, reimbursementCode.GetCode(), reimbursementCode.GetDisplay()};
            reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                    "reimbursementcode",
                    std::make_shared<FhirCodeableConceptValue>(codeable)
            ));
        }
        if (!reimbursementParagraph.GetCode().empty()) {
            auto system = reimbursementParagraph.GetSystem();
            if (!system.starts_with("urn:oid:")) {
                std::string sys{"urn:oid:"};
                sys.append(system);
                system = sys;
            }
            FhirCodeableConcept codeable{system, reimbursementParagraph.GetCode(), reimbursementParagraph.GetDisplay()};
            reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                    "reimbursementparagraph",
                    std::make_shared<FhirCodeableConceptValue>(codeable)
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
        if (lockedPrescription) {
            reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                    "lockedresept",
                    std::make_shared<FhirBooleanValue>(true)
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
        if (!rfstatus.GetCode().empty()) {
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
        if (this->typeresept.GetCode() != "U") {
            reseptAmendment->AddExtension(std::make_shared<FhirValueExtension>(
                    "createeresept",
                    std::make_shared<FhirBooleanValue>(true)
            ));
        }
        fhir.AddExtension(reseptAmendment);
    }
    if (ceaseDate) {
        auto discontinuation = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-discontinuation");
        {
            {
                auto dt = DateTimeOffset::FromDate(ceaseDate);
                auto timedate = std::make_shared<FhirValueExtension>("timedate", std::make_shared<FhirDateTimeValue>(dt.to_iso8601()));
                discontinuation->AddExtension(timedate);
            }
            {
                std::vector<FhirCoding> codings{};
                codings.emplace_back("urn:oid:2.16.578.1.12.4.1.1.7494", "A", "Avsluttet behandling");
                FhirCodeableConcept codeable{std::move(codings)};
                discontinuation->AddExtension(std::make_shared<FhirValueExtension>("reason", std::make_shared<FhirCodeableConceptValue>(codeable)));
            }
        }
        fhir.AddExtension(discontinuation);
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
            std::string nowString = DateTimeOffset::Now().to_iso8601();

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
    if (this->typeresept.GetCode() != "U") {
        FhirCodeableConcept identifierType{"ReseptId"};
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        FhirIdentifier identifier{identifierType, "usual", uuidStr};
        fhir.AddIdentifier(identifier);
    }
    return fhir;
}
