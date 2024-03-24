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

std::vector<LegemiddelVirkestoff> FestDb::FindLegemiddelVirkestoff(const std::string &term) const {
    std::vector<LegemiddelVirkestoff> results{};
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelVirkestoff(*festDeserializer);
    for (const auto &poppf : oppfs) {
        PString pnavnFormStyrke = poppf.GetNavnFormStyrke();
        std::string navnFormStyrke = festDeserializer->Unpack(pnavnFormStyrke);
        if (navnFormStyrke.contains(term)) {
            PLegemiddelVirkestoff pLegemiddelVirkestoff = poppf;
            results.emplace_back(festDeserializer->Unpack(pLegemiddelVirkestoff));
        }
    }
    return results;
}

std::vector<LegemiddelMerkevare> FestDb::FindLegemiddelMerkevare(const std::string &term) const {
    std::vector<LegemiddelMerkevare> results{};
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelMerkevare(*festDeserializer);
    for (const auto &poppf : oppfs) {
        PString pnavnFormStyrke = poppf.GetNavnFormStyrke();
        std::string navnFormStyrke = festDeserializer->Unpack(pnavnFormStyrke);
        if (navnFormStyrke.contains(term)) {
            PLegemiddelMerkevare pLegemiddelMerkevare = poppf;
            results.emplace_back(festDeserializer->Unpack(pLegemiddelMerkevare));
        }
    }
    return results;
}

std::vector<Legemiddelpakning> FestDb::FindLegemiddelpakning(const std::string &term) const {
    std::vector<Legemiddelpakning> results{};
    FestDbContainer festDbContainer = GetActiveFestDb();
    if (!festDbContainer.festVectors) {
        return {};
    }
    auto oppfs = festDbContainer.festVectors->GetLegemiddelPakning(*festDeserializer);
    for (const auto &poppf : oppfs) {
        PString pnavnFormStyrke = poppf.GetNavnFormStyrke();
        std::string navnFormStyrke = festDeserializer->Unpack(pnavnFormStyrke);
        if (navnFormStyrke.contains(term)) {
            PLegemiddelpakning pLegemiddelPakning = poppf;
            results.emplace_back(festDeserializer->Unpack(pLegemiddelPakning));
        }
    }
    return results;
}
