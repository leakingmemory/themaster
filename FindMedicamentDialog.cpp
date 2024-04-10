//
// Created by sigsegv on 3/19/24.
//

#include "FindMedicamentDialog.h"
#include <wx/listctrl.h>
#include <medfest/Struct/Decoded/LegemiddelVirkestoff.h>
#include <medfest/Struct/Decoded/LegemiddelMerkevare.h>
#include <medfest/Struct/Decoded/Legemiddelpakning.h>
#include <medfest/Struct/Packed/FestUuid.h>
#include <medfest/Struct/Packed/POppfLegemiddelVirkestoff.h>

struct FindMedicamentDialogSearchResult {
    std::vector<LegemiddelVirkestoff> legemiddelVirkestoffList{};
    std::vector<LegemiddelMerkevare> legemiddelMerkevareList{};
    std::vector<Legemiddelpakning> legemiddelpakningList{};
};

FindMedicamentDialog::FindMedicamentDialog(wxWindow *parent) :
    wxDialog(parent, wxID_ANY, wxT("Find medicament")),
    searchDebouncer() {
    searchDebouncer.Func<FindMedicamentDialogSearchResult>([this] () { return PerformSearch(); }, [this] (const FindMedicamentDialogSearchResult &result) { ShowSearchResult(result); });

    // Add a sizer to handle the layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Add a text field for search input
    searchInput = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    sizer->Add(searchInput, 0, wxEXPAND|wxALL, 5);

    // Add a list view for potential matches
    listView = new wxListView(this, wxID_ANY);
    listView->AppendColumn(wxT("Id"));
    listView->AppendColumn(wxT("Name"));
    sizer->Add(listView, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer* sizerButtons = new wxBoxSizer(wxHORIZONTAL);

    // Add buttons at the bottom
    // Ok button
    okButton = new wxButton(this, wxID_OK, wxT("OK"));
    okButton->Enable(false);
    sizerButtons->Add(okButton, 0, wxALIGN_CENTER | wxALL, 5);

    // Cancel button
    wxButton *cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
    sizerButtons->Add(cancelButton, 0, wxALIGN_CENTER | wxALL, 5);

    sizer->Add(sizerButtons, 0, wxALIGN_CENTER | wxALL, 5);

    // Apply sizer to the dialog
    SetSizer(sizer);

    searchInput->Bind(wxEVT_TEXT, &FindMedicamentDialog::OnText, this);
    listView->Bind(wxEVT_LIST_ITEM_SELECTED, &FindMedicamentDialog::OnSelect, this);
    listView->Bind(wxEVT_LIST_ITEM_DESELECTED, &FindMedicamentDialog::OnSelect, this);
}

bool FindMedicamentDialog::CanOpen() const {
    return festDb.IsOpen();
}

FindMedicamentDialogSearchResult FindMedicamentDialog::PerformSearch() {
    FindMedicamentDialogSearchResult result{};
    auto term = searchInput->GetValue().ToStdString();
    if (term.size() > 2) {
        auto legemiddelVirkestoffOppfs = festDb.GetAllPLegemiddelVirkestoff();
        {
            std::vector<FestUuid> legemiddelVirkestoffIds{};
            std::vector<FestUuid> legemiddelMerkevareIds{};
            std::vector<FestUuid> legemiddelpakningIds{};
            result.legemiddelVirkestoffList = festDb.FindLegemiddelVirkestoff(legemiddelVirkestoffOppfs, term);
            for (const auto &legemiddelVirkestoff: result.legemiddelVirkestoffList) {
                legemiddelVirkestoffIds.emplace_back(legemiddelVirkestoff.GetId());
                auto refMerkevare = legemiddelVirkestoff.GetRefLegemiddelMerkevare();
                for (const auto &merkevareId: refMerkevare) {
                    FestUuid festId{merkevareId};
                    auto merkevare = festDb.GetLegemiddelMerkevare(festId);
                    legemiddelMerkevareIds.emplace_back(festId);
                    result.legemiddelMerkevareList.emplace_back(merkevare);
                }
                auto refPakning = legemiddelVirkestoff.GetRefPakning();
                for (const auto &pakningId: refPakning) {
                    FestUuid festId{pakningId};
                    auto pakning = festDb.GetLegemiddelpakning(festId);
                    legemiddelpakningIds.emplace_back(festId);
                    result.legemiddelpakningList.emplace_back(pakning);
                }
            }
            auto legemiddelMerkevareSearchList = festDb.FindLegemiddelMerkevare(term);
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
                    result.legemiddelMerkevareList.emplace_back(legemiddelMerkevare);
                }
            }
            auto legemiddelpakningSearchList = festDb.FindLegemiddelpakning(term);
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
                    result.legemiddelpakningList.emplace_back(legemiddelpakning);
                    legemiddelpakningIds.emplace_back(legemiddelpakning.GetId());
                }
            }
            for (const auto &plv: legemiddelVirkestoffOppfs) {
                auto unpacked = festDb.GetLegemiddelVirkestoff(plv);
                if (festDb.PLegemiddelVirkestoffHasOneOfMerkevare(plv, legemiddelMerkevareIds) ||
                    festDb.PLegemiddelVirkestoffHasOneOfPakning(plv, legemiddelpakningIds)) {
                    auto id = festDb.GetLegemiddelVirkestoffId(plv);
                    bool found{false};
                    for (const auto &eid: legemiddelVirkestoffIds) {
                        if (id == eid) {
                            found = true;
                            break;
                        }
                    }
                    if (!found) {
                        legemiddelVirkestoffIds.emplace_back(id);
                        auto legemiddelVirkestoff = festDb.GetLegemiddelVirkestoff(id);
                        result.legemiddelVirkestoffList.emplace_back(legemiddelVirkestoff);
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
                                result.legemiddelMerkevareList.emplace_back(festDb.GetLegemiddelMerkevare(festId));
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
                                result.legemiddelpakningList.emplace_back(festDb.GetLegemiddelpakning(festId));
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

void FindMedicamentDialog::ShowSearchResult(const FindMedicamentDialogSearchResult &result) {
    listView->ClearAll();
    listView->AppendColumn(wxT("Name form strength"));
    listView->SetColumnWidth(0, 400);
    int i = 0;
    for (const auto &legemiddelVirkestoff : result.legemiddelVirkestoffList) {
        std::string navnFormStyrke = legemiddelVirkestoff.GetNavnFormStyrke();
        wxString navnFormStyrkeWx = wxString::FromUTF8(navnFormStyrke.c_str());
        listView->InsertItem(i++, navnFormStyrkeWx);
    }
    for (const auto &legemiddelMerkevare : result.legemiddelMerkevareList) {
        std::string navnFormStyrke = legemiddelMerkevare.GetNavnFormStyrke();
        wxString navnFormStyrkeWx = wxString::FromUTF8(navnFormStyrke.c_str());
        listView->InsertItem(i++, navnFormStyrkeWx);
    }
    for (const auto &legemiddelpakning : result.legemiddelpakningList) {
        std::string navnFormStyrke = legemiddelpakning.GetNavnFormStyrke();
        wxString navnFormStyrkeWx = wxString::FromUTF8(navnFormStyrke.c_str());
        listView->InsertItem(i++, navnFormStyrkeWx);
    }
}

void FindMedicamentDialog::OnText(wxCommandEvent &e) {
    searchDebouncer.Schedule();
}

void FindMedicamentDialog::OnSelect(wxCommandEvent &e) {

}
