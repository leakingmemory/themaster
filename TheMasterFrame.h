//
// Created by sigsegv on 12/13/23.
//

#ifndef DRWHATSNOT_THEMASTERFRAME_H
#define DRWHATSNOT_THEMASTERFRAME_H

#include <wx/wx.h>
#include <memory>
#include <ctime>
#include <map>
#include <jjwtid/DpopHost.h>
#include "WeakRefUiDispatcher.h"
#include "PrescriptionData.h"
#include "MerchData.h"
#include "MedBundleData.h"
#include "TheMasterIds.h"
#include "FestDb.h"
#ifdef WIN32
#include <pplx/pplxtasks.h>
#endif

class FhirAllergyIntolerance;
class PatientStore;
class PatientInformation;
class PrescriberRef;
class FhirBundle;
class FhirExtension;
class FhirReference;
class FhirCoding;
class wxListView;
class wxNotebook;
#ifndef WIN32
namespace pplx {
    template<class ReturnType> class task;
}
#endif
class PrescriptionDialog;
class PrescribeMerchandiseDialog;
class CallContext;

class TheMasterFrame : public wxFrame {
private:
    std::shared_ptr<WeakRefUiDispatcher<TheMasterFrame>> weakRefDispatcher;
    std::shared_ptr<PatientStore> patientStore{};
    std::shared_ptr<PatientInformation> patientInformation;
    std::shared_ptr<FestDb> festDb;
    std::string kjConsent{};
    std::string kjLockedConsent{};
    std::string kjBarredConsent{};
    std::vector<std::string> refNumbers{};
    std::string medicationBundleResetData{};
    std::unique_ptr<MedBundleData> medicationBundle{};
    std::vector<std::vector<std::shared_ptr<FhirMedicationStatement>>> displayedMedicationStatements{};
    std::vector<std::vector<std::shared_ptr<FhirBasic>>> displayedMerch{};
    std::vector<std::shared_ptr<FhirMedicationDispense>> displayedDispenses{};
    std::vector<std::shared_ptr<FhirAllergyIntolerance>> displayedAllergies{};
    std::string lastGetmedRequest{};
    std::string lastGetmedResponse{};
    std::string lastSendmedRequest{};
    std::string lastSendmedResponse{};
    std::string url{};
    std::string helseidUrl{};
    std::string helseidClientId{};
    std::string helseidSecretJwk{};
    std::vector<std::string> helseidScopes{};
    std::string helseidRefreshToken{};
    std::time_t helseidRefreshTokenValidTo{0};
    std::string helseidIdToken{};
    std::string journalId{};
    std::string orgNo{};
    std::string childOrgNo{};
    std::shared_ptr<std::string> accessToken{};
    DpopHost dpopHost;
    std::string helseidDpopNonce{};
    wxComboBox *kjConsentSelect;
    wxComboBox *kjLockedConsentSelect;
    wxComboBox *kjBarredConsentSelect;
    wxTextCtrl *refNumberListing;
    wxListView *header;
    wxNotebook *mainCategories;
    wxListView *prescriptions;
    wxListView *merchPrescriptions;
    wxListView *dispensesList;
    wxListView *caveListView;
public:
    TheMasterFrame();
    void UpdateHeader();
    void UpdateMedications();
    void UpdateMerch();
    void UpdateDispenses();
    void UpdateCave();
    void OnKjConsent(wxCommandEvent &e);
    void OnKjLockedConsent(wxCommandEvent &e);
    void OnKjBarredConsent(wxCommandEvent &e);
    void OnRefNumsEdit(wxCommandEvent &e);
    void OnConnect(wxCommandEvent &e);
    void OnFindPatient(wxCommandEvent &e);
    void OnCreatePatient(wxCommandEvent &e);
    pplx::task<std::string> GetAccessToken();
private:
    void GetMedication(CallContext &ctx, const std::function<void (const std::string &err)> &callback);
public:
    void OnResetMedication(wxCommandEvent &e);
    void OnGetMedication(wxCommandEvent &e);
    static void FilterRecallInfos(FhirBundle &bundle, const std::function<bool (const std::string &,const std::shared_ptr<FhirExtension> &)> &predicate);
    static std::map<std::string,std::shared_ptr<FhirExtension>> GetRecallInfos(FhirBundle &bundle);
private:
    void SendMedication(CallContext &ctx, const std::function<void (const std::shared_ptr<FhirBundle> &)> &preprocessing, const std::function<void (const std::map<std::string,FhirCoding> &)> &pllResultsFunc, const std::function<void (const std::string &err)> &callback);
    void SendMedication(const std::function<void (const std::shared_ptr<FhirBundle> &)> &preprocessing, const std::function<void (const std::map<std::string,FhirCoding> &)> &pllResultsFunc);
public:
    void OnSendMedication(wxCommandEvent &e);
    void OnSendPll(wxCommandEvent &e);
    void OnSaveLastGetmedRequest(wxCommandEvent &e);
    void OnSaveLastSendmedRequest(wxCommandEvent &e);
    void OnSaveLastGetmed(wxCommandEvent &e);
    void OnSaveLastSendmed(wxCommandEvent &e);
    void OnSaveBundle(wxCommandEvent &e);
    [[nodiscard]] PrescriberRef GetPrescriber() const ;
    void SetPrescriber(PrescriptionData &prescriptionData) const ;
    void SetPrescriber(MerchData &merchData) const;
    [[nodiscard]] FhirReference GetSubjectRef() const ;
    void SetPatient(PrescriptionData &prescriptionData) const ;
    void SetPatient(MerchData &merchData) const;
    //pplx::task<PrescriptionData> SetPrescriber(const PrescriptionData &prescriptionData);
    void PrescribeMedicament(const PrescriptionDialog &prescriptionDialog, const std::string &renewPrescriptionId = "", const std::string &pllId = "");
    void PrescribeMerch(const PrescribeMerchandiseDialog &prescriptionDialog, const std::string &renewPrescriptionId = "");
    void OnPrescribeMagistral(wxCommandEvent &e);
    void OnPrescribeMedicament(wxCommandEvent &e);
    void OnPrescribeMerch(wxCommandEvent &e);
    void OnPrescribeNourishment(wxCommandEvent &e);
    WeakRefUiDispatcherRef<TheMasterFrame> GetWeakRefDispatcher();
    void SetHelseid(const std::string &url, const std::string &clientId, const std::string &secretJwk, const std::vector<std::string> &scopes, const std::string &refreshToken, long expiresIn, const std::string &idToken, const std::string &journalId, const std::string &orgNo, const std::string &childOrgNo, const DpopHost &dpopHost, const std::string &helseidDpopNonce);
    void Connect(const std::string &url);
    void OnUpdateFest(wxCommandEvent &e);
    void OnShowFestVersions(wxCommandEvent &e);
    void OnShowFestDbQuotas(wxCommandEvent &e);
    void OnPrescriptionContextMenu(const wxContextMenuEvent &e);
    void OnMerchPrescriptionContextMenu(const wxContextMenuEvent &e);
    void OnDispenseContextMenu(const wxContextMenuEvent &e);
    void OnPrescriptionDetails(const wxCommandEvent &e);
    void OnMerchPrescriptionDetails(const wxCommandEvent &e);
    void OnDispenseDetails(const wxCommandEvent &e);
    void OnPrescriptionRecall(const wxCommandEvent &e);
    void OnMerchPrescriptionRecall(const wxCommandEvent &e);
    void OnPrescriptionCease(const wxCommandEvent &e);
    void OnPrescriptionRenew(const wxCommandEvent &e);
    void OnMerchPrescriptionRenew(const wxCommandEvent &e);
    void OnPrescriptionRenewWithChanges(const wxCommandEvent &e);
    void OnConvertToWithoutPrescription(const wxCommandEvent &e);
    void OnMerchPrescriptionRenewWithChanges(const wxCommandEvent &e);
    void OnTreatmentEdit(const wxCommandEvent &e);
    void OnConnectToPll(const wxCommandEvent &e);
    void OnCaveContextMenu(const wxContextMenuEvent &e);
    void OnCaveDetails(const wxCommandEvent &e);
    void OnAddCaveMedicament(const wxCommandEvent &e);
    void OnEditCaveMedicament(const wxCommandEvent &e);
    void OnDeleteCaveMedicament(const wxCommandEvent &e);
};


#endif //DRWHATSNOT_THEMASTERFRAME_H
