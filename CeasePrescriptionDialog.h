//
// Created by sigsegv on 5/24/24.
//

#ifndef THEMASTER_CEASEPRESCRIPTIONDIALOG_H
#define THEMASTER_CEASEPRESCRIPTIONDIALOG_H

#include <wx/wx.h>
#include <memory>
#include "MedicalCodedValue.h"

class FhirMedicationStatement;

class CeasePrescriptionDialog : public wxDialog {
private:
    wxComboBox *cessationCode;
    wxTextCtrl *cessationReason;
    wxButton *ceaseButton;
    MedicalCodedValue reason{};
    std::string reasonText{};
public:
    CeasePrescriptionDialog(wxWindow *parent, const std::shared_ptr<FhirMedicationStatement> &medicationStatement);
    void OnCancel(wxCommandEvent &e);
    void OnCease(wxCommandEvent &e);
    void OnModified(wxCommandEvent &e);
    [[nodiscard]] MedicalCodedValue GetReason() const {
        return reason;
    }
    [[nodiscard]] std::string GetReasonText() const {
        return reasonText;
    }
};


#endif //THEMASTER_CEASEPRESCRIPTIONDIALOG_H
