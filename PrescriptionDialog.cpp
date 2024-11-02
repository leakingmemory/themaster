//
// Created by sigsegv on 1/22/24.
//

#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include <wx/listctrl.h>
#include <wx/datectrl.h>
#include <wx/dateevt.h>
#include "PrescriptionDialog.h"
#include "FestDb.h"
#include "TheMasterFrame.h"
#include "AdvancedDosingPeriodDialog.h"
#include <iomanip>
#include <sstream>
#include <medfest/Struct/Decoded/OppfKodeverk.h>
#include <array>

struct NumPackagesSizers {
    wxBoxSizer *packageSelectorSizer;
    wxBoxSizer *numPackagesSizer;
};

NumPackagesSizers PrescriptionDialog::CreateNumPackages(wxWindow *parent) {
    NumPackagesSizers sizers{};
    if (!packages.empty()) {
        sizers.packageSelectorSizer = new wxBoxSizer(wxHORIZONTAL);
        auto *packageSelectorLabel = new wxStaticText(parent, wxID_ANY, wxT("Select package:"));
        selectPackage = new wxComboBox(parent, wxID_ANY);
        selectPackage->SetEditable(false);
        for (const auto &p : packages) {
            selectPackage->Append(p.GetDescription());
        }
        sizers.packageSelectorSizer->Add(packageSelectorLabel, 0, wxEXPAND | wxALL, 5);
        sizers.packageSelectorSizer->Add(selectPackage, 1, wxEXPAND | wxALL, 5);
    }
    sizers.numPackagesSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *numPackagesLabel = new wxStaticText(parent, wxID_ANY, wxT("Num packages:"));
    numberOfPackagesCtrl = new wxSpinCtrlDouble(parent, wxID_ANY);
    sizers.numPackagesSizer->Add(numPackagesLabel, 0, wxEXPAND | wxALL, 5);
    sizers.numPackagesSizer->Add(numberOfPackagesCtrl, 1, wxEXPAND | wxALL, 5);
    return sizers;
}

wxBoxSizer *PrescriptionDialog::CreateAmount(wxWindow *parent) {
    auto *amountSizer = new wxBoxSizer(wxHORIZONTAL);
    auto amountLabel = new wxStaticText(parent, wxID_ANY, wxT("Amount:"));
    amountCtrl = new wxSpinCtrlDouble(parent, wxID_ANY);
    amountUnitCtrl = new wxComboBox(parent, wxID_ANY);
    amountUnitCtrl->SetEditable(false);
    for (const auto &unit : amountUnit) {
        amountUnitCtrl->Append(unit.GetDisplay());
    }
    if (!amountUnit.empty()) {
        amountUnitCtrl->SetSelection(0);
    }
    amountSizer->Add(amountLabel, 0, wxEXPAND | wxALL, 5);
    amountSizer->Add(amountCtrl, 1, wxEXPAND | wxALL, 5);
    amountSizer->Add(amountUnitCtrl, 1, wxEXPAND | wxALL, 5);
    return amountSizer;
}

static wxDateTime ToWxDateTime(DateOnly dateOnly) {
    auto setTime = wxDateTime::Now();
    setTime.ResetTime();
    setTime.SetMonth(wxDateTime::Jan);
    constexpr std::array<wxDateTime::Month,12> monthMap = {wxDateTime::Jan, wxDateTime::Feb, wxDateTime::Mar, wxDateTime::Apr, wxDateTime::May, wxDateTime::Jun, wxDateTime::Jul, wxDateTime::Aug, wxDateTime::Sep, wxDateTime::Oct, wxDateTime::Nov, wxDateTime::Dec};
    setTime.SetYear(dateOnly.GetYear());
    setTime.SetDay(dateOnly.GetDayOfMonth());
    setTime.SetMonth(monthMap.at(dateOnly.GetMonth() - 1));
    return setTime;
}

