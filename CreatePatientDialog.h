//
// Created by sigsegv on 12/15/23.
//

#ifndef DRWHATSNOT_CREATEPATIENTDIALOG_H
#define DRWHATSNOT_CREATEPATIENTDIALOG_H

#include <wx/wx.h>
#include <memory>

class PatientStore;
class TheMasterFrame;
class PatientInformation;

class CreatePatientDialog : public wxDialog {
private:
    std::shared_ptr<PatientStore> patientStore;
    wxChoice* patientIDTypeChoice;
    wxTextCtrl* patientIDField;
    wxTextCtrl* firstNameField;
    wxTextCtrl* lastNameField;
    wxChoice* genderChoice;
    wxTextCtrl* dobField;
    wxTextCtrl* postCodeField;
    wxTextCtrl* cityField;
public:
    CreatePatientDialog(const std::shared_ptr<PatientStore> &patientStore, TheMasterFrame *);
    PatientInformation GetPatientInformation();
};


#endif //DRWHATSNOT_CREATEPATIENTDIALOG_H
