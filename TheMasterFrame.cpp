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
#include "GetLegemiddelKortdoser.h"
#include "GetMedicamentDosingUnit.h"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <cstdio>
#include <ctime>
#include <sfmbasisapi/fhir/bundleentry.h>
#include <sfmbasisapi/fhir/person.h>
#include <sfmbasisapi/fhir/parameters.h>
#include <sfmbasisapi/fhir/bundle.h>
#include <sfmbasisapi/fhir/value.h>
#include <sfmbasisapi/fhir/medstatement.h>
#include <sfmbasisapi/fhir/fhirbasic.h>
#include <sfmbasisapi/fhir/operationoutcome.h>
#include <sfmbasisapi/fhir/fhir.h>
#include <sfmbasisapi/fhir/allergy.h>
#include <jjwtid/Jwt.h>
#include <jjwtid/OidcTokenRequest.h>
#include <cpprest/http_client.h>
#include <wx/listctrl.h>
#include <wx/notebook.h>
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
#include "PrescriptionChangesService.h"
#include "EditTreatmentDialog.h"
#include "CaveDetailsDialog.h"
#include "RegisterCaveDialog.h"
#include "ConnectToPllDialog.h"
#include "win32/w32strings.h"

constexpr int PrescriptionNameColumnWidth = 250;
constexpr int PllColumnWidth = 50;
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
    auto *caveMenu = new wxMenu();
    caveMenu->Append(TheMaster_AddCaveMedicament_Id, "Add medicament");
    auto *medicationMenu = new wxMenu();
    medicationMenu->Append(TheMaster_PrescribeMagistral_Id, "Prescribe magistral");
    medicationMenu->Append(TheMaster_PrescribeMedicament_Id, "Prescribe medicament");
    auto *patientMenu = new wxMenu();
    patientMenu->Append(TheMaster_FindPatient_Id, "Find patient");
    patientMenu->Append(TheMaster_CreatePatient_Id, "Create patient");
    auto *serverMenu = new wxMenu();
    serverMenu->Append(TheMaster_Connect_Id, "Connect");
    serverMenu->Append(TheMaster_ResetMedication_Id, "Reset local changes");
    serverMenu->Append(TheMaster_GetMedication_Id, "Get medication");
    serverMenu->Append(TheMaster_SendMedication_Id, "Send medication");
    serverMenu->Append(TheMaster_SendPll_Id, "Send PLL");
    serverMenu->Append(TheMaster_SaveLastGetmedRequest_Id, "Save last \"getmed\" request");
    serverMenu->Append(TheMaster_SaveLastSendmedRequest_Id, "Save last \"sendmed\" request");
    serverMenu->Append(TheMaster_SaveLastGetmed_Id, "Save last \"getmed\" response");
    serverMenu->Append(TheMaster_SaveLastSendmed_Id, "Save last \"sendmed\" response");
    serverMenu->Append(TheMaster_SaveBundle_Id, "Save bundle");
    auto *menuBar = new wxMenuBar();
    menuBar->Append(serverMenu, "Server");
    menuBar->Append(patientMenu, "Patient");
    menuBar->Append(medicationMenu, "Medication");
    menuBar->Append(caveMenu, "CAVE");
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

    mainCategories = new wxNotebook(this, wxID_ANY);

    auto *medicationPage = new wxPanel(mainCategories, wxID_ANY);
    auto *medicationPageSizer = new wxBoxSizer(wxVERTICAL);
    prescriptions = new wxListView(medicationPage, wxID_ANY);
    prescriptions->AppendColumn(wxT("Name"));
    prescriptions->AppendColumn(wxT("Published"));
    prescriptions->SetColumnWidth(0, PrescriptionNameColumnWidth);
    prescriptions->SetColumnWidth(1, PrescriptionRemoteColumnWidth);
    prescriptions->Bind(wxEVT_CONTEXT_MENU, &TheMasterFrame::OnPrescriptionContextMenu, this, wxID_ANY);
    medicationPageSizer->Add(prescriptions, 1, wxEXPAND | wxALL, 5);
    medicationPage->SetSizerAndFit(medicationPageSizer);
    mainCategories->AddPage(medicationPage, wxT("Medication"));

    auto *cavePage = new wxPanel(mainCategories, wxID_ANY);
    auto *cavePageSizer = new wxBoxSizer(wxVERTICAL);
    caveListView = new wxListView(cavePage, wxID_ANY);
    caveListView->Bind(wxEVT_CONTEXT_MENU, &TheMasterFrame::OnCaveContextMenu, this, wxID_ANY);
    cavePageSizer->Add(caveListView, 1, wxEXPAND | wxALL, 5);
    cavePage->SetSizerAndFit(cavePageSizer);
    mainCategories->AddPage(cavePage, wxT("CAVE"));

    sizer->Add(mainCategories, 1, wxEXPAND | wxALL, 5);

    SetSizerAndFit(sizer);

    Bind(wxEVT_MENU, &TheMasterFrame::OnConnect, this, TheMaster_Connect_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnFindPatient, this, TheMaster_FindPatient_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnCreatePatient, this, TheMaster_CreatePatient_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnResetMedication, this, TheMaster_ResetMedication_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnGetMedication, this, TheMaster_GetMedication_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSendMedication, this, TheMaster_SendMedication_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSendPll, this, TheMaster_SendPll_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSaveLastGetmedRequest, this, TheMaster_SaveLastGetmedRequest_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSaveLastSendmedRequest, this, TheMaster_SaveLastSendmedRequest_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSaveLastGetmed, this, TheMaster_SaveLastGetmed_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSaveLastSendmed, this, TheMaster_SaveLastSendmed_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnSaveBundle, this, TheMaster_SaveBundle_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescribeMagistral, this, TheMaster_PrescribeMagistral_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescribeMedicament, this, TheMaster_PrescribeMedicament_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnUpdateFest, this, TheMaster_UpdateFest_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnShowFestVersions, this, TheMaster_ShowFestVersions_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnShowFestDbQuotas, this, TheMaster_ShowFestDbQuotas_Id);

    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescriptionDetails, this, TheMaster_PrescriptionDetails_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescriptionRecall, this, TheMaster_PrescriptionRecall_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescriptionCease, this, TheMaster_PrescriptionCease_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescriptionRenew, this, TheMaster_PrescriptionRenew_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnPrescriptionRenewWithChanges, this, TheMaster_PrescriptionRenewWithChanges_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnTreatmentEdit, this, TheMaster_TreatmentEdit_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnConnectToPll, this, TheMaster_ConnectToPll_Id);

    Bind(wxEVT_MENU, &TheMasterFrame::OnCaveDetails, this, TheMaster_CaveDetails_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnAddCaveMedicament, this, TheMaster_AddCaveMedicament_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnEditCaveMedicament, this, TheMaster_EditCaveMedicament_Id);
    Bind(wxEVT_MENU, &TheMasterFrame::OnDeleteCaveMedicament, this, TheMaster_DeleteCaveMedicament_Id);
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
    prescriptions->AppendColumn(wxT("PLL"));
    prescriptions->AppendColumn(wxT("Published"));
    prescriptions->SetColumnWidth(0, PrescriptionNameColumnWidth);
    prescriptions->SetColumnWidth(1, PllColumnWidth);
    prescriptions->SetColumnWidth(2, PrescriptionRemoteColumnWidth);
    std::vector<std::vector<std::shared_ptr<FhirMedicationStatement>>> displayedMedicationStatements{};
    auto pos = 0;
    std::map<std::string,std::shared_ptr<FhirMedicationStatement>> medicationStatementMap{};
    FhirCompositionSection medicationSection{};
    if (medicationBundle && medicationBundle->medBundle) {
        for (const auto &bundleEntry: medicationBundle->medBundle->GetEntries()) {
            auto resource = bundleEntry.GetResource();
            auto medicationStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(resource);
            if (medicationStatement) {
                medicationStatementMap.insert_or_assign(bundleEntry.GetFullUrl(), medicationStatement);
                continue;
            }
            auto composition = std::dynamic_pointer_cast<FhirComposition>(resource);
            if (composition) {
                auto sections = composition->GetSections();
                for (const auto &section: sections) {
                    auto coding = section.GetCode().GetCoding();
                    if (coding.size() == 1 && coding[0].GetCode() == "sectionMedication") {
                        medicationSection = section;
                        break;
                    }
                }
            }
        }
    }
    for (const auto &sectionEntry : medicationSection.GetEntries()) {
        auto medicationStatementIterator = medicationStatementMap.find(sectionEntry.GetReference());
        if (medicationStatementIterator == medicationStatementMap.end()) {
            continue;
        }
        auto medicationStatement = medicationStatementIterator->second;
        std::vector<std::shared_ptr<FhirMedicationStatement>> versions{};
        versions.emplace_back(medicationStatementIterator->second);
        auto basedOn = medicationStatement->GetBasedOn();
        while (!basedOn.empty()) {
            medicationStatement = {};
            for (const auto &bo : basedOn) {
                medicationStatementIterator = medicationStatementMap.find(bo.GetReference());
                if (medicationStatementIterator != medicationStatementMap.end()) {
                    medicationStatement = medicationStatementIterator->second;
                    break;
                }
            }
            if (!medicationStatement) {
                break;
            }
            versions.emplace_back(medicationStatement);
            basedOn = medicationStatement->GetBasedOn();
        }
        medicationStatement = versions[0];
        if (medicationStatement) {
            auto row = pos++;
            displayedMedicationStatements.emplace_back(versions);
            prescriptions->InsertItem(row, medicationStatement->GetMedicationReference().GetDisplay());
            auto statusInfo = PrescriptionChangesService::GetPrescriptionStatusInfo(*medicationStatement);
            if (statusInfo.IsPll) {
                if (versions.size() > 1) {
                    prescriptions->SetItem(row, 1, wxT("PLL*"));
                } else {
                    prescriptions->SetItem(row, 1, wxT("PLL"));
                }
            } else if (versions.size() > 1) {
                bool hidesPll{false};
                for (const auto &v : versions) {
                    auto statusInfo = PrescriptionChangesService::GetPrescriptionStatusInfo(*v);
                    if (statusInfo.IsPll) {
                        hidesPll = true;
                    }
                }
                if (hidesPll) {
                    prescriptions->SetItem(row, 1, wxT("(PLL)"));
                } else {
                    prescriptions->SetItem(row, 1, wxT("*"));
                }
            } else {
                prescriptions->SetItem(row, 1, wxT(""));
            }
            auto statusStr = PrescriptionChangesService::GetPrescriptionStatusString(statusInfo);
            prescriptions->SetItem(row, 2, wxString::FromUTF8(statusStr));
        }
    }
    this->displayedMedicationStatements = displayedMedicationStatements;
}

