//
// Created by sigsegv on 5/18/24.
//

#include "FestDbQuotasDialog.h"
#include <sstream>
#include <wx/listctrl.h>
#include "FestDb.h"
#include <medfest/FestDeserializer.h>

FestDbQuotasDialog::FestDbQuotasDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, wxT("Database quotas")) {
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    auto *listView = new wxListView(this, wxID_ANY);
    listView->AppendColumn(wxT("Name"));
    listView->AppendColumn(wxT("Records"));
    listView->AppendColumn(wxT("Compat fill"));
    listView->AppendColumn(wxT("Hard max"));
    FestDb festDb{};
    if (!festDb.IsOpen()) {
        return;
    }
    auto quotas = festDb.GetDbQuotas();
    int rowNum = 0;
    for (const auto &quota : quotas) {
        auto row = rowNum++;
        listView->InsertItem(row, wxString::FromUTF8(quota.name));
        {
            std::stringstream str{};
            str << quota.total;
            listView->SetItem(row, 1, wxString::FromUTF8(str.str()));
        }
        {
            uint64_t total = quota.total * ((uint64_t) 100);
            uint64_t pct = total / quota.compatMax;
            std::stringstream str{};
            str << pct << "%";
            listView->SetItem(row, 2, wxString::FromUTF8(str.str()));
        }
        {
            uint64_t total = quota.total * ((uint64_t) 100);
            uint64_t pct = total / quota.hardMax;
            std::stringstream str{};
            str << pct << "%";
            listView->SetItem(row, 3, wxString::FromUTF8(str.str()));
        }
    }
    sizer->Add(listView, 1, wxALL | wxEXPAND, 5);
    SetSizerAndFit(sizer);
}