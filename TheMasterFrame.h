//
// Created by sigsegv on 12/13/23.
//

#ifndef DRWHATSNOT_THEMASTERFRAME_H
#define DRWHATSNOT_THEMASTERFRAME_H

#include <wx/wx.h>
#include <memory>
#include <ctime>
#include "WeakRefUiDispatcher.h"

class PatientStore;
class PatientInformation;
class MedBundleData;
class wxListView;

enum {
    TheMaster_Connect_Id = 1,
    TheMaster_FindPatient_Id = 2,
    TheMaster_CreatePatient_Id = 3,
    TheMaster_GetMedication_Id = 4,
    TheMaster_SendMedication_Id = 5
};

class TheMasterFrame : public wxFrame {
private:
    std::shared_ptr<WeakRefUiDispatcher<TheMasterFrame>> weakRefDispatcher;
    std::shared_ptr<PatientStore> patientStore{};
    std::shared_ptr<PatientInformation> patientInformation;
    std::unique_ptr<MedBundleData> medicationBundle{};
    std::string url{};
    std::string helseidUrl{};
    std::string helseidClientId{};
    std::string helseidSecretJwk{};
    std::string helseidRefreshToken{};
    std::time_t helseidRefreshTokenValidTo{0};
    wxListView *header;
public:
    TheMasterFrame();
    void UpdateHeader();
    void OnConnect(wxCommandEvent &e);
    void OnFindPatient(wxCommandEvent &e);
    void OnCreatePatient(wxCommandEvent &e);
    void OnGetMedication(wxCommandEvent &e);
    void OnSendMedication(wxCommandEvent &e);
    WeakRefUiDispatcherRef<TheMasterFrame> GetWeakRefDispatcher();
    void SetHelseid(const std::string &url, const std::string &clientId, const std::string &secretJwk, const std::string &refreshToken, long expiresIn);
    void Connect(const std::string &url);
};


#endif //DRWHATSNOT_THEMASTERFRAME_H
