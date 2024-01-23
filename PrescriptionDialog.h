//
// Created by sigsegv on 1/22/24.
//

#ifndef DRWHATSNOT_PRESCRIPTIONDIALOG_H
#define DRWHATSNOT_PRESCRIPTIONDIALOG_H

#include <wx/wx.h>
#include "MedicalCodedValue.h"

class TheMasterFrame;
class wxSpinCtrl;
class wxSpinCtrlDouble;

struct PrescriptionData {
    std::string reseptdate{}; // I dag, YYYY-MM-DD
    std::string expirationdate{}; // Gyldig til (typ 1 år), YYYY-MM-DD
    std::string festUpdate{"2023-12-20T11:54:48.9287539+0000"}; // TODO
    bool guardianTransparencyReservation{false};
    bool inDoctorsName{false};
    bool lockedPrescription{false};
    std::string dssn{};
    bool numberOfPackagesSet{false};
    double numberOfPackages{};
    int reit{0};
    MedicalCodedValue itemGroup{};
    MedicalCodedValue reimbursementCode{};
    MedicalCodedValue reimbursementParagraph{};
    MedicalCodedValue rfstatus{};
    std::string lastChanged{};
    MedicalCodedValue typeresept{};
    MedicalCodedValue use{};
    std::string applicationArea{};
};

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
};


#endif //DRWHATSNOT_PRESCRIPTIONDIALOG_H
