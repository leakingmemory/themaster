//
// Created by sigsegv on 1/6/25.
//

#include "MerchTree.h"
#include <medfest/Struct/Decoded/OppfHandelsvare.h>
#include <medfest/Struct/Decoded/OppfRefusjon.h>
#include "DateOnly.h"
#include "FestDb.h"
#include "MedicalCodedValue.h"

Handelsvare GetHandelsvare(const OppfMedForbrMatr &oppf) {
    return oppf.GetMedForbrMatr();
}

Handelsvare GetHandelsvare(const OppfNaringsmiddel &oppf) {
    return oppf.GetNaringsmiddel();
}

Handelsvare GetHandelsvare(const OppfBrystprotese &oppf) {
    return oppf.GetBrystprotese();
}

template <CanGetHandelsvare T> std::vector<std::string> GetRid(const T &oppf) {
    auto refusjon = GetHandelsvare(oppf).GetRefusjon();
    auto dateToday = DateOnly::Today();
    if (!refusjon.GetForskrivesTilDato().empty()) {
        DateOnly validFrom{refusjon.GetGyldigFraDato()};
        if (dateToday < validFrom) {
            return {};
        }
    }
    if (!refusjon.GetForskrivesTilDato().empty()) {
        DateOnly validTo{refusjon.GetForskrivesTilDato()};
        if (dateToday > validTo) {
            return {};
        }
    }
    return refusjon.GetRefRefusjonsgruppe();
}

template <CanGetHandelsvare T> std::string GetNr(const T &oppf) {
    return GetHandelsvare(oppf).GetNr();
}

class RidExplorer {
private:
    std::vector<OppfRefusjon> refusjonVec{};
    std::vector<OppfRefusjon>::const_iterator iterator{};
    std::map<std::string,std::shared_ptr<OppfRefusjon>> refusjonMap{};
public:
    RidExplorer(const FestDb &festDb, const std::string &festVersion);
    std::shared_ptr<OppfRefusjon> GetByRid(const std::string &rid);
};

RidExplorer::RidExplorer(const FestDb &festDb, const std::string &festVersion) {
    refusjonVec = festDb.GetOppfRefusjon(festVersion);
    iterator = refusjonVec.begin();
}

std::shared_ptr<OppfRefusjon> RidExplorer::GetByRid(const std::string &rid) {
    {
        auto f = refusjonMap.find(rid);
        if (f != refusjonMap.end()) {
            return f->second;
        }
    }
    while (iterator != refusjonVec.end()) {
        auto &refusjon = *iterator;
        ++iterator;
        auto refusjonshjemmel = refusjon.GetRefusjonshjemmel();
        auto refusjonsgruppeList = refusjonshjemmel.GetRefusjonsgruppeList();
        std::shared_ptr<OppfRefusjon> shptr{};
        bool found{false};
        for (const auto &refusjonsgruppe : refusjonsgruppeList) {
            auto theRid = refusjonsgruppe.GetId();
            if (refusjonMap.find(theRid) == refusjonMap.end()) {
                if (!shptr) {
                    shptr = std::make_shared<OppfRefusjon>(refusjon);
                }
                refusjonMap.insert_or_assign(theRid, shptr);
                if (theRid == rid) {
                    found = true;
                }
            }
        }
        if (found) {
            return shptr;
        }
    }
    return {};
}

