//
// Created by sigsegv on 5/18/24.
//

#include "FestDiffVersionsDialog.h"
#include "FestDb.h"
#include <medfest/Struct/Decoded/OppfRefusjon.h>
#include <medfest/Struct/Decoded/OppfLegemiddelMerkevare.h>
#include <medfest/Struct/Decoded/OppfLegemiddelVirkestoff.h>
#include <medfest/Struct/Decoded/OppfLegemiddelpakning.h>
#include <medfest/Struct/Decoded/OppfLegemiddeldose.h>
#include <medfest/Struct/Decoded/OppfKodeverk.h>
#include <sstream>
#include "FestExploreVersionDialog.h"

FestDiffVersionsDialog::FestDiffVersionsDialog(wxWindow *parent, const std::string &fromVersion, const std::string &toVersion) :
    wxDialog(parent, wxID_ANY, wxT("Diff versions")),
    fromVersion(fromVersion),
    toVersion(toVersion),
    refusjon(std::make_shared<FestDiff<OppfRefusjon>>()),
    legemiddelMerkevare(std::make_shared<FestDiff<OppfLegemiddelMerkevare>>()),
    legemiddelVirkestoff(std::make_shared<FestDiff<OppfLegemiddelVirkestoff>>()),
    legemiddelpakning(std::make_shared<FestDiff<OppfLegemiddelpakning>>()),
    legemiddeldose(std::make_shared<FestDiff<OppfLegemiddeldose>>()),
    kodeverk(std::make_shared<FestDiff<OppfKodeverk>>()),
    atc(std::make_shared<FestDiff<Element>>())
{
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    {
        std::stringstream str{};
        str << "Diff from version " << fromVersion << " to " << toVersion;
        auto label = new wxStaticText(this, wxID_ANY, wxString::FromUTF8(str.str()));
        sizer->Add(label, 0, wxEXPAND | wxALL, 5);
    }
    auto *grid = new wxGridSizer(2);
    auto *removedButton = new wxButton(this, wxID_ANY, wxT("Removed"));
    grid->Add(removedButton);
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("-")));
    auto *modifiedPreviousButton = new wxButton(this, wxID_ANY, wxT("Modified (previous)"));
    grid->Add(modifiedPreviousButton);
    auto *modifiedNewButton = new wxButton(this, wxID_ANY, wxT("Modified (new)"));
    grid->Add(modifiedNewButton);
    grid->Add(new wxStaticText(this, wxID_ANY, wxT("-")));
    auto *addedButton = new wxButton(this, wxID_ANY, wxT("Added"));
    grid->Add(addedButton);
    sizer->Add(grid, 1, wxEXPAND | wxALL, 5);
    removedButton->Bind(wxEVT_BUTTON, &FestDiffVersionsDialog::OnRemoved, this);
    modifiedPreviousButton->Bind(wxEVT_BUTTON, &FestDiffVersionsDialog::OnModifiedPrevious, this);
    modifiedNewButton->Bind(wxEVT_BUTTON, &FestDiffVersionsDialog::OnModifiedNew, this);
    addedButton->Bind(wxEVT_BUTTON, &FestDiffVersionsDialog::OnAdded, this);
    SetSizerAndFit(sizer);
}

