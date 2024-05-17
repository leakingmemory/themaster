//
// Created by sigsegv on 4/30/24.
//

#ifndef DRWHATSNOT_FESTVERSIONSDIALOG_H
#define DRWHATSNOT_FESTVERSIONSDIALOG_H

#include <memory>
#include <vector>
#include <string>
#include <wx/wx.h>

class FestDb;
class wxListView;

class FestVersionsDialog : public wxDialog {
private:
    std::shared_ptr<FestDb> db{};
    std::vector<std::string> versions{};
    wxListView *listView;
public:
    FestVersionsDialog(wxWindow *parent);
    void OnVersionContextMenu(wxContextMenuEvent &);
    void OnVersionExplore(wxCommandEvent &);
};


#endif //DRWHATSNOT_FESTVERSIONSDIALOG_H