void TheMasterFrame::UpdateCave() {
    caveListView->ClearAll();
    displayedAllergies.clear();
    if (!medicationBundle || !medicationBundle->medBundle) {
        return;
    }
    std::map<std::string,std::shared_ptr<FhirAllergyIntolerance>> allergies{};
    FhirCompositionSection allergySection{};
    {
        std::shared_ptr<FhirComposition> composition{};
        for (const auto &bundleEntry: medicationBundle->medBundle->GetEntries()) {
            auto resource = bundleEntry.GetResource();
            auto allergyObj = std::dynamic_pointer_cast<FhirAllergyIntolerance>(resource);
            if (allergyObj) {
                allergies.insert_or_assign(bundleEntry.GetFullUrl(), allergyObj);
                continue;
            }
            auto compositionObj = std::dynamic_pointer_cast<FhirComposition>(resource);
            if (compositionObj) {
                if (composition) {
                    return;
                }
                composition = compositionObj;
            }
        }
        if (!composition) {
            return;
        }
        for (const auto &section : composition->GetSections()) {
            auto codings = section.GetCode().GetCoding();
            if (codings.size() == 1 && codings[0].GetCode() == "sectionAllergies") {
                if (allergySection.GetCode().IsSet()) {
                    return;
                }
                allergySection = section;
            }
        }
        if (!allergySection.GetCode().IsSet()) {
            return;
        }
    }
    caveListView->AppendColumn(wxT("Display"));
    caveListView->SetColumnWidth(0, 300);
    int row = 0;
    for (const auto &reference : allergySection.GetEntries()) {
        std::shared_ptr<FhirAllergyIntolerance> allergy{};
        {
            auto iterator = allergies.find(reference.GetReference());
            if (iterator != allergies.end()) {
                allergy = iterator->second;
            }
        }
        if (!allergy) {
            continue;
        }
        FhirCoding coding{};
        {
            auto codings = allergy->GetCode().GetCoding();
            if (codings.size() == 1) {
                coding = codings[0];
            }
        }
        std::string display{coding.GetDisplay()};
        if (display.empty()) {
            display = allergy->GetCode().GetText();
        }
        displayedAllergies.emplace_back(allergy);
        caveListView->InsertItem(row++, wxString::FromUTF8(display));
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
        medicationBundleResetData = "";
        medicationBundle = {};
        UpdateHeader();
        UpdateMedications();
        UpdateCave();
    }
}

void TheMasterFrame::OnCreatePatient(wxCommandEvent &e) {
    CreatePatientDialog dialog{patientStore, this};
    if (dialog.ShowModal() == wxID_OK) {
        auto patient = dialog.GetPatientInformation();
        patientStore->AddPatient(patient);
        patientInformation = std::make_shared<PatientInformation>(std::move(patient));
        medicationBundleResetData = "";
        medicationBundle = {};
        UpdateHeader();
        UpdateMedications();
        UpdateCave();
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
        if (!journalId.empty()) {
            tokenRequest.AddHelseIdJournalId(journalId);
        }
        if (!orgNo.empty()) {
            tokenRequest.AddHelseIdConsumerOrgNo(orgNo);
        }
        if (!childOrgNo.empty()) {
            tokenRequest.AddHelseIdConsumerChildOrgNo(childOrgNo);
        }
        auto requestData = tokenRequest.GetTokenRequest();
        web::http::client::http_client client{as_wstring_on_win32(helseidUrl)};
        web::http::http_request req{web::http::methods::POST};
        req.set_request_uri(as_wstring_on_win32("/connect/token"));
        {
            std::string rqBody{};
            {
                std::stringstream sstr{};
                auto iterator = requestData.params.begin();
                if (iterator != requestData.params.end()) {
                    const auto &param = *iterator;
                    sstr << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(param.first))) << "=";
                    sstr << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(param.second)));
                    ++iterator;
                }
                while (iterator != requestData.params.end()) {
                    const auto &param = *iterator;
                    sstr << "&" << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(param.first))) << "=";
                    sstr << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(param.second)));
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
                    if (json.has_string_field(as_wstring_on_win32("access_token"))) {
                        *at = from_wstring_on_win32(json.at(as_wstring_on_win32("access_token")).as_string());
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

class CallContext {
private:
    std::mutex mtx{};
    std::function<void ()> finish{[] () {}};
    bool finished{false};
public:
    CallContext() = default;
    CallContext(const CallContext &) = delete;
    CallContext(CallContext &&) = delete;
    CallContext &operator =(const CallContext &) = delete;
    CallContext &operator =(CallContext &&) = delete;
    void SetFinish(const std::function<void ()> &);
    void Finish();
    ~CallContext();
};

void CallContext::SetFinish(const std::function<void()> &func) {
    std::lock_guard lock{mtx};
    finish = func;
}

void CallContext::Finish() {
    std::function<void()> finish{};
    bool finished;
    {
        std::lock_guard lock{mtx};
        finished = this->finished;
        finish = this->finish;
        this->finished = true;
    }
    if (!finished) {
        finish();
    }
}

CallContext::~CallContext() {
    bool finished;
    {
        finished = this->finished;
        this->finished = true;
    }
    if (!finished) {
        finish();
    }
}

class CallGuard {
private:
    std::mutex mtx{};
    std::function<void (const std::string &)> func{};
    bool called{false};
public:
    CallGuard() = delete;
    explicit CallGuard(const std::function<void (const std::string &)> &func) : func(func) {}
    CallGuard(const CallGuard &) = delete;
    CallGuard(CallGuard &&) = delete;
    CallGuard operator = (const CallGuard &) = delete;
    CallGuard operator = (CallGuard &&) = delete;
    ~CallGuard() {
        if (!called) {
            auto f = func;
            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([f]() {
                wxMessageBox(wxT("Completion not called"), wxT("Callback error"), wxICON_ERROR);
                f("");
            });
        }
    }
    void Call(const std::string &err) {
        bool callIt{false};
        {
            std::lock_guard lock{mtx};
            callIt = !called;
            called = true;
        }
        if (callIt) {
            auto f = func;
            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([f, err]() {
                f(err);
            });
        } else {
            wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([]() {
                wxMessageBox(wxT("Completion called repeatedly"), wxT("Callback error"), wxICON_ERROR);
            });
        }
    }
};

class SyncGetMedError : public std::exception {
private:
    std::string error;
public:
    SyncGetMedError(const std::string &error) : error(error) {}
    const char * what() const noexcept override;
};

const char *SyncGetMedError::what() const noexcept {
    return error.c_str();
}

class SyncSendMedError : public std::exception {
private:
    std::string error;
public:
    SyncSendMedError(const std::string &error) : error(error) {}
    const char * what() const noexcept override;
};

const char *SyncSendMedError::what() const noexcept {
    return error.c_str();
}

void TheMasterFrame::GetMedication(CallContext &ctx, const std::function<void(const std::string &)> &callbackF) {
    auto patientInformation = this->patientInformation;
    if (!patientInformation) {
        throw SyncGetMedError("Error: No patient information provided");
    }
    std::string apiUrl = url;
    if (apiUrl.empty()) {
        throw SyncGetMedError("Error: Not connected");
    }

    auto callback = std::make_shared<CallGuard>(callbackF);
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
            auto patientIdType = patientInformation->GetPatientIdType();
            if (patientIdType == PatientIdType::FODSELSNUMMER) {
                identifiers.emplace_back(
                        FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/FNR", "FNR-nummer", ""),
                        "official", "urn:oid:2.16.578.1.12.4.1.4.1",
                        patientInformation->GetPatientId());
                patient->SetIdentifiers(identifiers);
            } else if (patientIdType == PatientIdType::DNUMMER) {
                identifiers.emplace_back(
                        FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/DNR", "Dnummer", ""),
                        "official", "urn:oid:2.16.578.1.12.4.1.4.2",
                        patientInformation->GetPatientId());
                patient->SetIdentifiers(identifiers);
            }
        }
        patient->SetActive(true);
        patient->SetName({{"official", patientInformation->GetFamilyName(),
                           patientInformation->GetGivenName()}});
        {
            auto dob = patientInformation->GetDateOfBirth();
            if (!dob.empty()) {
                patient->SetBirthDate(dob);
            }
        }
        patient->SetGender(
                patientInformation->GetGender() == PersonGender::FEMALE ? "female" : "male");
        auto postCode = patientInformation->GetPostCode();
        auto city = patientInformation->GetCity();
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
            request.headers().add(as_wstring_on_win32("Authorization"), as_wstring_on_win32(bearer));
        }
        {
            web::json::value requestBody{};
            {
                FhirParameters requestParameters{};
                requestParameters.AddParameter("patient", patient);
                requestBody = requestParameters.ToJson();
            }
            request.set_request_uri(as_wstring_on_win32("patient/$getMedication"));
            {
                auto jsonString = requestBody.serialize();
                request.set_body(jsonString, as_wstring_on_win32("application/fhir+json; charset=utf-8"));
                *rawRequest = from_wstring_on_win32(jsonString);
            }
        }
        web::http::client::http_client client{as_wstring_on_win32(apiUrl)};
        return client.request(request);
    });

    std::shared_ptr<std::mutex> getMedResponseMtx = std::make_shared<std::mutex>();
    std::shared_ptr<std::unique_ptr<FhirParameters>> getMedResponse = std::make_shared<std::unique_ptr<FhirParameters>>();
    std::shared_ptr<std::string> rawResponse = std::make_shared<std::string>();

    ctx.SetFinish([this, patientInformation, getMedResponseMtx, getMedResponse, rawRequest, rawResponse] () {
        std::unique_ptr<FhirParameters> getMedResp{};
        {
            std::lock_guard lock{*getMedResponseMtx};
            getMedResp = std::move(*getMedResponse);
            lastGetmedRequest = *rawRequest;
            lastGetmedResponse = *rawResponse;
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
                std::shared_ptr<FhirBundle> previousBundle{};
                if (medicationBundle && medicationBundle->patientInformation == *patientInformation) {
                    previousBundle = medicationBundle->medBundle;
                }
                medicationBundleResetData = from_wstring_on_win32(medBundle->ToJson().serialize());
                medicationBundle = std::make_unique<MedBundleData>();
                *medicationBundle = {
                        .patientInformation = *patientInformation,
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
                if (previousBundle) {
                    medicationBundle->InsertNonexistingMedicationsFrom(previousBundle);
                    medicationBundle->InsertNonexistingMedicationPrescriptionsFrom(previousBundle, helseidIdToken);
                    medicationBundle->ReplayRenewals(previousBundle);
                }
                UpdateHeader();
                UpdateMedications();
                UpdateCave();
            }
        }
    });

    responseTask.then([callback, getMedResponse, getMedResponseMtx, rawResponse] (const pplx::task<web::http::http_response> &responseTask) {
        try {
            auto response = responseTask.get();
            try {
                auto contentType = from_wstring_on_win32(response.headers().content_type());
                if (!contentType.starts_with("application/fhir+json") && contentType != "application/json") {
                    std::string msg{"Wrong content type in response: "};
                    msg.append(contentType);
                    callback->Call(msg);
                } else {
                    response.extract_json(true).then([callback, getMedResponse, getMedResponseMtx, rawResponse](
                            const pplx::task<web::json::value> &responseBodyTask) {
                        try {
                            auto responseBody = responseBodyTask.get();
                            try {
                                {
                                    auto responseBodyStr = from_wstring_on_win32(responseBody.serialize());
                                    {
                                        std::lock_guard lock{*getMedResponseMtx};
                                        *rawResponse = responseBodyStr;
                                    }
                                }
                                FhirParameters responseParameterBundle = FhirParameters::Parse(responseBody);
                                {
                                    std::lock_guard lock{*getMedResponseMtx};
                                    *getMedResponse = std::make_unique<FhirParameters>(
                                            std::move(responseParameterBundle));
                                }
                                callback->Call("");
                            } catch (std::exception &e) {
                                std::string error = e.what();
                                std::string msg{"Error: std::exception: "};
                                msg.append(error);
                                callback->Call(msg);
                            } catch (...) {
                                callback->Call("Error: Decoding failed");
                            }
                        } catch (std::exception &e) {
                            std::string error = e.what();
                            std::string msg{"Error: std::exception: "};
                            msg.append(error);
                            callback->Call(msg);
                        } catch (...) {
                            callback->Call("Error: Downloading failed");
                        }
                    });
                }
            } catch (...) {
                callback->Call("Error: Downloading failed");
            }
        } catch (...) {
            callback->Call("Error: Get medications request failed");
        }
    });
}

