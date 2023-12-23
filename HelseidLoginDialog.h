//
// Created by sigsegv on 12/20/23.
//

#ifndef DRWHATSNOT_HELSEIDLOGINDIALOG_H
#define DRWHATSNOT_HELSEIDLOGINDIALOG_H

#include <string>
#include <wx/wx.h>

class wxWebViewEvent;

class HelseidLoginDialog : public wxDialog {
private:
    std::string resultUrl;
public:
    explicit HelseidLoginDialog(wxWindow *parent, const std::string &url, const std::string &clientId);
    void OnNavigating(wxWebViewEvent &);
    std::string GetResultUrl() const {
        return resultUrl;
    }
};


#endif //DRWHATSNOT_HELSEIDLOGINDIALOG_H
