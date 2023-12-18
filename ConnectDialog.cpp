//
// Created by sigsegv on 12/13/23.
//

#include "ConnectDialog.h"
#include "TheMasterFrame.h"

ConnectDialog::ConnectDialog(TheMasterFrame *parent) : wxDialog(parent, wxID_ANY, "Connect"), frame(parent) {

    // Create sizer for Url label and text field
    wxBoxSizer *urlSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *urlLabel = new wxStaticText(this, wxID_ANY, "Url:");
    urlTextCtrl = new wxTextCtrl(this, wxID_ANY);
    urlSizer->Add(urlLabel, 0, wxALL, 5);
    urlSizer->Add(urlTextCtrl, 1, wxEXPAND | wxALL, 5);

    // Create sizer for buttons
    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton *connectButton = new wxButton(this, wxID_ANY, "Connect");
    wxButton *cancelButton = new wxButton(this, wxID_ANY, "Cancel");
    buttonSizer->Add(connectButton, 1, wxALL, 5);
    buttonSizer->Add(cancelButton, 1, wxALL, 5);

    // Create main sizer and add to dialog
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(urlSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);
    this->SetSizerAndFit(mainSizer);

    connectButton->Bind(wxEVT_BUTTON, &ConnectDialog::OnConnect, this);
    cancelButton->Bind(wxEVT_BUTTON, &ConnectDialog::OnCancel, this);
}

void ConnectDialog::OnConnect(wxCommandEvent &) {
    wxString url = urlTextCtrl->GetValue();
    frame->Connect(std::string(url.ToUTF8()));
    Close();
}

void ConnectDialog::OnCancel(wxCommandEvent &) {
    Close();
}