//
// Created by sigsegv on 3/18/24.
//

#include "FestDb.h"
#include <medfest/FestVectors.h>
#include <medfest/FestDeserializer.h>
#include <medfest/Struct/Decoded/OppfLegemiddelVirkestoff.h>
#include <medfest/Struct/Decoded/OppfLegemiddelMerkevare.h>
#include <medfest/Struct/Decoded/OppfLegemiddelpakning.h>
#include <medfest/Struct/Decoded/OppfLegemiddeldose.h>
#include <medfest/Struct/Decoded/OppfRefusjon.h>
#include <medfest/Struct/Decoded/OppfKodeverk.h>
#include <medfest/Struct/Packed/PPakningsinfo.h>
#include "DataDirectory.h"
#include <filesystem>
#include <wx/wx.h>

struct FestDbContainer {
    std::unique_ptr<FestVectors> festVectors{};
};

FestDb::FestDb() {
    auto dbfile = DataDirectory::Data("themaster").Sub("FEST").GetPath("fest.db");
    if (std::filesystem::exists(dbfile)) {
        try {
            festDeserializer = std::make_shared<FestDeserializer>(dbfile);
        } catch (std::exception &e) {
            std::string err{e.what()};
            err.append(" - Would you like to delete the broken database?");
            std::cerr << "FEST deserializer error: " << err << "\n";
            if (wxMessageBox(err, "FEST deserializer error", wxYES_NO | wxICON_ERROR) == wxYES) {
                std::filesystem::remove(dbfile);
            }
        } catch (...) {
            std::cerr << "FEST deserializer error\n";
            if (wxMessageBox("Unknown error. Would you like to delete the broken database?", "FEST deserializer error", wxYES_NO | wxICON_ERROR) == wxYES) {
                std::filesystem::remove(dbfile);
            }
        }
    }
}

bool FestDb::IsOpen() const {
    return festDeserializer.operator bool();
}

std::map<std::string, std::unique_ptr<FestVectors>> FestDb::GetFestVersionMap() const {
    if (!festDeserializer) {
        return {};
    }
    FestDbContainer container{};
    std::map<std::string, std::unique_ptr<FestVectors>> festVersions{};
    festDeserializer->ForEachFests([this, &festVersions](const PFest &pfest) {
        auto fest = std::make_unique<FestVectors>(std::move(festDeserializer->Unpack(pfest)));
        std::string dato = fest->GetDato();
        festVersions.insert_or_assign(dato, std::move(fest));
    });
    return festVersions;
}

static std::vector<std::string> GetFestVersionsFromMap(const std::map<std::string, std::unique_ptr<FestVectors>> &festVersions) {
    std::vector<std::string> versions{};
    for (const auto &pair: festVersions) {
        versions.emplace_back(pair.first);
    }
    std::sort(versions.begin(), versions.end(), std::greater<>());
    return versions;
}

FestDbContainer FestDb::GetActiveFestDb() const {
    FestDbContainer container{};
    std::map<std::string, std::unique_ptr<FestVectors>> festVersions = GetFestVersionMap();
    std::vector<std::string> versions = GetFestVersionsFromMap(festVersions);
    if (!versions.empty()) {
        container.festVectors = std::move(festVersions[versions[0]]);
    }
    return container;
}

FestDbContainer FestDb::GetFestDb(const std::string &version) const {
    FestDbContainer container{};
    std::map<std::string, std::unique_ptr<FestVectors>> festVersions = GetFestVersionMap();
    auto iterator = festVersions.find(version);
    if (iterator != festVersions.end()) {
        container.festVectors = std::move(iterator->second);
    }
    return container;
}

std::vector<std::string> FestDb::GetFestVersions() const {
    std::map<std::string, std::unique_ptr<FestVectors>> festVersions = GetFestVersionMap();
    return GetFestVersionsFromMap(festVersions);
}

std::vector<FestDbQuota> FestDb::GetDbQuotas() const {
    if (festDeserializer.operator bool()) {
        return festDeserializer->GetQuotas();
    } else {
        return {};
    }
}

