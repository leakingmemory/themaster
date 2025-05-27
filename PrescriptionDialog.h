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
#include <medfest/Struct/Decoded/LegemiddelCore.h>
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
class FhirBundleEntry;

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

template <class T> concept MedicamentMapper = requires (const T &obj) {
    {obj.GetPrescriptionValidity()} -> std::convertible_to<std::vector<PrescriptionValidity>>;
    {obj.GetMedicamentUses()} -> std::convertible_to<std::vector<MedicalCodedValue>>;
    {obj.GetMedicamentRefunds()} -> std::convertible_to<std::vector<MedicamentRefund>>;
    {obj.GetPrescriptionUnit()} -> std::convertible_to<std::vector<MedicalCodedValue>>;
    {obj.GetMedications()} -> std::convertible_to<std::vector<FhirBundleEntry>>;
    {obj.GetDisplay()} -> std::convertible_to<std::string>;
    {obj.GetMedicamentType()} -> std::convertible_to<std::vector<MedicalCodedValue>>;
    {obj.IsPackage()} -> std::convertible_to<bool>;
};

struct MedicationAlternativeInfo {
    std::unique_ptr<SfmMedicamentMapper> mapper;
    std::shared_ptr<LegemiddelCore> legemiddelCore;
    MedicationAlternativeInfo() = default;
    MedicationAlternativeInfo(const std::shared_ptr<FestDb> &festDb, const std::shared_ptr<LegemiddelCore> &legemiddelCore) : mapper(std::make_unique<SfmMedicamentMapper>(festDb, legemiddelCore)), legemiddelCore(legemiddelCore) {}
    MedicationAlternativeInfo(const std::shared_ptr<FestDb> &festDb, const LegemiddelCore &legemiddelCore) : MedicationAlternativeInfo(festDb, std::make_shared<LegemiddelCore>(legemiddelCore)) {}
};

class PrescriptionDialog : public wxDialog {
private:
    PrescriptionData prescriptionData{};
    wxStaticText *medicationDisplayName;
    wxButton *changeMedication;
    wxNotebook *mainNotebook;
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
    bool hasPackage{false};
    bool hasAmount{false};
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
    wxListView *altMedListView;
    wxButton *changeMedicationAccept{};
    std::shared_ptr<FestDb> festDb;
    std::vector<FhirBundleEntry> medication;
    std::vector<std::shared_ptr<MedicationAlternativeInfo>> medicationAlternatives{};
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
    template <MedicamentMapper Mapper> PrescriptionDialog(TheMasterFrame *, const std::shared_ptr<FestDb> &festDb, const LegemiddelCore &, const std::vector<MedicalCodedValue> &dosingUnit, const std::vector<MedicalCodedValue> &kortdoser, const Mapper &);
public:
    PrescriptionDialog(TheMasterFrame *, const std::shared_ptr<FestDb> &festDb, const std::shared_ptr<LegemiddelCore> &);
    void SwitchMed(const LegemiddelCore &legemiddelCore, const SfmMedicamentMapper &mapper);
    PrescriptionDialog(TheMasterFrame *, const std::vector<FhirBundleEntry> &substances, const std::shared_ptr<FhirMedication> &magistralMedication, const std::vector<MedicamentRefund> &);
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
    void OnChangeMedication(wxCommandEvent &e);
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
    void OnAltMedSelectChange(wxCommandEvent &e);
    void OnAltMed(wxCommandEvent &e);
    [[nodiscard]] PrescriptionData GetPrescriptionData() const {
        return prescriptionData;
    }
    [[nodiscard]] std::vector<FhirBundleEntry> GetMedications() const;
};


#endif //DRWHATSNOT_PRESCRIPTIONDIALOG_H
