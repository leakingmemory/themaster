//
// Created by sigsegv on 4/30/24.
//

#include "FestVersionsDialog.h"
#include <wx/listctrl.h>
#include "FestDb.h"
#include "TheMasterIds.h"
#include "FestExploreVersionDialog.h"
#include "FestDiffVersionsDialog.h"
#include <thread>

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
    bool diffing{false};
    auto firstItem = listView->GetFirstSelected();
    if (firstItem < 0 || firstItem >= versions.size()) {
        return;
    }
    auto secondItem = listView->GetNextSelected(firstItem);
    if (secondItem >= 0 && secondItem < versions.size()) {
        auto thirdItem = listView->GetNextSelected(secondItem);
        if (thirdItem >= 0 && thirdItem < versions.size()) {
            return;
        }
        diffing = true;
    }
    auto version = versions[firstItem];
    wxMenu menu(wxString::FromUTF8(version));
    menu.Append(TheMaster_VersionDialog_Explore, diffing ? wxT("Show differences") : wxT("Explore"));
    PopupMenu(&menu);
}

class FestDiffProgressDialog : public wxDialog {
private:
    wxGauge *gauge;
    wxGauge *addsAndRemoves;
    wxGauge *modifications;
    bool finished{false};
public:
    FestDiffProgressDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, wxT("Creating diff between versions")){
        auto *sizer = new wxBoxSizer(wxVERTICAL);
        sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Total progress:")), 0, wxALL | wxEXPAND, 5);
        gauge = new wxGauge(this, wxID_ANY, 1);
        gauge->SetValue(0);
        sizer->Add(gauge, 1, wxALL | wxEXPAND, 5);
        sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Step, adds and removes:")), 0, wxALL | wxEXPAND, 5);
        addsAndRemoves = new wxGauge(this, wxID_ANY, 1);
        addsAndRemoves->SetValue(0);
        sizer->Add(addsAndRemoves, 1, wxALL | wxEXPAND, 5);
        sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Step, find modifications:")), 0, wxALL | wxEXPAND, 5);
        modifications = new wxGauge(this, wxID_ANY, 1);
        modifications->SetValue(0);
        sizer->Add(modifications, 1, wxALL | wxEXPAND, 5);
        SetSizerAndFit(sizer);
        Bind(wxEVT_CLOSE_WINDOW, &FestDiffProgressDialog::OnClose, this);
    }
    void OnClose(wxCloseEvent &e) {
        if (!finished) {
            e.Veto();
        }
    }
    void SetProgress(int toplevelDone, int toplevelMax, int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax) {
        gauge->SetRange(toplevelMax);
        gauge->SetValue(toplevelDone);
        addsAndRemoves->SetRange(addsAndRemovesMax);
        addsAndRemoves->SetValue(addsAndRemovesDone);
        modifications->SetRange(modificationsMax);
        modifications->SetValue(modificationsDone);
    }
    void Finished() {
        finished = true;
        wxDialog::EndModal(0);
    }
};

void FestVersionsDialog::OnVersionExplore(wxCommandEvent &event) {
    auto firstItem = listView->GetFirstSelected();
    if (firstItem < 0 || firstItem >= versions.size()) {
        return;
    }
    auto secondItem = listView->GetNextSelected(firstItem);
    if (secondItem >= 0 && secondItem < versions.size()) {
        {
            auto thirdItem = listView->GetNextSelected(secondItem);
            if (thirdItem >= 0 && thirdItem < versions.size()) {
                return;
            }
            auto toVersion = versions[firstItem];
            auto fromVersion = versions[secondItem];
            std::shared_ptr<FestDiffProgressDialog> progressDialog = std::make_shared<FestDiffProgressDialog>(this);
            std::shared_ptr<FestDiffVersionsDialog> dialog = std::make_shared<FestDiffVersionsDialog>(this, fromVersion, toVersion);
            std::shared_ptr<FestDb> festDb = db;
            std::thread thr{
                [progressDialog, dialog, festDb] () {
                    dialog->RunDiff([progressDialog] (int toplevelDone, int toplevelMax, int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax) {
                        wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([progressDialog, toplevelDone, toplevelMax, addsAndRemovesDone, addsAndRemovesMax, modificationsDone, modificationsMax] () {
                            progressDialog->SetProgress(toplevelDone, toplevelMax, addsAndRemovesDone, addsAndRemovesMax, modificationsDone, modificationsMax);
                        });
                    }, festDb);
                    wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([progressDialog] () {
                        progressDialog->Finished();
                    });
                }
            };
            thr.detach();
            progressDialog->ShowModal();
            dialog->ShowModal();
        }
        return;
    }
    auto version = versions[firstItem];
    FestExploreVersionDialog explore{this, db, version};
    explore.ShowModal();
}
