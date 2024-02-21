//
// Created by sigsegv on 12/13/23.
//

#include "ConnectDialog.h"
#include "TheMasterFrame.h"
#include "HelseidLoginDialog.h"
#include <cpprest/uri.h>
#include <jjwtid/OidcTokenRequest.h>
#include <cpprest/http_client.h>
#include <nlohmann/json.hpp>
#include "DataDirectory.h"
#include "Guard.h"

class ConnectionConfig {
public:
    std::string name{};
    std::string url{};
    std::string helseidUrl{};
    std::string helseidClientId{};
    std::string helseidSecretJwk{};

    void FromJson(const std::string &json);
    std::string ToJson() const;
};

void ConnectionConfig::FromJson(const std::string &json) {
    nlohmann::json obj = nlohmann::json::parse(json);
    name = obj.value("name", "");
    url = obj.value("url", "");
    helseidUrl = obj.value("helseidurl", "");
    helseidClientId = obj.value("helseidclientid", "");
    helseidSecretJwk = obj.value("helseidsecretjwt", "");
}

std::string ConnectionConfig::ToJson() const {
    auto obj = nlohmann::json::object();
    obj["name"] = name;
    obj["url"] = url;
    obj["helseidurl"] = helseidUrl;
    obj["helseidclientid"] = helseidClientId;
    obj["helseidsecretjwt"] = helseidSecretJwk;
    return obj.dump();
}

static std::vector<ConnectionConfig> ReadConfigs() {
    std::vector<ConnectionConfig> configs{};
    {
        DataDirectory configDir = DataDirectory::Config("themaster");
        auto json = configDir.ReadFile("clients.json");
        if (!json.empty()) {
            auto arr = nlohmann::json::parse(json);
            if (arr.is_array()) {
                for (const auto &val: arr) {
                    auto raw = val.dump();
                    ConnectionConfig config{};
                    config.FromJson(raw);
                    if (!config.name.empty() && (!config.url.empty() || !config.helseidUrl.empty())) {
                        configs.push_back(config);
                    }
                }
            }
        }
    }
    return configs;
}