std::vector<LegemiddelVirkestoff> FestDb::FindLegemiddelVirkestoff(const std::vector<POppfLegemiddelVirkestoff> &oppfs, const std::string &i_term) const {
    std::vector<LegemiddelVirkestoff> results{};
    std::string term{i_term};
    std::transform(term.begin(), term.end(), term.begin(), [] (auto c) { return std::tolower(c); });
    for (const auto &poppf : oppfs) {
        PString pnavnFormStyrke = poppf.GetNavnFormStyrke();
        std::string navnFormStyrke = festDeserializer->Unpack(pnavnFormStyrke);
        std::transform(navnFormStyrke.begin(), navnFormStyrke.end(), navnFormStyrke.begin(), [] (auto c) { return std::tolower(c); });
        if (navnFormStyrke.contains(term)) {
            PLegemiddelVirkestoff pLegemiddelVirkestoff = poppf;
            results.emplace_back(festDeserializer->Unpack(pLegemiddelVirkestoff));
        }
    }
    return results;
}

std::vector<LegemiddelVirkestoff> FestDb::FindLegemiddelVirkestoff(const std::string &term) const {
    std::vector<LegemiddelVirkestoff> results{};
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelVirkestoff(*festDeserializer);
    return FindLegemiddelVirkestoff(oppfs, term);
}

std::vector<LegemiddelMerkevare> FestDb::FindLegemiddelMerkevare(const std::string &i_term) const {
    std::vector<LegemiddelMerkevare> results{};
    std::string term{i_term};
    std::transform(term.begin(), term.end(), term.begin(), [] (auto c) { return std::tolower(c); });
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelMerkevare(*festDeserializer);
    for (const auto &poppf : oppfs) {
        PString pnavnFormStyrke = poppf.GetNavnFormStyrke();
        std::string navnFormStyrke = festDeserializer->Unpack(pnavnFormStyrke);
        std::transform(navnFormStyrke.begin(), navnFormStyrke.end(), navnFormStyrke.begin(), [] (auto c) { return std::tolower(c); });
        if (navnFormStyrke.contains(term)) {
            PLegemiddelMerkevare pLegemiddelMerkevare = poppf;
            results.emplace_back(festDeserializer->Unpack(pLegemiddelMerkevare));
        }
    }
    return results;
}

std::vector<LegemiddelMerkevare> FestDb::FindLegemiddelMerkevare(const std::vector<FestUuid> &uuids) const {
    std::vector<LegemiddelMerkevare> results{};
    std::vector<FestUuid> remainingUuids = uuids;
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelMerkevare(*festDeserializer);
    for (const auto &poppf : oppfs) {
        auto idIterator = remainingUuids.begin();
        if (idIterator == remainingUuids.end()) {
            return results;
        }
        do {
            if (*idIterator == festDeserializer->Unpack(poppf.GetId())) {
                idIterator = remainingUuids.erase(idIterator);
                results.emplace_back(festDeserializer->Unpack(static_cast<const PLegemiddelMerkevare &>(poppf)));
                break;
            }
            ++idIterator;
        } while (idIterator != remainingUuids.end());
    }
    return results;
}

std::vector<Legemiddelpakning> FestDb::FindLegemiddelpakning(const std::string &i_term) const {
    std::vector<Legemiddelpakning> results{};
    std::string term{i_term};
    std::transform(term.begin(), term.end(), term.begin(), [] (auto c) { return std::tolower(c); });
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelPakning(*festDeserializer);
    for (const auto &poppf : oppfs) {
        PString pnavnFormStyrke = poppf.GetNavnFormStyrke();
        std::string navnFormStyrke = festDeserializer->Unpack(pnavnFormStyrke);
        std::transform(navnFormStyrke.begin(), navnFormStyrke.end(), navnFormStyrke.begin(), [] (auto c) { return std::tolower(c); });
        if (navnFormStyrke.contains(term)) {
            PLegemiddelpakning pLegemiddelPakning = poppf;
            results.emplace_back(festDeserializer->Unpack(pLegemiddelPakning));
        }
    }
    return results;
}

