//
// Created by sigsegv on 12/13/23.
//

#include "ConnectDialog.h"
#include "TheMasterFrame.h"
#include "HelseidLoginDialog.h"

ConnectDialog::ConnectDialog(TheMasterFrame *parent) : wxDialog(parent, wxID_ANY, "Connect"), frame(parent) {

    // Create sizer for Url label and text field
    wxBoxSizer *urlSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *urlLabel = new wxStaticText(this, wxID_ANY, "Url: ");
    urlTextCtrl = new wxTextCtrl(this, wxID_ANY);
    urlSizer->Add(urlLabel, 0, wxALL, 5);
    urlSizer->Add(urlTextCtrl, 1, wxEXPAND | wxALL, 5);

    wxBoxSizer *helseidUrlSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *helseidUrlLabel = new wxStaticText(this, wxID_ANY, "HelseID Url: ");
    helseidUrlCtrl = new wxTextCtrl(this, wxID_ANY);
    helseidUrlSizer->Add(helseidUrlLabel, 0, wxALL, 5);
    helseidUrlSizer->Add(helseidUrlCtrl, 0, wxALL, 5);

    wxBoxSizer *helseidClientIdSizer = new wxBoxSizer(wxHORIZONTAL);
    wxStaticText *helseidClientIdLabel = new wxStaticText(this, wxID_ANY, "HelseID Client ID: ");
    helseidClientIdCtrl = new wxTextCtrl(this, wxID_ANY);
    helseidClientIdSizer->Add(helseidClientIdLabel, 0, wxALL, 5);
    helseidClientIdSizer->Add(helseidClientIdCtrl, 0, wxALL, 5);

    // Create sizer for buttons
    wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
    wxButton *connectButton = new wxButton(this, wxID_ANY, "Connect");
    wxButton *cancelButton = new wxButton(this, wxID_ANY, "Cancel");
    buttonSizer->Add(connectButton, 1, wxALL, 5);
    buttonSizer->Add(cancelButton, 1, wxALL, 5);

    // Create main sizer and add to dialog
    wxBoxSizer *mainSizer = new wxBoxSizer(wxVERTICAL);
    mainSizer->Add(urlSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(helseidUrlSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(helseidClientIdSizer, 1, wxEXPAND | wxALL, 5);
    mainSizer->Add(buttonSizer, 0, wxALIGN_CENTER | wxALL, 5);
    this->SetSizerAndFit(mainSizer);

    connectButton->Bind(wxEVT_BUTTON, &ConnectDialog::OnConnect, this);
    cancelButton->Bind(wxEVT_BUTTON, &ConnectDialog::OnCancel, this);
}

void ConnectDialog::OnConnect(wxCommandEvent &) {
    wxString url = urlTextCtrl->GetValue();
    wxString helseidUrl = helseidUrlCtrl->GetValue();
    wxString helseidClientId = helseidClientIdCtrl->GetValue();
    if (!helseidUrl.empty()) {
        HelseidLoginDialog helseidLoginDialog{this, helseidUrl.ToStdString(), helseidClientId.ToStdString()};
        helseidLoginDialog.ShowModal();
        std::cout << "Result url: " << helseidLoginDialog.GetResultUrl() << "\n";
    }
    frame->Connect(std::string(url.ToUTF8()));
    Close();
}

void ConnectDialog::OnCancel(wxCommandEvent &) {
    Close();
}