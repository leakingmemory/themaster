//
// Created by sigsegv on 9/17/24.
//

#ifndef THEMASTER_GETMEDICAMENTDOSINGUNIT_H
#define THEMASTER_GETMEDICAMENTDOSINGUNIT_H

#include "MedicamentVisitor.h"

class FestDb;
class MedicalCodedValue;

class GetMedicamentDosingUnit {
private:
    std::shared_ptr<FestDb> festDb;
    std::vector<EnhetDosering> dosingUnits{};
public:
    GetMedicamentDosingUnit(const std::shared_ptr<FestDb> &, const LegemiddelCore &legemiddelCore);
    void Visit(const LegemiddelMerkevare &);
    void Visit(const Legemiddelpakning &);
    void Visit(const LegemiddelVirkestoff &);
    explicit operator std::vector<EnhetDosering> () const {
        return dosingUnits;
    }
    explicit operator std::vector<ValueWithCodeSet> () const;
    explicit operator std::vector<MedicalCodedValue> () const;
};


#endif //THEMASTER_GETMEDICAMENTDOSINGUNIT_H
