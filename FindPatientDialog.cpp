//
// Created by sigsegv on 12/13/23.
//

#include "FindPatientDialog.h"
#include "TheMasterFrame.h"
#include <wx/listctrl.h>

constexpr int PatientListPatientColumnWidth = 300;

FindPatientDialog::FindPatientDialog(const std::shared_ptr<PatientStore> &patientStore, TheMasterFrame *frame) : wxDialog(frame, wxID_ANY, wxT("Find patient"), wxDefaultPosition, wxSize(300, 400)), patientStore(patientStore) {
    // Add a sizer to handle the layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Add a text field for search input
    searchInput = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    sizer->Add(searchInput, 0, wxEXPAND|wxALL, 5);

    // Add a list view for potential matches
    listView = new wxListView(this, wxID_ANY);
    listView->AppendColumn(wxT("Patient"));
    listView->SetColumnWidth(0, PatientListPatientColumnWidth);
    sizer->Add(listView, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer* sizerButtons = new wxBoxSizer(wxHORIZONTAL);

    // Add buttons at the bottom
    // Ok button
    okButton = new wxButton(this, wxID_OK, wxT("OK"));
    okButton->Enable(false);
    sizerButtons->Add(okButton, 0, wxALIGN_CENTER | wxALL, 5);

    // Cancel button
    wxButton *cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
    sizerButtons->Add(cancelButton, 0, wxALIGN_CENTER | wxALL, 5);

    sizer->Add(sizerButtons, 0, wxALIGN_CENTER | wxALL, 5);

    // Apply sizer to the dialog
    SetSizer(sizer);

    searchInput->Bind(wxEVT_TEXT, &FindPatientDialog::OnText, this);
    listView->Bind(wxEVT_LIST_ITEM_SELECTED, &FindPatientDialog::OnSelect, this);
    listView->Bind(wxEVT_LIST_ITEM_DESELECTED, &FindPatientDialog::OnSelect, this);
}

void FindPatientDialog::OnText(wxCommandEvent &e) {
    std::string searchTerm{searchInput->GetValue().ToUTF8()};
    patients = patientStore->FindPatients(searchTerm);
    listView->ClearAll();
    listView->AppendColumn(wxT("Patient"));
    listView->SetColumnWidth(0, PatientListPatientColumnWidth);
    auto i = 0;
    for (auto patient : patients) {
        std::string patientDesc{};
        {
            auto id = patient.GetPatientId();
            if (!id.empty()) {
                patientDesc.append(id);
            } else {
                auto dob = patient.GetDateOfBirth();
                if (!dob.empty()) {
                    patientDesc.append(dob);
                }
            }
        }
        {
            auto givenName = patient.GetGivenName();
            if (!givenName.empty()) {
                if (!patientDesc.empty()) {
                    patientDesc.append(" ");
                }
                patientDesc.append(givenName);
            }
        }
        {
            auto familyName = patient.GetFamilyName();
            if (!familyName.empty()) {
                if (!patientDesc.empty()) {
                    patientDesc.append(" ");
                }
                patientDesc.append(familyName);
            }
        }
        if (patientDesc.empty()) {
            patientDesc = "No name";
        }
        listView->InsertItem(i++, patientDesc);
    }
}

void FindPatientDialog::OnSelect(wxCommandEvent &e) {
    if (listView->GetSelectedItemCount() == 1) {
        auto selectedIdx = listView->GetFirstSelected();
        if (selectedIdx < patients.size()) {
            if (patient) {
                *patient = patients.at(selectedIdx);
            } else {
                patient = std::make_shared<PatientInformation>(patients.at(selectedIdx));
            }
        } else {
            patient = {};
        }
    } else {
        patient = {};
    }
    if (patient) {
        okButton->Enable(true);
    } else {
        okButton->Enable(false);
    }
}