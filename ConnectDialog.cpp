//
// Created by sigsegv on 12/13/23.
//

#include "ConnectDialog.h"
#include "TheMasterFrame.h"
#include "HelseidLoginDialog.h"
#include <cpprest/uri.h>
#include <jjwtid/OidcTokenRequest.h>
#include "http_client.h"
#include <nlohmann/json.hpp>
#include <jjwtid/DpopHost.h>
#include "DataDirectory.h"
#include "Guard.h"
#include "win32/w32strings.h"

class ConnectionConfig {
public:
    std::string name{};
    std::string url{};
    std::string helseidUrl{};
    std::string helseidClientId{};
    std::string helseidSecretJwk{};
    std::string journalId{};
    std::string orgNo;
    std::string childOrgNo{};

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
    journalId = obj.value("journalid", "");
    orgNo = obj.value("orgno", "");
    childOrgNo = obj.value("childorgno", "");
}

std::string ConnectionConfig::ToJson() const {
    auto obj = nlohmann::json::object();
    obj["name"] = name;
    obj["url"] = url;
    obj["helseidurl"] = helseidUrl;
    obj["helseidclientid"] = helseidClientId;
    obj["helseidsecretjwt"] = helseidSecretJwk;
    obj["journalid"] = journalId;
    obj["orgno"] = orgNo;
    obj["childorgno"] = childOrgNo;
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

    wxBoxSizer *journalIdSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *journalIdLabel = new wxStaticText(this, wxID_ANY, "Journal ID: ");
    journalIdCtrl = new wxTextCtrl(this, wxID_ANY);
    journalIdSizer->Add(journalIdLabel, 0, wxALL, 5);
    journalIdSizer->Add(journalIdCtrl, 0, wxALL, 5);

    wxBoxSizer *orgNoSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *orgNoLabel = new wxStaticText(this, wxID_ANY, "Org no: ");
    orgNoCtrl = new wxTextCtrl(this, wxID_ANY);
    orgNoSizer->Add(orgNoLabel, 0, wxALL, 5);
    orgNoSizer->Add(orgNoCtrl, 0, wxALL, 5);

    wxBoxSizer *childOrgNoSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *childOrgNoLabel = new wxStaticText(this, wxID_ANY, "Child org no: ");
    childOrgNoCtrl = new wxTextCtrl(this, wxID_ANY);
    childOrgNoSizer->Add(childOrgNoLabel, 0, wxALL, 5);
    childOrgNoSizer->Add(childOrgNoCtrl, 0, wxALL, 5);

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
    mainSizer->Add(journalIdSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(orgNoSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(childOrgNoSizer, 1, wxEXPAND | wxALL, 5);
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
            journalIdCtrl->SetValue(config->journalId);
            orgNoCtrl->SetValue(config->orgNo);
            childOrgNoCtrl->SetValue(config->childOrgNo);
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
    wxString journalId = journalIdCtrl->GetValue();
    wxString orgNo = orgNoCtrl->GetValue();
    wxString childOrgNo = childOrgNoCtrl->GetValue();
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
                            config.journalId = journalId.ToStdString();
                            config.orgNo = orgNo.ToStdString();
                            config.childOrgNo = childOrgNo.ToStdString();
                        }
                    }
                    if (!found) {
                        ConnectionConfig config{};
                        config.name = stdConfigName;
                        config.url = url.ToStdString();
                        config.helseidUrl = helseidUrl.ToStdString();
                        config.helseidClientId = helseidClientId.ToStdString();
                        config.helseidSecretJwk = helseidSecretJwk.ToStdString();
                        config.journalId = journalId.ToStdString();
                        config.orgNo = orgNo.ToStdString();
                        config.childOrgNo = childOrgNo.ToStdString();
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
        HelseidLoginDialog helseidLoginDialog{this, helseidUrl.ToStdString(), helseidClientId.ToStdString(), helseidSecretJwk.ToStdString()};
        helseidLoginDialog.ShowModal();
        std::string uri{helseidLoginDialog.GetResultUrl()};
        auto querySeparatorPos = uri.find('?');
        if (querySeparatorPos < uri.size()) {
            auto query = uri.substr(querySeparatorPos + 1);
            std::map<std::string,std::string> queryMap{};
            {
                auto queryMapRaw = web::uri::split_query(as_wstring_on_win32(query));
                for (auto pair: queryMapRaw) {
                    queryMap.insert_or_assign(from_wstring_on_win32(web::uri::decode(pair.first)), from_wstring_on_win32(web::uri::decode(pair.second)));
                }
            }
            for (auto pair: queryMap) {
                std::cout << pair.first << ": " << pair.second << "\n";
            }
            auto findCode = queryMap.find("code");
            if (findCode != queryMap.end()) {
                try {
                    auto authorizationCode = findCode->second;
                    DpopHost dpopHost{};
                    OidcTokenRequest tokenRequest{helseidUrl.ToStdString(), helseidClientId.ToStdString(),
                                                     helseidSecretJwk.ToStdString(),
                                                     helseidLoginDialog.GetRedirectUri(), authorizationCode,
                                                     helseidLoginDialog.GetScopes(),
                                                     helseidLoginDialog.GetVerification()};
                    auto firstTokenRequest = tokenRequest.GetTokenRequest();
                    auto reqUrl = helseidUrl.ToStdString();
                    web::http::client::http_client client{as_wstring_on_win32(reqUrl)};
                    web::http::http_request req{web::http::methods::POST};
                    reqUrl.append("/connect/token");
                    auto dpop = dpopHost.Generate("POST", reqUrl);
                    std::cout << "DPoP: " << dpop << "\n";
                    req.headers().add(as_wstring_on_win32("DPoP"), as_wstring_on_win32(dpop));
                    req.set_request_uri(as_wstring_on_win32("/connect/token"));
                    std::string rqBody{};
                    {
                        std::stringstream sstr{};
                        auto iterator = firstTokenRequest.params.begin();
                        if (iterator != firstTokenRequest.params.end()) {
                            const auto &param = *iterator;
                            sstr << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(param.first))) << "=";
                            sstr << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(param.second)));
                            ++iterator;
                        }
                        while (iterator != firstTokenRequest.params.end()) {
                            const auto &param = *iterator;
                            sstr << "&" << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(param.first))) << "=";
                            sstr << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(param.second)));
                            ++iterator;
                        }
                        rqBody = sstr.str();
                    }
                    std::cout << rqBody << "\n";
                    req.set_body(rqBody, "application/x-www-form-urlencoded; charset=utf-8");
                    auto respTask = client.request(req);
                    auto frameWeakRefDispatcher = frame->GetWeakRefDispatcher();
                    auto helseidScopes = helseidLoginDialog.GetScopes();
                    respTask.then([frameWeakRefDispatcher, helseidUrl, helseidClientId, helseidSecretJwk, helseidScopes, journalId, orgNo, childOrgNo, dpopHost, rqBody](const pplx::task<web::http::http_response> &task) mutable {
                        try {
                            auto response = task.get();
                            auto tokenRecvFunc = [frameWeakRefDispatcher, helseidUrl, helseidClientId, helseidSecretJwk, helseidScopes, journalId, orgNo, childOrgNo, dpopHost](const pplx::task<web::json::value> &jsonTask, const std::string &dpopNonce) mutable {
                                try {
                                    auto json = jsonTask.get();
                                    if (json.has_string_field(as_wstring_on_win32("refresh_token")) && json.has_number_field(as_wstring_on_win32("rt_expires_in"))) {
                                        std::string refresh_token = from_wstring_on_win32(json.at(as_wstring_on_win32("refresh_token")).as_string());
                                        long rt_expires = json.at(as_wstring_on_win32("rt_expires_in")).as_number().to_int64();
                                        std::string id_token = from_wstring_on_win32(json.at(as_wstring_on_win32("id_token")).as_string());
                                        std::cout << "Refresh token: " << refresh_token << "\n";
                                        std::cout << "Expires: " << rt_expires << "\n";
                                        wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([frameWeakRefDispatcher, helseidUrl, helseidClientId, helseidSecretJwk, helseidScopes, refresh_token, rt_expires, id_token, journalId, orgNo, childOrgNo, dpopHost, dpopNonce]() {
                                            frameWeakRefDispatcher.Invoke([helseidUrl, helseidClientId, helseidSecretJwk, helseidScopes, refresh_token, rt_expires, id_token, journalId, orgNo, childOrgNo, dpopHost, dpopNonce] (TheMasterFrame *frame) {
                                                frame->SetHelseid(helseidUrl.ToStdString(), helseidClientId.ToStdString(), helseidSecretJwk.ToStdString(), helseidScopes, refresh_token, rt_expires, id_token, journalId.ToStdString(), orgNo.ToStdString(), childOrgNo.ToStdString(), dpopHost, dpopNonce);
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
                            };
                            if ((response.status_code() / 100) == 2) {
                                response.extract_json().then([tokenRecvFunc] (const pplx::task<web::json::value> &json) mutable { tokenRecvFunc(json, ""); });
                            } else {
                                std::string dpopNonce{};
                                {
                                    auto headers = response.headers();
                                    for (const auto &header: headers) {
                                        auto name = header.first;
                                        std::transform(name.cbegin(), name.cend(), name.begin(),
                                                       [](char ch) { return std::tolower(ch); });
                                        if (name == "dpop-nonce") {
                                            dpopNonce = header.second;
                                        }
                                    }
                                }
                                if (!dpopNonce.empty()) {
                                    std::cout << "DPoP nonce: " << dpopNonce << "\n";
                                    auto reqUrl = helseidUrl.ToStdString();
                                    web::http::client::http_client client{as_wstring_on_win32(reqUrl)};
                                    web::http::http_request req{web::http::methods::POST};
                                    reqUrl.append("/connect/token");
                                    auto dpop = dpopHost.Generate("POST", reqUrl, "", dpopNonce);
                                    std::cout << "DPoP: " << dpop << "\n";
                                    req.headers().add(as_wstring_on_win32("DPoP"), as_wstring_on_win32(dpop));
                                    req.set_request_uri(as_wstring_on_win32("/connect/token"));
                                    req.set_body(rqBody, "application/x-www-form-urlencoded; charset=utf-8");
                                    auto respTask = client.request(req);
                                    respTask.then([frameWeakRefDispatcher, helseidUrl, helseidClientId, helseidSecretJwk, helseidScopes, journalId, orgNo, childOrgNo, dpopHost, dpopNonce, tokenRecvFunc](const pplx::task<web::http::http_response> &task) {
                                        try {
                                            auto response = task.get();
                                            if ((response.status_code() / 100) == 2) {
                                                response.extract_json().then([tokenRecvFunc, dpopNonce] (const pplx::task<web::json::value> &json) mutable { tokenRecvFunc(json, dpopNonce); });
                                            } else {
                                                std::cerr << "HTTP error " << response.status_code() << "\n";
                                                typedef decltype(response.extract_string()) task_string_type;
                                                response.extract_string().then([](const task_string_type &t) {
                                                    try {
                                                        auto str = from_wstring_on_win32(t.get());
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
                                } else {
                                    std::cerr << "HTTP error " << response.status_code() << "\n";
                                    typedef decltype(response.extract_string()) task_string_type;
                                    response.extract_string().then([](const task_string_type &t) {
                                        try {
                                            auto str = from_wstring_on_win32(t.get());
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