//
// Created by sigsegv on 12/18/23.
//

#ifndef DRWHATSNOT_WAITINGFORAPIDIALOG_H
#define DRWHATSNOT_WAITINGFORAPIDIALOG_H

#include <wx/wx.h>

class WaitingForApiDialog : public wxDialog {
private:
    wxStaticText* text;
    bool closed{false};
public:
    explicit WaitingForApiDialog(wxWindow *parent, const std::string &title, const std::string &msg);
    void SetMessage(const std::string &msg);
    void Close();
    void OnClose(wxCloseEvent &);
};


#endif //DRWHATSNOT_WAITINGFORAPIDIALOG_H
