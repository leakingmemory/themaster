//
// Created by sigsegv on 12/13/23.
//

#ifndef DRWHATSNOT_CONNECTDIALOG_H
#define DRWHATSNOT_CONNECTDIALOG_H

#include <wx/wx.h>

class TheMasterFrame;

class ConnectDialog : public wxDialog {
private:
    TheMasterFrame *frame;
    wxTextCtrl *urlTextCtrl;
    wxTextCtrl *helseidUrlCtrl;
    wxTextCtrl *helseidClientIdCtrl;
public:
    ConnectDialog(TheMasterFrame *);
    void OnConnect(wxCommandEvent &);
    void OnCancel(wxCommandEvent &);
};


#endif //DRWHATSNOT_CONNECTDIALOG_H
