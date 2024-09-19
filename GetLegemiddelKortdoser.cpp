//
// Created by sigsegv on 9/12/24.
//

#include "GetLegemiddelKortdoser.h"
#include "MedicalCodedValue.h"
#include "FestDb.h"
#include <medfest/Struct/Packed/FestUuid.h>

GetLegemiddelKortdoser::GetLegemiddelKortdoser(const std::shared_ptr<FestDb> &festDb, const LegemiddelCore &legemiddelCore) : festDb(festDb) {
    MedicamentVisitorBase::Visit<void>(legemiddelCore, *this);
}

void GetLegemiddelKortdoser::Visit(const Legemiddelpakning &pakning) {
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

void GetLegemiddelKortdoser::Visit(const LegemiddelMerkevare &merkevare) {
    auto values = merkevare.GetAdministreringLegemiddel().GetKortdose();
    kortdoser.reserve(kortdoser.size() + values.size());
    for (const auto &value : values) {
        bool found{false};
        for (const auto &ex : kortdoser) {
            if (ex.GetValue() == value.GetValue()) {
                found = true;
                break;
            }
        }
        if (!found) {
            kortdoser.emplace_back(value);
        }
    }
}

void GetLegemiddelKortdoser::Visit(const LegemiddelVirkestoff &virkestoff) {
    std::vector<FestUuid> merkevareId{};
    {
        auto mIds = virkestoff.GetRefLegemiddelMerkevare();
        merkevareId.reserve(mIds.size());
        for (const auto &mId : mIds) {
            merkevareId.emplace_back(mId);
        }
    }
    auto merkevarer = festDb->FindLegemiddelMerkevare(merkevareId);
    for (const auto &merkevare : merkevarer) {
        Visit(merkevare);
    }
}

GetLegemiddelKortdoser::operator std::vector<MedicalCodedValue>() const {
    std::vector<MedicalCodedValue> mcv{};
    mcv.reserve(kortdoser.size());
    for (const auto &value : kortdoser) {
        mcv.emplace_back(value.GetCodeSet(), value.GetValue(), value.GetDistinguishedName(), value.GetDistinguishedName());
    }
    return mcv;
}