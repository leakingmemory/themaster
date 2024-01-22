//
// Created by sigsegv on 1/16/24.
//

#ifndef DRWHATSNOT_UNITSOFMEASURE_H
#define DRWHATSNOT_UNITSOFMEASURE_H

#include <map>
#include <string>

class UnitsOfMeasure {
protected:
    std::map<std::string,std::string> units{};
public:
    [[nodiscard]] std::map<std::string,std::string> GetUnits() const {
        return units;
    }
    static const UnitsOfMeasure &GetUnitsForStrength();
};


#endif //DRWHATSNOT_UNITSOFMEASURE_H
