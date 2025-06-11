//
// Created by sigsegv on 5/28/25.
//

#include "MedicamentSearch.h"
#include "FestDb.h"
#include <medfest/Struct/Packed/POppfLegemiddelVirkestoff.h>
#include <medfest/Struct/Packed/POppfLegemiddelMerkevare.h>
#include <medfest/Struct/Packed/PReseptgyldighet.h>
#include <medfest/Struct/Packed/POppfLegemiddelpakning.h>

std::map<FestUuid, std::vector<PReseptgyldighet>> MedicamentSearch::CreateMerkevareToPrescriptionValidity() const {
    std::map<FestUuid, std::vector<PReseptgyldighet>> map{};
    auto allPMerkevare = festDb->GetAllPLegemiddelMerkevare();
    for (const auto &pmerkevare : allPMerkevare) {
        auto id = festDb->GetLegemiddelMerkevareId(pmerkevare);
        auto prep = festDb->GetPReseptgyldighet(pmerkevare);
        map.insert_or_assign(id, prep);
    }
    return map;
}

std::vector<FestUuid> MedicamentSearch::FindMerkevareWithTwoOrMoreReseptgyldighet() {
    std::vector<FestUuid> ids{};
    auto &map = merkevareToPrescriptionValidity.operator std::map<FestUuid, std::vector<PReseptgyldighet>> &();
    for (const auto &[id, preseptgyldighet] : map) {
        if (preseptgyldighet.size() > 1) {
            ids.emplace_back(id);
        }
    }
    return ids;
}

std::vector<FestUuid> MedicamentSearch::FindLegemiddelVirkestoffWithTwoOrMoreReseptgyldighet() {
    std::vector<FestUuid> ids{};
    auto &map = merkevareToPrescriptionValidity.operator std::map<FestUuid, std::vector<PReseptgyldighet>> &();
    auto allPVirkestoff = festDb->GetAllPLegemiddelVirkestoff();
    for (const auto &pvirkestoff : allPVirkestoff) {
        auto merkevareIds = festDb->GetRefMerkevare(pvirkestoff);
        bool matching{false};
        for (auto &mId : merkevareIds) {
            auto iterator = map.find(mId);
            if (iterator != map.end()) {
                if (iterator->second.size() > 1) {
                    matching = true;
                    break;
                }
            }
        }
        if (matching) {
            ids.emplace_back(festDb->GetLegemiddelVirkestoffId(pvirkestoff));
        }
    }
    return ids;
}

std::vector<FestUuid> MedicamentSearch::FindLegemiddelpakningWithTwoOrMoreReseptgyldighet() {
    std::vector<FestUuid> ids{};
    auto &map = merkevareToPrescriptionValidity.operator std::map<FestUuid, std::vector<PReseptgyldighet>> &();
    auto allPPakning = festDb->GetAllPLegemiddelpakning();
    for (const auto &ppakning : allPPakning) {
        auto merkevareIds = festDb->GetRefMerkevare(ppakning);
        bool matching{false};
        for (auto &mId : merkevareIds) {
            auto iterator = map.find(mId);
            if (iterator != map.end()) {
                if (iterator->second.size() > 1) {
                    matching = true;
                    break;
                }
            }
        }
        if (matching) {
            ids.emplace_back(festDb->GetLegemiddelpakningId(ppakning));
        }
    }
    return ids;
}

MedicamentSearch::MedicamentSearch(const std::shared_ptr<FestDb> &festDb) : festDb(festDb),
                                                                            merkevareToPrescriptionValidity([this] () { return CreateMerkevareToPrescriptionValidity(); }),
                                                                            merkevareWithTwoOrMoreReseptgyldighet([this] () { return FindMerkevareWithTwoOrMoreReseptgyldighet(); }),
                                                                            legemiddelVirkestoffWithTwoOrMoreReseptgyldighet([this] () { return FindLegemiddelVirkestoffWithTwoOrMoreReseptgyldighet(); }),
                                                                            legemiddelpakningWithTwoOrMoreReseptgyldighet([this] () { return FindLegemiddelpakningWithTwoOrMoreReseptgyldighet(); }) {
}

