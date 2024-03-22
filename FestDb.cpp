//
// Created by sigsegv on 3/18/24.
//

#include "FestDb.h"
#include <medfest/FestVectors.h>
#include <medfest/FestDeserializer.h>
#include <medfest/Struct/Decoded/OppfLegemiddelVirkestoff.h>
#include "DataDirectory.h"
#include <filesystem>
#include <wx/wx.h>

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

std::vector<LegemiddelVirkestoff> FestDb::FindLegemiddelVirkestoff(const std::string &term) const {
    if (!festDeserializer) {
        return {};
    }
    std::vector<LegemiddelVirkestoff> results{};
    {
        std::unique_ptr<FestVectors> latestVersion{};
        {
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
                latestVersion = std::move(festVersions[versions[0]]);
            }
        }
        if (latestVersion) {
            auto oppfs = latestVersion->GetLegemiddelVirkestoff(*festDeserializer);
            for (const auto &poppf : oppfs) {
                PString pnavnFormStyrke = poppf.GetNavnFormStyrke();
                std::string navnFormStyrke = festDeserializer->Unpack(pnavnFormStyrke);
                if (navnFormStyrke.contains(term)) {
                    PLegemiddelVirkestoff pLegemiddelVirkestoff = poppf;
                    results.emplace_back(festDeserializer->Unpack(pLegemiddelVirkestoff));
                }
            }
        }
    }
    return results;
}