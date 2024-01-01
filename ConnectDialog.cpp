//
// Created by sigsegv on 12/13/23.
//

#include "ConnectDialog.h"
#include "TheMasterFrame.h"
#include "HelseidLoginDialog.h"
#include <cpprest/uri.h>
#include "HelseidTokenRequest.h"
#include <cpprest/http_client.h>

ConnectDialog::ConnectDialog(TheMasterFrame *parent) : wxDialog(parent, wxID_ANY, "Connect"), frame(parent) {

    // Create sizer for Url label and text field
    wxBoxSizer *urlSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *urlLabel = new wxStaticText(this, wxID_ANY, "Url: ");
    urlTextCtrl = new wxTextCtrl(this, wxID_ANY);
    urlSizer->Add(urlLabel, 0, wxALL, 5);
    urlSizer->Add(urlTextCtrl, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer *helseidUrlSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *helseidUrlLabel = new wxStaticText(this, wxID_ANY, "HelseID Url: ");
    helseidUrlCtrl = new wxTextCtrl(this, wxID_ANY);
    helseidUrlSizer->Add(helseidUrlLabel, 0, wxALL, 5);
    helseidUrlSizer->Add(helseidUrlCtrl, 0, wxALL, 5);

    wxBoxSizer *helseidClientIdSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *helseidClientIdLabel = new wxStaticText(this, wxID_ANY, "HelseID Client ID: ");
    helseidClientIdCtrl = new wxTextCtrl(this, wxID_ANY);
    helseidClientIdSizer->Add(helseidClientIdLabel, 0, wxALL, 5);
    helseidClientIdSizer->Add(helseidClientIdCtrl, 0, wxALL, 5);

    wxBoxSizer *helseidSecretJwkSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *helseidSecretJwkLabel = new wxStaticText(this, wxID_ANY, "HelseID Secret: ");
    helseidSecretJwkCtrl = new wxTextCtrl(this, wxID_ANY);
    helseidSecretJwkSizer->Add(helseidSecretJwkLabel, 0, wxALL, 5);
    helseidSecretJwkSizer->Add(helseidSecretJwkCtrl, 0, wxALL, 5);

    // Create sizer for buttons
    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton *connectButton = new wxButton(this, wxID_ANY, "Connect");
    wxButton *cancelButton = new wxButton(this, wxID_ANY, "Cancel");
    buttonSizer->Add(connectButton, 1, wxALL, 5);
    buttonSizer->Add(cancelButton, 1, wxALL, 5);

    // Create main sizer and add to dialog
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(urlSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(helseidUrlSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(helseidClientIdSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(helseidSecretJwkSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);
    this->SetSizerAndFit(mainSizer);

    connectButton->Bind(wxEVT_BUTTON, &ConnectDialog::OnConnect, this);
    cancelButton->Bind(wxEVT_BUTTON, &ConnectDialog::OnCancel, this);
}

void ConnectDialog::OnConnect(wxCommandEvent &) {
    wxString url = urlTextCtrl->GetValue();
    wxString helseidUrl = helseidUrlCtrl->GetValue();
    wxString helseidClientId = helseidClientIdCtrl->GetValue();
    wxString helseidSecretJwk = helseidSecretJwkCtrl->GetValue();
    if (!helseidUrl.empty()) {
        HelseidLoginDialog helseidLoginDialog{this, helseidUrl.ToStdString(), helseidClientId.ToStdString()};
        helseidLoginDialog.ShowModal();
        std::string uri{helseidLoginDialog.GetResultUrl()};
        auto querySeparatorPos = uri.find('?');
        if (querySeparatorPos < uri.size()) {
            auto query = uri.substr(querySeparatorPos + 1);
            std::map<std::string,std::string> queryMap{};
            {
                auto queryMapRaw = web::uri::split_query(query);
                for (auto pair: queryMapRaw) {
                    queryMap.insert_or_assign(web::uri::decode(pair.first), web::uri::decode(pair.second));
                }
            }
            for (auto pair: queryMap) {
                std::cout << pair.first << ": " << pair.second << "\n";
            }
            auto findCode = queryMap.find("code");
            if (findCode != queryMap.end()) {
                try {
                    auto authorizationCode = findCode->second;
                    HelseidTokenRequest tokenRequest{helseidUrl.ToStdString(), helseidClientId.ToStdString(),
                                                     helseidSecretJwk.ToStdString(),
                                                     helseidLoginDialog.GetRedirectUri(), authorizationCode,
                                                     helseidLoginDialog.GetScopes(),
                                                     helseidLoginDialog.GetVerification()};
                    auto firstTokenRequest = tokenRequest.GetTokenRequest();
                    web::http::client::http_client client{helseidUrl.ToStdString()};
                    web::http::http_request req{web::http::methods::POST};
                    req.set_request_uri("/connect/token");
                    {
                        std::string rqBody{};
                        {
                            std::stringstream sstr{};
                            auto iterator = firstTokenRequest.params.begin();
                            if (iterator != firstTokenRequest.params.end()) {
                                const auto &param = *iterator;
                                sstr << web::uri::encode_data_string(param.first) << "=";
                                sstr << web::uri::encode_data_string(param.second);
                                ++iterator;
                            }
                            while (iterator != firstTokenRequest.params.end()) {
                                const auto &param = *iterator;
                                sstr << "&" << web::uri::encode_data_string(param.first) << "=";
                                sstr << web::uri::encode_data_string(param.second);
                                ++iterator;
                            }
                            rqBody = sstr.str();
                        }
                        std::cout << rqBody << "\n";
                        req.set_body(rqBody, "application/x-www-form-urlencoded; charset=utf-8");
                    }
                    auto respTask = client.request(req);
                    respTask.then([](const pplx::task<web::http::http_response> &task) {
                        try {
                            auto response = task.get();
                            if ((response.status_code() / 100) == 2) {
                                response.extract_json().then([](const pplx::task<web::json::value> &jsonTask) {
                                    try {
                                        auto json = jsonTask.get();
                                        std::cout << json.to_string() << "\n";
                                    } catch (...) {
                                        wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([]() {
                                            wxMessageBox(
                                                    wxT("HelseID token request failed with json error, or no json response"),
                                                    wxT("HelseID failed"), wxOK | wxICON_ERROR);
                                        });
                                    }
                                });
                            } else {
                                std::cerr << "HTTP error " << response.status_code() << "\n";
                                typedef typeof(response.extract_string()) task_string_type;
                                response.extract_string().then([] (const task_string_type &t) {
                                    try {
                                        auto str = t.get();
                                        std::cerr << str << "\n";
                                    } catch (...) {
                                        std::cerr << "Unable to retrieve error string\n";
                                    }
                                });
                                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([]() {
                                    wxMessageBox(wxT("HelseID token request failed with error code"),
                                                 wxT("HelseID failed"), wxOK | wxICON_ERROR);
                                });
                            }
                        } catch (std::exception &e) {
                            std::string error{e.what()};
                            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([error]() {
                                wxMessageBox(error, wxT("HelseID request error exception"),
                                             wxOK | wxICON_ERROR);
                            });
                        } catch (...) {
                            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([]() {
                                wxMessageBox(wxT("HelseID token request failed"), wxT("HelseID failed"),
                                             wxOK | wxICON_ERROR);
                            });
                        }
                    });
                } catch (std::exception &e) {
                    wxMessageBox(e.what(), wxT("HelseID token request exception"), wxOK | wxICON_ERROR);
                } catch (...) {
                    wxMessageBox(wxT("Failed to generate a token request"), wxT("HelseID failed"), wxOK | wxICON_ERROR);
                }
            }
        } else {
            wxMessageBox(wxT("HelseID failed"), wxT("HelseID failed"), wxOK | wxICON_ERROR);
        }
    }
    frame->Connect(std::string(url.ToUTF8()));
    Close();
}

void ConnectDialog::OnCancel(wxCommandEvent &) {
    Close();
}