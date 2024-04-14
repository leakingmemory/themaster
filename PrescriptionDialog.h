//
// Created by sigsegv on 1/22/24.
//

#ifndef DRWHATSNOT_PRESCRIPTIONDIALOG_H
#define DRWHATSNOT_PRESCRIPTIONDIALOG_H

#include <wx/wx.h>
#include "MedicalCodedValue.h"
#include "PrescriptionData.h"
#include <memory>

class TheMasterFrame;
class wxSpinCtrl;
class wxSpinCtrlDouble;
class FhirMedication;

class PrescriptionDialog : public wxDialog {
private:
    PrescriptionData prescriptionData{};
    wxTextCtrl *dssnCtrl;
    wxSpinCtrlDouble *numberOfPackagesCtrl{};
    wxSpinCtrlDouble *amountCtrl{};
    wxComboBox *amountUnitCtrl{};
    wxSpinCtrl *reitCtrl{};
    wxTextCtrl *applicationAreaCtrl{};
    std::shared_ptr<FhirMedication> medication;
    std::vector<MedicalCodedValue> amountUnit;
public:
    PrescriptionDialog(TheMasterFrame *, const std::shared_ptr<FhirMedication> &, const std::vector<MedicalCodedValue> &, bool package = false);
    void OnCancel(wxCommandEvent &e);
    void OnProceed(wxCommandEvent &e);
    [[nodiscard]] PrescriptionData GetPrescriptionData() const {
        return prescriptionData;
    }
    [[nodiscard]] std::shared_ptr<FhirMedication> GetMedication() const {
        return medication;
    }
};


#endif //DRWHATSNOT_PRESCRIPTIONDIALOG_H
