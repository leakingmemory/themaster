//
// Created by sigsegv on 12/13/23.
//

#include "TheMasterFrame.h"
#include "ConnectDialog.h"
#include "FindPatientDialog.h"
#include "CreatePatientDialog.h"
#include "PatientStoreInMemoryWithPersistence.h"
#include "WaitingForApiDialog.h"
#include "MagistralBuilderDialog.h"
#include "PrescriptionDialog.h"
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
#include <sfmbasisapi/fhir/medication.h>
#include <sfmbasisapi/fhir/composition.h>

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
    auto *medicationMenu = new wxMenu();
    medicationMenu->Append(TheMaster_PrescribeMagistral_Id, "Prescribe magistral");
    auto *patientMenu = new wxMenu();
    patientMenu->Append(TheMaster_FindPatient_Id, "Find patient");
    patientMenu->Append(TheMaster_CreatePatient_Id, "Create patient");
    auto *serverMenu = new wxMenu();
    serverMenu->Append(TheMaster_Connect_Id, "Connect");
    serverMenu->Append(TheMaster_GetMedication_Id, "Get medication");
    serverMenu->Append(TheMaster_SendMedication_Id, "Send medication");
    serverMenu->Append(TheMaster_SaveLast_Id, "Save last response");
    serverMenu->Append(TheMaster_SaveBundle_Id, "Save bundle");
    auto *menuBar = new wxMenuBar();
    menuBar->Append(serverMenu, "Server");
    menuBar->Append(patientMenu, "Patient");
    menuBar->Append(medicationMenu, "Medication");
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
    Bind(wxEVT_MENU, &TheMasterFrame::OnSaveLast, this, TheMaster_SaveLast_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSaveBundle, this, TheMaster_SaveBundle_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescribeMagistral, this, TheMaster_PrescribeMagistral_Id);
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
            {
                auto jsonString = requestBody.serialize();
                request.set_body(jsonString, "application/fhir+json; charset=utf-8");
            }
        }
        web::http::client::http_client client{apiUrl};
        return client.request(request);
    });

    std::shared_ptr<std::mutex> getMedResponseMtx = std::make_shared<std::mutex>();
    std::shared_ptr<std::unique_ptr<FhirParameters>> getMedResponse = std::make_shared<std::unique_ptr<FhirParameters>>();
    std::shared_ptr<std::string> rawResponse = std::make_shared<std::string>();
    std::shared_ptr<WaitingForApiDialog> waitingDialog = std::make_shared<WaitingForApiDialog>(this, "Retrieving medication records", "Requested medication bundle...");
    responseTask.then([waitingDialog, getMedResponse, getMedResponseMtx, rawResponse] (const pplx::task<web::http::http_response> &responseTask) {
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
                    response.extract_json(true).then([waitingDialog, getMedResponse, getMedResponseMtx, rawResponse](
                            const pplx::task<web::json::value> &responseBodyTask) {
                        try {
                            auto responseBody = responseBodyTask.get();
                            try {
                                {
                                    auto responseBodyStr = responseBody.serialize();
                                    wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(
                                            [waitingDialog, rawResponse, responseBodyStr, getMedResponseMtx]() {
                                                {
                                                    std::lock_guard lock{*getMedResponseMtx};
                                                    *rawResponse = responseBodyStr;
                                                }
                                                waitingDialog->SetMessage("Decoding data...");
                                            });
                                }
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
        lastResponse = *rawResponse;
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
            {
                auto json = sendMedicationParameters.ToJson();
                auto jsonString = json.serialize();
                request.set_body(jsonString, "application/fhir+json; charset=utf-8");
            }
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
                auto contentType = response.headers().content_type();
                if (!contentType.starts_with("application/fhir+json")) {
                    wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog, contentType]() {
                        waitingDialog->Close();
                        std::string msg{"Wrong content type in response: "};
                        msg.append(contentType);
                        wxMessageBox(msg, "Error", wxICON_ERROR);
                    });
                } else {
                    response.extract_json(true).then([waitingDialog, sendMedResponse, sendMedResponseMtx](
                            const pplx::task<web::json::value> &responseBodyTask) {
                        try {
                            auto responseBody = responseBodyTask.get();
                            try {
                                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                                    waitingDialog->SetMessage("Decoding response...");
                                });
                                FhirParameters responseParameterBundle = FhirParameters::Parse(responseBody);
                                {
                                    std::lock_guard lock{*sendMedResponseMtx};
                                    *sendMedResponse = std::make_unique<FhirParameters>(
                                            std::move(responseParameterBundle));
                                }
                                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter(
                                        [waitingDialog, sendMedResponse]() {
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
                }
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

void TheMasterFrame::OnSaveLast(wxCommandEvent &e) {
    wxFileDialog saveFileDialog(this, _("Save last response"), "", "", "Json files (*.json)|*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return; // the user changed their mind
    std::ofstream ofs{saveFileDialog.GetPath().ToStdString()};
    ofs << lastResponse;
    ofs.close();
}

void TheMasterFrame::OnSaveBundle(wxCommandEvent &e) {
    std::string jsonStr{};
    {
        auto json = medicationBundle->medBundle->ToJson();
        jsonStr = json.serialize();
    }
    wxFileDialog saveFileDialog(this, _("Save bundle"), "", "", "Json files (*.json)|*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return; // the user changed their mind
    std::ofstream ofs{saveFileDialog.GetPath().ToStdString()};
    ofs << jsonStr;
    ofs.close();
}

void TheMasterFrame::SetPrescriber(PrescriptionData &prescriptionData) const {
    Jwt jwt{helseidIdToken};
    JwtPart jwtBody{jwt.GetUnverifiedBody()};
    std::string pid = jwtBody.GetString("helseid://claims/identity/pid");
    std::string hpr = jwtBody.GetString("helseid://claims/hpr/hpr_number");
    std::string name = jwtBody.GetString("name");
    std::string given_name = jwtBody.GetString("given_name");
    std::string middle_name = jwtBody.GetString("middle_name");
    std::string family_name = jwtBody.GetString("family_name");
    std::string dateOfBirth{};
    bool female{true};
    if (pid.size() == 11) {
        std::string sd = pid.substr(0, 2);
        std::string sm = pid.substr(2, 2);
        std::string sy2 = pid.substr(4, 2);
        std::string sc = pid.substr(6, 1);
        std::string sg = pid.substr(8, 1);
        std::size_t ccd;
        std::size_t ccm;
        std::size_t ccy;
        std::size_t ccc;
        std::size_t ccg;
        auto d = std::stoi(sd, &ccd);
        auto m = std::stoi(sm, &ccm);
        auto y = std::stoi(sy2, &ccy);
        auto c = std::stoi(sc, &ccc);
        auto g = std::stoi(sg, &ccg);
        if (ccd == 2 && ccm == 2 && ccy == 2 && ccc == 1 && ccg == 1 && y >= 0 && m > 0 && d > 0) {
            if (c <= 4) {
                if (y < 40) {
                    y += 2000;
                } else {
                    y += 1900;
                }
            } else if (c == 8) {
                y += 2000;
            } else if (c < 8) {
                if (y < 55) {
                    y += 2000;
                } else {
                    y += 1800;
                }
            } else {
                if (y < 40) {
                    y += 1900;
                } else {
                    y += 2000;
                }
            }
        }
        if ((g & 1) == 1) {
            female = false;
        }
        std::stringstream dob{};
        dob << y << "-";
        if (m < 10) {
            dob << "0";
        }
        dob << m << "-";
        if ( d < 10) {
            dob << "0";
        }
        dob << d;
        dateOfBirth = dob.str();
    }
    if (!medicationBundle) {
        return;
    }
    auto bundle = medicationBundle->medBundle;
    if (!bundle) {
        return;
    }
    for (const auto &entry : bundle->GetEntries()) {
        auto resource = entry.GetResource();
        auto practitioner = std::dynamic_pointer_cast<FhirPractitioner>(resource);

        if (practitioner &&
            practitioner->IsActive()) {
            std::string ppid{};
            std::string phpr{};
            for (const auto &identifier : practitioner->GetIdentifiers()) {
                if (identifier.GetSystem() == "urn:oid:2.16.578.1.12.4.1.4.1") {
                    ppid = identifier.GetValue();
                } else if (identifier.GetSystem() == "urn:oid:2.16.578.1.12.4.1.4.4") {
                    phpr = identifier.GetValue();
                }
            }
            bool matching{false};
            if (!pid.empty()) {
                if (!hpr.empty()) {
                    if (ppid == pid && phpr == hpr) {
                        matching = true;
                    }
                } else {
                    if (ppid == pid) {
                        matching = true;
                    }
                }
            } else if (!hpr.empty()) {
                if (phpr == hpr) {
                    matching = true;
                }
            }
            if (matching) {
                prescriptionData.prescribedByReference = entry.GetFullUrl();
                prescriptionData.prescribedByDisplay = practitioner->GetDisplay();
                return;
            }
        }
    }
    std::shared_ptr<FhirPractitioner> practitioner = std::make_shared<FhirPractitioner>();
    practitioner->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner");
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        practitioner->SetId(uuidStr);
    }
    practitioner->SetStatus(FhirStatus::ACTIVE);
    {
        std::vector<FhirIdentifier> identifiers{};
        if (!hpr.empty()) {
            identifiers.emplace_back(FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/HPR", "HPR-nummer", ""), "official", "urn:oid:2.16.578.1.12.4.1.4.4", hpr);
        }
        if (!pid.empty()) {
            identifiers.emplace_back(FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/FNR", "FNR-nummer", ""), "official", "urn:oid:2.16.578.1.12.4.1.4.1", pid);
        }
        practitioner->SetIdentifiers(identifiers);
    }
    practitioner->SetActive(true);
    practitioner->SetBirthDate(dateOfBirth);
    practitioner->SetGender(female ? "female" : "male");
    {
        std::vector<FhirName> setName{};
        std::string name{given_name};
        if (!name.empty()) {
            if (!middle_name.empty()) {
                name.append(" ");
                name.append(middle_name);
            }
        } else {
            name = middle_name;
        }
        setName.emplace_back("official", family_name, name);
        practitioner->SetName(setName);
    }
    std::string fullUrl{"urn:uuid:"};
    fullUrl.append(practitioner->GetId());
    FhirBundleEntry bundleEntry{fullUrl, practitioner};
    bundle->AddEntry(bundleEntry);
    prescriptionData.prescribedByReference = fullUrl;
    prescriptionData.prescribedByDisplay = practitioner->GetDisplay();
}

void TheMasterFrame::SetPatient(PrescriptionData &prescriptionData) const {
    std::string pid{};
    auto patientInformation = this->patientInformation;
    if (patientInformation && patientInformation->GetPatientIdType() == PatientIdType::FODSELSNUMMER) {
        pid = patientInformation->GetPatientId();
    }
    auto bundle = medicationBundle->medBundle;
    if (!bundle) {
        return;
    }
    if (!pid.empty()) {
        for (const auto &entry: bundle->GetEntries()) {
            auto resource = entry.GetResource();
            auto patient = std::dynamic_pointer_cast<FhirPatient>(resource);

            if (patient &&
                patient->IsActive()) {
                std::string ppid{};
                for (const auto &identifier: patient->GetIdentifiers()) {
                    if (identifier.GetSystem() == "urn:oid:2.16.578.1.12.4.1.4.1") {
                        ppid = identifier.GetValue();
                    }
                }
                if (ppid == pid) {
                    prescriptionData.subjectReference = entry.GetFullUrl();
                    prescriptionData.subjectDisplay = patient->GetDisplay();
                    return;
                }
            }
        }
    }
    std::shared_ptr<FhirPatient> patient = std::make_shared<FhirPatient>();
    patient->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Patient");
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        patient->SetId(uuidStr);
    }
    patient->SetStatus(FhirStatus::NOT_SET);
    {
        std::vector<FhirIdentifier> identifiers{};
        if (!pid.empty()) {
            identifiers.emplace_back(FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/FNR", "FNR-nummer", ""), "official", "urn:oid:2.16.578.1.12.4.1.4.1", pid);
        }
        patient->SetIdentifiers(identifiers);
    }
    patient->SetActive(true);
    patient->SetBirthDate(patientInformation->GetDateOfBirth());
    patient->SetGender(patientInformation->GetGender() == PersonGender::FEMALE ? "female" : "male");
    {
        std::vector<FhirName> setName{};
        setName.emplace_back("official", patientInformation->GetFamilyName(), patientInformation->GetGivenName());
        patient->SetName(setName);
    }
    std::string fullUrl{"urn:uuid:"};
    fullUrl.append(patient->GetId());
    FhirBundleEntry bundleEntry{fullUrl, patient};
    bundle->AddEntry(bundleEntry);
    prescriptionData.subjectReference = fullUrl;
    prescriptionData.subjectDisplay = patient->GetDisplay();
}

void TheMasterFrame::OnPrescribeMagistral(wxCommandEvent &e) {
    PrescriptionData prescriptionData{};
    MagistralBuilderDialog magistralBuilderDialog{this};
    if (magistralBuilderDialog.ShowModal() == wxID_OK) {
        PrescriptionDialog prescriptionDialog{this};
        auto res = prescriptionDialog.ShowModal();
        if (res != wxID_OK) {
            return;
        }
        prescriptionData = prescriptionDialog.GetPrescriptionData();
    }
    SetPrescriber(prescriptionData);
    SetPatient(prescriptionData);
    std::shared_ptr<FhirMedication> magistralMedicament = std::make_shared<FhirMedication>(magistralBuilderDialog.GetMagistralMedicament().ToFhir());
    std::string magistralMedicamentFullUrl{"urn:uuid:"};
    magistralMedicamentFullUrl.append(magistralMedicament->GetId());
    FhirBundleEntry magistralMedicamentEntry{magistralMedicamentFullUrl, magistralMedicament};
    std::shared_ptr<FhirMedicationStatement> medicationStatement = std::make_shared<FhirMedicationStatement>(prescriptionData.ToFhir());
    {
        auto medicationProfile = magistralMedicament->GetProfile();
        FhirReference medicationReference{magistralMedicamentFullUrl, medicationProfile.size() == 1 ? medicationProfile[0] : "", magistralMedicament->GetDisplay()};
        medicationStatement->SetMedicationReference(medicationReference);
    }
    std::string medicationStatementFullUrl{"urn:uuid:"};
    medicationStatementFullUrl.append(medicationStatement->GetId());
    FhirBundleEntry medicationStatementEntry{medicationStatementFullUrl, medicationStatement};
    medicationBundle->medBundle->AddEntry(magistralMedicamentEntry);
    medicationBundle->medBundle->AddEntry(medicationStatementEntry);
    for (const auto &entry : medicationBundle->medBundle->GetEntries()) {
        auto composition = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
        if (composition) {
            auto profile = composition->GetProfile();
            if (profile.size() == 1 && profile[0] == "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationComposition") {
                auto sections = composition->GetSections();
                for (auto &section : sections) {
                    auto coding = section.GetCode().GetCoding();
                    if (coding.size() == 1 && coding[0].GetCode() == "sectionMedication") {
                        auto entries = section.GetEntries();
                        if (entries.empty()) {
                            section.SetTextStatus("generated");
                            section.SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\">List of medications</xhtml:div>");
                            section.SetEmptyReason({});
                        }
                        entries.emplace_back(medicationStatementFullUrl,
                                             "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement",
                                             medicationStatement->GetDisplay());
                        section.SetEntries(entries);
                    }
                }
                composition->SetSections(sections);
            }
        }
    }
}

WeakRefUiDispatcherRef<TheMasterFrame> TheMasterFrame::GetWeakRefDispatcher() {
    return weakRefDispatcher->GetRef();
}

void TheMasterFrame::SetHelseid(const std::string &url, const std::string &clientId, const std::string &secretJwk,
                                const std::vector<std::string> &scopes, const std::string &refreshToken,
                                long expiresIn, const std::string &idToken) {
    helseidUrl = url;
    helseidClientId = clientId;
    helseidSecretJwk = secretJwk;
    helseidScopes = scopes;
    helseidRefreshToken = refreshToken;
    helseidRefreshTokenValidTo = std::time(NULL) + expiresIn - 60;
    helseidIdToken = idToken;
}

void TheMasterFrame::Connect(const std::string &url) {
    this->url = url;
}