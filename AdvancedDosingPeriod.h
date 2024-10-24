//
// Created by sigsegv on 10/15/24.
//

#ifndef THEMASTER_ADVANCEDDOSINGPERIOD_H
#define THEMASTER_ADVANCEDDOSINGPERIOD_H

#include <string>
#include <memory>
#include "MedicalCodedValue.h"

class FhirExtendable;

class AdvancedDosingPeriod {
protected:
    MedicalCodedValue dosingUnit;
    int days;
public:
    AdvancedDosingPeriod(int days) : dosingUnit(), days(days) {}
    virtual std::string ToString() const = 0;
    virtual std::string ToDosingText() const = 0;
    virtual void Apply(FhirExtendable &extension) = 0;
    void SetDosingUnit(const MedicalCodedValue &);
    MedicalCodedValue GetDosingUnit();
    constexpr int GetDays() const {
        return days;
    }
};

class FixedTimeAdvancedDosingPeriod : public AdvancedDosingPeriod {
private:
    float morgen, formiddag, middag, ettermiddag, kveld, natt;
public:
    FixedTimeAdvancedDosingPeriod(float morgen, float formiddag, float middag, float ettermiddag, float kveld, float natt, int days) : AdvancedDosingPeriod(days), morgen(morgen), formiddag(formiddag), middag(middag), ettermiddag(ettermiddag), kveld(kveld), natt(natt) {}
    std::string ToString() const override;
    std::string ToDosingText() const override;
    void Apply(FhirExtendable &extension) override;
};

std::string ToDosingText(const std::vector<std::shared_ptr<AdvancedDosingPeriod>> &);

#endif //THEMASTER_ADVANCEDDOSINGPERIOD_H