void TheMasterFrame::OnResetMedication(wxCommandEvent &e) {
    if (medicationBundleResetData.empty() || !medicationBundle || !medicationBundle->medBundle) {
        wxMessageBox(wxT("Please run 'get medication' before reset local changes"), wxT("Run get medication"), wxICON_ERROR);
        return;
    }
    *(medicationBundle->medBundle) = FhirBundle::Parse(web::json::value::parse(medicationBundleResetData));
    UpdateHeader();
    UpdateMedications();
    UpdateCave();
}

void TheMasterFrame::OnGetMedication(wxCommandEvent &e) {
    std::shared_ptr<WaitingForApiDialog> waitingDialog = std::make_shared<WaitingForApiDialog>(this, "Retrieving medication records", "Requested medication bundle...");
    CallContext ctx{};
    try {
        GetMedication(ctx, [waitingDialog](const std::string &err) {
            if (!err.empty()) {
                wxMessageBox(wxString::FromUTF8(err), wxT("Get medication error"), wxICON_ERROR);
            }
            waitingDialog->Close();
        });
    } catch (SyncGetMedError &e) {
        wxMessageBox(wxString::FromUTF8(e.what()), wxT("Get medication error"), wxICON_ERROR);
        return;
    }
    waitingDialog->ShowModal();
    ctx.Finish();
}

void TheMasterFrame::FilterRecallInfos(FhirBundle &bundle, const std::function<bool(const std::string &,
                                                                                    const std::shared_ptr<FhirExtension> &)> &predicate) {
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
                auto iterator = extensions.begin();
                while (iterator != extensions.end()) {
                    const auto &extension = *iterator;
                    auto url = extension->GetUrl();
                    bool keep{true};
                    if (url == "recallinfo") {
                        keep = predicate(reseptId, extension);
                    }
                    if (keep) {
                        ++iterator;
                    } else {
                        iterator = extensions.erase(iterator);
                    }
                }
                reseptAmendment->SetExtensions(extensions);
            }
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

void TheMasterFrame::SendMedication(CallContext &ctx,
                                    const std::function<void(const std::shared_ptr<FhirBundle> &)> &preprocessing,
                                    const std::function<void(const std::map<std::string, FhirCoding> &)> &pllResultsFunc,
                                    const std::function<void (const std::string &err)> &callbackF) {
    if (!patientInformation) {
        throw SyncSendMedError("Error: No patient information provided");
    }
    if (url.empty()) {
        throw SyncSendMedError("Error: Not connected");
    }
    std::string apiUrl = url;
    std::shared_ptr<FhirBundle> bundle{};
    if (medicationBundle) {
        bundle = medicationBundle->medBundle;
    }
    if (!bundle) {
        throw SyncSendMedError("Error: Get medications first please");
    }

    pplx::task<web::http::http_response> responseTask{};
    auto accessTokenTask = GetAccessToken();
    std::shared_ptr<std::string> rawRequest = std::make_shared<std::string>();
    medicationBundleResetData = "";
    responseTask = accessTokenTask.then([apiUrl, bundle, preprocessing, rawRequest] (const std::string &accessToken) {
        web::http::http_request request{web::http::methods::POST};
        if (!accessToken.empty()) {
            std::cout << "Access token: " << accessToken << "\n";
            std::string bearer{"Bearer "};
            bearer.append(accessToken);
            request.headers().add(as_wstring_on_win32("Authorization"), as_wstring_on_win32(bearer));
        }
        request.set_request_uri(as_wstring_on_win32("Patient/$sendMedication"));
        {
            FhirParameters sendMedicationParameters{};
            {
                std::shared_ptr<FhirBundle> medBundle = std::make_shared<FhirBundle>();
                *medBundle = FhirBundle::Parse(bundle->ToJson());
                FilterRecallInfos(*medBundle, [] (const auto &prescriptionId, const auto &recallInfo) -> bool {
                    bool keep{false};
                    auto extensions = recallInfo->GetExtensions();
                    for (const auto &extension : extensions) {
                        auto url = extension->GetUrl();
                        if (url == "notsent") {
                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            if (valueExtension) {
                                auto value = std::dynamic_pointer_cast<FhirBooleanValue>(valueExtension->GetValue());
                                if (value && value->IsTrue()) {
                                    keep = true;
                                }
                            }
                        }
                    }
                    return keep;
                });

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
                                boost::uuids::random_generator generator;
                                boost::uuids::uuid randomUUID = generator();
                                std::string uuidStr = boost::uuids::to_string(randomUUID);
                                FhirIdentifier identifier{"official", uuidStr};
                                composition->SetIdentifier(identifier);
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
                request.set_body(jsonString, as_wstring_on_win32("application/fhir+json; charset=utf-8"));
                *rawRequest = from_wstring_on_win32(jsonString);
            }
        }
        web::http::client::http_client client{as_wstring_on_win32(apiUrl)};
        return client.request(request);
    });
    std::shared_ptr<std::mutex> sendMedResponseMtx = std::make_shared<std::mutex>();
    std::shared_ptr<std::shared_ptr<Fhir>> sendMedResponse = std::make_shared<std::shared_ptr<Fhir>>();
    std::shared_ptr<std::string> rawResponse = std::make_shared<std::string>();
    ctx.SetFinish([this, bundle, sendMedResponseMtx, sendMedResponse, rawRequest, rawResponse, pllResultsFunc] () -> void {
        std::shared_ptr<FhirOperationOutcome> opOutcome{};
        std::shared_ptr<FhirParameters> sendMedResp{};
        {
            std::lock_guard lock{*sendMedResponseMtx};
            opOutcome = std::dynamic_pointer_cast<FhirOperationOutcome>(*sendMedResponse);
            sendMedResp = std::dynamic_pointer_cast<FhirParameters>(*sendMedResponse);
            lastSendmedRequest = std::move(*rawRequest);
            lastSendmedResponse = std::move(*rawResponse);
        }
        if (!sendMedResp) {
            if (opOutcome) {
                std::stringstream str{};
                auto issues = opOutcome->GetIssue();
                auto iterator = issues.begin();
                while (iterator != issues.end()) {
                    const auto &issue = *iterator;
                    str << issue.GetSeverity() << ": " << issue.GetDiagnostics() << " (" << issue.GetCode() << ")";
                    ++iterator;
                    if (iterator != issues.end()) {
                        str << "\n";
                    }
                }
                wxMessageBox(wxString::FromUTF8(str.str()), wxT("Error"), wxICON_ERROR);
            } else {
                wxMessageBox(wxT("Error: Server did not respond properly to sending medications."), wxT("Error"), wxICON_ERROR);
            }
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
                        FilterRecallInfos(*bundle, [&reseptId] (const auto &recallPrescriptionId, const auto &recallInfo) -> bool {
                            if (reseptId == recallPrescriptionId) {
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
                            } else {
                                auto extensions = recallInfo->GetExtensions();
                                bool idMatch{false};
                                bool notSent{false};
                                for (const auto &extension : extensions) {
                                    auto url = extension->GetUrl();
                                    std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                                    if (url == "recallid") {
                                        auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                        if (valueExt) {
                                            auto value = std::dynamic_pointer_cast<FhirString>(valueExt->GetValue());
                                            if (value) {
                                                if (value->GetValue() == reseptId) {
                                                    idMatch = true;
                                                }
                                            }
                                        }
                                    }
                                    if (url == "notsent") {
                                        notSent = true;
                                    }
                                }
                                if (idMatch && notSent) {
                                    return false;
                                }
                            }
                            return true;
                        });
                    } else {
                        std::string display = resultCode.GetDisplay();
                        wxMessageBox(wxString::FromUTF8(display), wxT("Prescription sending error"), wxICON_ERROR);
                    }
                }
            }
        }
        pllResultsFunc(pllResults);
        UpdateMedications();
        UpdateCave();
        std::stringstream str{};
        str << "Recalled " << recallCount << (recallCount == 1 ? " prescription and prescribed " : " prescriptions and prescribed ")
            << prescriptionCount << (prescriptionCount == 1 ? " prescription." : " prescriptions.");
        wxMessageBox(str.str(), wxT("Successful sending"), wxICON_INFORMATION);
    });
    auto callback = std::make_shared<CallGuard>(callbackF);
    responseTask.then([callback, sendMedResponse, rawResponse, sendMedResponseMtx] (const pplx::task<web::http::http_response> &responseTask) {
        try {
            auto response = responseTask.get();
            try {
                auto contentType = from_wstring_on_win32(response.headers().content_type());
                if (!contentType.starts_with("application/fhir+json")) {
                    std::string msg{"Wrong content type in response: "};
                    msg.append(contentType);
                    callback->Call(msg);
                } else {
                    response.extract_json(true).then([callback, sendMedResponse, rawResponse, sendMedResponseMtx](
                            const pplx::task<web::json::value> &responseBodyTask) {
                        try {
                            auto responseBody = responseBodyTask.get();
                            try {
                                {
                                    std::lock_guard lock{*sendMedResponseMtx};
                                    *rawResponse = from_wstring_on_win32(responseBody.serialize());
                                }
                                std::shared_ptr<Fhir> responseParameterBundle = Fhir::Parse(responseBody);
                                {
                                    std::lock_guard lock{*sendMedResponseMtx};
                                    *sendMedResponse = responseParameterBundle;
                                }
                                callback->Call("");
                            } catch (std::exception &e) {
                                std::string error = e.what();
                                std::string msg{"Error: std::exception: "};
                                msg.append(error);
                                callback->Call(msg);
                            } catch (...) {
                                callback->Call("Error: Decoding failed");
                            }
                        } catch (std::exception &e) {
                            std::string error = e.what();
                            std::string msg{"Error: std::exception: "};
                            msg.append(error);
                            callback->Call(msg);
                        } catch (...) {
                            callback->Call("Error: Send medication failed");
                        }
                    });
                }
            } catch (...) {
                callback->Call("Error: Send medication failed");
            }
        } catch (...) {
            callback->Call("Error: Send medications failed");
        }
    });
}

