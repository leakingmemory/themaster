//
// Created by sigsegv on 1/17/24.
//

#ifndef DRWHATSNOT_MEDICALCODEDVALUE_H
#define DRWHATSNOT_MEDICALCODEDVALUE_H

#include <string>
#include <vector>

class FhirCodeableConcept;

class MedicalCodedValue {
private:
    std::string system;
    std::string code;
    std::string display;
    std::string shortDisplay;
public:
    MedicalCodedValue() : system(), code(), display(), shortDisplay() {}
    MedicalCodedValue(const std::string &system, const std::string &code, const std::string &display, const std::string &shortDisplay) : system(system), code(code), display(display), shortDisplay(shortDisplay) {}
    [[nodiscard]] std::string GetSystem() const { return system; }
    [[nodiscard]] std::string GetCode() const { return code; }
    [[nodiscard]] std::string GetDisplay() const { return display; }
    [[nodiscard]] std::string GetShortDisplay() const { return shortDisplay; }
    [[nodiscard]] FhirCodeableConcept ToCodeableConcept() const;
    static std::vector<MedicalCodedValue> GetVolvenMedicamentForm();
    static std::vector<MedicalCodedValue> GetVolvenRecallCode();
};

#endif //DRWHATSNOT_MEDICALCODEDVALUE_H