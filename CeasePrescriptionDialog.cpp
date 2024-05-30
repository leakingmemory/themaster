//
// Created by sigsegv on 5/24/24.
//

#include "CeasePrescriptionDialog.h"
#include <sfmbasisapi/fhir/medstatement.h>

static std::vector<MedicalCodedValue> cessationCodes = MedicalCodedValue::GetVolvenCessationCode();

CeasePrescriptionDialog::CeasePrescriptionDialog(wxWindow *parent, const std::shared_ptr<FhirMedicationStatement> &medicationStatement) : wxDialog(parent, wxID_ANY, wxT("Cease prescription")) {
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    {
        auto display = medicationStatement->GetDisplay();
        sizer->Add(new wxStaticText(this, wxID_ANY, wxString::FromUTF8(display)), 1, wxALL | wxEXPAND, 5);
    }
    sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Recall code:")), 1, wxALL | wxEXPAND, 5);
    cessationCode = new wxComboBox(this, wxID_ANY);
    cessationCode->SetEditable(false);
    for (const auto &code : cessationCodes) {
        cessationCode->Append(wxString::FromUTF8(code.GetDisplay()));
    }
    sizer->Add(cessationCode, 1, wxALL | wxEXPAND, 5);
    sizer->Add(new wxStaticText(this, wxID_ANY, wxT("Reason:")), 1, wxALL | wxEXPAND, 5);
    cessationReason = new wxTextCtrl(this, wxID_ANY);
    sizer->Add(cessationReason, 1, wxALL | wxEXPAND, 5);
    auto *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *cancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"));
    ceaseButton = new wxButton(this, wxID_ANY, wxT("Cease"));
    ceaseButton->Enable(false);
    buttonsSizer->Add(cancelButton, 0, wxEXPAND | wxALL, 5);
    buttonsSizer->Add(ceaseButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(sizer);
    cessationCode->Bind(wxEVT_COMBOBOX, &CeasePrescriptionDialog::OnModified, this);
    cessationReason->Bind(wxEVT_TEXT, &CeasePrescriptionDialog::OnModified, this);
    cancelButton->Bind(wxEVT_BUTTON, &CeasePrescriptionDialog::OnCancel, this);
    ceaseButton->Bind(wxEVT_BUTTON, &CeasePrescriptionDialog::OnCease, this);
}

void CeasePrescriptionDialog::OnCancel(wxCommandEvent &e) {
    EndDialog(wxID_CANCEL);
}

void CeasePrescriptionDialog::OnCease(wxCommandEvent &e) {
    auto selection = cessationCode->GetSelection();
    if (selection < 0 || selection >= cessationCodes.size()) {
        return;
    }
    reason = cessationCodes[selection];
    EndDialog(wxID_OK);
}

void CeasePrescriptionDialog::OnModified(wxCommandEvent &e) {
    bool recallCodeValid;
    auto selection = cessationCode->GetSelection();
    if (selection >= 0 && selection < cessationCodes.size()) {
        recallCodeValid = true;
    } else {
        recallCodeValid = false;
    }
    ceaseButton->Enable(recallCodeValid);
    reasonText = cessationReason->GetValue().ToStdString();
}
