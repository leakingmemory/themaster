//
// Created by sigsegv on 3/19/24.
//

#ifndef DRWHATSNOT_FINDMEDICAMENTDIALOG_H
#define DRWHATSNOT_FINDMEDICAMENTDIALOG_H

#include <wx/wx.h>
#include "FestDb.h"

class wxListView;

class FindMedicamentDialog : public wxDialog {
private:
    FestDb festDb{};
    wxTextCtrl *searchInput;
    wxListView *listView;
    wxButton *okButton;
public:
    FindMedicamentDialog(wxWindow *frame);
    bool CanOpen() const;
    void OnText(wxCommandEvent &e);
    void OnSelect(wxCommandEvent &e);
};


#endif //DRWHATSNOT_FINDMEDICAMENTDIALOG_H
