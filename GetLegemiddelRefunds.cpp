//
// Created by sigsegv on 2/14/25.
//

#include "GetLegemiddelRefunds.h"
#include "MedicamentVisitor.h"
#include "DateOnly.h"
#include "FestDb.h"
#include <medfest/Struct/Decoded/OppfRefusjon.h>

GetLegemiddelRefunds::GetLegemiddelRefunds(const LegemiddelCore &legemiddelCore) {
    MedicamentVisitorBase::Visit<void>(legemiddelCore, *this);
}

void GetLegemiddelRefunds::Visit(const LegemiddelMerkevare &merkevare) {
}

void GetLegemiddelRefunds::Visit(const Legemiddelpakning &pakning) {
    auto today = DateOnly::Today();
    for (const auto &refund : pakning.GetRefusjonList()) {
        {
            auto validFromDateStr = refund.GetGyldigFraDato();
            if (!validFromDateStr.empty()) {
                DateOnly validFromDate{validFromDateStr};
                if (validFromDate > today) {
                    continue;
                }
            }
            auto validToDateStr = refund.GetForskrivesTilDato();
            if (!validToDateStr.empty()) {
                DateOnly validToDate{validToDateStr};
                if (validToDate < today) {
                    continue;
                }
            }
        }
        for (const auto &ref : refund.GetRefRefusjonsgruppe()) {
            if (std::find(refunds.begin(), refunds.end(), ref) == refunds.end()) {
                refunds.emplace_back(ref);
            }
        }
    }
}

void GetLegemiddelRefunds::Visit(const LegemiddelVirkestoff &virkestoff) {
    auto today = DateOnly::Today();
    for (const auto &refund : virkestoff.GetRefusjon()) {
        {
            auto validFromDateStr = refund.GetGyldigFraDato();
            if (!validFromDateStr.empty()) {
                DateOnly validFromDate{validFromDateStr};
                if (validFromDate > today) {
                    continue;
                }
            }
            auto validToDateStr = refund.GetForskrivesTilDato();
            if (!validToDateStr.empty()) {
                DateOnly validToDate{validToDateStr};
                if (validToDate < today) {
                    continue;
                }
            }
        }
        for (const auto &ref : refund.GetRefRefusjonsgruppe()) {
            if (std::find(refunds.begin(), refunds.end(), ref) == refunds.end()) {
                refunds.emplace_back(ref);
            }
        }
    }
}

std::vector<MedicamentRefund> GetLegemiddelRefunds::GetMedicamentRefunds(FestDb &festDb, const std::vector<std::string> &refs) {
    std::vector<MedicamentRefund> refunds{};
    auto activeVersion = festDb.GetFestVersions();
    if (activeVersion.empty()) {
        return {};
    }
    auto oppfs = festDb.GetOppfRefusjon(activeVersion[0]);
    auto iterator = oppfs.begin();
    std::map<std::string,std::string> gruppeToOppf{};
    std::map<std::string,OppfRefusjon> oppfMap{};
    for (const auto &ref : refs) {
        auto find_grp = gruppeToOppf.find(ref);
        std::string oppfId{};
        if (find_grp != gruppeToOppf.end()) {
            oppfId = find_grp->second;
        } else {
            while (iterator != oppfs.end()) {
                const auto &oppf = *iterator;
                oppfMap.insert_or_assign(oppf.GetId(), oppf);
                auto refusjonsgrupper = oppf.GetRefusjonshjemmel().GetRefusjonsgruppeList();
                for (const auto &refGrp : refusjonsgrupper) {
                    gruppeToOppf.insert_or_assign(refGrp.GetId(), oppf.GetId());
                    if (refGrp.GetId() == ref) {
                        oppfId = oppf.GetId();
                    }
                }
                ++iterator;
                if (!oppfId.empty()) {
                    break;
                }
            }
        }
        if (oppfId.empty()) {
            continue;
        }
        auto oppfFind = oppfMap.find(oppfId);
        if (oppfFind == oppfMap.end()) {
            throw std::exception();
        }
        std::vector<MedicalCodedValue> codes{};
        auto refhj = oppfFind->second.GetRefusjonshjemmel().GetRefusjonshjemmel();
        for (const auto &grp : oppfFind->second.GetRefusjonshjemmel().GetRefusjonsgruppeList()) {
            if (ref == grp.GetId()) {
                auto today = DateOnly::Today();
                for (const auto &code : grp.GetRefusjonskode()) {
                    if (!code.GetGyldigFraDato().empty()) {
                        DateOnly gyldigFra{code.GetGyldigFraDato()};
                        if (gyldigFra > today) {
                            continue;
                        }
                    }
                    if (!code.GetForskrivesTilDato().empty()) {
                        DateOnly forskrivesTil{code.GetForskrivesTilDato()};
                        if (forskrivesTil < today) {
                            continue;
                        }
                    }
                    auto refCode = code.GetRefusjonskode();
                    codes.emplace_back(refCode.GetCodeSet(), refCode.GetValue(), refCode.GetDistinguishedName(), refCode.GetDistinguishedName());
                }
            }
        }
        MedicamentRefund medicamentRefund{
            .refund = MedicalCodedValue(refhj.GetCodeSet(), refhj.GetValue(), refhj.GetDistinguishedName(), refhj.GetDistinguishedName()),
            .codes = codes
        };
        refunds.emplace_back(std::move(medicamentRefund));
    }
    return refunds;
}
