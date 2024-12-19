//
// Created by sigsegv on 12/18/24.
//

#ifndef THEMASTER_CONNECTTOPLLDIALOG_H
#define THEMASTER_CONNECTTOPLLDIALOG_H

#include <wx/wx.h>
#include <memory>

class wxListView;
class MedBundleData;
class FhirMedicationStatement;

class ConnectToPllDialog : public wxDialog {
private:
    wxListView *listView;
    wxButton *okButton;
    std::vector<std::shared_ptr<FhirMedicationStatement>> statements{};
public:
    ConnectToPllDialog(wxWindow *parent, const MedBundleData &bundleData);
    bool HasSelectedMedicationStatement() const;
    std::shared_ptr<FhirMedicationStatement> GetSelectedMedicationStatement() const;
    void RevalidateOkToProceed(wxCommandEvent &);
};


#endif //THEMASTER_CONNECTTOPLLDIALOG_H
