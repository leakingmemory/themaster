//
// Created by sigsegv on 12/20/23.
//

#ifndef DRWHATSNOT_HELSEIDLOGINDIALOG_H
#define DRWHATSNOT_HELSEIDLOGINDIALOG_H

#include <string>
#include <vector>
#include <wx/wx.h>

class wxWebViewEvent;

class HelseidLoginDialog : public wxDialog {
private:
    std::string resultUrl;
    std::vector<std::string> scopes{};
    std::string redirectUri;
    std::string verification;
    std::string state;
public:
    explicit HelseidLoginDialog(wxWindow *parent, const std::string &url, const std::string &clientId, const std::string &secretJwk);
    void OnNavigating(wxWebViewEvent &);
    [[nodiscard]] std::string GetResultUrl() const {
        return resultUrl;
    }
    [[nodiscard]] std::vector<std::string> GetScopes() const {
        return scopes;
    }
    [[nodiscard]] std::string GetRedirectUri() const {
        return redirectUri;
    }
    [[nodiscard]] std::string GetVerification() const {
        return verification;
    }
    [[nodiscard]] std::string GetState() const {
        return state;
    }
};


#endif //DRWHATSNOT_HELSEIDLOGINDIALOG_H
