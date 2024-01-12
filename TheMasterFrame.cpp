//
// Created by sigsegv on 12/13/23.
//

#include "TheMasterFrame.h"
#include "ConnectDialog.h"
#include "FindPatientDialog.h"
#include "CreatePatientDialog.h"
#include "PatientStoreInMemoryWithPersistence.h"
#include "WaitingForApiDialog.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <sfmbasisapi/fhir/bundleentry.h>
#include <sfmbasisapi/fhir/person.h>
#include <sfmbasisapi/fhir/parameters.h>
#include <sfmbasisapi/fhir/bundle.h>
#include <sfmbasisapi/fhir/value.h>
#include <sfmbasisapi/fhir/medstatement.h>
#include <jjwtid/Jwt.h>
#include <jjwtid/OidcTokenRequest.h>
#include <cpprest/http_client.h>
#include <wx/listctrl.h>
#include "MedBundleData.h"
#include <InstallPrefix.h>

TheMasterFrame::TheMasterFrame() : wxFrame(nullptr, wxID_ANY, "The Master"),
                                   weakRefDispatcher(std::make_shared<WeakRefUiDispatcher<TheMasterFrame>>(this)),
                                   patientStore(std::make_shared<PatientStoreInMemoryWithPersistence>())
{
    std::string iconPath{GetInstallPrefix()};
    if (!iconPath.ends_with("/")) {
        iconPath.append("/");
    }
    iconPath.append("share/themaster/TheMasterLogo.png");
    wxIcon icon{iconPath, wxBITMAP_TYPE_PNG};
    SetIcon(icon);
    auto *patientMenu = new wxMenu();
    patientMenu->Append(TheMaster_FindPatient_Id, "Find patient");
    patientMenu->Append(TheMaster_CreatePatient_Id, "Create patient");
    auto *serverMenu = new wxMenu();
    serverMenu->Append(TheMaster_Connect_Id, "Connect");
    serverMenu->Append(TheMaster_GetMedication_Id, "Get medication");
    serverMenu->Append(TheMaster_SendMedication_Id, "Send medication");
    auto *menuBar = new wxMenuBar();
    menuBar->Append(serverMenu, "Server");
    menuBar->Append(patientMenu, "Patient");
    SetMenuBar(menuBar);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    header = new wxListView(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 100));
    header->AppendColumn(wxT(""));
    header->AppendColumn(wxT(""));
    header->AppendColumn(wxT(""));
    header->AppendColumn(wxT(""));
    header->AppendColumn(wxT(""));
    header->AppendColumn(wxT(""));
    header->InsertItem(0, wxT("KJ Time:"));
    header->SetItem(0, 2, wxT("RF Time:"));
    header->SetItem(0, 4, wxT("KJ Code:"));
    header->InsertItem(1, wxT("M96 Code:"));
    header->SetItem(1, 2, wxT("M912 Code:"));
    header->SetItem(1, 4, wxT("KJ Meds:"));
    header->InsertItem(2, wxT("KJ Locked:"));
    header->SetItem(2, 2, wxT("RF Locked:"));
    sizer->Add(header, 0, wxEXPAND | wxALL, 5);

    prescriptions = new wxListView(this, wxID_ANY);
    prescriptions->AppendColumn(wxT("Name"));
    sizer->Add(prescriptions, 1, wxEXPAND | wxALL, 5);

    SetSizerAndFit(sizer);

    Bind(wxEVT_MENU, &TheMasterFrame::OnConnect, this, TheMaster_Connect_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnFindPatient, this, TheMaster_FindPatient_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnCreatePatient, this, TheMaster_CreatePatient_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnGetMedication, this, TheMaster_GetMedication_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSendMedication, this, TheMaster_SendMedication_Id);
}

