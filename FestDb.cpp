//
// Created by sigsegv on 3/18/24.
//

#include "FestDb.h"
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
    festDeserializer->ForEachLegemiddelVirkestoff([this, &results, &term] (const auto &poppf) {
        LegemiddelVirkestoff legemiddelVirkestoff = festDeserializer->Unpack(poppf).GetLegemiddelVirkestoff();
        auto nfs = legemiddelVirkestoff.GetNavnFormStyrke();
        if (nfs.contains(term)) {
            results.emplace_back(legemiddelVirkestoff);
        }
    });
    return results;
}