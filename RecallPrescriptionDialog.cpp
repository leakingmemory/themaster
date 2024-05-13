//
// Created by sigsegv on 5/7/24.
//

#include "RecallPrescriptionDialog.h"
#include <sfmbasisapi/fhir/medstatement.h>
#include "MedicalCodedValue.h"

static std::vector<MedicalCodedValue> recallCodes = MedicalCodedValue::GetVolvenRecallCode();

RecallPrescriptionDialog::RecallPrescriptionDialog(wxWindow *parent, const std::shared_ptr<FhirMedicationStatement> &medicationStatement) : wxDialog(parent, wxID_ANY, wxT("Recall prescription")) {
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    {
        auto display = medicationStatement->GetDisplay();
        sizer->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8(display)), 1, wxALL | wxEXPAND, 5);
    }
    sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Recall code:")), 1, wxALL | wxEXPAND, 5);
    recallCode = new wxComboBox(this, wxID_ANY);
    recallCode->SetEditable(false);
    for (const auto &code : recallCodes) {
        recallCode->Append(wxString::FromUTF8(code.GetDisplay()));
    }
    sizer->Add(recallCode, 1, wxALL | wxEXPAND, 5);
    auto *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *cancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"));
    recallButton = new wxButton(this, wxID_ANY, wxT("Recall"));
    recallButton->Enable(false);
    buttonsSizer->Add(cancelButton, 0, wxEXPAND | wxALL, 5);
    buttonsSizer->Add(recallButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(sizer);
    recallCode->Bind(wxEVT_COMBOBOX, &RecallPrescriptionDialog::OnModified, this);
    cancelButton->Bind(wxEVT_BUTTON, &RecallPrescriptionDialog::OnCancel, this);
    recallButton->Bind(wxEVT_BUTTON, &RecallPrescriptionDialog::OnRecall, this);
}

void RecallPrescriptionDialog::OnCancel(wxCommandEvent &e) {
    EndDialog(wxID_CANCEL);
}

void RecallPrescriptionDialog::OnRecall(wxCommandEvent &e) {
    auto selection = recallCode->GetSelection();
    if (selection < 0 || selection >= recallCodes.size()) {
        return;
    }
    reason = recallCodes[selection];
    EndDialog(wxID_OK);
}

void RecallPrescriptionDialog::OnModified(wxCommandEvent &e) {
    bool recallCodeValid;
    auto selection = recallCode->GetSelection();
    if (selection >= 0 && selection < recallCodes.size()) {
        recallCodeValid = true;
    } else {
        recallCodeValid = false;
    }
    recallButton->Enable(recallCodeValid);
}