PrescriptionDialog::PrescriptionDialog(TheMasterFrame *frame, const std::shared_ptr<FestDb> &festDb, const std::shared_ptr<FhirMedication> &medication, const std::vector<MedicalCodedValue> &amountUnit, const std::vector<MedicalCodedValue> &medicamentType, bool package, const std::vector<MedicamentPackage> &packages, const std::vector<MedicalCodedValue> &dosingUnit, const std::vector<MedicalCodedValue> &kortdoser, const std::vector<PrescriptionValidity> &prescriptionValidity) : wxDialog(frame, wxID_ANY, wxT("Prescription")), festDb(festDb), medication(medication), amountUnit(amountUnit), packages(packages), dosingUnit(dosingUnit), kortdoser(kortdoser), prescriptionValidity(prescriptionValidity) {
    DateOnly startDate = DateOnly::Today();
    DateOnly endDate = startDate;
    endDate.AddYears(1);
    auto *vSizer = new wxBoxSizer(wxVERTICAL);
    auto *hzSizer = new wxBoxSizer(wxHORIZONTAL);
    {
        auto *sizer = new wxBoxSizer(wxVERTICAL);
        wxArrayString typeSelectionChoices{};
        typeSelectionChoices.Add(wxT("E-prescription"));
        typeSelectionChoices.Add(wxT("PLL-entry"));
        typeSelection = new wxRadioBox(this, wxID_ANY, wxT("Record type:"), wxDefaultPosition, wxDefaultSize,
                                       typeSelectionChoices, typeSelectionChoices.size(), wxRA_HORIZONTAL);
        wxArrayString useSelectionChoices{};
        useSelectionChoices.Add(wxT("Permanent"));
        useSelectionChoices.Add(wxT("When needed"));
        useSelectionChoices.Add(wxT("Cure"));
        useSelectionChoices.Add(wxT("Vaccine"));
        useSelectionChoices.Add(wxT("Nutrition"));
        useSelection = new wxRadioBox(this, wxID_ANY, wxT("Use:"), wxDefaultPosition, wxDefaultSize,
                                      useSelectionChoices, useSelectionChoices.size(), wxRA_HORIZONTAL);
        if (!medicamentType.empty()) {
            auto mt = medicamentType[0];
            auto mtCode = mt.GetCode();
            if (mtCode == "6") {
                useSelection->SetSelection(3);
            } else if (mtCode == "16") {
                useSelection->SetSelection(4);
            }
        }
        wxBoxSizer *packageSelectorSizer = nullptr;
        wxBoxSizer *numPackagesSizer = nullptr;
        wxBoxSizer *amountSizer = nullptr;
        if (!package && !packages.empty()) {
            packageAmountNotebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);
            auto packagesPage = new wxPanel(packageAmountNotebook, wxID_ANY);
            {
                auto sizers = CreateNumPackages(packagesPage);
                auto *pageSizer = new wxBoxSizer(wxVERTICAL);
                pageSizer->Add(sizers.packageSelectorSizer, 0, wxEXPAND | wxALL, 5);
                pageSizer->Add(sizers.numPackagesSizer, 0, wxEXPAND | wxALL, 5);
                packagesPage->SetSizerAndFit(pageSizer);
            }
            packageAmountNotebook->AddPage(packagesPage, wxT("Package"));
            auto amountPage = new wxPanel(packageAmountNotebook, wxID_ANY);
            {
                auto *sizer = CreateAmount(amountPage);
                amountPage->SetSizerAndFit(sizer);
            }
            packageAmountNotebook->AddPage(amountPage, wxT("Amount"));
        } else if (package || !packages.empty()) {
            auto sizers = CreateNumPackages(this);
            packageSelectorSizer = sizers.packageSelectorSizer;
            numPackagesSizer = sizers.numPackagesSizer;
        } else {
            amountSizer = CreateAmount(this);
        }
        auto *reitSizer = new wxBoxSizer(wxHORIZONTAL);
        auto *reitLabel = new wxStaticText(this, wxID_ANY, wxT("Reit:"));
        reitCtrl = new wxSpinCtrl(this, wxID_ANY);
        reitSizer->Add(reitLabel, 0, wxEXPAND | wxALL, 5);
        reitSizer->Add(reitCtrl, 1, wxEXPAND | wxALL, 5);
        auto *applicationAreaSizer = new wxBoxSizer(wxHORIZONTAL);
        auto *applicationAreaLabel = new wxStaticText(this, wxID_ANY, wxT("Application:"));
        applicationAreaCtrl = new wxTextCtrl(this, wxID_ANY);
        applicationAreaSizer->Add(applicationAreaLabel, 0, wxEXPAND | wxALL, 5);
        applicationAreaSizer->Add(applicationAreaCtrl, 1, wxEXPAND | wxALL, 5);
        sizer->Add(typeSelection, 0, wxEXPAND | wxALL, 5);
        sizer->Add(useSelection, 0, wxEXPAND | wxALL, 5);
        if (packageAmountNotebook != nullptr) {
            sizer->Add(packageAmountNotebook, 0, wxEXPAND | wxALL, 5);
        }
        if (packageSelectorSizer != nullptr) {
            sizer->Add(packageSelectorSizer, 0, wxEXPAND | wxALL, 5);
        }
        if (numPackagesSizer != nullptr) {
            sizer->Add(numPackagesSizer, 0, wxEXPAND | wxALL, 5);
        }
        if (amountSizer != nullptr) {
            sizer->Add(amountSizer, 0, wxEXPAND | wxALL, 5);
        }
        sizer->Add(reitSizer, 0, wxEXPAND | wxALL, 5);
        sizer->Add(applicationAreaSizer, 0, wxEXPAND | wxALL, 5);
        hzSizer->Add(sizer, 0, wxEXPAND | wxALL, 5);
    }
    {
        auto *sizer = new wxBoxSizer(wxVERTICAL);
        dosingNotebook = new wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNB_TOP);
        auto *dssnPage = new wxPanel(dosingNotebook, wxID_ANY);
        auto *dssnSizer = new wxBoxSizer(wxHORIZONTAL);
        auto *dssnLabel = new wxStaticText(dssnPage, wxID_ANY, wxT("DSSN:"));
        dssnCtrl = new wxTextCtrl(dssnPage, wxID_ANY);
        dssnSizer->Add(dssnLabel, 0, wxEXPAND | wxALL, 5);
        dssnSizer->Add(dssnCtrl, 1, wxEXPAND | wxALL, 5);
        dssnPage->SetSizerAndFit(dssnSizer);
        dosingNotebook->AddPage(dssnPage, wxT("DSSN"));
        auto *kortdoserPage = new wxPanel(dosingNotebook, wxID_ANY);
        auto *kortdoserPageSizer = new wxBoxSizer(wxVERTICAL);
        {
            auto *kortdoseDosingUnitSizer = new wxBoxSizer(wxHORIZONTAL);
            auto *kortdoseDosingUnitLabel = new wxStaticText(kortdoserPage, wxID_ANY, wxT("Doseringsenhet:"));
            kortdoseDosingUnitCtrl = new wxComboBox(kortdoserPage, wxID_ANY);
            kortdoseDosingUnitCtrl->SetEditable(false);
            for (auto &unit: dosingUnit) {
                auto display{unit.GetCode()};
                display.append(" ");
                display.append(unit.GetDisplay());
                kortdoseDosingUnitCtrl->Append(display);
            }
            if (dosingUnit.size() == 1) {
                kortdoseDosingUnitCtrl->SetSelection(0);
            }
            kortdoseDosingUnitSizer->Add(kortdoseDosingUnitLabel, 0, wxEXPAND | wxALL, 5);
            kortdoseDosingUnitSizer->Add(kortdoseDosingUnitCtrl, 1, wxEXPAND | wxALL, 5);
            kortdoserPageSizer->Add(kortdoseDosingUnitSizer, 0, wxEXPAND | wxALL, 5);
        }
        auto *kortdoserSizer = new wxBoxSizer(wxHORIZONTAL);
        auto *kortdoserLabel = new wxStaticText(kortdoserPage, wxID_ANY, wxT("Kortdoser:"));
        kortdoserCtrl = new wxComboBox(kortdoserPage, wxID_ANY);
        kortdoserCtrl->SetEditable(false);
        for (const auto &kd: kortdoser) {
            std::string label{kd.GetCode()};
            label.append(" ");
            label.append(kd.GetDisplay());
            kortdoserCtrl->Append(label);
        }
        kortdoserSizer->Add(kortdoserLabel, 0, wxEXPAND | wxALL, 5);
        kortdoserSizer->Add(kortdoserCtrl, 1, wxEXPAND | wxALL, 5);
        kortdoserPageSizer->Add(kortdoserSizer, 0, wxEXPAND | wxALL, 5);
        kortdoserPage->SetSizerAndFit(kortdoserPageSizer);
        dosingNotebook->AddPage(kortdoserPage, wxT("Kortdose"));
        auto *advancedDosingPage = new wxPanel(dosingNotebook, wxID_ANY);
        auto *advancedDosingSizer = new wxBoxSizer(wxVERTICAL);
        {
            auto *dosingPeriodsDosingUnitSizer = new wxBoxSizer(wxHORIZONTAL);
            auto *dosingPeriodsDosingUnitLabel = new wxStaticText(advancedDosingPage, wxID_ANY, wxT("Doseringsenhet:"));
            dosingPeriodsDosingUnitCtrl = new wxComboBox(advancedDosingPage, wxID_ANY);
            dosingPeriodsDosingUnitCtrl->SetEditable(false);
            for (auto &unit: dosingUnit) {
                auto display{unit.GetCode()};
                display.append(" ");
                display.append(unit.GetDisplay());
                dosingPeriodsDosingUnitCtrl->Append(wxString::FromUTF8(display));
            }
            if (dosingUnit.size() == 1) {
                dosingPeriodsDosingUnitCtrl->SetSelection(0);
            }
            dosingPeriodsDosingUnitSizer->Add(dosingPeriodsDosingUnitLabel, 0, wxEXPAND | wxALL, 5);
            dosingPeriodsDosingUnitSizer->Add(dosingPeriodsDosingUnitCtrl, 1, wxEXPAND | wxALL, 5);
            advancedDosingSizer->Add(dosingPeriodsDosingUnitSizer, 0, wxEXPAND | wxALL, 5);
        }
        dosingPeriodsView = new wxListView(advancedDosingPage, wxID_ANY);
        dosingPeriodsView->AppendColumn(wxT("Dosing periods"));
        dosingPeriodsView->Bind(wxEVT_CONTEXT_MENU, &PrescriptionDialog::OnDosingPeriodsContextMenu, this, wxID_ANY);
        advancedDosingSizer->Add(dosingPeriodsView, 0, wxALL | wxEXPAND, 5);
        advancedDosingPage->SetSizerAndFit(advancedDosingSizer);
        dosingNotebook->AddPage(advancedDosingPage, wxT("Avansert"));
        sizer->Add(dosingNotebook, 0, wxEXPAND | wxALL, 5);
        auto *prescriptionValiditySizer = new wxBoxSizer(wxHORIZONTAL);
        prescriptionValiditySizer->Add(new wxStaticText(this, wxID_ANY, wxT("Validity: ")), 0, wxEXPAND | wxALL, 5);
        prescriptionValidityCtrl = new wxComboBox(this, wxID_ANY);
        prescriptionValidityCtrl->SetEditable(false);
        prescriptionValidityCtrl->Append(wxT(""));
        for (const auto &pv : prescriptionValidity) {
            std::string str{pv.duration.ToString()};
            if (!pv.gender.GetCode().empty() || !pv.gender.GetDisplay().empty()) {
                str.append(" (");
                str.append(pv.gender.GetCode());
                str.append(" ");
                str.append(pv.gender.GetDisplay());
                str.append(")");
            }
            prescriptionValidityCtrl->Append(wxString::FromUTF8(str));
        }
        if (!prescriptionValidity.empty()) {
            prescriptionValidityCtrl->SetSelection(1);
            endDate = DateOnly::Today() + prescriptionValidity[0].duration;
        }
        prescriptionValiditySizer->Add(prescriptionValidityCtrl, 0, wxEXPAND | wxALL, 5);
        sizer->Add(prescriptionValiditySizer, 0, wxEXPAND | wxALL, 5);
        auto *startSizer = new wxBoxSizer(wxHORIZONTAL);
        startSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Start: ")), 0, wxEXPAND | wxALL, 5);
        this->startDate = new wxDatePickerCtrl(this, wxID_ANY, ToWxDateTime(startDate));
        startSizer->Add(this->startDate, 0, wxEXPAND | wxALL, 5);
        sizer->Add(startSizer, 0, wxEXPAND | wxALL, 5);
        auto *expireSizer = new wxBoxSizer(wxHORIZONTAL);
        expireSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Expires: ")), 0, wxEXPAND | wxALL, 5);
        expirationDate = new wxDatePickerCtrl(this, wxID_ANY, ToWxDateTime(endDate));
        expireSizer->Add(expirationDate, 0, wxEXPAND | wxALL, 5);
        sizer->Add(expireSizer, 0, wxEXPAND | wxALL, 5);
        auto *ceaseSizer = new wxBoxSizer(wxHORIZONTAL);
        ceaseDateSet = new wxCheckBox(this, wxID_ANY, wxT("Cease: "));
        ceaseSizer->Add(ceaseDateSet, 0, wxEXPAND | wxALL, 5);
        ceaseDate = new wxDatePickerCtrl(this, wxID_ANY);
        ceaseDate->Enable(false);
        ceaseSizer->Add(ceaseDate, 0, wxEXPAND | wxALL, 5);
        sizer->Add(ceaseSizer, 0, wxEXPAND | wxALL, 5);
        hzSizer->Add(sizer, 0, wxEXPAND | wxALL, 5);
    }
    vSizer->Add(hzSizer, 0, wxEXPAND | wxALL, 5);
    auto *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *cancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"));
    proceedButton = new wxButton(this, wxID_ANY, wxT("Finish"));
    proceedButton->Enable(false);
    buttonsSizer->Add(cancelButton, 0, wxEXPAND | wxALL, 5);
    buttonsSizer->Add(proceedButton, 0, wxEXPAND | wxALL, 5);
    vSizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(vSizer);
    dosingNotebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &PrescriptionDialog::OnModified, this);
    dssnCtrl->Bind(wxEVT_TEXT, &PrescriptionDialog::OnModified, this);
    kortdoseDosingUnitCtrl->Bind(wxEVT_COMBOBOX, &PrescriptionDialog::OnModified, this);
    kortdoserCtrl->Bind(wxEVT_COMBOBOX, &PrescriptionDialog::OnModified, this);
    this->startDate->Bind(wxEVT_DATE_CHANGED, &PrescriptionDialog::OnModifiedDate, this);
    expirationDate->Bind(wxEVT_DATE_CHANGED, &PrescriptionDialog::OnModifiedExpiryDate, this);
    ceaseDateSet->Bind(wxEVT_CHECKBOX, &PrescriptionDialog::OnModifiedCeaseIsSet, this);
    ceaseDate->Bind(wxEVT_DATE_CHANGED, &PrescriptionDialog::OnModifiedDate, this);
    if (packageAmountNotebook != nullptr) {
        packageAmountNotebook->Bind(wxEVT_NOTEBOOK_PAGE_CHANGED, &PrescriptionDialog::OnModifiedPC, this);
    }
    if (selectPackage != nullptr) {
        selectPackage->Bind(wxEVT_COMBOBOX, &PrescriptionDialog::OnModified, this);
    }
    if (numberOfPackagesCtrl != nullptr) {
        numberOfPackagesCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &PrescriptionDialog::OnModified, this);
    }
    if (amountCtrl != nullptr) {
        amountCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &PrescriptionDialog::OnModified, this);
        amountUnitCtrl->Bind(wxEVT_SPINCTRLDOUBLE, &PrescriptionDialog::OnModified, this);
    }
    reitCtrl->Bind(wxEVT_SPINCTRL, &PrescriptionDialog::OnModified, this);
    applicationAreaCtrl->Bind(wxEVT_TEXT, &PrescriptionDialog::OnModified, this);
    cancelButton->Bind(wxEVT_BUTTON, &PrescriptionDialog::OnCancel, this);
    proceedButton->Bind(wxEVT_BUTTON, &PrescriptionDialog::OnProceed, this);
    prescriptionValidityCtrl->Bind(wxEVT_COMBOBOX, &PrescriptionDialog::OnModifiedPrescriptionValidity, this);
    Bind(wxEVT_MENU, &PrescriptionDialog::OnAddDosingPeriod, this, TheMaster_PrescriptionDialog_AddDosingPeriod);
    Bind(wxEVT_MENU, &PrescriptionDialog::OnMoveUp, this, TheMaster_PrescriptionDialog_MoveDosingPeriodUp);
    Bind(wxEVT_MENU, &PrescriptionDialog::OnMoveDown, this, TheMaster_PrescriptionDialog_MoveDosingPeriodDown);
    Bind(wxEVT_MENU, &PrescriptionDialog::OnDeleteDosingPeriod, this, TheMaster_PrescriptionDialog_DeleteDosingPeriod);
}

