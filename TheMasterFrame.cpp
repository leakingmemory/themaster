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
#include <sfmbasisapi/fhir/fhirbasic.h>
#include <jjwtid/Jwt.h>
#include <jjwtid/OidcTokenRequest.h>
#include <cpprest/http_client.h>
#include <wx/listctrl.h>
#include "MedBundleData.h"
#include <InstallPrefix.h>
#include <sfmbasisapi/fhir/medication.h>
#include <sfmbasisapi/fhir/composition.h>
#include "FestDbUi.h"
#include "FindMedicamentDialog.h"
#include <medfest/Struct/Decoded/LegemiddelCore.h>
#include "SfmMedicamentMapper.h"
#include "FestVersionsDialog.h"
#include "PrescriptionDetailsDialog.h"
#include "RecallPrescriptionDialog.h"
#include "FestDbQuotasDialog.h"
#include "CeasePrescriptionDialog.h"
#include "DateTime.h"
#include "SignPllDialog.h"

constexpr int PrescriptionNameColumnWidth = 250;
constexpr int PrescriptionRemoteColumnWidth = 75;

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
    auto *festMenu = new wxMenu();
    festMenu->Append(TheMaster_UpdateFest_Id, "Update FEST");
    festMenu->Append(TheMaster_ShowFestVersions_Id, "FEST versions");
    festMenu->Append(TheMaster_ShowFestDbQuotas_Id, "FEST DB quotas");
    auto *medicationMenu = new wxMenu();
    medicationMenu->Append(TheMaster_PrescribeMagistral_Id, "Prescribe magistral");
    medicationMenu->Append(TheMaster_PrescribeMedicament_Id, "Prescribe medicament");
    auto *patientMenu = new wxMenu();
    patientMenu->Append(TheMaster_FindPatient_Id, "Find patient");
    patientMenu->Append(TheMaster_CreatePatient_Id, "Create patient");
    auto *serverMenu = new wxMenu();
    serverMenu->Append(TheMaster_Connect_Id, "Connect");
    serverMenu->Append(TheMaster_GetMedication_Id, "Get medication");
    serverMenu->Append(TheMaster_SendMedication_Id, "Send medication");
    serverMenu->Append(TheMaster_SendPll_Id, "Send PLL");
    serverMenu->Append(TheMaster_SaveLastRequest_Id, "Save last request");
    serverMenu->Append(TheMaster_SaveLast_Id, "Save last response");
    serverMenu->Append(TheMaster_SaveBundle_Id, "Save bundle");
    auto *menuBar = new wxMenuBar();
    menuBar->Append(serverMenu, "Server");
    menuBar->Append(patientMenu, "Patient");
    menuBar->Append(medicationMenu, "Medication");
    menuBar->Append(festMenu, "FEST");
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
    prescriptions->AppendColumn(wxT("Published"));
    prescriptions->SetColumnWidth(0, PrescriptionNameColumnWidth);
    prescriptions->SetColumnWidth(1, PrescriptionRemoteColumnWidth);
    prescriptions->Bind(wxEVT_CONTEXT_MENU, &TheMasterFrame::OnPrescriptionContextMenu, this, wxID_ANY);
    sizer->Add(prescriptions, 1, wxEXPAND | wxALL, 5);

    SetSizerAndFit(sizer);

    Bind(wxEVT_MENU, &TheMasterFrame::OnConnect, this, TheMaster_Connect_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnFindPatient, this, TheMaster_FindPatient_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnCreatePatient, this, TheMaster_CreatePatient_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnGetMedication, this, TheMaster_GetMedication_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSendMedication, this, TheMaster_SendMedication_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSendPll, this, TheMaster_SendPll_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSaveLastRequest, this, TheMaster_SaveLastRequest_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSaveLast, this, TheMaster_SaveLast_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSaveBundle, this, TheMaster_SaveBundle_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescribeMagistral, this, TheMaster_PrescribeMagistral_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescribeMedicament, this, TheMaster_PrescribeMedicament_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnUpdateFest, this, TheMaster_UpdateFest_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnShowFestVersions, this, TheMaster_ShowFestVersions_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnShowFestDbQuotas, this, TheMaster_ShowFestDbQuotas_Id);

    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescriptionDetails, this, TheMaster_PrescriptionDetails_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescriptionRecall, this, TheMaster_PrescriptionRecall_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescriptionCease, this, TheMaster_PrescriptionCease_Id);
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
    prescriptions->AppendColumn(wxT("Published"));
    prescriptions->SetColumnWidth(0, PrescriptionNameColumnWidth);
    prescriptions->SetColumnWidth(1, PrescriptionRemoteColumnWidth);
    std::vector<std::shared_ptr<FhirMedicationStatement>> displayedMedicationStatements{};
    auto pos = 0;
    for (const auto &bundleEntry : medicationBundle->medBundle->GetEntries()) {
        auto resource = bundleEntry.GetResource();
        auto resourceType = resource->GetResourceType();
        auto medicationStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(resource);
        if (medicationStatement) {
            auto row = pos++;
            displayedMedicationStatements.emplace_back(medicationStatement);
            prescriptions->InsertItem(row, medicationStatement->GetMedicationReference().GetDisplay());
            bool createeresept{false};
            bool recalled{false};
            bool recallNotSent{false};
            auto statementExtensions = medicationStatement->GetExtensions();
            for (const auto &statementExtension : statementExtensions) {
                auto url = statementExtension->GetUrl();
                if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
                    auto reseptAmendmentExtensions = statementExtension->GetExtensions();
                    for (const auto &reseptAmendmentExtension : reseptAmendmentExtensions) {
                        auto url = reseptAmendmentExtension->GetUrl();
                        if (url == "createeresept") {
                            auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(reseptAmendmentExtension);
                            if (valueExt) {
                                auto value = std::dynamic_pointer_cast<FhirBooleanValue>(valueExt->GetValue());
                                if (value && value->IsTrue()) {
                                    createeresept = true;
                                }
                            }
                        }
                        if (url == "recallinfo") {
                            auto extensions = reseptAmendmentExtension->GetExtensions();
                            for (const auto &extension : extensions) {
                                auto url = extension->GetUrl();
                                if (url == "recallcode") {
                                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                    if (valueExtension) {
                                        auto codeableConceptValue = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                                        if (codeableConceptValue && codeableConceptValue->IsSet()) {
                                            recalled = true;
                                        }
                                    }
                                }
                                if (url == "notsent") {
                                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                    if (valueExtension) {
                                        auto booleanExtension = std::dynamic_pointer_cast<FhirBooleanValue>(valueExtension->GetValue());
                                        if (booleanExtension) {
                                            recallNotSent = booleanExtension->IsTrue();
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (createeresept) {
                prescriptions->SetItem(row, 1, wxT("Draft"));
            } else if (recalled) {
                if (recallNotSent) {
                    prescriptions->SetItem(row, 1, wxT("To be recalled"));
                } else {
                    prescriptions->SetItem(row, 1, wxT("Recalled"));
                }
            } else {
                prescriptions->SetItem(row, 1, wxT("Published"));
            }
        }
    }
    this->displayedMedicationStatements = displayedMedicationStatements;
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
    std::shared_ptr<std::string> rawRequest = std::make_shared<std::string>();
    responseTask = accessTokenTask.then([apiUrl, patient, rawRequest] (const std::string &accessToken) {
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
                *rawRequest = jsonString;
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
        lastRequest = *rawRequest;
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

std::map<std::string,std::shared_ptr<FhirExtension>> TheMasterFrame::GetRecallInfos(FhirBundle &bundle) {
    std::map<std::string,std::shared_ptr<FhirExtension>> map{};
    for (const auto &entry : bundle.GetEntries()) {
        auto medStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(entry.GetResource());
        if (medStatement) {
            std::string reseptId{};
            {
                auto identifiers = medStatement->GetIdentifiers();
                for (const auto &identifier : identifiers) {
                    if (identifier.GetType().GetText() == "ReseptId") {
                        reseptId = identifier.GetValue();
                    }
                }
            }
            std::shared_ptr<FhirExtension> reseptAmendment{};
            {
                auto extensions = medStatement->GetExtensions();
                for (const auto &extension: extensions) {
                    auto url = extension->GetUrl();
                    if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
                        reseptAmendment = extension;
                    }
                }
            }
            if (reseptAmendment) {
                auto extensions = reseptAmendment->GetExtensions();
                for (const auto &extension : extensions) {
                    auto url = extension->GetUrl();
                    if (url == "recallinfo") {
                        map.insert_or_assign(reseptId, extension);
                    }
                }
            }
        }
    }
    return map;
}

void TheMasterFrame::SendMedication(const std::function<void (const std::shared_ptr<FhirBundle> &)> &preprocessing, const std::function<void (const std::map<std::string,FhirCoding> &)> &pllResultsFunc) {
    if (!patientInformation) {
        wxMessageBox("Error: No patient information provided", "Error", wxICON_ERROR);
        return;
    }
    if (url.empty()) {
        wxMessageBox("Error: Not connected", "Error", wxICON_ERROR);
        return;
    }
    std::string apiUrl = url;
    std::shared_ptr<FhirBundle> bundle{};
    if (medicationBundle) {
        bundle = medicationBundle->medBundle;
    }
    if (!bundle) {
        wxMessageBox("Error: Get medications first please", "Error", wxICON_ERROR);
        return;
    }
    std::shared_ptr<std::map<std::string,std::shared_ptr<FhirExtension>>> recallInfos = std::make_shared<std::map<std::string,std::shared_ptr<FhirExtension>>>();
    for (const auto &riPair : GetRecallInfos(*bundle)) {
        auto recallInfo = riPair.second;
        auto extensions = recallInfo->GetExtensions();
        for (const auto &extension : extensions) {
            auto url = extension->GetUrl();
            if (url == "notsent") {
                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                if (valueExtension) {
                    auto value = std::dynamic_pointer_cast<FhirBooleanValue>(valueExtension->GetValue());
                    if (value && value->IsTrue()) {
                        recallInfos->insert_or_assign(riPair.first, recallInfo);
                    }
                }
            }
        }
    }

    pplx::task<web::http::http_response> responseTask{};
    auto accessTokenTask = GetAccessToken();
    std::shared_ptr<std::string> rawRequest = std::make_shared<std::string>();
    responseTask = accessTokenTask.then([apiUrl, bundle, recallInfos, preprocessing, rawRequest] (const std::string &accessToken) {
        web::http::http_request request{web::http::methods::POST};
        if (!accessToken.empty()) {
            std::cout << "Access token: " << accessToken << "\n";
            std::string bearer{"Bearer "};
            bearer.append(accessToken);
            request.headers().add("Authorization", bearer);
        }
        request.set_request_uri("Patient/$sendMedication");
        {
            FhirParameters sendMedicationParameters{};
            {
                std::shared_ptr<FhirBundle> medBundle = std::make_shared<FhirBundle>();
                {
                    *medBundle = FhirBundle::Parse(bundle->ToJson());
                }

                for (const auto &riPair : GetRecallInfos(*medBundle)) {
                    auto extensions = riPair.second->GetExtensions();
                    std::vector<std::shared_ptr<FhirExtension>> preservedExtensions{};
                    for (const auto &extension : extensions) {
                        auto url = extension->GetUrl();
                        if (url != "notsent") {
                            preservedExtensions.emplace_back(extension);
                        }
                    }
                    riPair.second->SetExtensions(preservedExtensions);
                }

                for (const auto &entry : medBundle->GetEntries()) {
                    auto composition = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
                    if (composition) {
                        auto relatesTo = composition->GetRelatesTo();
                        if (!relatesTo.IsSet()) {
                            relatesTo = composition->GetIdentifier();
                            if (relatesTo.IsSet()) {
                                composition->SetRelatesToCode("replaces");
                                composition->SetRelatesTo(relatesTo);
                            }
                        }
                    }
                }

                preprocessing(medBundle);

                sendMedicationParameters.AddParameter("medication", medBundle);
            }
            {
                auto json = sendMedicationParameters.ToJson();
                auto jsonString = json.serialize();
                request.set_body(jsonString, "application/fhir+json; charset=utf-8");
                *rawRequest = jsonString;
            }
        }
        web::http::client::http_client client{apiUrl};
        return client.request(request);
    });
    std::shared_ptr<std::mutex> sendMedResponseMtx = std::make_shared<std::mutex>();
    std::shared_ptr<std::unique_ptr<FhirParameters>> sendMedResponse = std::make_shared<std::unique_ptr<FhirParameters>>();
    std::shared_ptr<std::string> rawResponse = std::make_shared<std::string>();
    std::shared_ptr<WaitingForApiDialog> waitingDialog = std::make_shared<WaitingForApiDialog>(this, "Sending medication records", "Sent medication bundle...");
    responseTask.then([waitingDialog, sendMedResponse, rawResponse, sendMedResponseMtx] (const pplx::task<web::http::http_response> &responseTask) {
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
                    response.extract_json(true).then([waitingDialog, sendMedResponse, rawResponse, sendMedResponseMtx](
                            const pplx::task<web::json::value> &responseBodyTask) {
                        try {
                            auto responseBody = responseBodyTask.get();
                            try {
                                wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([waitingDialog]() {
                                    waitingDialog->SetMessage("Decoding response...");
                                });
                                {
                                    std::lock_guard lock{*sendMedResponseMtx};
                                    *rawResponse = responseBody.serialize();
                                }
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
        lastRequest = std::move(*rawRequest);
        lastResponse = std::move(*rawResponse);
    }
    if (!sendMedResp) {
        wxMessageBox("Error: Server did not respond properly to sending medications.", "Error", wxICON_ERROR);
        return;
    }
    int recallCount{0};
    int prescriptionCount{0};
    std::map<std::string,FhirCoding> pllResults{};
    for (const auto &param : sendMedResp->GetParameters()) {
        auto name = param.GetName();
        if (name == "recallCount") {
            auto value = std::dynamic_pointer_cast<FhirIntegerValue>(param.GetFhirValue());
            recallCount = (int) value->GetValue();
        } else if (name == "prescriptionCount") {
            auto value = std::dynamic_pointer_cast<FhirIntegerValue>(param.GetFhirValue());
            prescriptionCount = (int) value->GetValue();
        } else if (name == "PllResult") {
            std::string pllId{};
            FhirCoding resultCode{};
            {
                auto subparams = param.GetPart();
                for (const auto &subparam : subparams) {
                    auto name = subparam->GetName();
                    if (name == "PllmessageID") {
                        auto value = std::dynamic_pointer_cast<FhirString>(subparam->GetFhirValue());
                        if (value) {
                            pllId = value->GetValue();
                        }
                    } else if (name == "resultCode") {
                        auto value = std::dynamic_pointer_cast<FhirCodingValue>(subparam->GetFhirValue());
                        if (value) {
                            resultCode = *value;
                        }
                    }
                }
            }
            if (!pllId.empty()) {
                pllResults.insert_or_assign(pllId, resultCode);
            }
        } else if (name == "prescriptionOperationResult") {
            std::string reseptId{};
            FhirCoding resultCode{};
            {
                auto subparams = param.GetPart();
                for (const auto &subparam : subparams) {
                    auto name = subparam->GetName();
                    if (name == "reseptID") {
                        auto value = std::dynamic_pointer_cast<FhirString>(subparam->GetFhirValue());
                        if (value) {
                            reseptId = value->GetValue();
                        }
                    } else if (name == "resultCode") {
                        auto value = std::dynamic_pointer_cast<FhirCodingValue>(subparam->GetFhirValue());
                        if (value) {
                            resultCode = *value;
                        }
                    }
                }
            }
            if (!reseptId.empty()) {
                if (resultCode.GetCode() == "0") {
                    auto bundleEntries = bundle->GetEntries();
                    for (auto &bundleEntry: bundleEntries) {
                        auto medicationStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(
                                bundleEntry.GetResource());
                        if (medicationStatement) {
                            std::string reseptid{};
                            {
                                auto identifiers = medicationStatement->GetIdentifiers();
                                for (const auto &identifier: identifiers) {
                                    auto type = identifier.GetType().GetText();
                                    std::transform(type.begin(), type.end(), type.begin(),
                                                   [](char ch) { return std::tolower(ch); });
                                    if (type == "reseptid") {
                                        reseptid = identifier.GetValue();
                                    }
                                }
                            }
                            if (reseptid != reseptId) {
                                continue;
                            }
                            auto extensions = medicationStatement->GetExtensions();
                            for (const auto &ext: extensions) {
                                auto url = ext->GetUrl();
                                if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
                                    auto extensions = ext->GetExtensions();
                                    auto iterator = extensions.begin();
                                    while (iterator != extensions.end()) {
                                        auto ext = *iterator;
                                        if (ext->GetUrl() == "createeresept") {
                                            iterator = extensions.erase(iterator);
                                        } else {
                                            ++iterator;
                                        }
                                    }
                                    ext->SetExtensions(std::move(extensions));
                                }
                            }
                        }
                    }
                    bundle->SetEntries(bundleEntries);
                } else if (resultCode.GetCode() == "1") {
                    auto iterator = recallInfos->find(reseptId);
                    if (iterator != recallInfos->end()) {
                        auto recallInfo = iterator->second;
                        auto extensions = recallInfo->GetExtensions();
                        auto iterator = extensions.begin();
                        while (iterator != extensions.end()) {
                            auto url = (*iterator)->GetUrl();
                            if (url != "notsent") {
                                ++iterator;
                            } else {
                                iterator = extensions.erase(iterator);
                            };
                        }
                        recallInfo->SetExtensions(extensions);
                    }
                }
            }
        }
    }
    pllResultsFunc(pllResults);
    UpdateMedications();
    std::stringstream str{};
    str << "Recalled " << recallCount << (recallCount == 1 ? " prescription and prescribed " : " prescriptions and prescribed ")
        << prescriptionCount << (prescriptionCount == 1 ? " prescription." : " prescriptions.");
    wxMessageBox(str.str(), wxT("Successful sending"), wxICON_INFORMATION);
}

void TheMasterFrame::OnSendMedication(wxCommandEvent &e) {
    SendMedication(
            [] (const std::shared_ptr<FhirBundle> &) {},
            [] (const std::map<std::string,FhirCoding> &) {}
    );
}

void TheMasterFrame::OnSendPll(wxCommandEvent &e) {
    std::vector<std::string> selected{};
    std::map<std::string,std::shared_ptr<FhirMedicationStatement>> medicationStatements{};
    std::map<std::string,std::shared_ptr<FhirMedicationStatement>> pllMedicationStatements{};
    std::vector<std::string> newPllIds{};
    {
        auto bundle = medicationBundle->medBundle;
        if (!bundle) {
            return;
        }
        {
            auto bundleEntries = bundle->GetEntries();
            for (const auto &bundleEntry : bundleEntries) {
                auto url = bundleEntry.GetFullUrl();
                auto resource = bundleEntry.GetResource();
                std::shared_ptr<FhirMedicationStatement> medStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(resource);
                if (medStatement) {
                    medicationStatements.insert_or_assign(url, medStatement);
                }
            }
        }
        {
            std::map<std::string, std::string> idToDisplay{};
            for (const auto &statement : medicationStatements) {
                idToDisplay.insert_or_assign(statement.first, statement.second->GetDisplay());
            }
            SignPllDialog signPllDialog{this, idToDisplay};
            if (signPllDialog.ShowModal() != wxID_OK) {
                return;
            }
            selected = signPllDialog.GetSelected();
            for (const auto &id : selected) {
                auto iterator = medicationStatements.find(id);
                if (iterator != medicationStatements.end()) {
                    auto medicationStatement = iterator->second;
                    std::string pllId{};
                    {
                        auto identifiers = medicationStatement->GetIdentifiers();
                        for (const auto &identifier: identifiers) {
                            auto key = identifier.GetType().GetText();
                            std::transform(key.begin(), key.end(), key.begin(),
                                           [](auto ch) -> char { return std::tolower(ch); });
                            if (key == "pll") {
                                pllId = identifier.GetValue();
                            }
                        }
                        if (pllId.empty()) {
                            boost::uuids::uuid uuid = boost::uuids::random_generator()();
                            pllId = to_string(uuid);
                            std::vector<FhirIdentifier> newIdentifiers{};
                            {
                                FhirIdentifier pllIdentifier{FhirCodeableConcept("PLL"), "usual", pllId};
                                newIdentifiers.emplace_back(std::move(pllIdentifier));
                            }
                            for (auto &identifier : identifiers) {
                                newIdentifiers.emplace_back(std::move(identifier));
                            }
                            identifiers = std::move(newIdentifiers);
                            newPllIds.emplace_back(pllId);
                        }
                        medicationStatement->SetIdentifiers(identifiers);
                        pllMedicationStatements.insert_or_assign(pllId, medicationStatement);
                    }
                }
            }
        }
        auto lastChanged = DateTimeOffset::Now().to_iso8601();
        for (const auto &pair : medicationStatements) {
            auto &medicationStatement = *(pair.second);
            auto extensions = medicationStatement.GetExtensions();
            {
                std::string pllId{};
                {
                    auto identifiers = medicationStatement.GetIdentifiers();
                    for (const auto &identifier : identifiers) {
                        std::string key = identifier.GetType().GetText();
                        std::transform(key.cbegin(), key.cend(), key.begin(), [] (char ch) { return std::tolower(ch); });
                        if (key == "pll") {
                            pllId = identifier.GetValue();
                            break;
                        }
                    }
                }
                bool found{false};
                if (!pllId.empty()) {
                    for (const auto &newId : newPllIds) {
                        if (pllId == newId) {
                            found = true;
                            break;
                        }
                    }
                }
                if (!found) {
                    // We should not set lastchanged for an unchanged medication statement with pll id
                    continue;
                }
            }
            for (auto &extension : extensions) {
                auto url = extension->GetUrl();
                if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
                    auto extensions = extension->GetExtensions();
                    auto extensionIterator = extensions.begin();
                    auto replaceIterator = extensions.end();
                    bool found{false};
                    while (extensionIterator != extensions.end()) {
                        auto &extension = *extensionIterator;
                        auto url = extension->GetUrl();
                        if (url == "lastchanged") {
                            replaceIterator = extensionIterator;
                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExtension) {
                                auto value = std::dynamic_pointer_cast<FhirDateTimeValue>(valueExtension->GetValue());
                                if (value) {
                                    value->SetDateTime(lastChanged);
                                    found = true;
                                }
                            }
                        }
                        ++extensionIterator;
                    }
                    if (!found) {
                        if (replaceIterator != extensions.end()) {
                            extensions.erase(replaceIterator);
                        }
                        extensions.emplace_back(std::make_shared<FhirValueExtension>("lastchanged", std::make_shared<FhirDateTimeValue>(lastChanged)));
                        extension->SetExtensions(extensions);
                    }
                }
            }
        }
    }
    auto subjectRef = GetSubjectRef();
    SendMedication([subjectRef, selected, pllMedicationStatements] (const std::shared_ptr<FhirBundle> &bundle) {
        auto entries = bundle->GetEntries();
        std::vector<FhirBundleEntry> appendEntries{};
        for (const auto &entry : entries) {
            auto composition = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
            if (composition) {
                auto sections = composition->GetSections();
                for (auto &section : sections) {
                    auto codings = section.GetCode().GetCoding();
                    if (codings.empty()) {
                        continue;
                    }
                    if (codings[0].GetCode() == "sectionPLLinfo") {
                        auto sectionEntries = section.GetEntries();
                        std::shared_ptr<FhirBasic> m251Message{};
                        {
                            std::vector<std::shared_ptr<FhirBasic>> m25Messages{};
                            for (const auto &entry: sectionEntries) {
                                auto ref = entry.GetReference();
                                for (const auto &bundleEntry: entries) {
                                    if (bundleEntry.GetFullUrl() == ref) {
                                        auto fhirBasic = std::dynamic_pointer_cast<FhirBasic>(
                                                bundleEntry.GetResource());
                                        if (fhirBasic) {
                                            m25Messages.emplace_back(fhirBasic);
                                        }
                                    }
                                }
                            }
                            for (const auto &m25Message : m25Messages) {
                                std::string code{};
                                {
                                    auto codings = m25Message->GetCode().GetCoding();
                                    if (!codings.empty()) {
                                        code = codings[0].GetCode();
                                    }
                                }
                                if (code == "M25.1") {
                                    m251Message = m25Message;
                                }
                            }
                        }
                        if (!m251Message) {
                            std::string url{"urn:uuid:"};
                            m251Message = std::make_shared<FhirBasic>();
                            {
                                boost::uuids::random_generator generator;
                                boost::uuids::uuid randomUUID = generator();
                                std::string uuidStr = boost::uuids::to_string(randomUUID);
                                m251Message->SetId(uuidStr);
                                url.append(uuidStr);
                            }
                            m251Message->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-PLL-info");
                            m251Message->SetCode(FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-message-type", "M25.1", "PLL"));
                            if (subjectRef.IsSet()) {
                                m251Message->SetSubject(subjectRef);
                            }
                            appendEntries.emplace_back(url, m251Message);
                            sectionEntries.emplace_back(url, "http://ehelse.no/fhir/StructureDefinition/sfm-PLL-info", "sfm-PLL-info");
                            section.SetEntries(sectionEntries);
                            section.SetEmptyReason({});
                        }
                        std::shared_ptr<FhirExtension> metadataPll{};
                        {
                            auto extensions = m251Message->GetExtensions();
                            for (const auto &extension: extensions) {
                                auto urlLower = extension->GetUrl();
                                std::transform(urlLower.begin(), urlLower.end(), urlLower.begin(), [] (auto ch) -> char {return std::tolower(ch);});
                                if (urlLower == "http://ehelse.no/fhir/StructureDefinition/sfm-pllInformation") {
                                    metadataPll = extension;
                                }
                            }
                        }
                        if (!metadataPll) {
                            metadataPll = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-pllInformation");
                            m251Message->AddExtension(metadataPll);
                        }
                        std::shared_ptr<FhirBooleanValue> createPll{};
                        {
                            auto extensions = metadataPll->GetExtensions();
                            for (const auto &extension: extensions) {
                                auto urlLower = extension->GetUrl();
                                std::transform(urlLower.begin(), urlLower.end(), urlLower.begin(), [] (auto ch) -> char {return std::tolower(ch);});
                                if (urlLower == "createpll") {
                                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                    if (valueExtension) {
                                        auto value = std::dynamic_pointer_cast<FhirBooleanValue>(valueExtension->GetValue());
                                        if (value) {
                                            createPll = value;
                                        }
                                    }
                                }
                            }
                        }
                        if (createPll) {
                            createPll->SetValue(true);
                        } else {
                            createPll = std::make_shared<FhirBooleanValue>(true);
                            metadataPll->AddExtension(std::make_shared<FhirValueExtension>("createPLL", createPll));
                        }
                    } else if (codings[0].GetCode() == "sectionMedication") {
                        auto entries = section.GetEntries();
                        auto iterator = entries.begin();
                        while (iterator != entries.end()) {
                            auto reference = iterator->GetReference();
                            bool found{false};
                            for (const auto &id : selected) {
                                if (reference == id) {
                                    found = true;
                                    break;
                                }
                            }
                            if (found) {
                                ++iterator;
                            } else {
                                iterator = entries.erase(iterator);
                            }
                        }
                        section.SetEntries(entries);
                    }
                }
                composition->SetSections(sections);
                continue;
            }
        }
        {
            auto iterator = entries.begin();
            while (iterator != entries.end()) {
                auto medicationStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(iterator->GetResource());
                if (medicationStatement) {
                    std::string pllId{};
                    {
                        auto identifiers = medicationStatement->GetIdentifiers();
                        for (const auto &identifier : identifiers) {
                            auto key = identifier.GetType().GetText();
                            std::transform(key.cbegin(), key.cend(), key.begin(), [] (char ch) {return std::tolower(ch);});
                            if (key == "pll") {
                                pllId = identifier.GetValue();
                            }
                        }
                    }
                    bool found{false};
                    if (!pllId.empty()) {
                        found = pllMedicationStatements.find(pllId) != pllMedicationStatements.end();
                    }
                    if (found) {
                        ++iterator;
                    } else {
                        iterator = entries.erase(iterator);
                    }
                } else {
                    ++iterator;
                }
            }
        }
        auto iterator = appendEntries.begin();
        while (iterator != appendEntries.end()) {
            entries.emplace_back(std::move(*iterator));
            iterator = appendEntries.erase(iterator);
        }
        bundle->SetEntries(entries);
    }, [medicationStatements, newPllIds, pllMedicationStatements] (const std::map<std::string,FhirCoding> &pllResults) {
        std::vector<std::string> removePllIds = newPllIds;
        int pllsSent{0};
        for (const auto &resultPair : pllResults) {
            const auto &pllId = resultPair.first;
            const auto &resultCode = resultPair.second;
            if (resultCode.GetCode() == "0") {
                ++pllsSent;
                auto iterator = removePllIds.begin();
                while (iterator != removePllIds.end()) {
                    if (*iterator == pllId) {
                        iterator = removePllIds.erase(iterator);
                    } else {
                        ++iterator;
                    }
                }
            } else {
                std::stringstream sstr{};
                sstr << "Failed to send PLL with id " << pllId << " with code " << resultCode.GetCode()
                << " " << resultCode.GetDisplay();
                wxMessageBox(wxString::FromUTF8(sstr.str()), wxT("PLL result error code"), wxICON_ERROR);
            }
        }
        if (pllsSent > 0) {
            {
                std::stringstream sstr{};
                sstr << "Successfully sent PLL with " << pllsSent << " messages.";
                wxMessageBox(wxString::FromUTF8(sstr.str()), wxT("PLL updated"), wxICON_INFORMATION);
            }
#if (false)
            for (const auto &medStatementPair : medicationStatements) {
                auto identifiers = medStatementPair.second->GetIdentifiers();
                auto pllIdIterator = identifiers.begin();
                std::string pllId{};
                while (pllIdIterator != identifiers.end()) {
                    std::string key = pllIdIterator->GetType().GetText();
                    std::transform(key.cbegin(), key.cend(), key.begin(), [] (char ch) {return std::tolower(ch); });
                    if (key == "pll") {
                        pllId = pllIdIterator->GetValue();
                        break;
                    }
                    ++pllIdIterator;
                }
                if (pllIdIterator != identifiers.end()) {
                    if (pllMedicationStatements.find(pllId) == pllMedicationStatements.end()) {
                        for (const auto &removeId : removePllIds) {
                            if (pllId == removeId) {
                                identifiers.erase(pllIdIterator);
                                medStatementPair.second->SetIdentifiers(identifiers);
                                break;
                            }
                        }
                    } else {
                        identifiers.erase(pllIdIterator);
                        medStatementPair.second->SetIdentifiers(identifiers);
                    }
                }
            }
#endif
        }
    });
}

void TheMasterFrame::OnSaveLastRequest(wxCommandEvent &e) {
    wxFileDialog saveFileDialog(this, _("Save last request"), "", "", "Json files (*.json)|*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return; // the user changed their mind
    std::ofstream ofs{saveFileDialog.GetPath().ToStdString()};
    ofs << lastRequest;
    ofs.close();
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

FhirReference TheMasterFrame::GetSubjectRef() const {
    std::string pid{};
    auto patientInformation = this->patientInformation;
    if (patientInformation && patientInformation->GetPatientIdType() == PatientIdType::FODSELSNUMMER) {
        pid = patientInformation->GetPatientId();
    }
    auto bundle = medicationBundle->medBundle;
    if (!bundle) {
        return {};
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
                    return FhirReference(entry.GetFullUrl(), "http://ehelse.no/fhir/StructureDefinition/sfm-Patient", patient->GetDisplay());
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
    return FhirReference(fullUrl, "http://ehelse.no/fhir/StructureDefinition/sfm-Patient", patient->GetDisplay());
}

void TheMasterFrame::SetPatient(PrescriptionData &prescriptionData) const {
    auto ref = GetSubjectRef();
    if (!ref.IsSet()) {
        return;
    }
    prescriptionData.subjectReference = ref.GetReference();
    prescriptionData.subjectDisplay = ref.GetDisplay();
}

void TheMasterFrame::PrescribeMedicament(const PrescriptionDialog &prescriptionDialog) {
    PrescriptionData prescriptionData = prescriptionDialog.GetPrescriptionData();
    std::shared_ptr<FhirMedication> medicament = prescriptionDialog.GetMedication();
    SetPrescriber(prescriptionData);
    SetPatient(prescriptionData);
    std::string medicamentFullUrl{"urn:uuid:"};
    medicamentFullUrl.append(medicament->GetId());
    FhirBundleEntry medicamentEntry{medicamentFullUrl, medicament};
    std::shared_ptr<FhirMedicationStatement> medicationStatement = std::make_shared<FhirMedicationStatement>(prescriptionData.ToFhir());
    {
        auto medicationProfile = medicament->GetProfile();
        FhirReference medicationReference{medicamentFullUrl, medicationProfile.size() == 1 ? medicationProfile[0] : "", medicament->GetDisplay()};
        medicationStatement->SetMedicationReference(medicationReference);
    }
    std::string medicationStatementFullUrl{"urn:uuid:"};
    medicationStatementFullUrl.append(medicationStatement->GetId());
    FhirBundleEntry medicationStatementEntry{medicationStatementFullUrl, medicationStatement};
    medicationBundle->medBundle->AddEntry(medicamentEntry);
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
    UpdateMedications();
}

void TheMasterFrame::OnPrescribeMagistral(wxCommandEvent &e) {
    MagistralBuilderDialog magistralBuilderDialog{this};
    if (magistralBuilderDialog.ShowModal() == wxID_OK) {
        PrescriptionDialog prescriptionDialog{this, std::make_shared<FhirMedication>(magistralBuilderDialog.GetMagistralMedicament().ToFhir()), {}, true};
        auto res = prescriptionDialog.ShowModal();
        if (res != wxID_OK) {
            return;
        }
        PrescribeMedicament(prescriptionDialog);
    }
}

void TheMasterFrame::OnPrescribeMedicament(wxCommandEvent &e) {
    FindMedicamentDialog findMedicamentDialog{this};
    if (!findMedicamentDialog.CanOpen()) {
        wxMessageBox(wxT("Prescribe medicament requires a FEST DB. Please update FEST and try again."), wxT("No FEST DB"), wxICON_ERROR);
        return;
    }
    findMedicamentDialog.ShowModal();
    auto medicament = findMedicamentDialog.GetSelected();
    if (medicament) {
        std::shared_ptr<FestDb> festDb = std::make_shared<FestDb>();
        if (!festDb->IsOpen()) {
            return;
        }
        SfmMedicamentMapper medicamentMapper{festDb, medicament};
        std::vector<MedicamentPackage> packages{};
        {
            auto packageMappers = medicamentMapper.GetPackages();
            packages.reserve(packageMappers.size());
            for (const auto &packageMapper : packageMappers) {
                auto description = packageMapper.GetPackageDescription();
                packages.emplace_back(std::make_shared<FhirMedication>(packageMapper.GetMedication()), description);
            }
        }
        PrescriptionDialog prescriptionDialog{this, std::make_shared<FhirMedication>(medicamentMapper.GetMedication()), medicamentMapper.GetPrescriptionUnit(), medicamentMapper.IsPackage(), packages};
        auto res = prescriptionDialog.ShowModal();
        if (res != wxID_OK) {
            return;
        }
        PrescribeMedicament(prescriptionDialog);
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

void TheMasterFrame::OnUpdateFest(wxCommandEvent &e) {
    std::shared_ptr<FestDbUi> festDbUi = std::make_shared<FestDbUi>(this);
    festDbUi->Update();
}

void TheMasterFrame::OnShowFestVersions(wxCommandEvent &e) {
    FestVersionsDialog festVersionsDialog{this};
    festVersionsDialog.ShowModal();
}

void TheMasterFrame::OnShowFestDbQuotas(wxCommandEvent &e) {
    FestDbQuotasDialog dialog{this};
    dialog.ShowModal();
}

void TheMasterFrame::OnPrescriptionContextMenu(wxContextMenuEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatement = displayedMedicationStatements[selected];
    auto display = medicationStatement->GetDisplay();
    wxMenu menu(wxString::FromUTF8(display));
    menu.Append(TheMaster_PrescriptionDetails_Id, wxT("Details"));
    menu.Append(TheMaster_PrescriptionRecall_Id, wxT("Recall"));
    menu.Append(TheMaster_PrescriptionCease_Id, wxT("Cease"));
    PopupMenu(&menu);
}

void TheMasterFrame::OnPrescriptionDetails(wxCommandEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatement = displayedMedicationStatements[selected];
    PrescriptionDetailsDialog dialog{this, medicationStatement};
    dialog.ShowModal();
}

void TheMasterFrame::OnPrescriptionRecall(wxCommandEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatement = displayedMedicationStatements[selected];
    std::shared_ptr<FhirExtension> reseptAmendment{};
    {
        auto extensions = medicationStatement->GetExtensions();
        for (const auto extension : extensions) {
            auto url = extension->GetUrl();
            if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
                reseptAmendment = extension;
            }
        }
    }
    if (!reseptAmendment) {
        wxMessageBox(wxT("The prescription is not valid"), wxT("Can not recall prescription"), wxICON_ERROR);
        return;
    }
    {
        auto extensions = reseptAmendment->GetExtensions();
        for (const auto &extension : extensions) {
            auto url = extension->GetUrl();
            if (url == "recallinfo") {
                wxMessageBox(wxT("The prescription is already recalled"), wxT("Can not recall prescription"), wxICON_ERROR);
                return;
            }
        }
    }
    RecallPrescriptionDialog dialog{this, medicationStatement};
    {
        auto res = dialog.ShowModal();
        if (res != wxID_OK) {
            return;
        }
    }
    {
        auto recallCode = std::make_shared<FhirCodeableConceptValue>(dialog.GetReason().ToCodeableConcept());
        auto recallInfoExt = std::make_shared<FhirExtension>("recallinfo");
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("recallcode", recallCode));
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("notsent", std::make_shared<FhirBooleanValue>(true)));
        reseptAmendment->AddExtension(recallInfoExt);
    }
    UpdateMedications();
}

void TheMasterFrame::OnPrescriptionCease(wxCommandEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatement = displayedMedicationStatements[selected];
    std::shared_ptr<FhirExtension> reseptAmendment{};
    {
        auto extensions = medicationStatement->GetExtensions();
        for (const auto extension : extensions) {
            auto url = extension->GetUrl();
            if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
                reseptAmendment = extension;
            }
        }
    }
    if (!reseptAmendment) {
        wxMessageBox(wxT("The prescription is not valid"), wxT("Can not recall prescription"), wxICON_ERROR);
        return;
    }
    {
        auto extensions = reseptAmendment->GetExtensions();
        for (const auto &extension : extensions) {
            auto url = extension->GetUrl();
            if (url == "recallinfo") {
                wxMessageBox(wxT("The prescription is already recalled"), wxT("Can not recall prescription"), wxICON_ERROR);
                return;
            }
        }
    }
    CeasePrescriptionDialog dialog{this, medicationStatement};
    {
        auto res = dialog.ShowModal();
        if (res != wxID_OK) {
            return;
        }
    }
    {
        auto recallCode = std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept("urn:oid:2.16.578.1.12.4.1.1.7500", "2", "Seponering"));
        auto recallInfoExt = std::make_shared<FhirExtension>("recallinfo");
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("recallcode", recallCode));
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("notsent", std::make_shared<FhirBooleanValue>(true)));
        reseptAmendment->AddExtension(recallInfoExt);
    }
    {
        auto discontinuation = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-discontinuation");
        {
            {
                auto dt = DateTimeOffset::Now();
                auto timedate = std::make_shared<FhirValueExtension>("timedate", std::make_shared<FhirDateTimeValue>(dt.to_iso8601()));
                discontinuation->AddExtension(timedate);
            }
            {
                std::vector<FhirCoding> codings{};
                {
                    auto reason = dialog.GetReason();
                    std::string system{reason.GetSystem()};
                    std::string code{reason.GetCode()};
                    std::string display{reason.GetDisplay()};
                    codings.emplace_back(std::move(system), std::move(code), std::move(display));
                }
                std::string text{dialog.GetReasonText()};
                FhirCodeableConcept codeable{std::move(codings), std::move(text)};
                discontinuation->AddExtension(std::make_shared<FhirValueExtension>("reason", std::make_shared<FhirCodeableConceptValue>(codeable)));
            }
        }
        medicationStatement->AddExtension(discontinuation);
    }
    UpdateMedications();
}
