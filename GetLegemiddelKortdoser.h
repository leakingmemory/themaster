//
// Created by sigsegv on 9/12/24.
//

#ifndef THEMASTER_GETLEGEMIDDELKORTDOSER_H
#define THEMASTER_GETLEGEMIDDELKORTDOSER_H

#include "MedicamentVisitor.h"

class FestDb;
class MedicalCodedValue;

class GetLegemiddelKortdoser {
private:
    std::shared_ptr<FestDb> festDb;
    std::vector<ValueWithCodeSet> kortdoser{};
public:
    GetLegemiddelKortdoser(const std::shared_ptr<FestDb> &, const LegemiddelCore &legemiddelCore);
    void Visit(const LegemiddelMerkevare &);
    void Visit(const Legemiddelpakning &);
    void Visit(const LegemiddelVirkestoff &);
    explicit operator std::vector<ValueWithCodeSet> () const {
        return kortdoser;
    }
    explicit operator std::vector<MedicalCodedValue> () const;
};


#endif //THEMASTER_GETLEGEMIDDELKORTDOSER_H