void TheMasterFrame::UpdateHeader() {
    if (medicationBundle) {
        header->SetItem(0, 1, medicationBundle->kjHentet);
        header->SetItem(0, 3, medicationBundle->rfHentet);
        header->SetItem(0, 5, medicationBundle->kjFeilkode.GetDisplay());
        header->SetItem(1, 1, medicationBundle->rfM96Feilkode.GetDisplay());
        header->SetItem(1, 3, medicationBundle->rfM912Feilkode.GetDisplay());
        header->SetItem(1, 5, medicationBundle->kjHarLegemidler ? wxT("Yes") : wxT("No"));
        header->SetItem(2, 1, medicationBundle->kjHarLaste ? wxT("Yes") : wxT("No"));
        header->SetItem(2, 3, medicationBundle->rfHarLaste ? wxT("Yes") : wxT("No"));
    } else {
        header->SetItem(0, 1, wxT(""));
        header->SetItem(0, 3, wxT(""));
        header->SetItem(0, 5, wxT(""));
        header->SetItem(1, 1, wxT(""));
        header->SetItem(1, 3, wxT(""));
        header->SetItem(1, 5, wxT(""));
        header->SetItem(2, 1, wxT(""));
        header->SetItem(1, 3, wxT(""));
    }
}

void TheMasterFrame::UpdateMedications() {
    prescriptions->ClearAll();
    prescriptions->AppendColumn(wxT("Name"));
    auto pos = 0;
    for (const auto &bundleEntry : medicationBundle->medBundle->GetEntries()) {
        auto resource = bundleEntry.GetResource();
        auto resourceType = resource->GetResourceType();
        auto medicationStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(resource);
        if (medicationStatement) {
            prescriptions->InsertItem(pos++, medicationStatement->GetMedicationReference().GetDisplay());
        }
    }
}

void TheMasterFrame::OnConnect(wxCommandEvent( &e)) {
    ConnectDialog dialog{this};
    dialog.ShowModal();
}

void TheMasterFrame::OnFindPatient(wxCommandEvent &e) {
    FindPatientDialog dialog{patientStore, this};
    if (dialog.ShowModal() == wxID_OK) {
        patientInformation = dialog.GetPatient();
    }
}

void TheMasterFrame::OnCreatePatient(wxCommandEvent &e) {
    CreatePatientDialog dialog{patientStore, this};
    if (dialog.ShowModal() == wxID_OK) {
        auto patient = dialog.GetPatientInformation();
        patientStore->AddPatient(patient);
        patientInformation = std::make_shared<PatientInformation>(std::move(patient));
    }
}