LegemiddelVirkestoff FestDb::GetLegemiddelVirkestoff(FestUuid id) const {
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelVirkestoff(*festDeserializer);
    for (const auto &poppf : oppfs) {
        PFestId pId = poppf.GetId();
        FestUuid legemiddelVirkestoffId = festDeserializer->Unpack(pId);
        if (legemiddelVirkestoffId == id) {
            PLegemiddelVirkestoff pLegemiddelVirkestoff = poppf;
            return festDeserializer->Unpack(pLegemiddelVirkestoff);
        }
    }
    return {};
}

LegemiddelVirkestoff FestDb::GetLegemiddelVirkestoff(const PLegemiddelVirkestoff &packed) const {
    return festDeserializer->Unpack(packed);
}

LegemiddelVirkestoff FestDb::GetLegemiddelVirkestoffForMerkevare(FestUuid uuid) const {
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelVirkestoff(*festDeserializer);
    for (const auto &poppf : oppfs) {
        auto refList = poppf.GetRefLegemiddelMerkevare();
        auto ids = festDeserializer->GetFestUuids(refList);
        for (auto id : ids) {
            if (uuid == id) {
                PLegemiddelVirkestoff pLegemiddelVirkestoff = poppf;
                return festDeserializer->Unpack(pLegemiddelVirkestoff);
            }
        }
    }
    return {};
}

LegemiddelMerkevare FestDb::GetLegemiddelMerkevare(FestUuid id) const {
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelMerkevare(*festDeserializer);
    for (const auto &poppf : oppfs) {
        PFestId pId = poppf.GetId();
        FestUuid legemiddelMerkevareId = festDeserializer->Unpack(pId);
        if (legemiddelMerkevareId == id) {
            PLegemiddelMerkevare pLegemiddelMerkevare = poppf;
            return festDeserializer->Unpack(pLegemiddelMerkevare);
        }
    }
    return {};
}

Legemiddelpakning FestDb::GetLegemiddelpakning(FestUuid id) const {
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelPakning(*festDeserializer);
    for (const auto &poppf : oppfs) {
        PFestId pId = poppf.GetId();
        FestUuid legemiddelpakningId = festDeserializer->Unpack(pId);
        if (legemiddelpakningId == id) {
            PLegemiddelpakning pLegemiddelpakning = poppf;
            return festDeserializer->Unpack(pLegemiddelpakning);
        }
    }
    return {};
}

OppfKodeverk FestDb::GetKodeverkById(const std::string &id) const {
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto poppfs = festDbContainer.festVectors->GetKodeverk(*festDeserializer);
    for (const auto &poppf : poppfs) {
        auto info = festDeserializer->Unpack(static_cast<const PInfo &>(poppf));
        if (info.GetId() == id) {
            return festDeserializer->Unpack(poppf);
        }
    }
    return {};
}

std::vector<Legemiddelpakning> FestDb::GetLegemiddelpakningForMerkevare(FestUuid merkevareId) const {
    FestDbContainer festDbContainer = GetActiveFestDb();
    std::vector<Legemiddelpakning> list{};
    if (!festDbContainer.festVectors) {
        return list;
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelPakning(*festDeserializer);
    for (const auto &poppf : oppfs) {
        auto pList = festDeserializer->GetPakningsinfoList(poppf);
        for (const auto &ppakningsinfo : pList) {
            auto pId = ppakningsinfo.GetMerkevareId();
            auto id = festDeserializer->Unpack(pId);
            if (id == merkevareId) {
                auto unpacked = festDeserializer->Unpack(static_cast<const PLegemiddelpakning &>(poppf));
                list.emplace_back(std::move(unpacked));
                break;
            }
        }
    }
    return list;
}

FestUuid FestDb::GetVirkestoffForVirkestoffMedStyrkeId(FestUuid virkestoffMedStyrkeId) const {
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetVirkestoffMedStyrke(*festDeserializer);
    for (const auto &poppf: oppfs) {
        PFestId pVMSId = poppf.GetId();
        FestUuid iteratingVirkestoffMedStyrkeId = festDeserializer->Unpack(pVMSId);
        if (iteratingVirkestoffMedStyrkeId == virkestoffMedStyrkeId) {
            auto pId = poppf.GetRefVirkestoff();
            return festDeserializer->Unpack(pId);
        }
    }
    return {};
}

std::vector<FestUuid> FestDb::GetVirkestoffMedStyrkeForVirkestoffId(FestUuid virkestoffId) {
    std::vector<FestUuid> virkestoffMedStyrkeId{};
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetVirkestoffMedStyrke(*festDeserializer);
    for (const auto &poppf: oppfs) {
        PFestId pVId = poppf.GetRefVirkestoff();
        auto vId = festDeserializer->Unpack(pVId);
        if (vId == virkestoffId) {
            auto pId = poppf.GetId();
            auto id = festDeserializer->Unpack(pId);
            virkestoffMedStyrkeId.push_back(id);
        }
    }
    return virkestoffMedStyrkeId;
}

std::vector<POppfLegemiddelVirkestoff> FestDb::GetAllPLegemiddelVirkestoff() const {
    std::vector<FestUuid> virkestoffMedStyrkeId{};
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    return festDbContainer.festVectors->GetLegemiddelVirkestoff(*festDeserializer);
}

bool FestDb::PLegemiddelVirkestoffHasOneOfMerkevare(const PLegemiddelVirkestoff &pLegemiddelVirkestoff,
                                                    const std::vector<FestUuid> &merkevareId) {
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return false;
    }
    auto ids = festDeserializer->GetFestUuids(pLegemiddelVirkestoff.GetRefLegemiddelMerkevare());
    for (const auto &mId : ids) {
        for (const auto &id : merkevareId) {
            if (mId == id) {
                return true;
            }
        }
    }
    return false;
}