void TheMasterFrame::SendMedication(const std::function<void (const std::shared_ptr<FhirBundle> &)> &preprocessing, const std::function<void (const std::map<std::string,FhirCoding> &)> &pllResultsFunc) {
    std::shared_ptr<WaitingForApiDialog> waitingDialog = std::make_shared<WaitingForApiDialog>(this, "Sending medication records", "Sending medication bundle...");
    CallContext ctx{};
    try {
        SendMedication(ctx, preprocessing, pllResultsFunc, [waitingDialog] (const std::string &err) {
            if (!err.empty()) {
                wxMessageBox(wxString::FromUTF8(err), wxT("Send medication error"), wxICON_ERROR);
            }
            waitingDialog->Close();
        });
    } catch (SyncSendMedError &e) {
        wxMessageBox(wxString::FromUTF8(e.what()), wxT("Send medication error"), wxICON_ERROR);
        return;
    }
    waitingDialog->ShowModal();
    ctx.Finish();
}

void TheMasterFrame::OnSendMedication(wxCommandEvent &e) {
    SendMedication(
            [] (const std::shared_ptr<FhirBundle> &) {},
            [] (const std::map<std::string,FhirCoding> &) {}
    );
}

#define SENDPLL_UPDATE_RENEWAL_SETID
#define SENDPLL_UPDATE_RENEWAL_REMOVEID

