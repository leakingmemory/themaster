//
// Created by sigsegv on 1/22/24.
//

#ifndef DRWHATSNOT_PRESCRIPTIONDIALOG_H
#define DRWHATSNOT_PRESCRIPTIONDIALOG_H

#include <wx/wx.h>
#include "MedicalCodedValue.h"
#include "PrescriptionData.h"

class TheMasterFrame;
class wxSpinCtrl;
class wxSpinCtrlDouble;

class PrescriptionDialog : public wxDialog {
private:
    PrescriptionData prescriptionData{};
    wxTextCtrl *dssnCtrl;
    wxSpinCtrlDouble *numberOfPackagesCtrl{};
    wxSpinCtrl *reitCtrl{};
    wxTextCtrl *applicationAreaCtrl{};
public:
    PrescriptionDialog(TheMasterFrame *);
    void OnCancel(wxCommandEvent &e);
    void OnProceed(wxCommandEvent &e);
    [[nodiscard]] PrescriptionData GetPrescriptionData() const {
        return prescriptionData;
    }
};


#endif //DRWHATSNOT_PRESCRIPTIONDIALOG_H
