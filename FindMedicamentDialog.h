//
// Created by sigsegv on 3/19/24.
//

#ifndef DRWHATSNOT_FINDMEDICAMENTDIALOG_H
#define DRWHATSNOT_FINDMEDICAMENTDIALOG_H

#include <wx/wx.h>
#include "FestDb.h"
#include "WxDebouncer.h"

class wxListView;

struct FindMedicamentDialogSearchResult;
class LegemiddelCore;

class FindMedicamentDialog : public wxDialog {
private:
    FestDb festDb{};
    WxDebouncer searchDebouncer;
    std::shared_ptr<FindMedicamentDialogSearchResult> medicamentLists{};
    std::shared_ptr<LegemiddelCore> selected{};
    wxTextCtrl *searchInput;
    wxListView *listView;
    wxButton *okButton;
public:
    FindMedicamentDialog(wxWindow *frame);
    bool CanOpen() const;
    std::shared_ptr<FindMedicamentDialogSearchResult> PerformSearch();
    void ShowSearchResult(const std::shared_ptr<FindMedicamentDialogSearchResult> &);
    void OnText(wxCommandEvent &e);
    void UpdateSelected();
    void OnSelect(wxCommandEvent &e);
    [[nodiscard]] std::shared_ptr<LegemiddelCore> GetSelected() const {
        return selected;
    }
};


#endif //DRWHATSNOT_FINDMEDICAMENTDIALOG_H
