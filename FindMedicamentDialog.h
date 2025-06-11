//
// Created by sigsegv on 3/19/24.
//

#ifndef DRWHATSNOT_FINDMEDICAMENTDIALOG_H
#define DRWHATSNOT_FINDMEDICAMENTDIALOG_H

#include <wx/wx.h>
#include "FestDb.h"
#include "WxDebouncer.h"
#include "Lazy.h"
#include <medfest/Struct/Packed/FestUuid.h>
#include <medfest/Struct/Packed/PReseptgyldighet.h>
#include "MedicamentSearch.h"

class wxListView;

struct FindMedicamentDialogSearchResult;
class LegemiddelCore;

class FindMedicamentDialog : public wxDialog {
private:
    std::shared_ptr<FestDb> festDb{};
    WxDebouncer searchDebouncer;
    MedicamentSearch medicamentSearch;
    Lazy<std::function<std::map<FestUuid,std::vector<PReseptgyldighet>> ()>> merkevareToPrescriptionValidity;
    Lazy<std::function<std::vector<FestUuid> ()>> merkevareWithTwoOrMoreReseptgyldighet;
    Lazy<std::function<std::vector<FestUuid> ()>> legemiddelVirkestoffWithTwoOrMoreReseptgyldighet;
    Lazy<std::function<std::vector<FestUuid> ()>> legemiddelpakningWithTwoOrMoreReseptgyldighet;
    std::shared_ptr<MedicamentSearchResult> medicamentLists{};
    std::shared_ptr<LegemiddelCore> selected{};
    wxComboBox *selections;
    wxTextCtrl *searchInput;
    wxListView *listView;
    wxButton *okButton;
public:
    FindMedicamentDialog(wxWindow *frame, const std::shared_ptr<FestDb> &);
    bool CanOpen() const;
    std::shared_ptr<MedicamentSearchResult> PerformSearch();
    std::shared_ptr<FindMedicamentDialogSearchResult> OldPerformSearch();
    std::map<FestUuid,std::vector<PReseptgyldighet>> CreateMerkevareToPrescriptionValidity() const;
    std::vector<FestUuid> FindMerkevareWithTwoOrMoreReseptgyldighet();
    std::vector<FestUuid> FindLegemiddelVirkestoffWithTwoOrMoreReseptgyldighet();
    std::vector<FestUuid> FindLegemiddelpakningWithTwoOrMoreReseptgyldighet();
    void ShowSearchResult(const std::shared_ptr<MedicamentSearchResult> &);
    void OnSelection(wxCommandEvent &e);
    void OnText(wxCommandEvent &e);
    void UpdateSelected();
    void OnSelect(wxCommandEvent &e);
    [[nodiscard]] std::shared_ptr<LegemiddelCore> GetSelected() const {
        return selected;
    }
};


#endif //DRWHATSNOT_FINDMEDICAMENTDIALOG_H