void PrescriptionDialog::OnCancel(wxCommandEvent &e) {
    EndDialog(wxID_CANCEL);
}

enum class PrescriptionRecordType {
    EPRESCRIPTION,
    PLLENTRY
};

enum class UseType {
    PERMANENT,
    WHEN_NEEDED,
    CURE,
    VACCINE,
    NUTRITION
};

enum class DosingType {
    NOT_SET,
    DSSN,
    KORTDOSE,
    DOSINGPERIODS
};

struct PrescriptionDialogData {
    PrescriptionRecordType recordType{PrescriptionRecordType::EPRESCRIPTION};
    UseType useType{UseType::PERMANENT};
    MedicalCodedValue amountUnit{};
    std::shared_ptr<FhirMedication> package{};
    DosingType dosingType{};
    MedicalCodedValue dosingUnit{};
    MedicalCodedValue kortdose{};
    std::vector<std::shared_ptr<AdvancedDosingPeriod>> dosingPeriods{};
    std::string dosingUnitPlural{};
    std::string dosingText{};
    std::string dssn{};
    std::string applicationArea{};
    double numberOfPackages{0};
    double amount{0};
    DateOnly startDate{};
    DateOnly expirationDate{};
    DateOnly ceaseDate{};
    int reit{0};
    bool numberOfPackagesSet{false};
    bool amountIsSet{false};
    bool invalidField{false};
};

