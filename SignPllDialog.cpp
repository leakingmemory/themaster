//
// Created by sigsegv on 6/11/24.
//

#include "SignPllDialog.h"

SignPllDialog::SignPllDialog(wxWindow *parent, const std::map<std::string,std::string> &map, const std::vector<std::string> &preselect) : wxDialog(parent, wxID_ANY, wxT("Sign PLL")) {

// Create a wxCheckListBox to show the items with checkboxes
    listbox = new wxCheckListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);

// Iterate over the map and add the items to the wxCheckListBox
    for (const auto &element: map) {
        candidates.emplace_back(element.first);
        listbox->Append(wxString::FromUTF8(element.second.c_str()));
    }

// Add the listbox to the dialog's sizer
    wxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(listbox, 1, wxEXPAND | wxALL, 5);
// Create ok and cancel buttons
    wxButton *okButton = new wxButton(this, wxID_ANY, _("OK"));
    wxButton *cancelButton = new wxButton(this, wxID_ANY, _("Cancel"));

// Create a wxStdDialogButtonSizer to help ensure standard placement of buttons
    auto *sizerButtons = new wxBoxSizer(wxHORIZONTAL);
    sizerButtons->Add(okButton, 1, wxEXPAND | wxALL, 5);
    sizerButtons->Add(cancelButton, 1, wxEXPAND | wxALL, 5);

// Add buttons sizer to dialog's sizer
    sizer->Add(sizerButtons, 0, wxEXPAND | wxALL, 5);

// Call Layout to adjust the dialog's layout
    SetSizerAndFit(sizer);

    int index = 0;
    for (const auto &element: map) {
        auto id = element.first;
        if (std::find(preselect.cbegin(), preselect.cend(), id) != preselect.cend()) {
            listbox->Check(index, true);
        } else {
            listbox->Check(index, false);
        }
        ++index;
    }

    cancelButton->Bind(wxEVT_BUTTON, &SignPllDialog::OnCancel, this);
    okButton->Bind(wxEVT_BUTTON, &SignPllDialog::OnOk, this);
}

void SignPllDialog::OnCancel(wxCommandEvent &e) {
    EndDialog(wxID_CANCEL);
}

void SignPllDialog::OnOk(wxCommandEvent &e) {
    typeof(candidates.size()) i = 0;
    for (auto &candidate : candidates) {
        if (listbox->IsChecked(i)) {
            selected.emplace_back(std::move(candidate));
        }
        ++i;
    }
    candidates.clear();
    EndDialog(wxID_OK);
}
