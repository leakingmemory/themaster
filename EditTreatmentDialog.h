//
// Created by jeo on 11/13/24.
//

#ifndef THEMASTER_EDITTREATMENTDIALOG_H
#define THEMASTER_EDITTREATMENTDIALOG_H

#include <wx/wx.h>
#include <memory>

class MedBundleData;
class FhirMedicationStatement;

class EditTreatmentDialog : public wxDialog {
private:
    std::vector<std::string> practitionerUrls{};
    std::vector<std::string> practitionerDisplays{};
    std::shared_ptr<FhirMedicationStatement> medicationStatement;
    wxComboBox *institutedSelect;
public:
    EditTreatmentDialog(wxWindow *parent, const MedBundleData &medBundleData, const std::shared_ptr<FhirMedicationStatement> &);
    void OnCancel(wxCommandEvent &e);
    void OnOk(wxCommandEvent &e);
};


#endif //THEMASTER_EDITTREATMENTDIALOG_H