void TheMasterFrame::OnSendPll(wxCommandEvent &e) {
    std::vector<std::string> selected{};
    std::vector<std::string> heads{};
    std::vector<std::string> headIsPll{};
    std::vector<std::string> headIsAheadOfPll{};
    std::vector<std::vector<std::string>> medicationStatementRefChains{};
    std::map<std::string,std::string> headMap{};
    std::map<std::string,std::shared_ptr<FhirMedicationStatement>> medicationStatements{};
    std::map<std::string,std::shared_ptr<FhirMedicationStatement>> pllMedicationStatements{};
    std::vector<std::string> newPllIds{};
    {
        if (!medicationBundle) {
            return;
        }
        auto bundle = medicationBundle->medBundle;
        if (!bundle) {
            return;
        }
        {
            std::shared_ptr<FhirComposition> composition{};
            auto bundleEntries = bundle->GetEntries();
            for (const auto &bundleEntry : bundleEntries) {
                auto url = bundleEntry.GetFullUrl();
                auto resource = bundleEntry.GetResource();
                std::shared_ptr<FhirMedicationStatement> medStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(resource);
                if (medStatement) {
                    medicationStatements.insert_or_assign(url, medStatement);
                    continue;
                }
                std::shared_ptr<FhirComposition> comp = std::dynamic_pointer_cast<FhirComposition>(resource);
                if (comp) {
                    composition = comp;
                }
            }
            std::vector<FhirReference> medicationEntries{};
            if (composition) {
                auto sections = composition->GetSections();
                for (const auto &section : sections) {
                    std::string code{};
                    {
                        auto codings = section.GetCode().GetCoding();
                        if (!codings.empty()) {
                            code = codings[0].GetCode();
                        }
                    }
                    std::transform(code.cbegin(), code.cend(), code.begin(), [] (char ch) { return std::tolower(ch); });
                    if (code == "sectionmedication") {
                        medicationEntries = section.GetEntries();
                    }
                }
            }
            for (const auto &entry : medicationEntries) {
                std::shared_ptr<FhirMedicationStatement> medicationStatement{};
                std::string firstRef{};
                std::string ref{};
                bool headIsPllValue{false};
                {
                    auto iterator = medicationStatements.find(entry.GetReference());
                    if (iterator == medicationStatements.end()) {
                        continue;
                    }
                    medicationStatement = iterator->second;
                    ref = iterator->first;
                    firstRef = ref;
                    heads.emplace_back(ref);
                    auto firstStatementIdentifiers = medicationStatement->GetIdentifiers();
                    if (std::find_if(firstStatementIdentifiers.cbegin(), firstStatementIdentifiers.cend(), [] (const FhirIdentifier &identifier) -> bool{
                        auto type = identifier.GetType().GetText();
                        std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) { return std::tolower(ch); });
                        return type == "pll" && !identifier.GetValue().empty();
                    }) != firstStatementIdentifiers.cend()) {
                        headIsPll.emplace_back(ref);
                        headIsPllValue = true;
                    }
                }
                std::vector<std::string> chain{};
                do {
                    chain.emplace_back(ref);
                    auto basedOn = medicationStatement->GetBasedOn();
                    if (basedOn.empty()) {
                        break;
                    }
                    ref = basedOn[0].GetReference();
                    headMap.insert_or_assign(ref, firstRef);
                    auto iterator = medicationStatements.find(ref);
                    if (iterator == medicationStatements.end()) {
                        break;
                    }
                    medicationStatement = iterator->second;
                    if (!headIsPllValue) {
                        auto statementIdentifiers = medicationStatement->GetIdentifiers();
                        if (std::find_if(statementIdentifiers.cbegin(), statementIdentifiers.cend(),
                                         [](const FhirIdentifier &identifier) -> bool {
                                             auto type = identifier.GetType().GetText();
                                             std::transform(type.cbegin(), type.cend(), type.begin(),
                                                            [](char ch) { return std::tolower(ch); });
                                             return type == "pll" && !identifier.GetValue().empty();
                                         }) != statementIdentifiers.cend()) {
                            headIsAheadOfPll.emplace_back(firstRef);
                            headIsPllValue = true;
                        }
                    }
                } while (medicationStatement);
                medicationStatementRefChains.emplace_back(std::move(chain));
            }
        }
        {
            std::map<std::string, std::string> idToDisplay{};
            std::vector<std::string> idsToPreselect{};
            for (const auto &statement : medicationStatements) {
                auto id = statement.first;
                if (std::find(heads.cbegin(), heads.cend(), id) == heads.cend()) {
                    continue;
                }
                auto medstat = statement.second;
                auto statusInfo = PrescriptionChangesService::GetPrescriptionStatusInfo(*medstat);
                std::string display{medstat->GetDisplay()};
                if (statusInfo.IsPll) {
                    display.append(" (PLL");
                    if (!statusInfo.IsValidPrescription || statusInfo.IsRenewedWithoutChanges) {
                        display.append(",");
                        display.append(PrescriptionChangesService::GetPrescriptionStatusString(statusInfo));
                    }
                    display.append(")");
                } else if (!statusInfo.IsValidPrescription || statusInfo.IsRenewedWithoutChanges) {
                    display.append(" (");
                    display.append(PrescriptionChangesService::GetPrescriptionStatusString(statusInfo));
                    display.append(")");
                }
                idToDisplay.insert_or_assign(id, display);
                if (std::find(headIsPll.cbegin(), headIsPll.cend(), id) != headIsPll.cend() ||
                    std::find(headIsAheadOfPll.cbegin(), headIsAheadOfPll.cend(), id) != headIsAheadOfPll.cend()) {
                    auto idToSelect = id;
                    auto headIterator = headMap.find(idToSelect);
                    if (headIterator != headMap.end()) {
                        idToSelect = headIterator->second;
                    }
                    if (std::find(idsToPreselect.cbegin(), idsToPreselect.cend(), idToSelect) == idsToPreselect.cend()) {
                        idsToPreselect.emplace_back(idToSelect);
                    }
                }
            }
            SignPllDialog signPllDialog{this, idToDisplay, idsToPreselect};
            if (signPllDialog.ShowModal() != wxID_OK) {
                return;
            }
            selected = signPllDialog.GetSelected();
            for (const auto &id : selected) {
                if (std::find(headIsPll.cbegin(), headIsPll.cend(), id) != headIsPll.cend()) {
                    continue;
                }
                if (std::find(headIsAheadOfPll.cbegin(), headIsAheadOfPll.cend(), id) != headIsAheadOfPll.cend()) {
                    continue;
                }
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
            auto &medicationStatementUrl = pair.first;
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
            }
            for (auto &extension : extensions) {
                auto url = extension->GetUrl();
                if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-reseptamendment") {
                    auto extensions = extension->GetExtensions();
                    bool changedExtensions{false};
                    {
                        auto extensionIterator = extensions.begin();
                        auto replaceIterator = extensions.end();
                        std::shared_ptr<FhirDateTimeValue> lastChangedValue{};
                        bool createeresept{false};
                        while (extensionIterator != extensions.end()) {
                            auto &extension = *extensionIterator;
                            auto url = extension->GetUrl();
                            std::transform(url.cbegin(), url.cend(), url.begin(),
                                           [](char ch) { return std::tolower(ch); });
                            if (url == "createeresept") {
                                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                if (valueExtension) {
                                    auto value = std::dynamic_pointer_cast<FhirBooleanValue>(
                                            valueExtension->GetValue());
                                    if (value && value->IsTrue()) {
                                        createeresept = true;
                                    }
                                }
                            }
                            if (url == "lastchanged") {
                                replaceIterator = extensionIterator;
                                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                if (valueExtension) {
                                    auto value = std::dynamic_pointer_cast<FhirDateTimeValue>(
                                            valueExtension->GetValue());
                                    if (value) {
                                        lastChangedValue = value;
                                    }
                                }
                            }
                            ++extensionIterator;
                        }
                        if (lastChangedValue) {
                            if (createeresept) {
                                lastChangedValue->SetDateTime(lastChanged);
                            }
                        } else {
                            if (replaceIterator != extensions.end()) {
                                extensions.erase(replaceIterator);
                            }
                            extensions.emplace_back(std::make_shared<FhirValueExtension>("lastchanged",
                                                                                         std::make_shared<FhirDateTimeValue>(
                                                                                                 lastChanged)));
                            changedExtensions = true;
                        }
                    }
                    {
                        auto extensionIterator = extensions.begin();
                        while (extensionIterator != extensions.end()) {
                            auto url = (*extensionIterator)->GetUrl();
                            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                            if (url == "recallinfo") {
                                if (std::find(heads.cbegin(), heads.cend(), medicationStatementUrl) == heads.cend()) {
                                    extensionIterator = extensions.erase(extensionIterator);
                                    changedExtensions = true;
                                    continue;
                                }
                            }
                            ++extensionIterator;
                        }
                    }
                    if (changedExtensions) {
                        extension->SetExtensions(extensions);
                    }
                }
            }
        }
    }
    auto subjectRef = GetSubjectRef();
    auto prescriberRef = GetPrescriber();
    std::shared_ptr<WaitingForApiDialog> waitingDialog = std::make_shared<WaitingForApiDialog>(this, "Sending PLL", "Sending medication bundle...");
    auto ctx = std::make_shared<std::unique_ptr<CallContext>>();
    *ctx = std::make_unique<CallContext>();
    try {
        wxTheApp->GetTopWindow()->GetEventHandler()->CallAfter([this, ctx, subjectRef, prescriberRef, selected, medicationStatementRefChains, pllMedicationStatements, newPllIds, medicationStatements, waitingDialog]() {
            SendMedication(**ctx, [subjectRef, prescriberRef, selected, medicationStatementRefChains, pllMedicationStatements, newPllIds](
                    const std::shared_ptr<FhirBundle> &bundle) {
                auto entries = bundle->GetEntries();
                std::vector<FhirBundleEntry> appendEntries{};
                std::map<std::string,std::shared_ptr<FhirMedicationStatement>> medStatementMap{};
                for (const auto &entry: entries) {
                    auto medstat = std::dynamic_pointer_cast<FhirMedicationStatement>(entry.GetResource());
                    if (medstat) {
                        medStatementMap.insert_or_assign(entry.GetFullUrl(), medstat);
                    }
                    auto composition = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
                    if (composition) {
                        {
                            std::vector<FhirAttester> attesterVec{};
                            auto &attester = attesterVec.emplace_back();
                            attester.SetMode("legal");
                            attester.SetDateTime(DateTimeOffset::Now().to_iso8601());
                            attester.SetParty(FhirReference(prescriberRef.uuid, "http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner", prescriberRef.name));
                            composition->SetAttester(attesterVec);
                        }
                        auto sections = composition->GetSections();
                        for (auto &section: sections) {
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
                                    for (const auto &m25Message: m25Messages) {
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
                                    m251Message->SetCode(
                                            FhirCodeableConcept("http://ehelse.no/fhir/CodeSystem/sfm-message-type",
                                                                "M25.1", "PLL"));
                                    if (subjectRef.IsSet()) {
                                        m251Message->SetSubject(subjectRef);
                                    }
                                    appendEntries.emplace_back(url, m251Message);
                                    sectionEntries.emplace_back(url,
                                                                "http://ehelse.no/fhir/StructureDefinition/sfm-PLL-info",
                                                                "sfm-PLL-info");
                                    section.SetEntries(sectionEntries);
                                    section.SetEmptyReason({});
                                }
                                std::shared_ptr<FhirExtension> metadataPll{};
                                {
                                    auto extensions = m251Message->GetExtensions();
                                    for (const auto &extension: extensions) {
                                        auto urlLower = extension->GetUrl();
                                        std::transform(urlLower.begin(), urlLower.end(), urlLower.begin(),
                                                       [](auto ch) -> char { return std::tolower(ch); });
                                        if (urlLower == "http://ehelse.no/fhir/structuredefinition/sfm-pllinformation") {
                                            metadataPll = extension;
                                        }
                                    }
                                }
                                if (!metadataPll) {
                                    metadataPll = std::make_shared<FhirExtension>(
                                            "http://ehelse.no/fhir/StructureDefinition/sfm-pllInformation");
                                    m251Message->AddExtension(metadataPll);
                                }
                                std::shared_ptr<FhirBooleanValue> createPll{};
                                std::shared_ptr<FhirDateTimeValue> pllDate{};
                                {
                                    auto extensions = metadataPll->GetExtensions();
                                    for (const auto &extension: extensions) {
                                        auto urlLower = extension->GetUrl();
                                        std::transform(urlLower.begin(), urlLower.end(), urlLower.begin(),
                                                       [](auto ch) -> char { return std::tolower(ch); });
                                        if (urlLower == "createpll") {
                                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                            if (valueExtension) {
                                                auto value = std::dynamic_pointer_cast<FhirBooleanValue>(
                                                        valueExtension->GetValue());
                                                if (value) {
                                                    createPll = value;
                                                }
                                            }
                                        }
                                        if (urlLower == "plldate") {
                                            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                            if (valueExtension) {
                                                auto value = std::dynamic_pointer_cast<FhirDateTimeValue>(
                                                        valueExtension->GetValue());
                                                if (value) {
                                                    pllDate = value;
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
                                if (pllDate) {
                                    //pllDate->SetDateTime(DateTimeOffset::Now().to_iso8601());
                                }
                            } else if (codings[0].GetCode() == "sectionMedication") {
                                auto entries = section.GetEntries();
                                auto iterator = entries.begin();
                                while (iterator != entries.end()) {
                                    auto reference = iterator->GetReference();
                                    bool found{false};
                                    for (const auto &id: selected) {
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
                for (const auto &firstRef : selected) {
                    for (const auto &chain : medicationStatementRefChains) {
                        auto iterator = chain.begin();
                        while (iterator != chain.end()) {
                            if (*iterator == firstRef) {
                                break;
                            }
                            ++iterator;
                        }
                        if (iterator != chain.end()) {
                            auto firstStatement = medStatementMap.find(*iterator)->second;
                            auto firstStatementIdentifiers = firstStatement->GetIdentifiers();
                            if (std::find_if(firstStatementIdentifiers.cbegin(), firstStatementIdentifiers.cend(), [] (const FhirIdentifier &identifier) -> bool{
                                auto type = identifier.GetType().GetText();
                                std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) { return std::tolower(ch); });
                                return type == "pll" && !identifier.GetValue().empty();
                            }) != firstStatementIdentifiers.cend()) {
                                continue;
                            }
                            ++iterator;
                            while (iterator != chain.end()) {
                                auto statement = medStatementMap.find(*iterator)->second;
                                auto statementIdentifiers = statement->GetIdentifiers();
                                auto pllIdIterator = std::find_if(statementIdentifiers.cbegin(), statementIdentifiers.cend(), [] (const FhirIdentifier &identifier) -> bool{
                                    auto type = identifier.GetType().GetText();
                                    std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) { return std::tolower(ch); });
                                    return type == "pll" && !identifier.GetValue().empty();
                                });
                                if (pllIdIterator != statementIdentifiers.cend()) {
#ifdef SENDPLL_UPDATE_RENEWAL_SETID
                                    std::vector<FhirIdentifier> newIdentifiers{};
                                    newIdentifiers.emplace_back(*pllIdIterator);
                                    for (const auto &identifier : firstStatementIdentifiers) {
                                        newIdentifiers.emplace_back(identifier);
                                    }
                                    firstStatement->SetIdentifiers(newIdentifiers);
#endif
#ifdef SENDPLL_UPDATE_RENEWAL_REMOVEID
                                    statementIdentifiers.erase(pllIdIterator);
                                    statement->SetIdentifiers(statementIdentifiers);
#endif
                                    break;
                                }
                                ++iterator;
                            }
                        }
                    }
                }
                {
                    auto iterator = entries.begin();
                    while (iterator != entries.end()) {
                        auto medicationStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(
                                iterator->GetResource());
                        if (medicationStatement) {
                            std::string pllId{};
                            {
                                auto identifiers = medicationStatement->GetIdentifiers();
                                auto iterator = identifiers.begin();
                                while (iterator != identifiers.end()) {
                                    const auto &identifier = *iterator;
                                    auto key = identifier.GetType().GetText();
                                    std::transform(key.cbegin(), key.cend(), key.begin(),
                                                   [](char ch) { return std::tolower(ch); });
                                    if (key == "pll") {
                                        pllId = identifier.GetValue();
                                        bool found{false};
                                        for (const auto &newId: newPllIds) {
                                            if (newId == pllId) {
                                                found = true;
                                                break;
                                            }
                                        }
                                        if (found) {
                                            iterator = identifiers.erase(iterator);
                                        } else {
                                            ++iterator;
                                        }
                                    } else {
                                        ++iterator;
                                    }
                                }
                                medicationStatement->SetIdentifiers(identifiers);
                            }
                            ++iterator;
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
            }, [medicationStatements, newPllIds, pllMedicationStatements](
                    const std::map<std::string, FhirCoding> &pllResults) {
                std::vector<std::string> removePllIds = newPllIds;
                int pllsSent{0};
                for (const auto &resultPair: pllResults) {
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

                }
            }, [this, waitingDialog, ctx](const std::string &err) {
                if (err.empty()) {
                    (*ctx)->Finish();
                    waitingDialog->SetMessage("Requesting new medication bundle...");
                    *ctx = std::make_unique<CallContext>();
                    try {
                        GetMedication(**ctx, [waitingDialog](const std::string &err) {
                            waitingDialog->Close();
                            if (!err.empty()) {
                                wxMessageBox(wxString::FromUTF8(err), wxT("Get medication error"), wxICON_ERROR);
                            }
                        });
                    } catch (SyncGetMedError &e) {
                        waitingDialog->Close();
                        wxMessageBox(wxString::FromUTF8(e.what()), wxT("Get medication error"), wxICON_ERROR);
                    } catch (...) {
                        waitingDialog->Close();
                        wxMessageBox(wxT("Unknown error"), wxT("Get medication error"), wxICON_ERROR);
                    }
                } else {
                    waitingDialog->Close();
                    wxMessageBox(wxString::FromUTF8(err), wxT("Send medication error"), wxICON_ERROR);
                }
            });
        });
    } catch (SyncSendMedError &e) {
        wxMessageBox(wxString::FromUTF8(e.what()), wxT("Send medication error"), wxICON_ERROR);
        return;
    }
    waitingDialog->ShowModal();
    (*ctx)->Finish();
}

void TheMasterFrame::OnSaveLastGetmedRequest(wxCommandEvent &e) {
    wxFileDialog saveFileDialog(this, _("Save last \"get medication\" request"), "", "", "Json files (*.json)|*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return; // the user changed their mind
    std::ofstream ofs{saveFileDialog.GetPath().ToStdString()};
    ofs << lastGetmedRequest;
    ofs.close();
}

void TheMasterFrame::OnSaveLastSendmedRequest(wxCommandEvent &e) {
    wxFileDialog saveFileDialog(this, _("Save last \"send medication\" request"), "", "", "Json files (*.json)|*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return; // the user changed their mind
    std::ofstream ofs{saveFileDialog.GetPath().ToStdString()};
    ofs << lastSendmedRequest;
    ofs.close();
}

void TheMasterFrame::OnSaveLastGetmed(wxCommandEvent &e) {
    wxFileDialog saveFileDialog(this, _("Save last \"get medication\" response"), "", "", "Json files (*.json)|*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return; // the user changed their mind
    std::ofstream ofs{saveFileDialog.GetPath().ToStdString()};
    ofs << lastGetmedResponse;
    ofs.close();
}

void TheMasterFrame::OnSaveLastSendmed(wxCommandEvent &e) {
    wxFileDialog saveFileDialog(this, _("Save last \"send medication\" response"), "", "", "Json files (*.json)|*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return; // the user changed their mind
    std::ofstream ofs{saveFileDialog.GetPath().ToStdString()};
    ofs << lastSendmedResponse;
    ofs.close();
}

void TheMasterFrame::OnSaveBundle(wxCommandEvent &e) {
    std::string jsonStr{};
    {
        auto json = medicationBundle->medBundle->ToJson();
        jsonStr = from_wstring_on_win32(json.serialize());
    }
    wxFileDialog saveFileDialog(this, _("Save bundle"), "", "", "Json files (*.json)|*.json", wxFD_SAVE|wxFD_OVERWRITE_PROMPT);
    if (saveFileDialog.ShowModal() == wxID_CANCEL)
        return; // the user changed their mind
    std::ofstream ofs{saveFileDialog.GetPath().ToStdString()};
    ofs << jsonStr;
    ofs.close();
}

PrescriberRef TheMasterFrame::GetPrescriber() const {
    if (!medicationBundle) {
        return {};
    }
    return medicationBundle->GetPrescriber(helseidIdToken);
}

void TheMasterFrame::SetPrescriber(PrescriptionData &prescriptionData) const {
    auto prescriber = GetPrescriber();
    prescriptionData.prescribedByReference = prescriber.uuid;
    prescriptionData.prescribedByDisplay = prescriber.name;
}

FhirReference TheMasterFrame::GetSubjectRef() const {
    if (!medicationBundle) {
        return {};
    }
    return medicationBundle->GetSubjectRef();
}

void TheMasterFrame::SetPatient(PrescriptionData &prescriptionData) const {
    auto ref = GetSubjectRef();
    if (!ref.IsSet()) {
        return;
    }
    prescriptionData.subjectReference = ref.GetReference();
    prescriptionData.subjectDisplay = ref.GetDisplay();
}

void TheMasterFrame::PrescribeMedicament(const PrescriptionDialog &prescriptionDialog, const std::string &renewPrescriptionId) {
    PrescriptionData prescriptionData = prescriptionDialog.GetPrescriptionData();
    std::shared_ptr<FhirMedication> medicament = prescriptionDialog.GetMedication();
    SetPrescriber(prescriptionData);
    SetPatient(prescriptionData);
    medicationBundle->Prescribe(medicament, prescriptionData, renewPrescriptionId);
    UpdateMedications();
}

void TheMasterFrame::OnPrescribeMagistral(wxCommandEvent &e) {
    MagistralBuilderDialog magistralBuilderDialog{this};
    if (!medicationBundle || !medicationBundle->medBundle) {
        wxMessageBox(wxT("Select patient and run 'get medication' to prescribe medication"), wxT("No open patient"), wxICON_ERROR);
        return;
    }
    if (magistralBuilderDialog.ShowModal() == wxID_OK) {
        PrescriptionDialog prescriptionDialog{this, std::make_shared<FestDb>(), std::make_shared<FhirMedication>(magistralBuilderDialog.GetMagistralMedicament().ToFhir()), {}, {}, {}, true};
        auto res = prescriptionDialog.ShowModal();
        if (res != wxID_OK) {
            return;
        }
        PrescribeMedicament(prescriptionDialog);
    }
}

void TheMasterFrame::OnPrescribeMedicament(wxCommandEvent &e) {
    std::shared_ptr<FestDb> festDb = std::make_shared<FestDb>();
    if (!festDb->IsOpen()) {
        return;
    }
    FindMedicamentDialog findMedicamentDialog{this, festDb};
    if (!findMedicamentDialog.CanOpen()) {
        wxMessageBox(wxT("Prescribe medicament requires a FEST DB. Please update FEST and try again."), wxT("No FEST DB"), wxICON_ERROR);
        return;
    }
    if (!medicationBundle || !medicationBundle->medBundle) {
        wxMessageBox(wxT("Select patient and run 'get medication' to prescribe medication"), wxT("No open patient"), wxICON_ERROR);
        return;
    }
    findMedicamentDialog.ShowModal();
    auto medicament = findMedicamentDialog.GetSelected();
    if (medicament) {
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
        std::vector<MedicalCodedValue> dosingUnits = GetMedicamentDosingUnit(festDb, *medicament).operator std::vector<MedicalCodedValue>();
        std::vector<MedicalCodedValue> kortdoser = GetLegemiddelKortdoser(festDb, *medicament).operator std::vector<MedicalCodedValue>();
        PrescriptionDialog prescriptionDialog{this, festDb, std::make_shared<FhirMedication>(medicamentMapper.GetMedication()), medicamentMapper.GetPrescriptionUnit(), medicamentMapper.GetMedicamentType(), medicamentMapper.GetMedicamentUses(), medicamentMapper.IsPackage(), packages, dosingUnits, kortdoser, medicamentMapper.GetPrescriptionValidity()};
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
                                long expiresIn, const std::string &idToken, const std::string &journalId,
                                const std::string &orgNo, const std::string &childOrgNo) {
    helseidUrl = url;
    helseidClientId = clientId;
    helseidSecretJwk = secretJwk;
    helseidScopes = scopes;
    helseidRefreshToken = refreshToken;
    helseidRefreshTokenValidTo = std::time(NULL) + expiresIn - 60;
    helseidIdToken = idToken;
    this->journalId = journalId;
    this->orgNo = orgNo;
    this->childOrgNo = childOrgNo;
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

void TheMasterFrame::OnPrescriptionContextMenu(const wxContextMenuEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatement = displayedMedicationStatements[selected][0];
    auto display = medicationStatement->GetDisplay();
    wxMenu menu(wxString::FromUTF8(display));
    menu.Append(TheMaster_PrescriptionDetails_Id, wxT("Details"));
    menu.Append(TheMaster_PrescriptionRecall_Id, wxT("Recall"));
    menu.Append(TheMaster_PrescriptionCease_Id, wxT("Cease"));
    menu.Append(TheMaster_PrescriptionRenew_Id, wxT("Renew"));
    menu.Append(TheMaster_PrescriptionRenewWithChanges_Id, wxT("Renew with changes"));
    menu.Append(TheMaster_TreatmentEdit_Id, wxT("Edit treatment"));
    auto identifiers = medicationStatement->GetIdentifiers();
    if (std::find_if(identifiers.cbegin(), identifiers.cend(), [] (const auto &identifier) {
        auto type = identifier.GetType().GetText();
        std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
        return type == "pll";
    }) == identifiers.cend()) {
        menu.Append(TheMaster_ConnectToPll_Id, wxT("Connect to PLL"));
    }
    PopupMenu(&menu);
}

void TheMasterFrame::OnPrescriptionDetails(const wxCommandEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatements = displayedMedicationStatements[selected];
    PrescriptionDetailsDialog dialog{this, medicationStatements};
    dialog.ShowModal();
}

void TheMasterFrame::OnPrescriptionRecall(const wxCommandEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatement = displayedMedicationStatements[selected][0];
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
            if (url == "lastchanged") {
                auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                if (valueExt) {
                    valueExt->SetValue(std::make_shared<FhirDateTimeValue>(DateTimeOffset::Now().to_iso8601()));
                }
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
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("text", std::make_shared<FhirString>("Tilbakekalt")));
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("notsent", std::make_shared<FhirBooleanValue>(true)));
        reseptAmendment->AddExtension(recallInfoExt);
    }
    auto prescriber = GetPrescriber();
    {
        std::shared_ptr<FhirExtension> regInfo = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-regInfo");
        regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                "status",
                std::make_shared<FhirCodeableConceptValue>(
                        FhirCodeableConcept(
                                "http://ehelse.no/fhir/CodeSystem/sfm-medicationstatement-registration-status",
                                "3",
                                "Godkjent"
                        )
                )
        ));
        regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                "type",
                std::make_shared<FhirCodeableConceptValue>(
                        FhirCodeableConcept(
                                "http://ehelse.no/fhir/CodeSystem/sfm-performer-roles",
                                "3",
                                "Seponert av"
                        )
                )
        ));
        regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                "provider",
                std::make_shared<FhirReference>(
                        prescriber.uuid,
                        "http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner",
                        prescriber.name
                )
        ));
        {
            std::string nowString = DateTimeOffset::Now().to_iso8601();

            regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                    "timestamp",
                    std::make_shared<FhirDateTimeValue>(nowString)
            ));
        }
        bool noRegInfo{true};
        {
            auto extensions = medicationStatement->GetExtensions();
            auto iterator = extensions.begin();
            while (iterator != extensions.end()) {
                const auto &extension = *iterator;
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) {return std::tolower(ch);});
                if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reginfo") {
                    if (noRegInfo) {
                        *iterator = regInfo;
                        ++iterator;
                        noRegInfo = false;
                    } else {
                        iterator = extensions.erase(iterator);
                    }
                } else {
                    ++iterator;
                }
            }
            medicationStatement->SetExtensions(extensions);
        }
        if (noRegInfo) {
            medicationStatement->AddExtension(regInfo);
        }
    }
    UpdateMedications();
}

void TheMasterFrame::OnPrescriptionCease(const wxCommandEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatement = displayedMedicationStatements[selected][0];
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
    bool recall{true};
    {
        FhirCodeableConcept typeResept{};
        auto extensions = reseptAmendment->GetExtensions();
        for (const auto &extension : extensions) {
            auto url = extension->GetUrl();
            if (url == "recallinfo") {
                recall = false;
                break;
            }
            if (url == "typeresept") {
                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                if (valueExtension) {
                    auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                    if (value) {
                        typeResept = *value;
                    }
                }
            }
        }
        auto typeReseptCodings = typeResept.GetCoding();
        if (typeReseptCodings.size() == 1) {
            auto typeReseptCoding = typeReseptCodings[0];
            if (typeReseptCoding.GetCode() != "E") {
                recall = false;
            }
        } else {
            recall = false;
        }
    }
    CeasePrescriptionDialog dialog{this, medicationStatement};
    {
        auto res = dialog.ShowModal();
        if (res != wxID_OK) {
            return;
        }
    }
    if (recall) {
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
                FhirCodeableConcept codeable{std::move(codings)};
                discontinuation->AddExtension(std::make_shared<FhirValueExtension>("reason", std::make_shared<FhirCodeableConceptValue>(codeable)));
                if (!codeable.IsSet()) {
                    discontinuation->AddExtension(
                            std::make_shared<FhirValueExtension>("note", std::make_shared<FhirString>(text)));
                }
            }
        }
        medicationStatement->AddExtension(discontinuation);
    }
    auto prescriber = GetPrescriber();
    {
        std::shared_ptr<FhirExtension> regInfo = std::make_shared<FhirExtension>("http://ehelse.no/fhir/StructureDefinition/sfm-regInfo");
        regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                "status",
                std::make_shared<FhirCodeableConceptValue>(
                        FhirCodeableConcept(
                                "http://ehelse.no/fhir/CodeSystem/sfm-medicationstatement-registration-status",
                                "3",
                                "Godkjent"
                        )
                )
        ));
        regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                "type",
                std::make_shared<FhirCodeableConceptValue>(
                        FhirCodeableConcept(
                                "http://ehelse.no/fhir/CodeSystem/sfm-performer-roles",
                                "3",
                                "Seponert av"
                        )
                )
        ));
        regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                "provider",
                std::make_shared<FhirReference>(
                        prescriber.uuid,
                        "http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner",
                        prescriber.name
                )
        ));
        {
            std::string nowString = DateTimeOffset::Now().to_iso8601();

            regInfo->AddExtension(std::make_shared<FhirValueExtension>(
                    "timestamp",
                    std::make_shared<FhirDateTimeValue>(nowString)
            ));
        }
        bool noRegInfo{true};
        {
            auto extensions = medicationStatement->GetExtensions();
            auto iterator = extensions.begin();
            while (iterator != extensions.end()) {
                const auto &extension = *iterator;
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) {return std::tolower(ch);});
                if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reginfo") {
                    if (noRegInfo) {
                        *iterator = regInfo;
                        ++iterator;
                        noRegInfo = false;
                    } else {
                        iterator = extensions.erase(iterator);
                    }
                } else {
                    ++iterator;
                }
            }
            medicationStatement->SetExtensions(extensions);
        }
        if (noRegInfo) {
            medicationStatement->AddExtension(regInfo);
        }
    }
    UpdateMedications();
}

