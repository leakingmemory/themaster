//
// Created by sigsegv on 1/14/24.
//

#include "MagistralBuilderDialog.h"
#include "TheMasterFrame.h"
#include "UnitsOfMeasure.h"
#include "MedicalCodedValue.h"
#include <wx/listctrl.h>
#include <wx/spinctrl.h>
#include <sstream>

MagistralBuilderDialog::MagistralBuilderDialog(TheMasterFrame *masterFrame) : wxDialog(masterFrame, wxID_ANY, "Magistral") {
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    auto *dilutionListSizer = new wxBoxSizer(wxHORIZONTAL);
    dilutionList = new wxListView(this, wxID_ANY);
    dilutionList->AppendColumn(wxT("Name"));
    dilutionList->AppendColumn(wxT("AD/QS"));
    dilutionListSizer->Add(dilutionList, 1, wxEXPAND | wxALL, 5);
    auto *dilutionAddSizer = new wxBoxSizer(wxHORIZONTAL);
    dilutionSearch = new wxComboBox(this, wxID_ANY);
    adqsSelect = new wxComboBox(this, wxID_ANY);
    adqsSelect->SetEditable(false);
    adqsSelect->Append(wxT("AD"));
    adqsSelect->Append(wxT("QS"));
    adqsSelect->SetSelection(0);
    auto *dilutionAdd = new wxButton(this, wxID_ANY);
    dilutionAdd->SetLabel("Add");
    dilutionAddSizer->Add(dilutionSearch, 1, wxEXPAND | wxALL, 5);
    dilutionAddSizer->Add(adqsSelect, 1, wxEXPAND | wxALL, 5);
    dilutionAddSizer->Add(dilutionAdd, 1, wxEXPAND | wxALL, 5);
    auto *substanceListSizer = new wxBoxSizer(wxHORIZONTAL);
    substanceList = new wxListView(this, wxID_ANY);
    substanceList->AppendColumn(wxT("Name"));
    substanceList->AppendColumn(wxT("Strength"));
    substanceList->AppendColumn(wxT("Unit"));
    substanceListSizer->Add(substanceList, 1, wxEXPAND | wxALL, 5);
    auto *substanceAddSizer = new wxBoxSizer(wxHORIZONTAL);
    substanceSearch = new wxComboBox(this, wxID_ANY);
    substanceStrength = new wxSpinCtrlDouble(this, wxID_ANY);
    substanceStrengthUnit = new wxComboBox(this, wxID_ANY);
    substanceStrengthUnit->SetEditable(false);
    strengthUnits = UnitsOfMeasure::GetUnitsForStrength().GetUnits();
    for (const auto &unit : strengthUnits) {
        substanceStrengthUnit->Append(unit.first);
    }
    auto *substanceAdd = new wxButton(this, wxID_ANY);
    substanceAdd->SetLabel("Add");
    substanceAddSizer->Add(substanceSearch, 1, wxEXPAND | wxALL, 5);
    substanceAddSizer->Add(substanceStrength, 1, wxEXPAND | wxALL, 5);
    substanceAddSizer->Add(substanceStrengthUnit, 1, wxEXPAND | wxALL, 5);
    substanceAddSizer->Add(substanceAdd, 1, wxEXPAND | wxALL, 5);
    auto *medicamentFormSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *medicamentFormLabel = new wxStaticText(this, wxID_ANY, wxT("Form:"));
    medicamentFormCtrl = new wxComboBox(this, wxID_ANY);
    for (const auto &form : MedicalCodedValue::GetVolvenMedicamentForm()) {
        medicamentFormCtrl->Append(wxString::FromUTF8(form.GetShortDisplay()));
    }
    medicamentFormSizer->Add(medicamentFormLabel, 0, wxEXPAND | wxALL, 5);
    medicamentFormSizer->Add(medicamentFormCtrl, 1, wxEXPAND | wxALL, 5);
    auto *amountSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *amountLabel = new wxStaticText(this, wxID_ANY, wxT("Amount:"));
    amountCtrl = new wxSpinCtrlDouble(this, wxID_ANY);
    amountUnitCtrl = new wxComboBox(this, wxID_ANY);
    amountUnitCtrl->SetEditable(false);
    for (const auto &unit : strengthUnits) {
        amountUnitCtrl->Append(unit.first);
    }
    amountSizer->Add(amountLabel, 0, wxEXPAND | wxALL, 5);
    amountSizer->Add(amountCtrl, 1, wxEXPAND | wxALL, 5);
    amountSizer->Add(amountUnitCtrl, 0, wxEXPAND | wxALL, 5);
    auto *instructionsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *instructionsLabel = new wxStaticText(this, wxID_ANY, wxT("Instructions:"));
    instructionsCtrl = new wxTextCtrl(this, wxID_ANY);
    instructionsSizer->Add(instructionsLabel, 0, wxEXPAND | wxALL, 5);
    instructionsSizer->Add(instructionsCtrl, 1, wxEXPAND | wxALL, 5);
    auto *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *cancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"));
    auto *proceedButton = new wxButton(this, wxID_ANY, wxT("Next"));
    buttonsSizer->Add(cancelButton, 0, wxEXPAND | wxALL, 5);
    buttonsSizer->Add(proceedButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(dilutionListSizer, 1, wxEXPAND | wxALL, 5);
    sizer->Add(dilutionAddSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(substanceListSizer, 1, wxEXPAND | wxALL, 5);
    sizer->Add(substanceAddSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(medicamentFormSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(amountSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(instructionsSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(buttonsSizer, 0, wxALL, 5);
    SetSizerAndFit(sizer);
    dilutionAdd->Bind(wxEVT_BUTTON, &MagistralBuilderDialog::OnAddDilution, this);
    substanceAdd->Bind(wxEVT_BUTTON, &MagistralBuilderDialog::OnAddSubstance, this);
    cancelButton->Bind(wxEVT_BUTTON, &MagistralBuilderDialog::OnCancel, this);
    proceedButton->Bind(wxEVT_BUTTON, &MagistralBuilderDialog::OnProceed, this);
}

void MagistralBuilderDialog::OnAddDilution(wxCommandEvent &e) {
    Dilution dilution{
        .name = dilutionSearch->GetValue().ToStdString(),
        .dilution = adqsSelect->GetSelection() == 0 ? DilutionType::AD : DilutionType::QS
    };
    dilutionList->InsertItem((long) magistralMedicament.dilutions.size(), dilution.name);
    dilutionList->SetItem((long) magistralMedicament.dilutions.size(), 1, dilution.dilution == DilutionType::AD ? wxT("AD") : wxT("QS"));
    magistralMedicament.dilutions.emplace_back(dilution);
}

void MagistralBuilderDialog::OnAddSubstance(wxCommandEvent &e) {
    std::vector<std::string> units{};
    for (const auto &kv : strengthUnits) {
        units.emplace_back(kv.first);
    }
    Substance substance{
        .name = substanceSearch->GetValue().ToStdString(),
        .strength = substanceStrength->GetValue(),
        .strengthUnit = units.at(substanceStrengthUnit->GetSelection())
    };
    substanceList->InsertItem((long) magistralMedicament.substances.size(), substance.name);
    {
        std::stringstream sstr{};
        sstr << substance.strength;
        substanceList->SetItem((long) magistralMedicament.substances.size(), 1, sstr.str());
    }
    substanceList->SetItem((long) magistralMedicament.substances.size(), 2, substance.strengthUnit);
    magistralMedicament.substances.emplace_back(substance);
}

void MagistralBuilderDialog::OnCancel(wxCommandEvent &e) {
    EndDialog(wxID_CANCEL);
}

void MagistralBuilderDialog::OnProceed(wxCommandEvent &e) {
    {
        auto formSet = MedicalCodedValue::GetVolvenMedicamentForm();
        auto formSel = medicamentFormCtrl->GetSelection();
        if (formSel >= 0 && formSel < formSet.size()) {
            magistralMedicament.form = formSet[formSel];
        }
    }
    {
        magistralMedicament.amount = amountCtrl->GetValue();
        auto amountUnitSel = amountUnitCtrl->GetSelection();
        std::vector<std::string> units{};
        for (const auto &kv: strengthUnits) {
            units.emplace_back(kv.first);
        }
        if (amountUnitSel >= 0 && amountUnitSel < units.size()) {
            magistralMedicament.amountUnit = units[amountUnitSel];
        }
    }
    magistralMedicament.instructions = instructionsCtrl->GetValue().ToStdString();
    EndDialog(wxID_OK);
}
