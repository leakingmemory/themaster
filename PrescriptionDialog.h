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
class wxNotebook;
class wxBookCtrlEvent;
class FhirMedication;

class MedicamentPackage {
private:
    std::shared_ptr<FhirMedication> medication;
    std::string description;
public:
    MedicamentPackage(std::shared_ptr<FhirMedication> medication, std::string description) : medication(medication), description(description) {}
    [[nodiscard]] std::shared_ptr<FhirMedication> GetMedication() const {
        return medication;
    }
    [[nodiscard]] std::string GetDescription() const {
        return description;
    }
};

struct NumPackagesSizers;
struct PrescriptionDialogData;
class FestDb;

class PrescriptionDialog : public wxDialog {
private:
    PrescriptionData prescriptionData{};
    wxButton *proceedButton;
    wxRadioBox *typeSelection;
    wxNotebook *dosingNotebook;
    wxTextCtrl *dssnCtrl;
    wxComboBox *kortdoseDosingUnitCtrl;
    wxComboBox *kortdoserCtrl;
    wxNotebook *packageAmountNotebook{nullptr};
    wxComboBox *selectPackage{nullptr};
    wxSpinCtrlDouble *numberOfPackagesCtrl{nullptr};
    wxSpinCtrlDouble *amountCtrl{nullptr};
    wxComboBox *amountUnitCtrl{nullptr};
    wxSpinCtrl *reitCtrl{};
    wxTextCtrl *applicationAreaCtrl{};
    std::shared_ptr<FestDb> festDb;
    std::shared_ptr<FhirMedication> medication;
    std::vector<MedicamentPackage> packages;
    std::vector<MedicalCodedValue> amountUnit;
    std::vector<MedicalCodedValue> dosingUnit;
    std::vector<MedicalCodedValue> kortdoser;
private:
    NumPackagesSizers CreateNumPackages(wxWindow *parent);
    wxBoxSizer *CreateAmount(wxWindow *parent);
public:
    PrescriptionDialog(TheMasterFrame *, const std::shared_ptr<FestDb> &festDb, const std::shared_ptr<FhirMedication> &, const std::vector<MedicalCodedValue> &amountUnit, bool package = false, const std::vector<MedicamentPackage> &packages = {}, const std::vector<MedicalCodedValue> &dosingUnit = {}, const std::vector<MedicalCodedValue> &kortdoser = {});
    void OnCancel(wxCommandEvent &e);
private:
    [[nodiscard]] PrescriptionDialogData GetDialogData() const;
    void ProcessDialogData(PrescriptionDialogData &) const;
    bool IsValid(const PrescriptionDialogData &dialogData) const;
    void OnModified();
public:
    void OnModified(wxCommandEvent &e);
    void OnModifiedPC(wxBookCtrlEvent &e);
    void OnProceed(wxCommandEvent &e);
    [[nodiscard]] PrescriptionData GetPrescriptionData() const {
        return prescriptionData;
    }
    [[nodiscard]] std::shared_ptr<FhirMedication> GetMedication() const {
        return medication;
    }
};


#endif //DRWHATSNOT_PRESCRIPTIONDIALOG_H