void TheMasterFrame::OnPrescriptionRenew(const wxCommandEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatement = displayedMedicationStatements[selected][0];
    try {
        PrescriptionChangesService::Renew(*medicationStatement);
    } catch (const std::exception &e) {
        const char *wht = e.what();
        wxMessageBox(wxString::FromUTF8(wht != nullptr ? wxString::FromUTF8(wht) : wxString::FromUTF8("Renew failed")), wxT("Renew failed"), wxICON_ERROR);
    }
    UpdateMedications();
}

void TheMasterFrame::OnPrescriptionRenewWithChanges(const wxCommandEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatement = displayedMedicationStatements[selected][0];
    std::string reseptId{};
    {
        auto identifiers = medicationStatement->GetIdentifiers();
        auto iterator = identifiers.begin();
        while (iterator != identifiers.end()) {
            auto identifier = *iterator;
            auto key = identifier.GetType().GetText();
            std::transform(key.cbegin(), key.cend(), key.begin(), [](char ch) -> char { return std::tolower(ch); });
            if (key == "reseptid") {
                reseptId = identifier.GetValue();
                break;
            }
            ++iterator;
        }
    }
    if (reseptId.empty()) {
        wxMessageBox(wxT("The entry does not contain a prescription to renew"), wxT("Renew failed"), wxICON_ERROR);
        return;
    }
    auto medication = std::dynamic_pointer_cast<FhirMedication>(medicationBundle->GetByUrl(medicationStatement->GetMedicationReference().GetReference()));
    if (!medication) {
        wxMessageBox(wxT("Could not find medication object for medication statement"), wxT("Medication object not found"), wxICON_ERROR);
        return;
    }
    std::string festId{};
    {
        auto codes = medication->GetCode().GetCoding();
        auto iterator = std::find_if(codes.cbegin(), codes.cend(), [] (const FhirCoding &coding) -> bool {
            auto system = coding.GetSystem();
            std::transform(system.cbegin(), system.cend(), system.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
            return (system == "http://ehelse.no/fhir/codesystem/fest");
        });
        if (iterator == codes.cend()) {
            wxMessageBox(wxT("Could not find FEST ID for medication object"), wxT("Medication not found"), wxICON_ERROR);
            return;
        }
        festId = iterator->GetCode();
    }
    std::shared_ptr<FestDb> festDb = std::make_shared<FestDb>();

    std::shared_ptr<LegemiddelCore> legemiddelCore{};
    {
        FestUuid festUuid{};
        bool hasUuid{};
        try {
            festUuid = FestUuid(festId, false);
            hasUuid = true;
        } catch (...) {
            auto pakning = festDb->GetLegemiddelpakningByVarenr(festId);
            if (!pakning.GetId().empty()) {
                legemiddelCore = std::make_shared<Legemiddelpakning>(std::move(pakning));
            }
        }
        if (hasUuid) {
            auto merkevare = festDb->GetLegemiddelMerkevare(festUuid);
            if (!merkevare.GetId().empty()) {
                legemiddelCore = std::make_shared<LegemiddelMerkevare>(std::move(merkevare));
            } else {
                auto virkestoff = festDb->GetLegemiddelVirkestoff(festUuid);
                if (!virkestoff.GetId().empty()) {
                    legemiddelCore = std::make_shared<LegemiddelVirkestoff>(std::move(virkestoff));
                }
            }
        }
        if (!legemiddelCore) {
            wxMessageBox(wxT("Could not find medicament in FEST"), wxT("Medicament not found"), wxICON_ERROR);
            return;
        }
    }
    PrescriptionData prescriptionData{};
    prescriptionData.FromFhir(*medicationStatement);
    SfmMedicamentMapper medicamentMapper{festDb, legemiddelCore};
    std::vector<MedicamentPackage> packages{};
    {
        auto packageMappers = medicamentMapper.GetPackages();
        packages.reserve(packageMappers.size());
        for (const auto &packageMapper : packageMappers) {
            auto description = packageMapper.GetPackageDescription();
            packages.emplace_back(std::make_shared<FhirMedication>(packageMapper.GetMedication()), description);
        }
    }
    std::vector<MedicalCodedValue> dosingUnits = GetMedicamentDosingUnit(festDb, *legemiddelCore).operator std::vector<MedicalCodedValue>();
    std::vector<MedicalCodedValue> kortdoser = GetLegemiddelKortdoser(festDb, *legemiddelCore).operator std::vector<MedicalCodedValue>();
    PrescriptionDialog prescriptionDialog{this, festDb, std::make_shared<FhirMedication>(medicamentMapper.GetMedication()), medicamentMapper.GetPrescriptionUnit(), medicamentMapper.GetMedicamentType(), medicamentMapper.GetMedicamentUses(), medicamentMapper.IsPackage(), packages, dosingUnits, kortdoser, medicamentMapper.GetPrescriptionValidity()};
    prescriptionDialog += prescriptionData;
    auto res = prescriptionDialog.ShowModal();
    if (res != wxID_OK) {
        return;
    }
    PrescribeMedicament(prescriptionDialog, reseptId);
}

void TheMasterFrame::OnTreatmentEdit(const wxCommandEvent &e) {
    if (prescriptions->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = prescriptions->GetFirstSelected();
    if (selected < 0 || selected >= displayedMedicationStatements.size()) {
        return;
    }
    auto medicationStatement = displayedMedicationStatements[selected][0];
    if (!medicationBundle || !medicationStatement) {
        wxMessageBox(wxT("Run get medication"), wxT("No bundle"), wxICON_ERROR);
    }
    EditTreatmentDialog editTreatmentDialog{this, *medicationBundle, medicationStatement};
    editTreatmentDialog.ShowModal();
}

void TheMasterFrame::OnConnectToPll(const wxCommandEvent &e) {
    std::shared_ptr<FhirMedicationStatement> medicationStatement{};
    {
        if (prescriptions->GetSelectedItemCount() != 1) {
            return;
        }
        auto selected = prescriptions->GetFirstSelected();
        if (selected < 0 || selected >= displayedMedicationStatements.size()) {
            return;
        }
        medicationStatement = displayedMedicationStatements[selected][0];
    }
    if (!medicationBundle || !medicationStatement) {
        wxMessageBox(wxT("Run get medication"), wxT("No bundle"), wxICON_ERROR);
        return;
    }
    std::shared_ptr<FhirMedicationStatement> connectToMedicationStatement{};
    {
        ConnectToPllDialog dialog{this, *medicationBundle};
        if (dialog.ShowModal() != wxID_OK) {
            return;
        }
        connectToMedicationStatement = dialog.GetSelectedMedicationStatement();
    }
    if (!connectToMedicationStatement) {
        return;
    }
    auto connectToIdentifiers = connectToMedicationStatement->GetIdentifiers();
    auto identifiers = medicationStatement->GetIdentifiers();
    FhirIdentifier pllId{};
    {
        auto iterator = connectToIdentifiers.begin();
        bool found{false};
        while (iterator != connectToIdentifiers.end()) {
            auto type = iterator->GetType().GetText();
            std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) -> char { return std::tolower(ch); });
            if (type == "pll") {
                pllId = *iterator;
                iterator = connectToIdentifiers.erase(iterator);
                found = true;
                break;
            }
            ++iterator;
        }
        if (!found) {
            return;
        }
    }
    identifiers.insert(identifiers.begin(), std::move(pllId));
    connectToMedicationStatement->SetIdentifiers(connectToIdentifiers);
    medicationStatement->SetIdentifiers(identifiers);
    UpdateMedications();
}

