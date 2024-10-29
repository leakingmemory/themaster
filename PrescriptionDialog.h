//
// Created by sigsegv on 1/22/24.
//

#ifndef DRWHATSNOT_PRESCRIPTIONDIALOG_H
#define DRWHATSNOT_PRESCRIPTIONDIALOG_H

#include <wx/wx.h>
#include "MedicalCodedValue.h"
#include "PrescriptionData.h"
#include "AdvancedDosingPeriod.h"
#include <memory>
#include <functional>

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
    wxRadioBox *useSelection;
    wxNotebook *dosingNotebook;
    wxTextCtrl *dssnCtrl;
    wxComboBox *kortdoseDosingUnitCtrl;
    wxComboBox *kortdoserCtrl;
    wxComboBox *dosingPeriodsDosingUnitCtrl;
    wxListView *dosingPeriodsView;
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
    std::vector<std::shared_ptr<AdvancedDosingPeriod>> dosingPeriods{};
    std::function<void (std::shared_ptr<AdvancedDosingPeriod> &&)> addDosingPeriod{[] (std::shared_ptr<AdvancedDosingPeriod> &&) {}};
    std::function<void ()> moveUp{[] () {}};
    std::function<void ()> moveDown{[] () {}};
    std::function<void ()> deleteDosingPeriod{[] () {}};
private:
    NumPackagesSizers CreateNumPackages(wxWindow *parent);
    wxBoxSizer *CreateAmount(wxWindow *parent);
public:
    PrescriptionDialog(TheMasterFrame *, const std::shared_ptr<FestDb> &festDb, const std::shared_ptr<FhirMedication> &, const std::vector<MedicalCodedValue> &amountUnit, const std::vector<MedicalCodedValue> &medicamentType, bool package = false, const std::vector<MedicamentPackage> &packages = {}, const std::vector<MedicalCodedValue> &dosingUnit = {}, const std::vector<MedicalCodedValue> &kortdoser = {});
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
    void OnDosingPeriodsContextMenu(wxContextMenuEvent &e);
    void OnAddDosingPeriod(wxCommandEvent &e);
    void OnMoveUp(wxCommandEvent &e);
    void OnMoveDown(wxCommandEvent &e);
    void OnDeleteDosingPeriod(wxCommandEvent &e);
    [[nodiscard]] PrescriptionData GetPrescriptionData() const {
        return prescriptionData;
    }
    [[nodiscard]] std::shared_ptr<FhirMedication> GetMedication() const {
        return medication;
    }
};


#endif //DRWHATSNOT_PRESCRIPTIONDIALOG_H
