//
// Created by sigsegv on 12/20/23.
//

#ifndef DRWHATSNOT_HELSEIDLOGINDIALOG_H
#define DRWHATSNOT_HELSEIDLOGINDIALOG_H

#include <string>
#include <wx/wx.h>

class HelseidLoginDialog : public wxDialog {
public:
    explicit HelseidLoginDialog(wxWindow *parent, const std::string &url, const std::string &clientId);
};


#endif //DRWHATSNOT_HELSEIDLOGINDIALOG_H