bool FestDb::PLegemiddelVirkestoffHasOneOfPakning(const PLegemiddelVirkestoff &pLegemiddelVirkestoff, const std::vector<FestUuid> &pakningId) {
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return false;
    }
    auto ids = festDeserializer->GetFestUuids(pLegemiddelVirkestoff.GetRefPakning());
    for (const auto &mId : ids) {
        for (const auto &id : pakningId) {
            if (mId == id) {
                return true;
            }
        }
    }
    return false;
}

FestUuid FestDb::GetLegemiddelVirkestoffId(const PLegemiddelVirkestoff &packed) {
    return festDeserializer->Unpack(packed.GetId());
}

std::vector<OppfRefusjon> FestDb::GetOppfRefusjon(const std::string &festVersion) const {
    std::vector<OppfRefusjon> oppfs{};
    {
        FestDbContainer festDbContainer = GetFestDb(festVersion);
        if (!festDbContainer.festVectors) {
            return {};
        }
        auto refusjon = festDbContainer.festVectors->GetRefusjon(*festDeserializer);
        for (const auto &poppf : refusjon) {
            oppfs.emplace_back(festDeserializer->Unpack(poppf));
        }
    }
    return oppfs;
}

std::vector<OppfLegemiddelMerkevare> FestDb::GetOppfLegemiddelMerkevare(const std::string &festVersion) const {
    std::vector<OppfLegemiddelMerkevare> oppfs{};
    {
        FestDbContainer festDbContainer = GetFestDb(festVersion);
        if (!festDbContainer.festVectors) {
            return {};
        }
        auto oppfLegemiddelMerkevares = festDbContainer.festVectors->GetLegemiddelMerkevare(*festDeserializer);
        for (const auto &poppf : oppfLegemiddelMerkevares) {
            oppfs.emplace_back(festDeserializer->Unpack(poppf));
        }
    }
    return oppfs;
}

std::vector<OppfLegemiddelVirkestoff> FestDb::GetOppfLegemiddelVirkestoff(const std::string &festVersion) const {
    std::vector<OppfLegemiddelVirkestoff> oppfs{};
    {
        FestDbContainer festDbContainer = GetFestDb(festVersion);
        if (!festDbContainer.festVectors) {
            return {};
        }
        auto oppfLegemiddelVirkestoffs = festDbContainer.festVectors->GetLegemiddelVirkestoff(*festDeserializer);
        for (const auto &poppf : oppfLegemiddelVirkestoffs) {
            oppfs.emplace_back(festDeserializer->Unpack(poppf));
        }
    }
    return oppfs;
}