constexpr bool IsValidDate(wxDateTime dateTime) {
    auto year = dateTime.GetYear();
    auto month = dateTime.GetMonth();
    auto day = dateTime.GetDay();
    return year != wxDateTime::Inv_Year && month != wxDateTime::Inv_Month && day > 0 && day < 32;
}

constexpr DateOnly ToDateOnly(wxDateTime dateTime) {
    if (!IsValidDate(dateTime)) {
        return {};
    }
    auto year = dateTime.GetYear();
    auto month = dateTime.GetMonth();
    auto day = dateTime.GetDay();
    uint8_t monthNum;
    switch (month) {
        case wxDateTime::Jan:
            monthNum = 1;
            break;
        case wxDateTime::Feb:
            monthNum = 2;
            break;
        case wxDateTime::Mar:
            monthNum = 3;
            break;
        case wxDateTime::Apr:
            monthNum = 4;
            break;
        case wxDateTime::May:
            monthNum = 5;
            break;
        case wxDateTime::Jun:
            monthNum = 6;
            break;
        case wxDateTime::Jul:
            monthNum = 7;
            break;
        case wxDateTime::Aug:
            monthNum = 8;
            break;
        case wxDateTime::Sep:
            monthNum = 9;
            break;
        case wxDateTime::Oct:
            monthNum = 10;
            break;
        case wxDateTime::Nov:
            monthNum = 11;
            break;
        case wxDateTime::Dec:
            monthNum = 12;
            break;
        default:
            return {};
    }
    return {year, monthNum, static_cast<uint8_t>(day)};
}

