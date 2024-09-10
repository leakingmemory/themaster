//
// Created by sigsegv on 5/2/24.
//

#ifndef DRWHATSNOT_PRESCRIPTIONDETAILSDIALOG_H
#define DRWHATSNOT_PRESCRIPTIONDETAILSDIALOG_H

#include <wx/wx.h>
#include <memory>

class FhirMedicationStatement;
class wxListView;

class PrescriptionDetailsDialog : public wxDialog {
private:
    std::vector<std::shared_ptr<FhirMedicationStatement>> statements{};
    wxListView *versionsView;
    wxListView *listView;
public:
    PrescriptionDetailsDialog(wxWindow *parent, const std::vector<std::shared_ptr<FhirMedicationStatement>> &);
private:
    void AddVersion(int row, const std::shared_ptr<FhirMedicationStatement> &);
    void DisplayStatement(const std::shared_ptr<FhirMedicationStatement> &);
public:
    void OnVersionSelect(wxCommandEvent &e);
};


#endif //DRWHATSNOT_PRESCRIPTIONDETAILSDIALOG_H
