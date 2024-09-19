//
// Created by sigsegv on 9/17/24.
//

#include <medfest/Struct/Packed/FestUuid.h>
#include "GetMedicamentDosingUnit.h"
#include "FestDb.h"
#include "MedicalCodedValue.h"

GetMedicamentDosingUnit::GetMedicamentDosingUnit(const std::shared_ptr<FestDb> &festDb, const LegemiddelCore &legemiddelCore) : festDb(festDb) {
    MedicamentVisitorBase::Visit<void>(legemiddelCore, *this);
}

void GetMedicamentDosingUnit::Visit(const Legemiddelpakning &pakning) {
    std::vector<FestUuid> merkevareId{};
    {
        auto pakningsinfo = pakning.GetPakningsinfo();
        merkevareId.reserve(pakningsinfo.size());
        for (const auto pakningsinfo: pakningsinfo) {
            bool found{false};
            for (const auto &mi : merkevareId) {
                if (mi == pakningsinfo.GetMerkevareId()) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                merkevareId.emplace_back(pakningsinfo.GetMerkevareId());
            }
        }
    }
    auto merkevarer = festDb->FindLegemiddelMerkevare(merkevareId);
    for (const auto &merkevare : merkevarer) {
        Visit(merkevare);
    }
}

void GetMedicamentDosingUnit::Visit(const LegemiddelMerkevare &merkevare) {
    auto enhetDosering = merkevare.GetAdministreringLegemiddel().GetEnhetDosering();
    for (const auto &ex : dosingUnits) {
        if (ex.GetValue() == enhetDosering.GetValue()) {
            return;
        }
    }
    dosingUnits.emplace_back(enhetDosering);
}

void GetMedicamentDosingUnit::Visit(const LegemiddelVirkestoff &virkestoff) {
    auto enhetDosering = virkestoff.GetAdministreringLegemiddel().GetEnhetDosering();
    for (const auto &ex : dosingUnits) {
        if (ex.GetValue() == enhetDosering.GetValue()) {
            return;
        }
    }
    dosingUnits.emplace_back(enhetDosering);
}

GetMedicamentDosingUnit::operator std::vector<ValueWithCodeSet>() const {
    std::vector<ValueWithCodeSet> vc{};
    vc.reserve(dosingUnits.size());
    for (const auto &value : dosingUnits) {
        vc.emplace_back(value);
    }
    return vc;
}

GetMedicamentDosingUnit::operator std::vector<MedicalCodedValue>() const {
    std::vector<MedicalCodedValue> cv{};
    cv.reserve(dosingUnits.size());
    for (const auto &value : dosingUnits) {
        cv.emplace_back(value.GetCodeSet(), value.GetValue(), value.GetDistinguishedName(), value.GetDistinguishedName());
    }
    return cv;
}