PrescriptionDialogData PrescriptionDialog::GetDialogData() const {
    PrescriptionDialogData dialogData{};
    switch (typeSelection->GetSelection()) {
        case 0:
            dialogData.recordType = PrescriptionRecordType::EPRESCRIPTION;
            break;
        case 1:
            dialogData.recordType = PrescriptionRecordType::PLLENTRY;
            break;
        default:
            dialogData.recordType = PrescriptionRecordType::EPRESCRIPTION;
    }
    switch (useSelection->GetSelection()) {
        case 0:
            dialogData.useType = UseType::PERMANENT;
            break;
        case 1:
            dialogData.useType = UseType::WHEN_NEEDED;
            break;
        case 2:
            dialogData.useType = UseType::CURE;
            break;
        case 3:
            dialogData.useType = UseType::VACCINE;
            break;
        case 4:
            dialogData.useType = UseType::NUTRITION;
            break;
        default:
            dialogData.useType = UseType::PERMANENT;
    }
    {
        auto page = dosingNotebook->GetSelection();
        if (page == 0) {
            dialogData.dssn = dssnCtrl->GetValue().ToStdString();
            dialogData.dosingType = dialogData.dssn.empty() ? DosingType::NOT_SET : DosingType::DSSN;
        } else if (page == 1) {
            {
                auto selection = kortdoseDosingUnitCtrl->GetSelection();
                if (selection >= 0 && selection < dosingUnit.size()) {
                    dialogData.dosingUnit = dosingUnit[selection];
                } else {
                    dialogData.dosingUnit = {};
                }
            }
            {
                auto selection = kortdoserCtrl->GetSelection();
                if (!dialogData.dosingUnit.GetCode().empty() && selection >= 0 && selection < kortdoser.size()) {
                    dialogData.kortdose = kortdoser[selection];
                    dialogData.dosingType = DosingType::KORTDOSE;
                } else {
                    dialogData.dosingType = DosingType::NOT_SET;
                }
            }
        } else if (page == 2) {
            {
                auto selection = dosingPeriodsDosingUnitCtrl->GetSelection();
                if (selection >= 0 && selection < dosingUnit.size()) {
                    dialogData.dosingUnit = dosingUnit[selection];
                } else {
                    dialogData.dosingUnit = {};
                }
            }
            if (!dosingPeriods.empty()) {
                dialogData.dosingPeriods = dosingPeriods;
                dialogData.dosingType = DosingType::DOSINGPERIODS;
            } else {
                dialogData.dosingType = DosingType::NOT_SET;
            }
        } else {
            dialogData.dosingType = DosingType::NOT_SET;
        }
    }
    if (selectPackage != nullptr) {
        auto selected = selectPackage->GetSelection();
        if (selected >= 0 && selected < packages.size()) {
            dialogData.package = packages[selected].GetMedication();
        }
    }
    if (numberOfPackagesCtrl != nullptr) {
        auto numPkg = numberOfPackagesCtrl->GetValue();
        if (numPkg > 0.001) {
            dialogData.numberOfPackages = numPkg;
            dialogData.numberOfPackagesSet = true;
        }
    }
    if (amountCtrl != nullptr) {
        auto amount = amountCtrl->GetValue();
        MedicalCodedValue amountUnit{};
        {
            auto selected = amountUnitCtrl->GetSelection();
            if (selected >= 0 && selected < this->amountUnit.size()) {
                amountUnit = this->amountUnit[selected];
            }
        }
        if (amount > 0.001) {
            dialogData.amount = amount;
            dialogData.amountUnit = amountUnit;
            dialogData.amountIsSet = true;
        }
    }
    if (packageAmountNotebook != nullptr) {
        int page = packageAmountNotebook->GetSelection();
        if (page == 0) {
            dialogData.amountIsSet = false;
        } else if (page == 1) {
            dialogData.numberOfPackagesSet = false;
        }
    }
    dialogData.startDate = ToDateOnly(startDate->GetValue());
    dialogData.expirationDate = ToDateOnly(expirationDate->GetValue());
    if (ceaseDateSet->GetValue()) {
        dialogData.ceaseDate = ToDateOnly(ceaseDate->GetValue());
        if (!dialogData.ceaseDate) {
            dialogData.invalidField = true;
        }
    }
    dialogData.reit = reitCtrl->GetValue();
    dialogData.applicationArea = applicationAreaCtrl->GetValue();
    return dialogData;
}

