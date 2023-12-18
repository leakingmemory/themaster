//
// Created by sigsegv on 12/13/23.
//

#ifndef DRWHATSNOT_THEMASTERFRAME_H
#define DRWHATSNOT_THEMASTERFRAME_H

#include <wx/wx.h>
#include <memory>

class PatientStore;
class PatientInformation;
class MedBundleData;
class wxListView;

enum {
    TheMaster_Connect_Id = 1,
    TheMaster_FindPatient_Id = 2,
    TheMaster_CreatePatient_Id = 3,
    TheMaster_GetMedication_Id = 4
};

class TheMasterFrame : public wxFrame {
private:
    std::shared_ptr<PatientStore> patientStore{};
    std::shared_ptr<PatientInformation> patientInformation;
    std::unique_ptr<MedBundleData> medicationBundle{};
    std::string url{};
    wxListView *header;
public:
    TheMasterFrame();
    void UpdateHeader();
    void OnConnect(wxCommandEvent &e);
    void OnFindPatient(wxCommandEvent &e);
    void OnCreatePatient(wxCommandEvent &e);
    void OnGetMedication(wxCommandEvent &e);
    void Connect(const std::string &url);
};


#endif //DRWHATSNOT_THEMASTERFRAME_H