std::shared_ptr<MedicamentSearchResult> MedicamentSearch::PerformSearch(const std::string &term, FindMedicamentSelections selection) {
    std::shared_ptr<MedicamentSearchResult> result = std::make_shared<MedicamentSearchResult>();
    if (term.size() > 2 || selection == FindMedicamentSelections::TWO_OR_MORE_PRESCRIPTION_VALIDITY) {
        auto legemiddelVirkestoffOppfs = festDb->GetAllPLegemiddelVirkestoff();
        if (selection == FindMedicamentSelections::TWO_OR_MORE_PRESCRIPTION_VALIDITY) {
            std::vector<FestUuid> ids{};
            {
                auto &refIds = legemiddelVirkestoffWithTwoOrMoreReseptgyldighet.operator std::vector<FestUuid> &();
                ids.reserve(refIds.size());
                for (const auto &id : refIds) {
                    ids.emplace_back(id);
                }
            }
            auto iterator = legemiddelVirkestoffOppfs.begin();
            while (iterator != legemiddelVirkestoffOppfs.end()) {
                auto id = festDb->GetLegemiddelVirkestoffId(*iterator);
                auto found = std::find(ids.begin(), ids.end(), id);
                if (found != ids.end()) {
                    ids.erase(found);
                    ++iterator;
                } else {
                    iterator = legemiddelVirkestoffOppfs.erase(iterator);
                }
            }
        }
        {
            std::vector<FestUuid> legemiddelVirkestoffIds{};
            std::vector<FestUuid> legemiddelMerkevareIds{};
            std::vector<FestUuid> legemiddelpakningIds{};
            result->legemiddelVirkestoffList = festDb->FindLegemiddelVirkestoff(legemiddelVirkestoffOppfs, term);
            for (const auto &legemiddelVirkestoff: result->legemiddelVirkestoffList) {
                legemiddelVirkestoffIds.emplace_back(legemiddelVirkestoff.GetId());
                auto refMerkevare = legemiddelVirkestoff.GetRefLegemiddelMerkevare();
                for (const auto &merkevareId: refMerkevare) {
                    FestUuid festId{merkevareId};
                    auto merkevare = festDb->GetLegemiddelMerkevare(festId);
                    legemiddelMerkevareIds.emplace_back(festId);
                    result->legemiddelMerkevareList.emplace_back(merkevare);
                }
                auto refPakning = legemiddelVirkestoff.GetRefPakning();
                for (const auto &pakningId: refPakning) {
                    FestUuid festId{pakningId};
                    auto pakning = festDb->GetLegemiddelpakning(festId);
                    legemiddelpakningIds.emplace_back(festId);
                    result->legemiddelpakningList.emplace_back(pakning);
                }
            }
            auto legemiddelMerkevareOppfs = festDb->GetAllPLegemiddelMerkevare();
            if (selection == FindMedicamentSelections::TWO_OR_MORE_PRESCRIPTION_VALIDITY) {
                std::vector<FestUuid> ids{};
                {
                    auto &refIds = merkevareWithTwoOrMoreReseptgyldighet.operator std::vector<FestUuid> &();
                    ids.reserve(refIds.size());
                    for (const auto &id : refIds) {
                        ids.emplace_back(id);
                    }
                }
                auto iterator = legemiddelMerkevareOppfs.begin();
                while (iterator != legemiddelMerkevareOppfs.end()) {
                    auto id = festDb->GetLegemiddelMerkevareId(*iterator);
                    auto found = std::find(ids.begin(), ids.end(), id);
                    if (found != ids.end()) {
                        ids.erase(found);
                        ++iterator;
                    } else {
                        iterator = legemiddelMerkevareOppfs.erase(iterator);
                    }
                }
            }
            auto legemiddelMerkevareSearchList = festDb->FindLegemiddelMerkevare(legemiddelMerkevareOppfs, term);
            for (const auto &legemiddelMerkevare: legemiddelMerkevareSearchList) {
                FestUuid festId{legemiddelMerkevare.GetId()};
                bool found{false};
                for (const auto eid: legemiddelMerkevareIds) {
                    if (festId == eid) {
                        found = true;
                        continue;
                    }
                }
                if (!found) {
                    legemiddelMerkevareIds.emplace_back(festId);
                    result->legemiddelMerkevareList.emplace_back(legemiddelMerkevare);
                }
            }
            auto legemiddelpakningOppfs = festDb->GetAllPLegemiddelpakning();
            if (selection == FindMedicamentSelections::TWO_OR_MORE_PRESCRIPTION_VALIDITY) {
                std::vector<FestUuid> ids{};
                {
                    auto &refIds = legemiddelpakningWithTwoOrMoreReseptgyldighet.operator std::vector<FestUuid> &();
                    ids.reserve(refIds.size());
                    for (const auto &id : refIds) {
                        ids.emplace_back(id);
                    }
                }
                auto iterator = legemiddelpakningOppfs.begin();
                while (iterator != legemiddelpakningOppfs.end()) {
                    auto id = festDb->GetLegemiddelpakningId(*iterator);
                    auto found = std::find(ids.begin(), ids.end(), id);
                    if (found != ids.end()) {
                        ids.erase(found);
                        ++iterator;
                    } else {
                        iterator = legemiddelpakningOppfs.erase(iterator);
                    }
                }
            }
            auto legemiddelpakningSearchList = festDb->FindLegemiddelpakning(legemiddelpakningOppfs, term);
            for (const auto &legemiddelpakning: legemiddelpakningSearchList) {
                FestUuid festId{legemiddelpakning.GetId()};
                bool found{false};
                for (const auto eid: legemiddelMerkevareIds) {
                    if (festId == eid) {
                        found = true;
                        continue;
                    }
                }
                if (!found) {
                    result->legemiddelpakningList.emplace_back(legemiddelpakning);
                    legemiddelpakningIds.emplace_back(legemiddelpakning.GetId());
                }
            }
            for (const auto &plv: legemiddelVirkestoffOppfs) {
                auto unpacked = festDb->GetLegemiddelVirkestoff(plv);
                if (festDb->PLegemiddelVirkestoffHasOneOfMerkevare(plv, legemiddelMerkevareIds) ||
                    festDb->PLegemiddelVirkestoffHasOneOfPakning(plv, legemiddelpakningIds)) {
                    auto id = festDb->GetLegemiddelVirkestoffId(plv);
                    bool found{false};
                    for (const auto &eid: legemiddelVirkestoffIds) {
                        if (id == eid) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        legemiddelVirkestoffIds.emplace_back(id);
                        auto legemiddelVirkestoff = festDb->GetLegemiddelVirkestoff(id);
                        result->legemiddelVirkestoffList.emplace_back(legemiddelVirkestoff);
                        auto refMerkevare = legemiddelVirkestoff.GetRefLegemiddelMerkevare();
                        for (auto merkevareId: refMerkevare) {
                            FestUuid festId{merkevareId};
                            bool found{false};
                            for (const auto &eid: legemiddelMerkevareIds) {
                                if (eid == festId) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                legemiddelMerkevareIds.emplace_back(festId);
                                result->legemiddelMerkevareList.emplace_back(festDb->GetLegemiddelMerkevare(festId));
                            }
                        }
                        auto refPakning = legemiddelVirkestoff.GetRefPakning();
                        for (auto pakningId: refPakning) {
                            FestUuid festId{pakningId};
                            bool found{false};
                            for (const auto &eid: legemiddelpakningIds) {
                                if (eid == festId) {
                                    found = true;
                                    break;
                                }
                            }
                            if (!found) {
                                legemiddelpakningIds.emplace_back(festId);
                                result->legemiddelpakningList.emplace_back(festDb->GetLegemiddelpakning(festId));
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}
