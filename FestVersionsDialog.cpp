//
// Created by sigsegv on 4/30/24.
//

#include "FestVersionsDialog.h"
#include <wx/listctrl.h>
#include "FestDb.h"

FestVersionsDialog::FestVersionsDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, wxT("FEST versions")) {
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    auto *listView = new wxListView(this, wxID_ANY);
    listView->AppendColumn(wxT("Version"));
    listView->SetColumnWidth(0, 200);
    FestDb db{};
    if (db.IsOpen()) {
        auto versions = db.GetFestVersions();
        int i = 0;
        for (const auto &version : versions) {
            wxString v = wxString::FromUTF8(version);
            listView->InsertItem(i++, v);
        }
    }
    sizer->Add(listView, 0, wxEXPAND | wxALL, 5);
}