std::vector<OppfLegemiddelpakning> FestDb::GetOppfLegemiddelpakning(const std::string &festVersion) const {
    std::vector<OppfLegemiddelpakning> oppfs{};
    {
        FestDbContainer festDbContainer = GetFestDb(festVersion);
        if (!festDbContainer.festVectors) {
            return {};
        }
        auto oppfLegemiddelpaknings = festDbContainer.festVectors->GetLegemiddelPakning(*festDeserializer);
        for (const auto &poppf : oppfLegemiddelpaknings) {
            oppfs.emplace_back(festDeserializer->Unpack(poppf));
        }
    }
    return oppfs;
}

std::vector<OppfLegemiddeldose> FestDb::GetOppfLegemiddeldose(const std::string &festVersion) const {
    std::vector<OppfLegemiddeldose> oppfs{};
    {
        FestDbContainer festDbContainer = GetFestDb(festVersion);
        if (!festDbContainer.festVectors) {
            return {};
        }
        auto oppfLegemiddeldoses = festDbContainer.festVectors->GetLegemiddeldose(*festDeserializer);
        for (const auto &poppf : oppfLegemiddeldoses) {
            oppfs.emplace_back(festDeserializer->Unpack(poppf));
        }
    }
    return oppfs;
}

std::vector<OppfKodeverk> FestDb::GetOppfKodeverk(const std::string &festVersion) const {
    std::vector<OppfKodeverk> oppfs{};
    {
        FestDbContainer festDbContainer = GetFestDb(festVersion);
        if (!festDbContainer.festVectors) {
            return {};
        }
        auto oppfKodeverks = festDbContainer.festVectors->GetKodeverk(*festDeserializer);
        for (const auto &poppf : oppfKodeverks) {
            oppfs.emplace_back(festDeserializer->Unpack(poppf));
        }
    }
    return oppfs;
}

std::vector<Element> FestDb::GetKodeverkElements(const std::string &kodeverkId, const std::string &festVersion) const {
    std::vector<Element> oppfs{};
    {
        FestDbContainer festDbContainer = GetFestDb(festVersion);
        if (!festDbContainer.festVectors) {
            return {};
        }
        auto oppfKodeverks = festDbContainer.festVectors->GetKodeverk(*festDeserializer);
        for (const auto &poppf : oppfKodeverks) {
            auto oppf = festDeserializer->Unpack(static_cast<const PInfo &>(poppf));
            if (oppf.GetId() == kodeverkId) {
                auto elements = festDeserializer->GetElementList(poppf);
                for (const auto &element : elements) {
                    oppfs.emplace_back(festDeserializer->Unpack(element));
                }
                break;
            }
        }
    }
    return oppfs;
}

