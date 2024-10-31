//
// Created by sigsegv on 12/13/23.
//

#ifndef DRWHATSNOT_THEMASTERFRAME_H
#define DRWHATSNOT_THEMASTERFRAME_H

#include <wx/wx.h>
#include <memory>
#include <ctime>
#include <map>
#include "WeakRefUiDispatcher.h"
#include "PrescriptionData.h"
#include "MedBundleData.h"
#include "TheMasterIds.h"

class PatientStore;
class PatientInformation;
class PrescriberRef;
class FhirBundle;
class FhirExtension;
class FhirReference;
class FhirCoding;
class wxListView;
namespace pplx {
    template<class ReturnType> class task;
}
class PrescriptionDialog;
class CallContext;

class TheMasterFrame : public wxFrame {
private:
    std::shared_ptr<WeakRefUiDispatcher<TheMasterFrame>> weakRefDispatcher;
    std::shared_ptr<PatientStore> patientStore{};
    std::shared_ptr<PatientInformation> patientInformation;
    std::string medicationBundleResetData{};
    std::unique_ptr<MedBundleData> medicationBundle{};
    std::vector<std::vector<std::shared_ptr<FhirMedicationStatement>>> displayedMedicationStatements{};
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
    wxListView *header;
    wxListView *prescriptions;
public:
    TheMasterFrame();
    void UpdateHeader();
    void UpdateMedications();
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
    [[nodiscard]] FhirReference GetSubjectRef() const ;
    void SetPatient(PrescriptionData &prescriptionData) const ;
    //pplx::task<PrescriptionData> SetPrescriber(const PrescriptionData &prescriptionData);
    void PrescribeMedicament(const PrescriptionDialog &prescriptionDialog);
    void OnPrescribeMagistral(wxCommandEvent &e);
    void OnPrescribeMedicament(wxCommandEvent &e);
    WeakRefUiDispatcherRef<TheMasterFrame> GetWeakRefDispatcher();
    void SetHelseid(const std::string &url, const std::string &clientId, const std::string &secretJwk, const std::vector<std::string> &scopes, const std::string &refreshToken, long expiresIn, const std::string &idToken, const std::string &journalId, const std::string &orgNo, const std::string &childOrgNo);
    void Connect(const std::string &url);
    void OnUpdateFest(wxCommandEvent &e);
    void OnShowFestVersions(wxCommandEvent &e);
    void OnShowFestDbQuotas(wxCommandEvent &e);
    void OnPrescriptionContextMenu(wxContextMenuEvent &e);
    void OnPrescriptionDetails(wxCommandEvent &e);
    void OnPrescriptionRecall(wxCommandEvent &e);
    void OnPrescriptionCease(wxCommandEvent &e);
    void OnPrescriptionRenew(wxCommandEvent &e);
};


#endif //DRWHATSNOT_THEMASTERFRAME_H