void TheMasterFrame::OnCaveContextMenu(const wxContextMenuEvent &e) {
    if (caveListView->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = caveListView->GetFirstSelected();
    if (selected < 0 || selected >= displayedAllergies.size()) {
        return;
    }
    auto allergy = displayedAllergies[selected];
    FhirCoding coding{};
    {
        auto codings = allergy->GetCode().GetCoding();
        if (codings.size() == 1) {
            coding = codings[0];
        }
    }
    std::string display{coding.GetDisplay()};
    if (display.empty()) {
        display = allergy->GetCode().GetText();
    }
    wxMenu menu(wxString::FromUTF8(display));
    menu.Append(TheMaster_CaveDetails_Id, wxT("Details"));
    menu.Append(TheMaster_EditCaveMedicament_Id, wxT("Edit"));
    menu.Append(TheMaster_DeleteCaveMedicament_Id, wxT("Delete"));
    PopupMenu(&menu);
}

void TheMasterFrame::OnCaveDetails(const wxCommandEvent &e) {
    if (caveListView->GetSelectedItemCount() != 1) {
        return;
    }
    auto selected = caveListView->GetFirstSelected();
    if (selected < 0 || selected >= displayedAllergies.size()) {
        return;
    }
    auto allergy = displayedAllergies[selected];
    CaveDetailsDialog caveDetailsDialog{this, allergy};
    caveDetailsDialog.ShowModal();
}

void TheMasterFrame::OnAddCaveMedicament(const wxCommandEvent &e) {
    std::shared_ptr<LegemiddelCore> legemiddelCore{};
    auto festDb = std::make_shared<FestDb>();
    if (!festDb->IsOpen()) {
        return;
    }
    {
        FindMedicamentDialog findMedicamentDialog{this, festDb};
        if (!findMedicamentDialog.CanOpen()) {
            wxMessageBox(wxT("Add CAVE medicament requires a FEST DB. Please update FEST and try again."),
                         wxT("No FEST DB"), wxICON_ERROR);
            return;
        }
        if (!medicationBundle || !medicationBundle->medBundle || helseidIdToken.empty()) {
            wxMessageBox(wxT("Select patient and run 'get medication' to add medicament to CAVE"),
                         wxT("No open patient"), wxICON_ERROR);
            return;
        }
        if (findMedicamentDialog.ShowModal() != wxID_OK) {
            return;
        }
        legemiddelCore = findMedicamentDialog.GetSelected();
        if (!legemiddelCore) {
            return;
        }
    }
    auto prescriberRef = medicationBundle->GetPrescriber(helseidIdToken);
    FhirReference fhirPractitioner{prescriberRef.uuid, "http://nhn.no/kj/fhir/StructureDefinition/KjPractitionerRole", prescriberRef.name};
    auto patientRef = medicationBundle->GetSubjectRef();
    RegisterCaveDialog registerCave{this, festDb, *legemiddelCore, fhirPractitioner, patientRef};
    if (registerCave.ShowModal() != wxID_OK) {
        return;
    }
    medicationBundle->AddCave(registerCave.ToFhir());
    UpdateCave();
}

void TheMasterFrame::OnEditCaveMedicament(const wxCommandEvent &e) {
    std::shared_ptr<FhirAllergyIntolerance> allergy;
    {
        if (caveListView->GetSelectedItemCount() != 1) {
            return;
        }
        auto selection = caveListView->GetFirstSelected();
        if (selection < 0 || displayedAllergies.size() <= selection) {
            return;
        }
        allergy = displayedAllergies[selection];
    }
    RegisterCaveDialog registerCaveDialog{this, *allergy};
    if (registerCaveDialog.ShowModal() == wxID_OK) {
        *allergy = *(registerCaveDialog.ToFhir());
        UpdateCave();
    }
}

void TheMasterFrame::OnDeleteCaveMedicament(const wxCommandEvent &e) {
    std::shared_ptr<FhirAllergyIntolerance> allergy;
    {
        if (caveListView->GetSelectedItemCount() != 1) {
            return;
        }
        auto selection = caveListView->GetFirstSelected();
        if (selection < 0 || displayedAllergies.size() <= selection) {
            return;
        }
        allergy = displayedAllergies[selection];
    }
    medicationBundle->DeleteCave(allergy);
    UpdateCave();
}
