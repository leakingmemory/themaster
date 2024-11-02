//
// Created by sigsegv on 3/19/24.
//

#include "FindMedicamentDialog.h"
#include <wx/listctrl.h>
#include <medfest/Struct/Decoded/LegemiddelVirkestoff.h>
#include <medfest/Struct/Decoded/LegemiddelMerkevare.h>
#include <medfest/Struct/Decoded/Legemiddelpakning.h>
#include <medfest/Struct/Packed/POppfLegemiddelVirkestoff.h>
#include <medfest/Struct/Packed/POppfLegemiddelMerkevare.h>
#include <medfest/Struct/Packed/POppfLegemiddelpakning.h>

struct FindMedicamentDialogSearchResult {
    std::vector<LegemiddelVirkestoff> legemiddelVirkestoffList{};
    std::vector<LegemiddelMerkevare> legemiddelMerkevareList{};
    std::vector<Legemiddelpakning> legemiddelpakningList{};
};

enum class FindMedicamentSelections {
    ALL = 0,
    TWO_OR_MORE_PRESCRIPTION_VALIDITY = 1
};

FindMedicamentDialog::FindMedicamentDialog(wxWindow *parent) :
    wxDialog(parent, wxID_ANY, wxT("Find medicament")),
    searchDebouncer(),
    merkevareToPrescriptionValidity([this] () { return CreateMerkevareToPrescriptionValidity(); }),
    merkevareWithTwoOrMoreReseptgyldighet([this] () { return FindMerkevareWithTwoOrMoreReseptgyldighet(); }),
    legemiddelVirkestoffWithTwoOrMoreReseptgyldighet([this] () { return FindLegemiddelVirkestoffWithTwoOrMoreReseptgyldighet(); }),
    legemiddelpakningWithTwoOrMoreReseptgyldighet([this] () { return FindLegemiddelpakningWithTwoOrMoreReseptgyldighet(); }) {
    searchDebouncer.Func<std::shared_ptr<FindMedicamentDialogSearchResult>>([this] () { return PerformSearch(); }, [this] (const std::shared_ptr<FindMedicamentDialogSearchResult> &result) { ShowSearchResult(result); });

    // Add a sizer to handle the layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    selections = new wxComboBox(this, wxID_ANY);
    selections->SetEditable(false);
    selections->Append(wxT("All"));
    selections->Append(wxT("Two or more pr.validity"));
    selections->SetSelection(0);
    sizer->Add(selections, 0, wxEXPAND|wxALL, 5);

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

    selections->Bind(wxEVT_COMBOBOX, &FindMedicamentDialog::OnSelection, this);
    searchInput->Bind(wxEVT_TEXT, &FindMedicamentDialog::OnText, this);
    listView->Bind(wxEVT_LIST_ITEM_SELECTED, &FindMedicamentDialog::OnSelect, this);
    listView->Bind(wxEVT_LIST_ITEM_DESELECTED, &FindMedicamentDialog::OnSelect, this);
}

bool FindMedicamentDialog::CanOpen() const {
    return festDb.IsOpen();
}

std::shared_ptr<FindMedicamentDialogSearchResult> FindMedicamentDialog::PerformSearch() {
    std::shared_ptr<FindMedicamentDialogSearchResult> result = std::make_shared<FindMedicamentDialogSearchResult>();
    auto selection = static_cast<FindMedicamentSelections>(selections->GetSelection());
    auto term = searchInput->GetValue().ToStdString();
    if (term.size() > 2 || selection == FindMedicamentSelections::TWO_OR_MORE_PRESCRIPTION_VALIDITY) {
        auto legemiddelVirkestoffOppfs = festDb.GetAllPLegemiddelVirkestoff();
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
                auto id = festDb.GetLegemiddelVirkestoffId(*iterator);
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
            result->legemiddelVirkestoffList = festDb.FindLegemiddelVirkestoff(legemiddelVirkestoffOppfs, term);
            for (const auto &legemiddelVirkestoff: result->legemiddelVirkestoffList) {
                legemiddelVirkestoffIds.emplace_back(legemiddelVirkestoff.GetId());
                auto refMerkevare = legemiddelVirkestoff.GetRefLegemiddelMerkevare();
                for (const auto &merkevareId: refMerkevare) {
                    FestUuid festId{merkevareId};
                    auto merkevare = festDb.GetLegemiddelMerkevare(festId);
                    legemiddelMerkevareIds.emplace_back(festId);
                    result->legemiddelMerkevareList.emplace_back(merkevare);
                }
                auto refPakning = legemiddelVirkestoff.GetRefPakning();
                for (const auto &pakningId: refPakning) {
                    FestUuid festId{pakningId};
                    auto pakning = festDb.GetLegemiddelpakning(festId);
                    legemiddelpakningIds.emplace_back(festId);
                    result->legemiddelpakningList.emplace_back(pakning);
                }
            }
            auto legemiddelMerkevareOppfs = festDb.GetAllPLegemiddelMerkevare();
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
                    auto id = festDb.GetLegemiddelMerkevareId(*iterator);
                    auto found = std::find(ids.begin(), ids.end(), id);
                    if (found != ids.end()) {
                        ids.erase(found);
                        ++iterator;
                    } else {
                        iterator = legemiddelMerkevareOppfs.erase(iterator);
                    }
                }
            }
            auto legemiddelMerkevareSearchList = festDb.FindLegemiddelMerkevare(legemiddelMerkevareOppfs, term);
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
            auto legemiddelpakningOppfs = festDb.GetAllPLegemiddelpakning();
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
                    auto id = festDb.GetLegemiddelpakningId(*iterator);
                    auto found = std::find(ids.begin(), ids.end(), id);
                    if (found != ids.end()) {
                        ids.erase(found);
                        ++iterator;
                    } else {
                        iterator = legemiddelpakningOppfs.erase(iterator);
                    }
                }
            }
            auto legemiddelpakningSearchList = festDb.FindLegemiddelpakning(legemiddelpakningOppfs, term);
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
                                result->legemiddelMerkevareList.emplace_back(festDb.GetLegemiddelMerkevare(festId));
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
                                result->legemiddelpakningList.emplace_back(festDb.GetLegemiddelpakning(festId));
                            }
                        }
                    }
                }
            }
        }
    }
    return result;
}

