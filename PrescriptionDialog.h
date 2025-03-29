//
// Created by sigsegv on 1/22/24.
//

#ifndef DRWHATSNOT_PRESCRIPTIONDIALOG_H
#define DRWHATSNOT_PRESCRIPTIONDIALOG_H

#include <wx/wx.h>
#include "MedicalCodedValue.h"
#include "PrescriptionData.h"
#include "AdvancedDosingPeriod.h"
#include "SfmMedicamentMapper.h"
#include "MedicamentRefund.h"
#include <memory>
#include <functional>

class TheMasterFrame;
class wxSpinCtrl;
class wxSpinCtrlDouble;
class wxNotebook;
class wxBookCtrlEvent;
class wxDatePickerCtrl;
class wxDateEvent;
class FhirMedication;

class MedicamentPackage {
private:
    std::shared_ptr<FhirMedication> medication;
    std::string description;
    std::vector<MedicamentRefund> refunds{};
public:
    MedicamentPackage(std::shared_ptr<FhirMedication> medication, std::string description) : medication(medication), description(description) {}
    [[nodiscard]] std::shared_ptr<FhirMedication> GetMedication() const {
        return medication;
    }
    [[nodiscard]] std::string GetDescription() const {
        return description;
    }
    [[nodiscard]] std::vector<MedicamentRefund> GetRefunds() const {
        return refunds;
    }
    void SetRefunds(const std::vector<MedicamentRefund> &refunds) {
        this->refunds = refunds;
    }
    void SetRefunds(std::vector<MedicamentRefund> &&refunds) {
        this->refunds = std::move(refunds);
    }
};

struct NumPackagesSizers;
struct PrescriptionDialogData;
class FestDb;
class ComboSearchControl;

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
    wxComboBox *refundSelection{nullptr};
    ComboSearchControl *refundCodeSelection{nullptr};
    wxSpinCtrl *reitCtrl{};
    wxComboBox *applicationAreaCtrl{};
    wxCheckBox *lockedPrescription{};
    wxComboBox *prescriptionValidityCtrl{};
    wxDatePickerCtrl *startDate{};
    wxDatePickerCtrl *expirationDate{};
    wxCheckBox *ceaseDateSet{};
    wxDatePickerCtrl *ceaseDate{};
    std::shared_ptr<FestDb> festDb;
    std::shared_ptr<FhirMedication> medication;
    std::vector<MedicamentPackage> packages;
    std::vector<MedicamentRefund> refunds;
    std::vector<MedicamentRefund> displayedRefunds{};
    std::vector<MedicalCodedValue> amountUnit;
    std::vector<MedicalCodedValue> dosingUnit;
    std::vector<MedicalCodedValue> kortdoser;
    std::vector<MedicalCodedValue> medicamentUses;
    std::vector<PrescriptionValidity> prescriptionValidity;
    std::vector<std::shared_ptr<AdvancedDosingPeriod>> dosingPeriods{};
    std::function<void (std::shared_ptr<AdvancedDosingPeriod> &&)> addDosingPeriod{[] (std::shared_ptr<AdvancedDosingPeriod> &&) {}};
    std::function<void ()> moveUp{[] () {}};
    std::function<void ()> moveDown{[] () {}};
    std::function<void ()> deleteDosingPeriod{[] () {}};
private:
    NumPackagesSizers CreateNumPackages(wxWindow *parent);
    wxBoxSizer *CreateAmount(wxWindow *parent);
public:
    PrescriptionDialog(TheMasterFrame *, const std::shared_ptr<FestDb> &festDb, const std::shared_ptr<FhirMedication> &, const std::vector<MedicalCodedValue> &amountUnit, const std::vector<MedicalCodedValue> &medicamentType, const std::vector<MedicalCodedValue> &listOfMedicamentUses, bool package = false, const std::vector<MedicamentPackage> &packages = {}, const std::vector<MedicamentRefund> &refunds = {}, const std::vector<MedicalCodedValue> &dosingUnit = {}, const std::vector<MedicalCodedValue> &kortdoser = {}, const std::vector<PrescriptionValidity> &prescriptionValidity = {});
    PrescriptionDialog & operator += (const PrescriptionData &);
    void OnCancel(wxCommandEvent &e);
private:
    void PopulateRefunds(const std::vector<MedicamentRefund> &refunds);
    [[nodiscard]] PrescriptionDialogData GetDialogData() const;
    void ProcessDialogData(PrescriptionDialogData &) const;
    bool IsValid(const PrescriptionDialogData &dialogData) const;
    void OnModified();
    void OnModifiedCeaseIsSet();
    void OnPotentiallyModifiedPackageSelecion();
    void OnPotentiallyModifiedRefundSelection();
public:
    void OnModified(wxCommandEvent &e);
    void OnModifiedPackageSelection(wxCommandEvent &e);
    void OnModifiedRefundSelection(wxCommandEvent &e);
    void OnModifiedPC(wxBookCtrlEvent &e);
    void OnModifiedCeaseIsSet(wxCommandEvent &e);
    void OnModifiedDate(wxDateEvent &e);
    void OnModifiedExpiryDate(wxDateEvent &e);
    void OnModifiedPrescriptionValidity(wxCommandEvent &e);
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