ConnectDialog::ConnectDialog(TheMasterFrame *parent) : wxDialog(parent, wxID_ANY, "Connect"), frame(parent) {
    try {
        for (const auto &config : ReadConfigs()) {
            configs.push_back(std::make_shared<ConnectionConfig>(config));
        }
    } catch (std::exception &e) {
        wxMessageBox(e.what(), wxT("Failed to read client configs"), wxOK | wxICON_ERROR);
    } catch (...) {
        wxMessageBox(wxT("Unknown type of error"), wxT("Failed to read client configs"), wxOK | wxICON_ERROR);
    }

    wxBoxSizer *configNameSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *configNameLabel = new wxStaticText(this, wxID_ANY, "Config: ");
    configNameCtrl = new wxComboBox(this, wxID_ANY);
    configNameSizer->Add(configNameLabel, 0, wxALL, 5);
    configNameSizer->Add(configNameCtrl, 1, wxEXPAND | wxALL, 5);
    for (const auto &config : configs) {
        configNameCtrl->Append(config->name);
    }

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
    mainSizer->Add(configNameSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(urlSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(helseidUrlSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(helseidClientIdSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(helseidSecretJwkSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);
    this->SetSizerAndFit(mainSizer);

    configNameCtrl->Bind(wxEVT_COMBOBOX, &ConnectDialog::OnSelect, this);
    connectButton->Bind(wxEVT_BUTTON, &ConnectDialog::OnConnect, this);
    cancelButton->Bind(wxEVT_BUTTON, &ConnectDialog::OnCancel, this);
}

void ConnectDialog::OnSelect(wxCommandEvent &) {
    auto selectedWx = configNameCtrl->GetValue();
    auto selected = selectedWx.ToStdString();
    for (const auto &config : configs) {
        if (config->name == selected) {
            urlTextCtrl->SetValue(config->url);
            helseidUrlCtrl->SetValue(config->helseidUrl);
            helseidClientIdCtrl->SetValue(config->helseidClientId);
            helseidSecretJwkCtrl->SetValue(config->helseidSecretJwk);
            return;
        }
    }
}

void ConnectDialog::OnConnect(wxCommandEvent &) {
    wxString configName = configNameCtrl->GetValue();
    wxString url = urlTextCtrl->GetValue();
    wxString helseidUrl = helseidUrlCtrl->GetValue();
    wxString helseidClientId = helseidClientIdCtrl->GetValue();
    wxString helseidSecretJwk = helseidSecretJwkCtrl->GetValue();
    if (!configName.empty() && (!url.empty() || !helseidUrl.empty())) {
        try {
            std::string configsJson{};
            {
                auto configs = ReadConfigs();
                {
                    std::string stdConfigName = configName.ToStdString();
                    bool found{false};
                    for (auto &config: configs) {
                        if (config.name == stdConfigName) {
                            found = true;
                            config.url = url.ToStdString();
                            config.helseidUrl = helseidUrl.ToStdString();
                            config.helseidClientId = helseidClientId.ToStdString();
                            config.helseidSecretJwk = helseidSecretJwk.ToStdString();
                        }
                    }
                    if (!found) {
                        ConnectionConfig config{};
                        config.name = stdConfigName;
                        config.url = url.ToStdString();
                        config.helseidUrl = helseidUrl.ToStdString();
                        config.helseidClientId = helseidClientId.ToStdString();
                        config.helseidSecretJwk = helseidSecretJwk.ToStdString();
                        configs.push_back(config);
                    }
                }
                auto i = 0;
                auto arr = nlohmann::json::array();
                for (const auto &config: configs) {
                    auto raw = config.ToJson();
                    arr[i++] = nlohmann::json::parse(raw);
                }
                configsJson = arr.dump();
            }
            DataDirectory configDir = DataDirectory::Config("themaster");
            auto prevMask = umask(00177);
            Guard resetMask{[prevMask]() { umask(prevMask); }};
            configDir.WriteFile("clients.json", configsJson);
        } catch (std::exception &e) {
            wxMessageBox(e.what(), wxT("Failed to update client configs"), wxOK | wxICON_ERROR);
        } catch (...) {
            wxMessageBox(wxT("Unknown type of error"), wxT("Failed to update client configs"), wxOK | wxICON_ERROR);
        }
    }
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
                    OidcTokenRequest tokenRequest{helseidUrl.ToStdString(), helseidClientId.ToStdString(),
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
                    auto frameWeakRefDispatcher = frame->GetWeakRefDispatcher();
                    auto helseidScopes = helseidLoginDialog.GetScopes();
                    respTask.then([frameWeakRefDispatcher, helseidUrl, helseidClientId, helseidSecretJwk, helseidScopes](const pplx::task<web::http::http_response> &task) {
                        try {
                            auto response = task.get();
                            if ((response.status_code() / 100) == 2) {
                                response.extract_json().then([frameWeakRefDispatcher, helseidUrl, helseidClientId, helseidSecretJwk, helseidScopes](const pplx::task<web::json::value> &jsonTask) {
                                    try {
                                        auto json = jsonTask.get();
                                        if (json.has_string_field("refresh_token") && json.has_number_field("rt_expires_in")) {
                                            std::string refresh_token = json.at("refresh_token").as_string();
                                            long rt_expires = json.at("rt_expires_in").as_number().to_int64();
                                            std::string id_token = json.at("id_token").as_string();
                                            std::cout << "Refresh token: " << refresh_token << "\n";
                                            std::cout << "Expires: " << rt_expires << "\n";
                                            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([frameWeakRefDispatcher, helseidUrl, helseidClientId, helseidSecretJwk, helseidScopes, refresh_token, rt_expires, id_token]() {
                                                frameWeakRefDispatcher.Invoke([helseidUrl, helseidClientId, helseidSecretJwk, helseidScopes, refresh_token, rt_expires, id_token] (TheMasterFrame *frame) {
                                                    frame->SetHelseid(helseidUrl.ToStdString(), helseidClientId.ToStdString(), helseidSecretJwk.ToStdString(), helseidScopes, refresh_token, rt_expires, id_token);
                                                });
                                            });
                                        } else {
                                            std::cout << json.serialize() << "\n";
                                            std::cerr << "Missing refresh_token and rt_expires\n";
                                            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([]() {
                                                wxMessageBox(
                                                        wxT("HelseID did not issue a refresh token"),
                                                        wxT("HelseID failed"), wxOK | wxICON_ERROR);
                                            });
                                        }
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