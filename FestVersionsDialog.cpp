//
// Created by sigsegv on 4/30/24.
//

#include "FestVersionsDialog.h"
#include <wx/listctrl.h>
#include "FestDb.h"
#include "TheMasterIds.h"
#include "FestExploreVersionDialog.h"

FestVersionsDialog::FestVersionsDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, wxT("FEST versions")) {
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    listView = new wxListView(this, wxID_ANY);
    listView->AppendColumn(wxT("Version"));
    listView->SetColumnWidth(0, 200);
    db = std::make_shared<FestDb>();
    if (db->IsOpen()) {
        versions = db->GetFestVersions();
        int i = 0;
        for (const auto &version : versions) {
            wxString v = wxString::FromUTF8(version);
            listView->InsertItem(i++, v);
        }
    }
    sizer->Add(listView, 0, wxEXPAND | wxALL, 5);
    listView->Bind(wxEVT_CONTEXT_MENU, &FestVersionsDialog::OnVersionContextMenu, this);
    Bind(wxEVT_MENU, &FestVersionsDialog::OnVersionExplore, this, TheMaster_VersionDialog_Explore);
}

void FestVersionsDialog::OnVersionContextMenu(wxContextMenuEvent &event) {
    auto firstItem = listView->GetFirstSelected();
    if (firstItem < 0 || firstItem >= versions.size()) {
        return;
    }
    auto secondItem = listView->GetNextSelected(firstItem);
    if (secondItem >= 0 && secondItem < versions.size()) {
        return;
    }
    auto version = versions[firstItem];
    wxMenu menu(wxString::FromUTF8(version));
    menu.Append(TheMaster_VersionDialog_Explore, wxT("Explore"));
    PopupMenu(&menu);
}

void FestVersionsDialog::OnVersionExplore(wxCommandEvent &event) {
    auto firstItem = listView->GetFirstSelected();
    if (firstItem < 0 || firstItem >= versions.size()) {
        return;
    }
    auto secondItem = listView->GetNextSelected(firstItem);
    if (secondItem >= 0 && secondItem < versions.size()) {
        return;
    }
    auto version = versions[firstItem];
    FestExploreVersionDialog explore{this, db, version};
    explore.ShowModal();
}
