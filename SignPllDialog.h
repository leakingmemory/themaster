//
// Created by sigsegv on 6/11/24.
//

#ifndef THEMASTER_SIGNPLLDIALOG_H
#define THEMASTER_SIGNPLLDIALOG_H

#include <wx/wx.h>
#include <map>

class SignPllDialog : public wxDialog {
private:
    std::vector<std::string> candidates{};
    std::vector<std::string> selected{};
    wxCheckListBox *listbox;
public:
    SignPllDialog(wxWindow *parent, const std::map<std::string,std::string> &map);
    void OnCancel(wxCommandEvent &e);
    void OnOk(wxCommandEvent &e);
    [[nodiscard]] std::vector<std::string> GetSelected() const {
        return selected;
    }
};


#endif //THEMASTER_SIGNPLLDIALOG_H