pplx::task<std::string> TheMasterFrame::GetAccessToken() {
    if (accessToken && !accessToken->empty()) {
        std::string atWithoutSignature{};
        {
            Jwt unverifierJwt{*accessToken};
            auto unverifiedHeader = unverifierJwt.GetUnverifiedHeader();
            auto unverifiedBody = unverifierJwt.GetUnverifiedBody();
            if (!unverifiedHeader.empty() || !unverifiedBody.empty()) {
                atWithoutSignature = unverifiedHeader;
                atWithoutSignature.append(".");
                atWithoutSignature.append(unverifiedBody);
            } else {
                atWithoutSignature = *accessToken;
            }
        }
        Jwt jwt{atWithoutSignature};
        auto body = jwt.Body();
        if (body->contains("exp")) {
            auto exp = body->GetInt("exp");
            if ((exp - 60) >= std::time(nullptr)) {
                std::string at{*accessToken};
                return pplx::task<std::string>([at] () {return at;});
            }
        }
    }
    if (accessToken) {
        accessToken->clear();
    } else {
        accessToken = std::make_shared<std::string>();
    }
    if (!helseidRefreshToken.empty()) {
        OidcTokenRequest tokenRequest{helseidUrl, helseidClientId, helseidSecretJwk, helseidScopes, helseidRefreshToken};
        auto requestData = tokenRequest.GetTokenRequest();
        web::http::client::http_client client{helseidUrl};
        web::http::http_request req{web::http::methods::POST};
        req.set_request_uri("/connect/token");
        {
            std::string rqBody{};
            {
                std::stringstream sstr{};
                auto iterator = requestData.params.begin();
                if (iterator != requestData.params.end()) {
                    const auto &param = *iterator;
                    sstr << web::uri::encode_data_string(param.first) << "=";
                    sstr << web::uri::encode_data_string(param.second);
                    ++iterator;
                }
                while (iterator != requestData.params.end()) {
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
        std::shared_ptr<std::string> at = accessToken;
        return respTask.then([at] (const web::http::http_response &response) {
            if ((response.status_code() / 100) == 2) {
                return response.extract_json().then([at] (const web::json::value &json) {
                    if (json.has_string_field("access_token")) {
                        *at = json.at("access_token").as_string();
                        return *at;
                    } else {
                        std::cerr << "Missing access_token\n";
                        throw std::exception();
                    }
                });
            } else {
                throw std::exception();
            }
        });
    }
    return pplx::task<std::string>([] () {return std::string();});
}

void TheMasterFrame::OnGetMedication(wxCommandEvent &e) {
    if (!patientInformation) {
        wxMessageBox("Error: No patient information provided", "Error", wxICON_ERROR);
        return;
    }
    if (url.empty()) {
        wxMessageBox("Error: Not connected", "Error", wxICON_ERROR);
        return;
    }
    std::string apiUrl = url;

    auto patient = std::make_shared<FhirPatient>();
    {
        {
            boost::uuids::uuid uuid = boost::uuids::random_generator()();
            std::string uuid_string = to_string(uuid);
            patient->SetId(uuid_string);
        }
        patient->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Patient");
        {
            std::vector <FhirIdentifier> identifiers{};
            auto patientIdType = this->patientInformation->GetPatientIdType();
            if (patientIdType == PatientIdType::FODSELSNUMMER) {
                identifiers.emplace_back(
                        FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/FNR", "FNR-nummer", ""),
                        "official", "urn:oid:2.16.578.1.12.4.1.4.1",
                        this->patientInformation->GetPatientId());
                patient->SetIdentifiers(identifiers);
            } else if (patientIdType == PatientIdType::DNUMMER) {
                identifiers.emplace_back(
                        FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/DNR", "Dnummer", ""),
                        "official", "urn:oid:2.16.578.1.12.4.1.4.2",
                        this->patientInformation->GetPatientId());
                patient->SetIdentifiers(identifiers);
            }
        }
        patient->SetActive(true);
        patient->SetName({{"official", this->patientInformation->GetFamilyName(),
                           this->patientInformation->GetGivenName()}});
        {
            auto dob = this->patientInformation->GetDateOfBirth();
            if (!dob.empty()) {
                patient->SetBirthDate(dob);
            }
        }
        patient->SetGender(
                this->patientInformation->GetGender() == PersonGender::FEMALE ? "female" : "male");
        auto postCode = this->patientInformation->GetPostCode();
        auto city = this->patientInformation->GetCity();
        if (!postCode.empty() && !city.empty()) {
            FhirAddress address{{}, "home", "physical", city, postCode};
            patient->SetAddress({address});
        }
    }

    pplx::task<web::http::http_response> responseTask;
    auto accessTokenTask = GetAccessToken();
    responseTask = accessTokenTask.then([apiUrl, patient] (const std::string &accessToken) {
        web::http::http_request request{web::http::methods::POST};
        if (!accessToken.empty()) {
            std::cout << "Access token: " << accessToken << "\n";
            std::string bearer{"Bearer "};
            bearer.append(accessToken);
            request.headers().add("Authorization", bearer);
        }
        {
            web::json::value requestBody{};
            {
                FhirParameters requestParameters{};
                requestParameters.AddParameter("patient", patient);
                requestBody = requestParameters.ToJson();
            }
            request.set_request_uri("patient/$getMedication");
            request.set_body(requestBody);
        }
        web::http::client::http_client client{apiUrl};
        return client.request(request);
    });

    std::shared_ptr<std::mutex> getMedResponseMtx = std::make_shared<std::mutex>();
    std::shared_ptr<std::unique_ptr<FhirParameters>> getMedResponse = std::make_shared<std::unique_ptr<FhirParameters>>();
    std::shared_ptr<WaitingForApiDialog> waitingDialog = std::make_shared<WaitingForApiDialog>(this, "Retrieving medication records", "Requested medication bundle...");
    responseTask.then([waitingDialog, getMedResponse, getMedResponseMtx] (const pplx::task<web::http::http_response> &responseTask) {
        try {
            auto response = responseTask.get();
            try {
                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                    waitingDialog->SetMessage("Downloading data...");
                });
                auto contentType = response.headers().content_type();
                if (!contentType.starts_with("application/fhir+json")) {
                    wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog, contentType]() {
                        waitingDialog->Close();
                        std::string msg{"Wrong content type in response: "};
                        msg.append(contentType);
                        wxMessageBox(msg, "Error", wxICON_ERROR);
                    });
                } else {
                    response.extract_json(true).then([waitingDialog, getMedResponse, getMedResponseMtx](
                            const pplx::task<web::json::value> &responseBodyTask) {
                        try {
                            auto responseBody = responseBodyTask.get();
                            try {
                                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                                    waitingDialog->SetMessage("Decoding data...");
                                });
                                FhirParameters responseParameterBundle = FhirParameters::Parse(responseBody);
                                {
                                    std::lock_guard lock{*getMedResponseMtx};
                                    *getMedResponse = std::make_unique<FhirParameters>(
                                            std::move(responseParameterBundle));
                                }
                                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(
                                        [waitingDialog, getMedResponse]() {
                                            waitingDialog->Close();
                                        });
                            } catch (std::exception &e) {
                                std::string error = e.what();
                                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog, error]() {
                                    waitingDialog->Close();
                                    std::string msg{"Error: std::exception: "};
                                    msg.append(error);
                                    wxMessageBox(msg, "Error", wxICON_ERROR);
                                });
                            } catch (...) {
                                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                                    waitingDialog->Close();
                                    wxMessageBox("Error: Decoding failed", "Error", wxICON_ERROR);
                                });
                            }
                        } catch (std::exception &e) {
                            std::string error = e.what();
                            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog, error]() {
                                waitingDialog->Close();
                                std::string msg{"Error: std::exception: "};
                                msg.append(error);
                                wxMessageBox(msg, "Error", wxICON_ERROR);
                            });
                        } catch (...) {
                            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                                waitingDialog->Close();
                                wxMessageBox("Error: Downloading failed", "Error", wxICON_ERROR);
                            });
                        }
                    });
                }
            } catch (...) {
                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                    waitingDialog->Close();
                    wxMessageBox("Error: Downloading failed", "Error", wxICON_ERROR);
                });
            }
        } catch (...) {
            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                waitingDialog->Close();
                wxMessageBox("Error: Get medications request failed", "Error", wxICON_ERROR);
            });
        }
    });
    waitingDialog->ShowModal();
    std::unique_ptr<FhirParameters> getMedResp{};
    {
        std::lock_guard lock{*getMedResponseMtx};
        getMedResp = std::move(*getMedResponse);
    }
    if (getMedResp) {
        std::shared_ptr<FhirBundle> medBundle{};
        std::string kjHentet{};
        std::string rfHentet{};
        FhirCoding kjFeilkode{};
        FhirCoding rfM96Feilkode{};
        FhirCoding rfM912Feilkode{};
        bool kjHarLegemidler{false};
        bool kjHarLaste{false};
        bool rfHarLaste{false};
        for (const auto &param : getMedResp->GetParameters()) {
            auto name = param.GetName();
            if (name == "medication") {
                medBundle = std::dynamic_pointer_cast<FhirBundle>(param.GetResource());
            } else if (name == "KJHentetTidspunkt") {
                auto datetime = std::dynamic_pointer_cast<FhirDateTimeValue>(param.GetFhirValue());
                if (datetime) {
                    kjHentet = datetime->GetDateTime();
                }
            } else if (name == "RFHentetTidspunkt") {
                auto datetime = std::dynamic_pointer_cast<FhirDateTimeValue>(param.GetFhirValue());
                if (datetime) {
                    rfHentet = datetime->GetDateTime();
                }
            } else if (name == "KJFeilkode") {
                auto code = std::dynamic_pointer_cast<FhirCodeableConceptValue>(param.GetFhirValue());
                if (code) {
                    auto coding = code->GetCoding();
                    if (!coding.empty()) {
                        kjFeilkode = coding.at(0);
                    }
                }
            } else if (name == "RFM96Feilkode") {
                auto code = std::dynamic_pointer_cast<FhirCodeableConceptValue>(param.GetFhirValue());
                if (code) {
                    auto coding = code->GetCoding();
                    if (!coding.empty()) {
                        rfM96Feilkode = coding.at(0);
                    }
                }
            } else if (name == "RFM912Feilkode") {
                auto code = std::dynamic_pointer_cast<FhirCodeableConceptValue>(param.GetFhirValue());
                if (code) {
                    auto coding = code->GetCoding();
                    if (!coding.empty()) {
                        rfM912Feilkode = coding.at(0);
                    }
                }
            } else if (name == "KJHarLegemidler") {
                auto boolval = std::dynamic_pointer_cast<FhirBooleanValue>(param.GetFhirValue());
                if (boolval) {
                    kjHarLegemidler = boolval->IsTrue();
                }
            } else if (name == "KJHarLaste") {
                auto boolval = std::dynamic_pointer_cast<FhirBooleanValue>(param.GetFhirValue());
                if (boolval) {
                    kjHarLaste = boolval->IsTrue();
                }
            } else if (name == "RFHarLaste") {
                auto boolval = std::dynamic_pointer_cast<FhirBooleanValue>(param.GetFhirValue());
                if (boolval) {
                    rfHarLaste = boolval->IsTrue();
                }
            }
        }
        if (medBundle) {
            medicationBundle = std::make_unique<MedBundleData>();
            *medicationBundle = {
                .medBundle = medBundle,
                .kjHentet = kjHentet,
                .rfHentet = rfHentet,
                .kjFeilkode = kjFeilkode,
                .rfM96Feilkode = rfM96Feilkode,
                .rfM912Feilkode = rfM912Feilkode,
                .kjHarLegemidler = kjHarLegemidler,
                .kjHarLaste = kjHarLaste,
                .rfHarLaste = rfHarLaste
            };
            UpdateHeader();
            UpdateMedications();
        }
    }
}

