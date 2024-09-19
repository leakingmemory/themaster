//
// Created by sigsegv on 1/22/24.
//

#include <wx/spinctrl.h>
#include <wx/notebook.h>
#include "PrescriptionDialog.h"
#include "FestDb.h"
#include "TheMasterFrame.h"
#include <iomanip>
#include <sstream>
#include <medfest/Struct/Decoded/OppfKodeverk.h>

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

PrescriptionDialog::PrescriptionDialog(TheMasterFrame *frame, const std::shared_ptr<FestDb> &festDb, const std::shared_ptr<FhirMedication> &medication, const std::vector<MedicalCodedValue> &amountUnit, bool package, const std::vector<MedicamentPackage> &packages, const std::vector<MedicalCodedValue> &dosingUnit, const std::vector<MedicalCodedValue> &kortdoser) : wxDialog(frame, wxID_ANY, wxT("Prescription")), festDb(festDb), medication(medication), amountUnit(amountUnit), packages(packages), dosingUnit(dosingUnit), kortdoser(kortdoser) {
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    wxArrayString typeSelectionChoices{};
    typeSelectionChoices.Add(wxT("E-prescription"));
    typeSelectionChoices.Add(wxT("PLL-entry"));
    typeSelection = new wxRadioBox(this, wxID_ANY, wxT("Record type:"), wxDefaultPosition, wxDefaultSize, typeSelectionChoices, typeSelectionChoices.size(), wxRA_HORIZONTAL);
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
    auto *kortdoseDosingUnitSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *kortdoseDosingUnitLabel = new wxStaticText(kortdoserPage, wxID_ANY, wxT("Doseringsenhet:"));
    kortdoseDosingUnitCtrl = new wxComboBox(kortdoserPage, wxID_ANY);
    kortdoseDosingUnitCtrl->SetEditable(false);
    for (auto &unit : dosingUnit) {
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
    auto *kortdoserSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *kortdoserLabel = new wxStaticText(kortdoserPage, wxID_ANY, wxT("Kortdoser:"));
    kortdoserCtrl = new wxComboBox(kortdoserPage, wxID_ANY);
    kortdoserCtrl->SetEditable(false);
    for (const auto &kd : kortdoser) {
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
    auto *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *cancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"));
    proceedButton = new wxButton(this, wxID_ANY, wxT("Finish"));
    proceedButton->Enable(false);
    buttonsSizer->Add(cancelButton, 0, wxEXPAND | wxALL, 5);
    buttonsSizer->Add(proceedButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(typeSelection, 0, wxEXPAND | wxALL, 5);
    sizer->Add(dosingNotebook, 0, wxEXPAND | wxALL, 5);
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
    sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(sizer);
    dssnCtrl->Bind(wxEVT_TEXT, &PrescriptionDialog::OnModified, this);
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
}

void PrescriptionDialog::OnCancel(wxCommandEvent &e) {
    EndDialog(wxID_CANCEL);
}

enum class PrescriptionRecordType {
    EPRESCRIPTION,
    PLLENTRY
};

enum class DosingType {
    NOT_SET,
    DSSN,
    KORTDOSE
};

struct PrescriptionDialogData {
    PrescriptionRecordType recordType{PrescriptionRecordType::EPRESCRIPTION};
    MedicalCodedValue amountUnit{};
    std::shared_ptr<FhirMedication> package{};
    DosingType dosingType{};
    MedicalCodedValue dosingUnit{};
    MedicalCodedValue kortdose{};
    std::string dosingUnitPlural{};
    std::string dosingText{};
    std::string dssn{};
    std::string applicationArea{};
    double numberOfPackages{0};
    double amount{0};
    int reit{0};
    bool numberOfPackagesSet{false};
    bool amountIsSet{false};
};

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
    if (dialogData.dosingType == DosingType::DSSN) {
        if (dialogData.dssn.empty()) {
            return false;
        }
    } else if (dialogData.dosingType == DosingType::KORTDOSE) {
        if (dialogData.kortdose.GetCode().empty()) {
            return false;
        }
    } else {
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

void PrescriptionDialog::OnModified(wxCommandEvent &e) {
    OnModified();
}

void PrescriptionDialog::OnModifiedPC(wxBookCtrlEvent &e) {
    OnModified();
}

static void SetPrescriptionData(PrescriptionData &prescriptionData, const PrescriptionDialogData &dialogData) {
    std::time_t now = std::time(nullptr);
    std::tm tm{};
    localtime_r(&now, &tm);
    std::tm tmp1y = tm;
    tmp1y.tm_year++;
    if (tmp1y.tm_mon == 1 && tmp1y.tm_mday == 29) {
        tmp1y.tm_mon++;
        tmp1y.tm_mday = 1;
    }
    std::time_t nowp1y = mktime(&tmp1y);
    std::time_t endtime = nowp1y - (24 * 3600);
    std::tm endtm{};
    localtime_r(&endtime, &endtm);
    char date[11];  // Date string will have 10 chars + null terminator
    std::strftime(date, sizeof(date), "%Y-%m-%d", &tm);
    prescriptionData.reseptdate = date;
    std::strftime(date, sizeof(date), "%Y-%m-%d", &endtm);
    prescriptionData.expirationdate = date;
    if (dialogData.dosingType == DosingType::KORTDOSE) {
        prescriptionData.kortdose = dialogData.kortdose;
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
    prescriptionData.use = {"urn:oid:2.16.578.1.12.4.1.1.9101", "1", "Fast", "Fast"};

    std::ostringstream nowStream;
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
