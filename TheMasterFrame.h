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
#include "TheMasterIds.h"

class PatientStore;
class PatientInformation;
class MedBundleData;
class FhirBundle;
class FhirExtension;
class FhirReference;
class FhirCoding;
class wxListView;
namespace pplx {
    template<class ReturnType> class task;
}
class PrescriptionDialog;

class TheMasterFrame : public wxFrame {
private:
    std::shared_ptr<WeakRefUiDispatcher<TheMasterFrame>> weakRefDispatcher;
    std::shared_ptr<PatientStore> patientStore{};
    std::shared_ptr<PatientInformation> patientInformation;
    std::unique_ptr<MedBundleData> medicationBundle{};
    std::vector<std::shared_ptr<FhirMedicationStatement>> displayedMedicationStatements{};
    std::string lastRequest{};
    std::string lastResponse{};
    std::string url{};
    std::string helseidUrl{};
    std::string helseidClientId{};
    std::string helseidSecretJwk{};
    std::vector<std::string> helseidScopes{};
    std::string helseidRefreshToken{};
    std::time_t helseidRefreshTokenValidTo{0};
    std::string helseidIdToken{};
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
    void OnGetMedication(wxCommandEvent &e);
    static std::map<std::string,std::shared_ptr<FhirExtension>> GetRecallInfos(FhirBundle &bundle);
    void SendMedication(const std::function<void (const std::shared_ptr<FhirBundle> &)> &preprocessing, const std::function<void (const std::map<std::string,FhirCoding> &)> &pllResultsFunc);
    void OnSendMedication(wxCommandEvent &e);
    void OnSendPll(wxCommandEvent &e);
    void OnSaveLastRequest(wxCommandEvent &e);
    void OnSaveLast(wxCommandEvent &e);
    void OnSaveBundle(wxCommandEvent &e);
    void SetPrescriber(PrescriptionData &prescriptionData) const ;
    [[nodiscard]] FhirReference GetSubjectRef() const ;
    void SetPatient(PrescriptionData &prescriptionData) const ;
    //pplx::task<PrescriptionData> SetPrescriber(const PrescriptionData &prescriptionData);
    void PrescribeMedicament(const PrescriptionDialog &prescriptionDialog);
    void OnPrescribeMagistral(wxCommandEvent &e);
    void OnPrescribeMedicament(wxCommandEvent &e);
    WeakRefUiDispatcherRef<TheMasterFrame> GetWeakRefDispatcher();
    void SetHelseid(const std::string &url, const std::string &clientId, const std::string &secretJwk, const std::vector<std::string> &scopes, const std::string &refreshToken, long expiresIn, const std::string &idToken);
    void Connect(const std::string &url);
    void OnUpdateFest(wxCommandEvent &e);
    void OnShowFestVersions(wxCommandEvent &e);
    void OnShowFestDbQuotas(wxCommandEvent &e);
    void OnPrescriptionContextMenu(wxContextMenuEvent &e);
    void OnPrescriptionDetails(wxCommandEvent &e);
    void OnPrescriptionRecall(wxCommandEvent &e);
    void OnPrescriptionCease(wxCommandEvent &e);
};


#endif //DRWHATSNOT_THEMASTERFRAME_H
