//
// Created by sigsegv on 3/13/24.
//

#include "DownloadFestDialog.h"

DownloadFestDialog::DownloadFestDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, "Downloading FEST") {
    wxPanel *panel = new wxPanel(this);
    wxBoxSizer *vbox = new wxBoxSizer(wxVERTICAL);
    {
        wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *label = new wxStaticText(panel, wxID_ANY, "Downloading:", wxDefaultPosition, wxSize(80, -1));
        downloadProgress = new wxGauge(panel, wxID_ANY, 100, wxDefaultPosition, wxSize(-1, 15));

        hbox->Add(label, 0, wxALL, 5);
        hbox->Add(downloadProgress, 1, wxALL, 5);
        vbox->Add(hbox, 0, wxEXPAND, 1);
    }
    {
        wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *label = new wxStaticText(panel, wxID_ANY, "Decoding:", wxDefaultPosition, wxSize(80, -1));
        decodingProgress = new wxGauge(panel, wxID_ANY, 100, wxDefaultPosition, wxSize(-1, 15));

        hbox->Add(label, 0, wxALL, 5);
        hbox->Add(decodingProgress, 1, wxALL, 5);
        vbox->Add(hbox, 0, wxEXPAND, 1);
    }
    {
        wxBoxSizer *hbox = new wxBoxSizer(wxHORIZONTAL);
        wxStaticText *label = new wxStaticText(panel, wxID_ANY, "Encoding:", wxDefaultPosition, wxSize(80, -1));
        serializeProgress = new wxGauge(panel, wxID_ANY, 100, wxDefaultPosition, wxSize(-1, 15));

        hbox->Add(label, 0, wxALL, 5);
        hbox->Add(serializeProgress, 1, wxALL, 5);
        vbox->Add(hbox, 0, wxEXPAND, 1);
    }
    panel->SetSizer(vbox);
}

void DownloadFestDialog::SetDownloadProgress(int prcnt) {
    downloadProgress->SetValue(prcnt);
}

void DownloadFestDialog::SetDecodingProgress(int prcnt) {
    decodingProgress->SetValue(prcnt);
}

void DownloadFestDialog::SetSerializeProgress(int prcnt) {
    serializeProgress->SetValue(prcnt);
}