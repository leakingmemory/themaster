//
// Created by sigsegv on 10/15/24.
//

#include "AdvancedDosingPeriod.h"
#include <sstream>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sfmbasisapi/fhir/fhir.h>
#include <sfmbasisapi/fhir/value.h>

void AdvancedDosingPeriod::SetDosingUnit(const MedicalCodedValue &du) {
    dosingUnit = du;
}

MedicalCodedValue AdvancedDosingPeriod::GetDosingUnit() {
    return dosingUnit;
}

std::string FixedTimeAdvancedDosingPeriod::ToString() const {
    std::stringstream str{};
    str << morgen << "+" << formiddag << "+" << middag << "+" << ettermiddag << "+" << kveld;
    if (days > 0) {
        str << "x" << days;
    }
    return str.str();
}

static constexpr void dosage(std::stringstream &str, float amount) {
    str << amount;
    if (std::abs(amount - 1.0) < 0.01) {
        str << " <dose>";
    } else {
        str << " <doser>";
    }
}

std::string FixedTimeAdvancedDosingPeriod::ToDosingText() const {
    std::stringstream str{};
    bool sep{false};
    if (morgen > 0.01) {
        dosage(str, morgen);
        str << " morgen";
        sep = true;
    }
    if (formiddag > 0.01) {
        if (sep) {
            str << " ";
        }
        dosage(str, formiddag);
        str << " formiddag";
        sep = true;
    }
    if (middag > 0.01) {
        if (sep) {
            str << " ";
        }
        dosage(str, middag);
        str << " midt på dagen";
        sep = true;
    }
    if (ettermiddag > 0.01) {
        if (sep) {
            str << " ";
        }
        dosage(str, ettermiddag);
        str << " ettermiddag";
        sep = true;
    }
    if (kveld > 0.01) {
        if (sep) {
            str << " ";
        }
        dosage(str, kveld);
        str << " kveld";
        sep = true;
    }
    if (natt > 0.01) {
        if (sep) {
            str << " ";
        }
        dosage(str, natt);
        str << " natt";
        sep = true;
    }
    if (!sep) {
        str << "ingen dosering";
    }
    str << " i " << days << " dager";
    return str.str();
}

constexpr void AddFixedDosing(FhirExtendable &extendable, double amount, std::string unit, double interval, std::string intervalUnit, bool accurate, const std::string &timerangeCode) {
    auto ext = std::make_shared<FhirExtension>("repeatingdosage");
    ext->AddExtension(std::make_shared<FhirValueExtension>("amount", std::make_shared<FhirQuantityValue>(FhirQuantity(amount, unit))));
    ext->AddExtension(std::make_shared<FhirValueExtension>("interval", std::make_shared<FhirQuantityValue>(FhirQuantity(interval, intervalUnit))));
    ext->AddExtension(std::make_shared<FhirValueExtension>("accurate", std::make_shared<FhirBooleanValue>(accurate)));
    std::string timerangeName;
    if (timerangeCode == "1") {
        timerangeName = "Morgen";
    } else if (timerangeCode == "2") {
        timerangeName = "Formiddag";
    } else if (timerangeCode == "3") {
        timerangeName = "Midt på dagen";
    } else if (timerangeCode == "4") {
        timerangeName = "Ettermiddag";
    } else if (timerangeCode == "5") {
        timerangeName = "Kveld";
    } else if (timerangeCode == "6") {
        timerangeName = "Natt";
    } else {
        timerangeName = "Ukjent";
    }
    ext->AddExtension(std::make_shared<FhirValueExtension>("timerange", std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept("urn:oid:2.16.578.1.12.4.1.1.8325", timerangeCode, timerangeName))));
    extendable.AddExtension(ext);
}

void FixedTimeAdvancedDosingPeriod::Apply(FhirExtendable &extension) {
    auto dosingUnit = GetDosingUnit().GetDisplay();
    std::transform(dosingUnit.cbegin(), dosingUnit.cend(), dosingUnit.begin(), [] (char ch) { return std::tolower(ch); });
    if (morgen > 0.01) {
        AddFixedDosing(extension, morgen, dosingUnit, 1, "Døgn", false, "1");
    }
    if (formiddag > 0.01) {
        AddFixedDosing(extension, formiddag, dosingUnit, 1, "Døgn", false, "2");
    }
    if (middag > 0.01) {
        AddFixedDosing(extension, middag, dosingUnit, 1, "Døgn", false, "3");
    }
    if (ettermiddag > 0.01) {
        AddFixedDosing(extension, ettermiddag, dosingUnit, 1, "Døgn", false, "4");
    }
    if (kveld > 0.01) {
        AddFixedDosing(extension, kveld, dosingUnit, 1, "Kveld", false, "5");
    }
    if (natt > 0.01) {
        AddFixedDosing(extension, natt, dosingUnit, 1, "Døgn", false, "6");
    }
}

constexpr static std::string __ToDosingText(const std::vector<std::shared_ptr<AdvancedDosingPeriod>> &dosingPeriods) {
    std::vector<std::string> dosingPeriodTexts{};
    dosingPeriodTexts.resize(dosingPeriods.size());
    std::transform(dosingPeriods.cbegin(), dosingPeriods.cend(), dosingPeriodTexts.begin(), [] (const std::shared_ptr<AdvancedDosingPeriod> &dp) constexpr -> std::string { return dp->ToDosingText(); });
    if (dosingPeriodTexts.empty()) {
        return "Ingen dosering";
    }
    std::string dosingText = std::accumulate(dosingPeriodTexts.cbegin() + 1, dosingPeriodTexts.cend(), dosingPeriodTexts[0], [] (const std::string &t1, const std::string &t2) constexpr -> std::string {
        static constexpr const char glue[] = ".\nDeretter ";
        std::string accumulated{};
        accumulated.reserve(t1.size() + t2.size() + sizeof(glue) - 1);
        accumulated.append(t1);
        accumulated.append(glue);
        accumulated.append(t2);
        return accumulated;
    });
    if (dosingText.empty()) {
        return "Ingen dosering";
    }
    dosingText[0] = static_cast<char>(std::toupper(dosingText[0]));
    return dosingText;
}

std::string ToDosingText(const std::vector<std::shared_ptr<AdvancedDosingPeriod>> &dosingPeriods) {
    return __ToDosingText(dosingPeriods);
}