//
// Created by sigsegv on 5/7/24.
//

#ifndef DRWHATSNOT_RECALLPRESCRIPTIONDIALOG_H
#define DRWHATSNOT_RECALLPRESCRIPTIONDIALOG_H

#include <wx/wx.h>
#include <memory>
#include "MedicalCodedValue.h"

class FhirMedicationStatement;

class RecallPrescriptionDialog : public wxDialog {
private:
    wxComboBox *recallCode;
    wxButton *recallButton;
    MedicalCodedValue reason{};
public:
    RecallPrescriptionDialog(wxWindow *parent, const std::shared_ptr<FhirMedicationStatement> &);
    void OnCancel(wxCommandEvent &e);
    void OnRecall(wxCommandEvent &e);
    void OnModified(wxCommandEvent &e);
    [[nodiscard]] MedicalCodedValue GetReason() const {
        return reason;
    }
};


#endif //DRWHATSNOT_RECALLPRESCRIPTIONDIALOG_H