static Element GetCodeSetElement(const std::shared_ptr<FestDb> &festDb, const MedicalCodedValue &codedValue) {
    auto codeSet = festDb->GetKodeverkById(codedValue.GetSystem());
    for (const auto &element : codeSet.GetElement()) {
        if (element.GetKode() == codedValue.GetCode()) {
            return element;
        }
    }
    return {};
}

static Term GetNorwegianTerm(const Element &element) {
    auto termList = element.GetTermList();
    for (const auto &term : termList) {
        auto sprak = term.GetSprak().GetValue();
        std::transform(sprak.cbegin(), sprak.cend(), sprak.begin(), [] (char ch) { return std::tolower(ch); });
        if (sprak == "no") {
            return term;
        }
    }
    if (!termList.empty()) {
        return termList[0];
    }
    return {};
}

void PrescriptionDialog::ProcessDialogData(PrescriptionDialogData &dialogData) const {
    if (!dialogData.dosingUnit.GetCode().empty() && !dialogData.dosingUnit.GetSystem().empty()) {
        auto dosingUnitElement = GetCodeSetElement(festDb, dialogData.dosingUnit);
        dialogData.dosingUnitPlural = GetNorwegianTerm(dosingUnitElement).GetBeskrivelseTerm();
    }
    if (dialogData.dosingType == DosingType::KORTDOSE) {
        auto kortdoseElement = GetCodeSetElement(festDb, dialogData.kortdose);
        dialogData.dosingText = GetNorwegianTerm(kortdoseElement).GetBeskrivelseTerm();
    }
    if (dialogData.dosingType == DosingType::DOSINGPERIODS) {
        dialogData.dosingText = ToDosingText(dialogData.dosingPeriods);
    }
    if (dialogData.dosingType == DosingType::KORTDOSE || dialogData.dosingType == DosingType::DOSINGPERIODS) {
        auto dosingUnitSingular = dialogData.dosingUnit.GetDisplay();
        auto dosingUnitPlural = dialogData.dosingUnitPlural;
        if (dosingUnitSingular.empty()) {
            dosingUnitSingular = dosingUnitPlural;
        }
        if (dosingUnitPlural.empty()) {
            dosingUnitPlural = dosingUnitSingular;
        }
        if (!dosingUnitSingular.empty()) {
            {
                std::string repl = "<dose>";
                typeof(dialogData.dosingText.find(repl)) dose;
                while ((dose = dialogData.dosingText.find(repl)) != std::string::npos) {
                    dialogData.dosingText.erase(dose, repl.size());
                    dialogData.dosingText.insert(dose, dosingUnitSingular);
                }
            }
            {
                std::string repl = "<doser>";
                typeof(dialogData.dosingText.find(repl)) dose;
                while ((dose = dialogData.dosingText.find(repl)) != std::string::npos) {
                    dialogData.dosingText.erase(dose, repl.size());
                    dialogData.dosingText.insert(dose, dosingUnitPlural);
                }
            }
        }
        dialogData.dssn = dialogData.dosingText;
    }
}

