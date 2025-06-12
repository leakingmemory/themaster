//
// Created by sigsegv on 12/15/23.
//

#include "CreatePatientDialog.h"
#include "TheMasterFrame.h"
#include "PatientStore.h"

CreatePatientDialog::CreatePatientDialog(const std::shared_ptr<PatientStore> &patientStore, TheMasterFrame *frame) :
        wxDialog(frame, wxID_ANY, wxT("Create patient")), patientStore(patientStore) {
    // Create patient ID type dropdown
    wxArrayString patientIDTypes;
    patientIDTypes.Add(wxT("Fødselsnummer"));
    patientIDTypes.Add(wxT("D-nummer"));
    patientIDTypes.Add(wxT("Ingen"));
    auto* patientIDTypeLabel = new wxStaticText(this, wxID_ANY, wxT("Patient ID Type:"));
    patientIDTypeChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, patientIDTypes);

    // Create text fields
    auto* patientIDFieldLabel = new wxStaticText(this, wxID_ANY, wxT("Patient ID:"));
    patientIDField = new wxTextCtrl(this, wxID_ANY);
    auto* firstNameFieldLabel = new wxStaticText(this, wxID_ANY, wxT("First Name:"));
    firstNameField = new wxTextCtrl(this, wxID_ANY);
    auto* lastNameFieldLabel = new wxStaticText(this, wxID_ANY, wxT("Last Name:"));
    lastNameField = new wxTextCtrl(this, wxID_ANY);

    wxArrayString genders;
    genders.Add(wxT("Female"));
    genders.Add(wxT("Male"));
    auto* genderLabel = new wxStaticText(this, wxID_ANY, wxT("Gender:"));
    genderChoice = new wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, genders);
    genderChoice->SetSelection(0);

    auto* dobFieldLabel = new wxStaticText(this, wxID_ANY, wxT("DOB:"));
    dobField = new wxTextCtrl(this, wxID_ANY);
    auto* postCodeFieldLabel = new wxStaticText(this, wxID_ANY, wxT("Postcode:"));
    postCodeField = new wxTextCtrl(this, wxID_ANY);
    auto* cityFieldLabel = new wxStaticText(this, wxID_ANY, wxT("City:"));
    cityField = new wxTextCtrl(this, wxID_ANY);

    // Create OK and Cancel buttons
    auto* okButton = new wxButton(this,wxID_OK,wxT("OK"));
    auto* cancelButton = new wxButton(this,wxID_CANCEL,wxT("Cancel"));

    // Create a sizer for layout
    auto* sizer = new wxFlexGridSizer(2, wxSize(5,5));

    sizer->Add(patientIDTypeLabel, 0, wxALIGN_LEFT);
    sizer->Add(patientIDTypeChoice, 0, wxEXPAND);
    sizer->Add(patientIDFieldLabel, 0, wxALIGN_LEFT);
    sizer->Add(patientIDField, 0, wxEXPAND);
    sizer->Add(firstNameFieldLabel, 0, wxALIGN_LEFT);
    sizer->Add(firstNameField, 0, wxEXPAND);
    sizer->Add(lastNameFieldLabel, 0, wxALIGN_LEFT);
    sizer->Add(lastNameField, 0, wxEXPAND);

    sizer->Add(genderLabel, 0, wxALIGN_LEFT);
    sizer->Add(genderChoice, 0, wxEXPAND);

    sizer->Add(dobFieldLabel, 0, wxALIGN_LEFT);
    sizer->Add(dobField, 0, wxEXPAND);
    sizer->Add(postCodeFieldLabel, 0, wxALIGN_LEFT);
    sizer->Add(postCodeField, 0, wxEXPAND);
    sizer->Add(cityFieldLabel, 0, wxALIGN_LEFT);
    sizer->Add(cityField, 1, wxEXPAND);

    // Add buttons to sizer
    sizer->Add(okButton, 0, wxALIGN_CENTER);
    sizer->Add(cancelButton, 0, wxALIGN_CENTER);

    // Set up the dialog to use the sizer for layout
    this->SetSizer(sizer);
    this->Fit();
}

PatientInformation CreatePatientDialog::GetPatientInformation() {
    PatientInformation patientInformation{};
    {
        int patientIdType = patientIDTypeChoice->GetSelection();
        PatientIdType pIdType;
        switch (patientIdType) {
            case 0:
                pIdType = PatientIdType::FODSELSNUMMER;
                break;
            case 1:
                pIdType = PatientIdType::DNUMMER;
                break;
            case 2:
                pIdType = PatientIdType::NOT_SET;
                break;
            default:
                pIdType = PatientIdType::NOT_SET;
        }
        patientInformation.SetPatientIdType(pIdType);
    }
    if (patientInformation.GetPatientIdType() != PatientIdType::NOT_SET) {
        std::string id{patientIDField->GetValue().utf8_string()};
        patientInformation.SetPatientId(id);
    }
    patientInformation.SetGivenName(firstNameField->GetValue().utf8_string());
    patientInformation.SetFamilyName(lastNameField->GetValue().utf8_string());
    patientInformation.SetGender(genderChoice->GetSelection() == 0 ? PersonGender::FEMALE : PersonGender::MALE);
    patientInformation.SetDateOfBirth(dobField->GetValue().utf8_string());
    patientInformation.SetCity(cityField->GetValue().utf8_string());
    patientInformation.SetPostCode(postCodeField->GetValue().utf8_string());
    return patientInformation;
}