void FestDiffVersionsDialog::RunDiff(const std::function<void (int toplevelDone,int toplevelMax, int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progressFuncRef, const std::shared_ptr<FestDb> &db) {
    std::function<void (int toplevelDone,int toplevelMax, int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> progress{progressFuncRef};
    int toplevelDone{0};
    int toplevelMax{7};
    progress(0, toplevelMax, 0, 1, 0, 1);
    std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> subProgress =
            [progress, &toplevelDone, toplevelMax] (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax) {
        progress(toplevelDone, toplevelMax, addsAndRemovesDone, addsAndRemovesMax, modificationsDone, modificationsMax);
    };
    *refusjon = db->GetOppfRefusjonDiff(subProgress, fromVersion, toVersion);
    ++toplevelDone;
    progress(toplevelDone, toplevelMax, 0, 1, 0, 1);
    *legemiddelMerkevare = db->GetOppfLegemiddelMerkevareDiff(subProgress, fromVersion, toVersion);
    ++toplevelDone;
    progress(toplevelDone, toplevelMax, 0, 1, 0, 1);
    *legemiddelVirkestoff = db->GetOppfLegemiddelVirkestoffDiff(subProgress, fromVersion, toVersion);
    ++toplevelDone;
    progress(toplevelDone, toplevelMax, 0, 1, 0, 1);
    *legemiddelpakning = db->GetOppfLegemiddelpakningDiff(subProgress, fromVersion, toVersion);
    ++toplevelDone;
    progress(toplevelDone, toplevelMax, 0, 1, 0, 1);
    *legemiddeldose = db->GetOppfLegemiddeldoseDiff(subProgress, fromVersion, toVersion);
    ++toplevelDone;
    progress(toplevelDone, toplevelMax, 0, 1, 0, 1);
    *kodeverk = db->GetOppfKodeverkDiff(subProgress, fromVersion, toVersion);
    ++toplevelDone;
    progress(toplevelDone, toplevelMax, 0, 1, 0, 1);
    *atc = db->GetKodeverkElementsDiff(subProgress, "2.16.578.1.12.4.1.1.7180", fromVersion, toVersion);
    ++toplevelDone;
    progress(toplevelDone, toplevelMax, 0, 1, 0, 1);
}

void FestDiffVersionsDialog::OnRemoved(wxCommandEvent &) {
    FestExploreVersionDialog dialog{
        this,
        std::make_shared<std::vector<OppfRefusjon>>(refusjon->removed),
        std::make_shared<std::vector<OppfLegemiddelMerkevare>>(legemiddelMerkevare->removed),
        std::make_shared<std::vector<OppfLegemiddelVirkestoff>>(legemiddelVirkestoff->removed),
        std::make_shared<std::vector<OppfLegemiddelpakning>>(legemiddelpakning->removed),
        std::make_shared<std::vector<OppfLegemiddeldose>>(legemiddeldose->removed),
        std::make_shared<std::vector<OppfKodeverk>>(kodeverk->removed),
        std::make_shared<std::vector<Element>>(atc->removed),
        fromVersion
    };
    dialog.ShowModal();
}

template <class T> std::vector<T> PreviousItems(std::vector<FestModified<T>> modified) {
    std::vector<T> result{};
    for (const auto &mod : modified) {
        result.emplace_back(mod.previous);
    }
    return result;
}

template <class T> std::vector<T> NewItems(std::vector<FestModified<T>> modified) {
    std::vector<T> result{};
    for (const auto &mod : modified) {
        result.emplace_back(mod.latest);
    }
    return result;
}

void FestDiffVersionsDialog::OnModifiedPrevious(wxCommandEvent &e) {
    FestExploreVersionDialog dialog{
            this,
            std::make_shared<std::vector<OppfRefusjon>>(PreviousItems<OppfRefusjon>(refusjon->modified)),
            std::make_shared<std::vector<OppfLegemiddelMerkevare>>(PreviousItems<OppfLegemiddelMerkevare>(legemiddelMerkevare->modified)),
            std::make_shared<std::vector<OppfLegemiddelVirkestoff>>(PreviousItems<OppfLegemiddelVirkestoff>(legemiddelVirkestoff->modified)),
            std::make_shared<std::vector<OppfLegemiddelpakning>>(PreviousItems<OppfLegemiddelpakning>(legemiddelpakning->modified)),
            std::make_shared<std::vector<OppfLegemiddeldose>>(PreviousItems<OppfLegemiddeldose>(legemiddeldose->modified)),
            std::make_shared<std::vector<OppfKodeverk>>(PreviousItems<OppfKodeverk>(kodeverk->modified)),
            std::make_shared<std::vector<Element>>(PreviousItems<Element>(atc->modified)),
            fromVersion
    };
    dialog.ShowModal();
}

void FestDiffVersionsDialog::OnModifiedNew(wxCommandEvent &e) {
    FestExploreVersionDialog dialog{
            this,
            std::make_shared<std::vector<OppfRefusjon>>(NewItems<OppfRefusjon>(refusjon->modified)),
            std::make_shared<std::vector<OppfLegemiddelMerkevare>>(NewItems<OppfLegemiddelMerkevare>(legemiddelMerkevare->modified)),
            std::make_shared<std::vector<OppfLegemiddelVirkestoff>>(NewItems<OppfLegemiddelVirkestoff>(legemiddelVirkestoff->modified)),
            std::make_shared<std::vector<OppfLegemiddelpakning>>(NewItems<OppfLegemiddelpakning>(legemiddelpakning->modified)),
            std::make_shared<std::vector<OppfLegemiddeldose>>(NewItems<OppfLegemiddeldose>(legemiddeldose->modified)),
            std::make_shared<std::vector<OppfKodeverk>>(NewItems<OppfKodeverk>(kodeverk->modified)),
            std::make_shared<std::vector<Element>>(NewItems<Element>(atc->modified)),
            fromVersion
    };
    dialog.ShowModal();
}

void FestDiffVersionsDialog::OnAdded(wxCommandEvent &e) {
    FestExploreVersionDialog dialog{
            this,
            std::make_shared<std::vector<OppfRefusjon>>(refusjon->added),
            std::make_shared<std::vector<OppfLegemiddelMerkevare>>(legemiddelMerkevare->added),
            std::make_shared<std::vector<OppfLegemiddelVirkestoff>>(legemiddelVirkestoff->added),
            std::make_shared<std::vector<OppfLegemiddelpakning>>(legemiddelpakning->added),
            std::make_shared<std::vector<OppfLegemiddeldose>>(legemiddeldose->added),
            std::make_shared<std::vector<OppfKodeverk>>(kodeverk->added),
            std::make_shared<std::vector<Element>>(atc->added),
            fromVersion
    };
    dialog.ShowModal();
}
