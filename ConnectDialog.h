//
// Created by sigsegv on 12/13/23.
//

#ifndef DRWHATSNOT_CONNECTDIALOG_H
#define DRWHATSNOT_CONNECTDIALOG_H

#include <wx/wx.h>
#include <vector>
#include <memory>

class TheMasterFrame;
class ConnectionConfig;

class ConnectDialog : public wxDialog {
private:
    TheMasterFrame *frame;
    std::vector<std::shared_ptr<ConnectionConfig>> configs{};
    wxComboBox *configNameCtrl;
    wxTextCtrl *urlTextCtrl;
    wxTextCtrl *helseidUrlCtrl;
    wxTextCtrl *helseidClientIdCtrl;
    wxTextCtrl *helseidSecretJwkCtrl;
    wxTextCtrl *journalIdCtrl;
    wxTextCtrl *orgNoCtrl;
    wxTextCtrl *childOrgNoCtrl;
public:
    ConnectDialog(TheMasterFrame *);
    void OnSelect(wxCommandEvent &);
    void OnConnect(wxCommandEvent &);
    void OnCancel(wxCommandEvent &);
};


#endif //DRWHATSNOT_CONNECTDIALOG_H