bool PrescriptionDialog::IsValid(const PrescriptionDialogData &dialogData) const {
    if (dialogData.invalidField) {
        return false;
    }
    if (dialogData.dosingType == DosingType::DSSN) {
        if (dialogData.dssn.empty()) {
            return false;
        }
    } else if (dialogData.dosingType == DosingType::KORTDOSE) {
        if (dialogData.kortdose.GetCode().empty()) {
            return false;
        }
    } else if (dialogData.dosingType == DosingType::DOSINGPERIODS) {
        if (dialogData.dosingPeriods.empty()) {
            return false;
        }
    } else {
        return false;
    }
    if (!dialogData.expirationDate || (dialogData.ceaseDate && dialogData.expirationDate > dialogData.ceaseDate)) {
        return false;
    }
    if (!dialogData.startDate || dialogData.startDate >= dialogData.expirationDate) {
        return false;
    }
    if (dialogData.ceaseDate && dialogData.ceaseDate <= DateOnly::Today()) {
        return false;
    }
    if (dialogData.numberOfPackagesSet) {
        if (dialogData.amountIsSet) {
            return false;
        }
        if (!packages.empty()) {
            if (!dialogData.package) {
                return false;
            }
        }
    } else if (dialogData.amountIsSet) {
        if (dialogData.amountUnit.GetDisplay().empty()) {
            return false;
        }
    } else {
        return false;
    }
    if (dialogData.reit < 0) {
        return false;
    }
    if (dialogData.applicationArea.empty()) {
        return false;
    }
    return true;
}

void PrescriptionDialog::OnModified() {
    bool isValid;
    {
        auto dialogData = GetDialogData();
        isValid = IsValid(dialogData);
    }
    proceedButton->Enable(isValid);
}

void PrescriptionDialog::OnModifiedCeaseIsSet() {
    ceaseDate->Enable(ceaseDateSet->GetValue());
    OnModified();
}

void PrescriptionDialog::OnModified(wxCommandEvent &e) {
    OnModified();
}

void PrescriptionDialog::OnModifiedPC(wxBookCtrlEvent &e) {
    OnModified();
}

void PrescriptionDialog::OnModifiedCeaseIsSet(wxCommandEvent &e) {
    OnModifiedCeaseIsSet();
}

void PrescriptionDialog::OnModifiedDate(wxDateEvent &e) {
    OnModified();
}

void PrescriptionDialog::OnModifiedExpiryDate(wxDateEvent &e) {
    prescriptionValidityCtrl->SetSelection(0);
    OnModified();
}

void PrescriptionDialog::OnModifiedPrescriptionValidity(wxCommandEvent &e) {
    auto selection = prescriptionValidityCtrl->GetSelection();
    if (selection > 0) {
        expirationDate->SetValue(ToWxDateTime(DateOnly::Today() + prescriptionValidity[selection - 1].duration));
        OnModified();
    }
}

static void SetPrescriptionData(PrescriptionData &prescriptionData, const PrescriptionDialogData &dialogData) {
    prescriptionData.reseptdate = dialogData.startDate;
    prescriptionData.expirationdate = dialogData.expirationDate;
    prescriptionData.ceaseDate = dialogData.ceaseDate;
    if (dialogData.dosingType == DosingType::KORTDOSE) {
        prescriptionData.kortdose = dialogData.kortdose;
    } else if (dialogData.dosingType == DosingType::DOSINGPERIODS) {
        prescriptionData.dosingUnit = dialogData.dosingUnit;
        prescriptionData.dosingPeriods = dialogData.dosingPeriods;
    }
    prescriptionData.dosingText = dialogData.dosingText;
    prescriptionData.dssn = dialogData.dssn;
    prescriptionData.numberOfPackages = dialogData.numberOfPackages;
    prescriptionData.numberOfPackagesSet = dialogData.numberOfPackagesSet;
    prescriptionData.amount = dialogData.amount;
    prescriptionData.amountUnit = dialogData.amountUnit;
    prescriptionData.amountIsSet = dialogData.amountIsSet;
    prescriptionData.reit = dialogData.reit;
    prescriptionData.applicationArea = dialogData.applicationArea;
    prescriptionData.itemGroup = {"urn:oid:2.16.578.1.12.4.1.1.7402", "L", "Legemiddel", "Legemiddel"};
    prescriptionData.rfstatus = {"urn:oid:2.16.578.1.12.4.1.1.7408", "E", "Ekspederbar", "Ekspederbar"};
    switch (dialogData.recordType) {
        case PrescriptionRecordType::EPRESCRIPTION:
            prescriptionData.typeresept = {"urn:oid:2.16.578.1.12.4.1.1.7491", "E", "Eresept", "Eresept"};
            break;
        case PrescriptionRecordType::PLLENTRY:
            prescriptionData.typeresept = {"urn:oid:2.16.578.1.12.4.1.1.7491", "U", "Uten resept", "Uten resept"};
            break;
        default:
            prescriptionData.typeresept = {"urn:oid:2.16.578.1.12.4.1.1.7491", "E", "Eresept", "Eresept"};
    }
    {
        std::string useCode{};
        std::string useName{};
        switch (dialogData.useType) {
            case UseType::PERMANENT:
                useCode = "1";
                useName = "Fast";
                break;
            case UseType::WHEN_NEEDED:
                useCode = "3";
                useName = "Ved behov";
                break;
            case UseType::CURE:
                useCode = "2";
                useName = "Kur";
                break;
            case UseType::VACCINE:
                useCode = "4";
                useName = "Vaksine";
                break;
            case UseType::NUTRITION:
                useCode = "5";
                useName = "Næringsmiddel/vitaminer";
                break;
            default:
                useCode = "1";
                useName = "Fast";
        }
        prescriptionData.use = {"urn:oid:2.16.578.1.12.4.1.1.9101", useCode, useName, useName};
    }

    std::ostringstream nowStream;
    std::tm tm{};
    std::time_t now = std::time(nullptr);
    localtime_r(&now, &tm);
    nowStream << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

    auto tzone = localtime(&now);
    if(tzone->tm_gmtoff >= 0)
        nowStream << "+";
    else
        nowStream << "-";
    nowStream << std::setfill('0') << std::setw(2) << abs(tzone->tm_gmtoff / 3600) << ":" << std::setw(2) << abs((tzone->tm_gmtoff / 60) % 60);

    std::string nowString = nowStream.str();
    prescriptionData.lastChanged = nowString;
}