std::map<FestUuid, std::vector<PReseptgyldighet>>
FindMedicamentDialog::CreateMerkevareToPrescriptionValidity() const {
    std::map<FestUuid, std::vector<PReseptgyldighet>> map{};
    auto allPMerkevare = festDb.GetAllPLegemiddelMerkevare();
    for (const auto &pmerkevare : allPMerkevare) {
        auto id = festDb.GetLegemiddelMerkevareId(pmerkevare);
        auto prep = festDb.GetPReseptgyldighet(pmerkevare);
        map.insert_or_assign(id, prep);
    }
    return map;
}

std::vector<FestUuid> FindMedicamentDialog::FindMerkevareWithTwoOrMoreReseptgyldighet() {
    std::vector<FestUuid> ids{};
    auto &map = merkevareToPrescriptionValidity.operator std::map<FestUuid, std::vector<PReseptgyldighet>> &();
    for (const auto &[id, preseptgyldighet] : map) {
        if (preseptgyldighet.size() > 1) {
            ids.emplace_back(id);
        }
    }
    return ids;
}

std::vector<FestUuid> FindMedicamentDialog::FindLegemiddelVirkestoffWithTwoOrMoreReseptgyldighet() {
    std::vector<FestUuid> ids{};
    auto &map = merkevareToPrescriptionValidity.operator std::map<FestUuid, std::vector<PReseptgyldighet>> &();
    auto allPVirkestoff = festDb.GetAllPLegemiddelVirkestoff();
    for (const auto &pvirkestoff : allPVirkestoff) {
        auto merkevareIds = festDb.GetRefMerkevare(pvirkestoff);
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
            ids.emplace_back(festDb.GetLegemiddelVirkestoffId(pvirkestoff));
        }
    }
    return ids;
}

std::vector<FestUuid> FindMedicamentDialog::FindLegemiddelpakningWithTwoOrMoreReseptgyldighet() {
    std::vector<FestUuid> ids{};
    auto &map = merkevareToPrescriptionValidity.operator std::map<FestUuid, std::vector<PReseptgyldighet>> &();
    auto allPPakning = festDb.GetAllPLegemiddelpakning();
    for (const auto &ppakning : allPPakning) {
        auto merkevareIds = festDb.GetRefMerkevare(ppakning);
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
            ids.emplace_back(festDb.GetLegemiddelpakningId(ppakning));
        }
    }
    return ids;
}

void FindMedicamentDialog::ShowSearchResult(const std::shared_ptr<FindMedicamentDialogSearchResult> &result) {
    medicamentLists = result;
    listView->ClearAll();
    listView->AppendColumn(wxT("Name form strength"));
    listView->SetColumnWidth(0, 400);
    int i = 0;
    for (const auto &legemiddelVirkestoff : result->legemiddelVirkestoffList) {
        std::string navnFormStyrke = legemiddelVirkestoff.GetNavnFormStyrke();
        wxString navnFormStyrkeWx = wxString::FromUTF8(navnFormStyrke.c_str());
        listView->InsertItem(i++, navnFormStyrkeWx);
    }
    for (const auto &legemiddelMerkevare : result->legemiddelMerkevareList) {
        std::string navnFormStyrke = legemiddelMerkevare.GetNavnFormStyrke();
        wxString navnFormStyrkeWx = wxString::FromUTF8(navnFormStyrke.c_str());
        listView->InsertItem(i++, navnFormStyrkeWx);
    }
    for (const auto &legemiddelpakning : result->legemiddelpakningList) {
        std::string navnFormStyrke = legemiddelpakning.GetNavnFormStyrke();
        wxString navnFormStyrkeWx = wxString::FromUTF8(navnFormStyrke.c_str());
        listView->InsertItem(i++, navnFormStyrkeWx);
    }
}

void FindMedicamentDialog::OnSelection(wxCommandEvent &e) {
    searchDebouncer.Schedule();
}

void FindMedicamentDialog::OnText(wxCommandEvent &e) {
    searchDebouncer.Schedule();
}

void FindMedicamentDialog::UpdateSelected() {
    size_t index = listView->GetFirstSelected();
    if (index < 0 || !medicamentLists) {
        selected = {};
        return;
    }
    if (index < medicamentLists->legemiddelVirkestoffList.size()) {
        selected = std::make_shared<LegemiddelVirkestoff>(medicamentLists->legemiddelVirkestoffList[index]);
        return;
    }
    index -= medicamentLists->legemiddelVirkestoffList.size();
    if (index < medicamentLists->legemiddelMerkevareList.size()) {
        selected = std::make_shared<LegemiddelMerkevare>(medicamentLists->legemiddelMerkevareList[index]);
        return;
    }
    index -= medicamentLists->legemiddelMerkevareList.size();
    if (index < medicamentLists->legemiddelpakningList.size()) {
        selected = std::make_shared<Legemiddelpakning>(medicamentLists->legemiddelpakningList[index]);
        return;
    }
    selected = {};
}

void FindMedicamentDialog::OnSelect(wxCommandEvent &e) {
    UpdateSelected();
    okButton->Enable(selected.operator bool());
}