template <CanGetHandelsvare T> void MerchTreeImpl::MapElements(const FestDb &festDb, const std::string &festVersion, const std::vector<T> &elements) {
    std::map<std::string,std::shared_ptr<ContainerElement>> refusjonToElement{};
    for (const auto &element : elements) {
        auto ridVec = GetRid(element);
        if (ridVec.empty()) {
            continue;
        }
        auto sh = std::make_shared<ContainerElement>(element);
        for (const auto rid : ridVec) {
            refusjonToElement.insert_or_assign(rid, sh);
            this->refusjonToElement.insert_or_assign(GetNr(element), sh);
        }
    }
    RidExplorer ridExplorer{festDb, festVersion};
    for (const auto &[rid, element] : refusjonToElement) {
        auto refusjon = ridExplorer.GetByRid(rid);
        std::map<std::string,std::string> rid_to_grp{};
        std::map<std::string,MedicalCodedValue> rid_to_grpname{};
        for (const auto &gruppe : refusjon->GetRefusjonshjemmel().GetRefusjonsgruppeList()) {
            auto rid = gruppe.GetId();
            auto grpsys = gruppe.GetGruppeNr().GetCodeSet();
            auto grpnr = gruppe.GetGruppeNr().GetValue();
            auto grpname = gruppe.GetGruppeNr().GetDistinguishedName();
            if (!rid.empty() && !grpnr.empty()) {
                rid_to_grp.insert_or_assign(rid, grpnr);
                rid_to_grpname.insert_or_assign(rid, MedicalCodedValue(grpsys, grpnr, grpname, grpname));
            }
        }
        std::string grpno{};
        {
            auto grpit = rid_to_grp.find(rid);
            if (grpit == rid_to_grp.end()) {
                continue;
            }
            grpno = grpit->second;
        }
        std::vector<std::string> substr_grps{};
        std::map<std::string,std::string> grp_to_rid{};
        for (const auto &[rid, grp] : rid_to_grp) {
            if (grp.size() < grpno.size() && grpno.starts_with(grp)) {
                substr_grps.emplace_back(grp);
                grp_to_rid.insert_or_assign(grp, rid);
            }
        }
        std::sort(substr_grps.begin(), substr_grps.end(), [] (const std::string &a, const std::string &b) -> bool {
            return a.size() < b.size();
        });
        {
            auto iterator = substr_grps.begin();
            while (iterator != substr_grps.end()) {
                auto value = *iterator;
                ++iterator;
                while (iterator != substr_grps.end() && value == *iterator) {
                    iterator = substr_grps.erase(iterator);
                }
            }
        }
        MerchRefund *refund = nullptr;
        for (auto &r : refunds) {
            if (r.id == refusjon->GetId()) {
                refund = &r;
                break;
            }
        }
        if (refund == nullptr) {
            refund = &(refunds.emplace_back());
            refund->id = refusjon->GetId();
            auto refusjonshjemmel = refusjon->GetRefusjonshjemmel().GetRefusjonshjemmel();
            refund->refund = {refusjonshjemmel.GetCodeSet(), refusjonshjemmel.GetValue(), refusjonshjemmel.GetDistinguishedName(), refusjonshjemmel.GetDistinguishedName()};
        }
        MerchNode *node = nullptr;
        {
            auto grpit = substr_grps.begin();
            if (grpit == substr_grps.end()) {
                continue;
            }
            {
                auto grp = *grpit;
                grpit = substr_grps.erase(grpit);
                auto rid = grp_to_rid.find(grp)->second;
                for (auto &n: refund->nodes) {
                    if (n.id == rid) {
                        node = &n;
                        break;
                    }
                }
                if (node == nullptr) {
                    node = &(refund->nodes.emplace_back());
                    node->id = rid;
                    {
                        auto f = rid_to_grpname.find(rid);
                        if (f != rid_to_grpname.end()) {
                            node->grp = f->second;
                        }
                    }
                }
            }
            while (grpit != substr_grps.end()) {
                auto grp = *grpit;
                grpit = substr_grps.erase(grpit);
                auto rid = grp_to_rid.find(grp)->second;
                std::shared_ptr<MerchNode> nn{};
                for (auto &n : node->nodes) {
                    if (!std::holds_alternative<std::shared_ptr<MerchNode>>(n)) {
                        continue;
                    }
                    if (std::get<std::shared_ptr<MerchNode>>(n)->id == rid) {
                        nn = std::get<std::shared_ptr<MerchNode>>(n);
                        break;
                    }
                }
                if (!nn) {
                    nn = std::get<std::shared_ptr<MerchNode>>(node->nodes.emplace_back(std::make_shared<MerchNode>()));
                    nn->id = rid;
                    {
                        auto f = rid_to_grpname.find(rid);
                        if (f != rid_to_grpname.end()) {
                            nn->grp = f->second;
                        }
                    }
                }
                node = &*nn;
            }
            auto nr = std::visit([](const auto &arg) -> std::string { return GetHandelsvare(arg).GetNr(); }, *element);
            node->nodes.emplace_back(nr);
        }
    }
}

MerchTreeImpl::MerchTreeImpl(const FestDb &festDb, const std::string &festVersion, const std::vector<OppfMedForbrMatr> &medForbrMatr) {
    MapElements(festDb, festVersion, medForbrMatr);
}

MerchTreeImpl::MerchTreeImpl(const FestDb &festDb, const std::string &festVersion, const std::vector<OppfNaringsmiddel> &naringsmidler) {
    MapElements(festDb, festVersion, naringsmidler);
}

MerchTreeImpl::MerchTreeImpl(const FestDb &festDb, const std::string &festVersion, const std::vector<OppfBrystprotese> &brystproteser) {
    MapElements(festDb, festVersion, brystproteser);
}

std::shared_ptr<MerchTree::ContainerElement> MerchTreeImpl::GetContainerElement(const std::string &id) const {
    auto iterator = refusjonToElement.find(id);
    if (iterator != refusjonToElement.end()) {
        return iterator->second;
    }
    return {};
}

std::vector<MerchRefund> MerchTreeImpl::GetRefunds() const {
    return refunds;
}