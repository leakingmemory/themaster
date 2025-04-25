//
// Created by sigsegv on 2/14/25.
//

#ifndef THEMASTER_GETLEGEMIDDELREFUNDS_H
#define THEMASTER_GETLEGEMIDDELREFUNDS_H

#include <string>
#include <vector>
#include "MedicamentRefund.h"

class MedicamentVisitorBase;
class LegemiddelCore;
class LegemiddelMerkevare;
class Legemiddelpakning;
class LegemiddelVirkestoff;
class FestDb;

class GetLegemiddelRefunds {
    friend MedicamentVisitorBase;
private:
    std::vector<std::string> refunds{};
public:
    GetLegemiddelRefunds(const LegemiddelCore &legemiddelCore);
private:
    void Visit(const LegemiddelMerkevare &);
    void Visit(const Legemiddelpakning &);
    void Visit(const LegemiddelVirkestoff &);
public:
    operator std::vector<std::string> () const {
        return refunds;
    }
    static std::vector<MedicamentRefund> GetMedicamentRefunds(FestDb &festDb, const std::vector<std::string> &refs);
};


#endif //THEMASTER_GETLEGEMIDDELREFUNDS_H
