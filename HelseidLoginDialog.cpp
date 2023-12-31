//
// Created by sigsegv on 12/20/23.
//

#include "HelseidLoginDialog.h"
#include "HelseidAuthorization.h"
#include <wx/webview.h>
#include <wx/filesys.h>

class HelseidAppHandler : public wxWebViewHandler {
public:
    HelseidAppHandler();
    wxFSFile * GetFile(const wxString &uri) override;
};

HelseidAppHandler::HelseidAppHandler() : wxWebViewHandler("app"){}

wxFSFile *HelseidAppHandler::GetFile(const wxString &uri) {
    return new wxFSFile(new wxStringInputStream(wxT("Please wait...")), uri, wxT("text/plain"), wxT(""), wxDateTime::Now());
}

HelseidLoginDialog::HelseidLoginDialog(wxWindow *parent, const std::string &url, const std::string &clientId) : wxDialog(parent, wxID_ANY, wxT("HelseID")) {
    auto *webView = wxWebView::New();
    webView->Create(this, wxID_ANY, wxWebViewDefaultURLStr, wxDefaultPosition, wxSize(800,600));
    webView->RegisterHandler(wxSharedPtr<wxWebViewHandler>(new HelseidAppHandler(), [] (auto *handler) { delete handler; }));
    webView->Bind(wxEVT_WEBVIEW_NAVIGATING, &HelseidLoginDialog::OnNavigating, this);

    auto *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(webView, 1, wxEXPAND | wxALL);
    SetSizerAndFit(sizer);
    HelseidAuthorization authorization{url, clientId};
    scopes = authorization.GetScopes();
    redirectUri = authorization.GetRedirectUri();
    verification = authorization.GetVerfication();
    state = authorization.GetState();
    webView->LoadURL(authorization.GetAuthorizeUrl());
}

void HelseidLoginDialog::OnNavigating(wxWebViewEvent &e) {
    std::string url{e.GetURL().ToStdString()};
    if (url.starts_with("app://")) {
        resultUrl = url;
        wxDialog::EndModal(0);
    }
}
