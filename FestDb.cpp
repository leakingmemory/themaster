//
// Created by sigsegv on 3/18/24.
//

#include "FestDb.h"
#include <medfest/FestVectors.h>
#include <medfest/FestDeserializer.h>
#include <medfest/Struct/Decoded/OppfLegemiddelVirkestoff.h>
#include <medfest/Struct/Decoded/OppfLegemiddelMerkevare.h>
#include <medfest/Struct/Decoded/OppfLegemiddelpakning.h>
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

FestDbContainer FestDb::GetActiveFestDb() const {
    if (!festDeserializer) {
        return {};
    }
    FestDbContainer container{};
    std::map<std::string, std::unique_ptr<FestVectors>> festVersions{};
    festDeserializer->ForEachFests([this, &festVersions](const PFest &pfest) {
        auto fest = std::make_unique<FestVectors>(std::move(festDeserializer->Unpack(pfest)));
        festVersions.insert_or_assign(fest->GetDato(), std::move(fest));
    });
    std::vector<std::string> versions{};
    for (const auto &pair: festVersions) {
        versions.emplace_back(pair.first);
    }
    std::sort(versions.begin(), versions.end(), std::greater<>());
    if (!versions.empty()) {
        container.festVectors = std::move(festVersions[versions[0]]);
    }
    return container;
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
