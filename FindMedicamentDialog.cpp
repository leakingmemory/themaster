//
// Created by sigsegv on 3/19/24.
//

#include "FindMedicamentDialog.h"
#include <wx/listctrl.h>

FindMedicamentDialog::FindMedicamentDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, wxT("Find medicament")) {
    // Add a sizer to handle the layout
    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

    // Add a text field for search input
    searchInput = new wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    sizer->Add(searchInput, 0, wxEXPAND|wxALL, 5);

    // Add a list view for potential matches
    listView = new wxListView(this, wxID_ANY);
    listView->AppendColumn(wxT("Id"));
    listView->AppendColumn(wxT("Name"));
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

    searchInput->Bind(wxEVT_TEXT, &FindMedicamentDialog::OnText, this);
    listView->Bind(wxEVT_LIST_ITEM_SELECTED, &FindMedicamentDialog::OnSelect, this);
    listView->Bind(wxEVT_LIST_ITEM_DESELECTED, &FindMedicamentDialog::OnSelect, this);
}

bool FindMedicamentDialog::CanOpen() const {
    return festDb.IsOpen();
}

void FindMedicamentDialog::OnText(wxCommandEvent &e) {

}

void FindMedicamentDialog::OnSelect(wxCommandEvent &e) {

}