void PrescriptionDialog::OnProceed(wxCommandEvent &e) {
    auto dialogData = GetDialogData();
    ProcessDialogData(dialogData);
    SetPrescriptionData(prescriptionData, dialogData);
    if (dialogData.package) {
        medication = dialogData.package;
    }
    EndDialog(wxID_OK);
}

void PrescriptionDialog::OnDosingPeriodsContextMenu(wxContextMenuEvent &e) {
    static constexpr const typeof(dosingPeriodsView->GetFirstSelected()) noneSelected = -1;
    typeof(dosingPeriodsView->GetFirstSelected()) selected = noneSelected;
    if (dosingPeriodsView->GetSelectedItemCount() == 1) {
        auto index = dosingPeriodsView->GetFirstSelected();
        if (index >= 0 && index < dosingPeriods.size()) {
            selected = index;
        }
    }
    addDosingPeriod = [this, selected] (std::shared_ptr<AdvancedDosingPeriod> &&dosingPeriod) {
        if (selected != noneSelected) {
            dosingPeriods.emplace_back();
            auto base = dosingPeriods.begin() + selected;
            auto iterator = dosingPeriods.end() - 1;
            while (iterator != base) {
                auto &ref = *iterator;
                --iterator;
                ref = std::move(*iterator);
            }
            *base = std::move(dosingPeriod);
            dosingPeriodsView->InsertItem(selected, wxString::FromUTF8((*base)->ToString()));
        } else {
            auto sz = dosingPeriods.size();
            auto &ref = dosingPeriods.emplace_back(std::move(dosingPeriod));
            dosingPeriodsView->InsertItem(sz, wxString::FromUTF8(ref->ToString()));
        }
    };
    if (selected > 0 && selected < dosingPeriods.size()) {
        moveUp = [this, selected] () {
            auto sw = dosingPeriods[selected - 1];
            dosingPeriods[selected - 1] = dosingPeriods[selected];
            dosingPeriods[selected] = sw;
            dosingPeriodsView->Select(dosingPeriodsView->GetFirstSelected(), false);
            dosingPeriodsView->SetItem(selected - 1, 0, wxString::FromUTF8(dosingPeriods[selected - 1]->ToString()));
            dosingPeriodsView->SetItem(selected, 0, wxString::FromUTF8(dosingPeriods[selected]->ToString()));
            dosingPeriodsView->Select(selected - 1);
        };
    }
    if (selected >= 0 && selected < (dosingPeriods.size() - 1)) {
        moveDown = [this, selected] () {
            auto sw = dosingPeriods[selected];
            dosingPeriods[selected] = dosingPeriods[selected + 1];
            dosingPeriods[selected + 1] = sw;
            dosingPeriodsView->Select(dosingPeriodsView->GetFirstSelected(), false);
            dosingPeriodsView->SetItem(selected, 0, wxString::FromUTF8(dosingPeriods[selected]->ToString()));
            dosingPeriodsView->SetItem(selected + 1, 0, wxString::FromUTF8(dosingPeriods[selected + 1]->ToString()));
            dosingPeriodsView->Select(selected + 1);
        };
    }
    if (selected >= 0 && selected < dosingPeriods.size()) {
        deleteDosingPeriod = [this, selected] () {
            dosingPeriods.erase(dosingPeriods.begin() + selected);
            dosingPeriodsView->DeleteItem(selected);
        };
    }
    wxMenu menu(wxT(""));
    menu.Append(TheMaster_PrescriptionDialog_AddDosingPeriod, wxT("Add"));
    if (selected > 0 && selected < dosingPeriods.size()) {
        menu.Append(TheMaster_PrescriptionDialog_MoveDosingPeriodUp, wxT("Move up"));
    }
    if (selected >= 0 && selected < (dosingPeriods.size() - 1)) {
        menu.Append(TheMaster_PrescriptionDialog_MoveDosingPeriodDown, wxT("Move down"));
    }
    if (selected >= 0 && selected < dosingPeriods.size()) {
        menu.Append(TheMaster_PrescriptionDialog_DeleteDosingPeriod, wxT("Delete"));
    }
    PopupMenu(&menu);
}

void PrescriptionDialog::OnAddDosingPeriod(wxCommandEvent &e) {
    AdvancedDosingPeriodDialog advancedDosingPeriodDialog{this};
    if (advancedDosingPeriodDialog.ShowModal() == wxID_OK) {
        std::shared_ptr<AdvancedDosingPeriod> dosingPeriod = advancedDosingPeriodDialog.GetDosingPeriod();
        addDosingPeriod(std::move(dosingPeriod));
        OnModified();
    }
}

void PrescriptionDialog::OnMoveUp(wxCommandEvent &e) {
    moveUp();
    OnModified();
}

void PrescriptionDialog::OnMoveDown(wxCommandEvent &e) {
    moveDown();
    OnModified();
}

void PrescriptionDialog::OnDeleteDosingPeriod(wxCommandEvent &e) {
    deleteDosingPeriod();
    OnModified();
}