void TheMasterFrame::OnSendMedication(wxCommandEvent &e) {
    if (!patientInformation) {
        wxMessageBox("Error: No patient information provided", "Error", wxICON_ERROR);
        return;
    }
    if (url.empty()) {
        wxMessageBox("Error: Not connected", "Error", wxICON_ERROR);
        return;
    }

    pplx::task<web::http::http_response> responseTask{};
    {
        web::http::http_request request{web::http::methods::POST};
        request.set_request_uri("patient/$sendMedication");
        {
            FhirParameters sendMedicationParameters{};
            {
                std::shared_ptr<FhirBundle> medBundle = std::make_shared<FhirBundle>();
                {
                    std::shared_ptr<FhirBundle> bundle{};
                    if (medicationBundle) {
                        bundle = medicationBundle->medBundle;
                    }
                    if (!bundle) {
                        wxMessageBox("Error: Get medications first please", "Error", wxICON_ERROR);
                        return;
                    }
                    *medBundle = FhirBundle::Parse(bundle->ToJson());
                }
                sendMedicationParameters.AddParameter("medication", medBundle);
            }
            request.set_body(sendMedicationParameters.ToJson());
        }
        web::http::client::http_client client{url};
        responseTask = client.request(request);
    }
    std::shared_ptr<std::mutex> sendMedResponseMtx = std::make_shared<std::mutex>();
    std::shared_ptr<std::unique_ptr<FhirParameters>> sendMedResponse = std::make_shared<std::unique_ptr<FhirParameters>>();
    std::shared_ptr<WaitingForApiDialog> waitingDialog = std::make_shared<WaitingForApiDialog>(this, "Sending medication records", "Sent medication bundle...");
    responseTask.then([waitingDialog, sendMedResponse, sendMedResponseMtx] (const pplx::task<web::http::http_response> &responseTask) {
        try {
            auto response = responseTask.get();
            try {
                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                    waitingDialog->SetMessage("Receiving results...");
                });
                response.extract_json().then([waitingDialog, sendMedResponse, sendMedResponseMtx](const pplx::task<web::json::value> &responseBodyTask) {
                    try {
                        auto responseBody = responseBodyTask.get();
                        try {
                            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                                waitingDialog->SetMessage("Decoding response...");
                            });
                            FhirParameters responseParameterBundle = FhirParameters::Parse(responseBody);
                            {
                                std::lock_guard lock{*sendMedResponseMtx};
                                *sendMedResponse = std::make_unique<FhirParameters>(std::move(responseParameterBundle));
                            }
                            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog, sendMedResponse]() {
                                waitingDialog->Close();
                            });
                        } catch (std::exception &e) {
                            std::string error = e.what();
                            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog, error]() {
                                waitingDialog->Close();
                                std::string msg{"Error: std::exception: "};
                                msg.append(error);
                                wxMessageBox(msg, "Error", wxICON_ERROR);
                            });
                        } catch (...) {
                            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                                waitingDialog->Close();
                                wxMessageBox("Error: Decoding failed", "Error", wxICON_ERROR);
                            });
                        }
                    } catch (std::exception &e) {
                        std::string error = e.what();
                        wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog, error]() {
                            waitingDialog->Close();
                            std::string msg{"Error: std::exception: "};
                            msg.append(error);
                            wxMessageBox(msg, "Error", wxICON_ERROR);
                        });
                    } catch (...) {
                        wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                            waitingDialog->Close();
                            wxMessageBox("Error: Send medication failed", "Error", wxICON_ERROR);
                        });
                    }
                });
            } catch (...) {
                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                    waitingDialog->Close();
                    wxMessageBox("Error: Send medication failed", "Error", wxICON_ERROR);
                });
            }
        } catch (...) {
            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                waitingDialog->Close();
                wxMessageBox("Error: Send medications failed", "Error", wxICON_ERROR);
            });
        }
    });
    waitingDialog->ShowModal();
    std::unique_ptr<FhirParameters> sendMedResp{};
    {
        std::lock_guard lock{*sendMedResponseMtx};
        sendMedResp = std::move(*sendMedResponse);
    }
    if (!sendMedResp) {
        wxMessageBox("Error: Server did not respond properly to sending medications.", "Error", wxICON_ERROR);
        return;
    }
    int recallCount{0};
    int prescriptionCount{0};
    for (const auto &param : sendMedResp->GetParameters()) {
        auto name = param.GetName();
        if (name == "recallCount") {
            auto value = std::dynamic_pointer_cast<FhirIntegerValue>(param.GetFhirValue());
            recallCount = (int) value->GetValue();
        } else if (name == "prescriptionCount") {
            auto value = std::dynamic_pointer_cast<FhirIntegerValue>(param.GetFhirValue());
            prescriptionCount = (int) value->GetValue();
        }
    }
    std::stringstream str{};
    str << "Recalled " << recallCount << (recallCount == 1 ? " prescription and prescribed " : " prescriptions and prescribed ")
        << prescriptionCount << (prescriptionCount == 1 ? " prescription." : " prescriptions.");
    wxMessageBox(str.str(), wxT("Successful sending"), wxICON_INFORMATION);
}

WeakRefUiDispatcherRef<TheMasterFrame> TheMasterFrame::GetWeakRefDispatcher() {
    return weakRefDispatcher->GetRef();
}

void TheMasterFrame::SetHelseid(const std::string &url, const std::string &clientId, const std::string &secretJwk,
                                const std::vector<std::string> &scopes, const std::string &refreshToken,
                                long expiresIn) {
    helseidUrl = url;
    helseidClientId = clientId;
    helseidSecretJwk = secretJwk;
    helseidScopes = scopes;
    helseidRefreshToken = refreshToken;
    helseidRefreshTokenValidTo = std::time(NULL) + expiresIn - 60;
}

void TheMasterFrame::Connect(const std::string &url) {
    this->url = url;
}