//
// Created by sigsegv on 12/13/23.
//

#ifndef DRWHATSNOT_FINDPATIENTDIALOG_H
#define DRWHATSNOT_FINDPATIENTDIALOG_H

#include "PatientStore.h"
#include <wx/wx.h>
#include <memory>

class TheMasterFrame;
class wxListView;

class FindPatientDialog : public wxDialog {
private:
    std::shared_ptr<PatientStore> patientStore;
    std::vector<PatientInformation> patients{};
    std::shared_ptr<PatientInformation> patient{};
    wxTextCtrl *searchInput;
    wxListView *listView;
    wxButton *okButton;
public:
    FindPatientDialog(const std::shared_ptr<PatientStore> &patientStore, TheMasterFrame *);
    void OnText(wxCommandEvent &e);
    void OnSelect(wxCommandEvent &e);
    std::shared_ptr<PatientInformation> GetPatient() const {
        return patient;
    }
};


#endif //DRWHATSNOT_FINDPATIENTDIALOG_H