FestDiff<OppfRefusjon> FestDb::GetOppfRefusjonDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion) const {
    FestDiff<OppfRefusjon> oppfs{};
    {
        FestDbContainer firstDbContainer = GetFestDb(firstVersion);
        FestDbContainer secondDbContainer = GetFestDb(secondVersion);
        if (!firstDbContainer.festVectors || !secondDbContainer.festVectors) {
            return {};
        }
        auto first = firstDbContainer.festVectors->GetRefusjon(*festDeserializer);
        auto second = secondDbContainer.festVectors->GetRefusjon(*festDeserializer);
        int max = first.size() + second.size();
        int counter = 0;
        progress(counter, max, 0, 1);
        for (const auto &poppf : first) {
            bool found{false};
            for (const auto &po : second) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.removed.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
        for (const auto &poppf : second) {
            bool found{false};
            for (const auto &po : first) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.added.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
    }
    int max = oppfs.removed.size();
    int counter = 0;
    progress(1, 1, counter, max);
    auto removedIterator = oppfs.removed.begin();
    while (removedIterator != oppfs.removed.end()) {
        auto addedIterator = oppfs.added.begin();
        bool found{false};
        while (addedIterator != oppfs.added.end()) {
            if (removedIterator->GetRefusjonshjemmel().GetRefusjonsgruppe().GetId() == addedIterator->GetRefusjonshjemmel().GetRefusjonsgruppe().GetId()) {
                found = true;
                FestModified<OppfRefusjon> modified{
                    .previous = *removedIterator,
                    .latest = *addedIterator
                };
                removedIterator = oppfs.removed.erase(removedIterator);
                addedIterator = oppfs.added.erase(addedIterator);
                oppfs.modified.emplace_back(std::move(modified));
                break;
            }
            ++addedIterator;
        }
        if (!found) {
            ++removedIterator;
        }
        ++counter;
        progress(1, 1, counter, max);
    }
    return oppfs;
}

FestDiff<OppfLegemiddelMerkevare> FestDb::GetOppfLegemiddelMerkevareDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion) const {
    FestDiff<OppfLegemiddelMerkevare> oppfs{};
    {
        FestDbContainer firstDbContainer = GetFestDb(firstVersion);
        FestDbContainer secondDbContainer = GetFestDb(secondVersion);
        if (!firstDbContainer.festVectors || !secondDbContainer.festVectors) {
            return {};
        }
        auto first = firstDbContainer.festVectors->GetLegemiddelMerkevare(*festDeserializer);
        auto second = secondDbContainer.festVectors->GetLegemiddelMerkevare(*festDeserializer);
        int max = first.size() + second.size();
        int counter = 0;
        progress(counter, max, 0, 1);
        for (const auto &poppf : first) {
            bool found{false};
            for (const auto &po : second) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.removed.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
        for (const auto &poppf : second) {
            bool found{false};
            for (const auto &po : first) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.added.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
    }
    int max = oppfs.removed.size();
    int counter = 0;
    auto removedIterator = oppfs.removed.begin();
    while (removedIterator != oppfs.removed.end()) {
        auto addedIterator = oppfs.added.begin();
        bool found{false};
        while (addedIterator != oppfs.added.end()) {
            if (removedIterator->GetLegemiddelMerkevare().GetId() == addedIterator->GetLegemiddelMerkevare().GetId()) {
                found = true;
                FestModified<OppfLegemiddelMerkevare> modified{
                        .previous = *removedIterator,
                        .latest = *addedIterator
                };
                removedIterator = oppfs.removed.erase(removedIterator);
                addedIterator = oppfs.added.erase(addedIterator);
                oppfs.modified.emplace_back(std::move(modified));
                break;
            }
            ++addedIterator;
        }
        if (!found) {
            ++removedIterator;
        }
        ++counter;
        progress(1, 1, counter, max);
    }
    return oppfs;
}

FestDiff<OppfLegemiddelVirkestoff> FestDb::GetOppfLegemiddelVirkestoffDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion) const {
    FestDiff<OppfLegemiddelVirkestoff> oppfs{};
    {
        FestDbContainer firstDbContainer = GetFestDb(firstVersion);
        FestDbContainer secondDbContainer = GetFestDb(secondVersion);
        if (!firstDbContainer.festVectors || !secondDbContainer.festVectors) {
            return {};
        }
        auto first = firstDbContainer.festVectors->GetLegemiddelVirkestoff(*festDeserializer);
        auto second = secondDbContainer.festVectors->GetLegemiddelVirkestoff(*festDeserializer);
        int max = first.size() + second.size();
        int counter = 0;
        progress(counter, max, 0, 1);
        for (const auto &poppf : first) {
            bool found{false};
            for (const auto &po : second) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.removed.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
        for (const auto &poppf : second) {
            bool found{false};
            for (const auto &po : first) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.added.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
    }
    int max = oppfs.removed.size();
    int counter = 0;
    auto removedIterator = oppfs.removed.begin();
    while (removedIterator != oppfs.removed.end()) {
        auto addedIterator = oppfs.added.begin();
        bool found{false};
        while (addedIterator != oppfs.added.end()) {
            if (removedIterator->GetLegemiddelVirkestoff().GetId() == addedIterator->GetLegemiddelVirkestoff().GetId()) {
                found = true;
                FestModified<OppfLegemiddelVirkestoff> modified{
                        .previous = *removedIterator,
                        .latest = *addedIterator
                };
                removedIterator = oppfs.removed.erase(removedIterator);
                addedIterator = oppfs.added.erase(addedIterator);
                oppfs.modified.emplace_back(std::move(modified));
                break;
            }
            ++addedIterator;
        }
        if (!found) {
            ++removedIterator;
        }
        ++counter;
        progress(1, 1, counter, max);
    }
    return oppfs;
}

FestDiff<OppfLegemiddelpakning> FestDb::GetOppfLegemiddelpakningDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion) const {
    FestDiff<OppfLegemiddelpakning> oppfs{};
    {
        FestDbContainer firstDbContainer = GetFestDb(firstVersion);
        FestDbContainer secondDbContainer = GetFestDb(secondVersion);
        if (!firstDbContainer.festVectors || !secondDbContainer.festVectors) {
            return {};
        }
        auto first = firstDbContainer.festVectors->GetLegemiddelPakning(*festDeserializer);
        auto second = secondDbContainer.festVectors->GetLegemiddelPakning(*festDeserializer);
        int max = first.size() + second.size();
        int counter = 0;
        progress(counter, max, 0, 1);
        for (const auto &poppf : first) {
            bool found{false};
            for (const auto &po : second) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.removed.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
        for (const auto &poppf : second) {
            bool found{false};
            for (const auto &po : first) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.added.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
    }
    int max = oppfs.removed.size();
    int counter = 0;
    auto removedIterator = oppfs.removed.begin();
    while (removedIterator != oppfs.removed.end()) {
        auto addedIterator = oppfs.added.begin();
        bool found{false};
        while (addedIterator != oppfs.added.end()) {
            if (removedIterator->GetLegemiddelpakning().GetId() == addedIterator->GetLegemiddelpakning().GetId()) {
                found = true;
                FestModified<OppfLegemiddelpakning> modified{
                        .previous = *removedIterator,
                        .latest = *addedIterator
                };
                removedIterator = oppfs.removed.erase(removedIterator);
                addedIterator = oppfs.added.erase(addedIterator);
                oppfs.modified.emplace_back(std::move(modified));
                break;
            }
            ++addedIterator;
        }
        if (!found) {
            ++removedIterator;
        }
        ++counter;
        progress(1, 1, counter, max);
    }
    return oppfs;
}

FestDiff<OppfLegemiddeldose> FestDb::GetOppfLegemiddeldoseDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion) const {
    FestDiff<OppfLegemiddeldose> oppfs{};
    {
        FestDbContainer firstDbContainer = GetFestDb(firstVersion);
        FestDbContainer secondDbContainer = GetFestDb(secondVersion);
        if (!firstDbContainer.festVectors || !secondDbContainer.festVectors) {
            return {};
        }
        auto first = firstDbContainer.festVectors->GetLegemiddeldose(*festDeserializer);
        auto second = secondDbContainer.festVectors->GetLegemiddeldose(*festDeserializer);
        int max = first.size() + second.size();
        int counter = 0;
        progress(counter, max, 0, 1);
        for (const auto &poppf : first) {
            bool found{false};
            for (const auto &po : second) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.removed.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
        for (const auto &poppf : second) {
            bool found{false};
            for (const auto &po : first) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.added.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
    }
    int max = oppfs.removed.size();
    int counter = 0;
    auto removedIterator = oppfs.removed.begin();
    while (removedIterator != oppfs.removed.end()) {
        auto addedIterator = oppfs.added.begin();
        bool found{false};
        while (addedIterator != oppfs.added.end()) {
            if (removedIterator->GetLegemiddeldose().GetId() == addedIterator->GetLegemiddeldose().GetId()) {
                found = true;
                FestModified<OppfLegemiddeldose> modified{
                        .previous = *removedIterator,
                        .latest = *addedIterator
                };
                removedIterator = oppfs.removed.erase(removedIterator);
                addedIterator = oppfs.added.erase(addedIterator);
                oppfs.modified.emplace_back(std::move(modified));
                break;
            }
            ++addedIterator;
        }
        if (!found) {
            ++removedIterator;
        }
        ++counter;
        progress(1, 1, counter, max);
    }
    return oppfs;
}

FestDiff<OppfKodeverk> FestDb::GetOppfKodeverkDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion) const {
    FestDiff<OppfKodeverk> oppfs{};
    {
        FestDbContainer firstDbContainer = GetFestDb(firstVersion);
        FestDbContainer secondDbContainer = GetFestDb(secondVersion);
        if (!firstDbContainer.festVectors || !secondDbContainer.festVectors) {
            return {};
        }
        auto first = firstDbContainer.festVectors->GetKodeverk(*festDeserializer);
        auto second = secondDbContainer.festVectors->GetKodeverk(*festDeserializer);
        int max = first.size() + second.size();
        int counter = 0;
        progress(counter, max, 0, 1);
        for (const auto &poppf : first) {
            bool found{false};
            for (const auto &po : second) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.removed.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
        for (const auto &poppf : second) {
            bool found{false};
            for (const auto &po : first) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.added.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
    }
    int max = oppfs.removed.size();
    int counter = 0;
    auto removedIterator = oppfs.removed.begin();
    while (removedIterator != oppfs.removed.end()) {
        auto addedIterator = oppfs.added.begin();
        bool found{false};
        while (addedIterator != oppfs.added.end()) {
            if (removedIterator->GetInfo().GetId() == addedIterator->GetInfo().GetId()) {
                found = true;
                FestModified<OppfKodeverk> modified{
                        .previous = *removedIterator,
                        .latest = *addedIterator
                };
                removedIterator = oppfs.removed.erase(removedIterator);
                addedIterator = oppfs.added.erase(addedIterator);
                oppfs.modified.emplace_back(std::move(modified));
                break;
            }
            ++addedIterator;
        }
        if (!found) {
            ++removedIterator;
        }
        ++counter;
        progress(1, 1, counter, max);
    }
    return oppfs;
}

FestDiff<Element>
FestDb::GetKodeverkElementsDiff(const std::function<void(int, int, int, int)> &progress, const std::string &kodeverkId,
                                const std::string &firstVersion, const std::string &secondVersion) const {
    FestDiff<Element> oppfs{};
    {
        FestDbContainer firstDbContainer = GetFestDb(firstVersion);
        FestDbContainer secondDbContainer = GetFestDb(secondVersion);
        if (!firstDbContainer.festVectors || !secondDbContainer.festVectors) {
            return {};
        }
        std::vector<PElement> first{};
        std::vector<PElement> second{};
        {
            auto firstKodeverk = firstDbContainer.festVectors->GetKodeverk(*festDeserializer);
            auto secondKodeverk = secondDbContainer.festVectors->GetKodeverk(*festDeserializer);
            for (const auto &poppf : firstKodeverk) {
                auto oppf = festDeserializer->Unpack(static_cast<const POppf &>(poppf));
                if (oppf.GetId() == kodeverkId) {
                    first = festDeserializer->GetElementList(poppf);
                    break;
                }
            }
            for (const auto &poppf : secondKodeverk) {
                auto oppf = festDeserializer->Unpack(static_cast<const POppf &>(poppf));
                if (oppf.GetId() == kodeverkId) {
                    second = festDeserializer->GetElementList(poppf);
                    break;
                }
            }
        }
        int max = first.size() + second.size();
        int counter = 0;
        progress(counter, max, 0, 1);
        for (const auto &poppf : first) {
            bool found{false};
            for (const auto &po : second) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.removed.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
        for (const auto &poppf : second) {
            bool found{false};
            for (const auto &po : first) {
                if (poppf == po) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                oppfs.added.emplace_back(festDeserializer->Unpack(poppf));
            }
            counter++;
            progress(counter, max, 0, 1);
        }
    }
    int max = oppfs.removed.size();
    int counter = 0;
    auto removedIterator = oppfs.removed.begin();
    while (removedIterator != oppfs.removed.end()) {
        auto addedIterator = oppfs.added.begin();
        bool found{false};
        while (addedIterator != oppfs.added.end()) {
            if (removedIterator->GetKode() == addedIterator->GetKode()) {
                found = true;
                FestModified<Element> modified{
                        .previous = *removedIterator,
                        .latest = *addedIterator
                };
                removedIterator = oppfs.removed.erase(removedIterator);
                addedIterator = oppfs.added.erase(addedIterator);
                oppfs.modified.emplace_back(std::move(modified));
                break;
            }
            ++addedIterator;
        }
        if (!found) {
            ++removedIterator;
        }
        ++counter;
        progress(1, 1, counter, max);
    }
    return oppfs;
}
