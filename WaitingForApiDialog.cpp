//
// Created by sigsegv on 12/18/23.
//

#include "WaitingForApiDialog.h"

WaitingForApiDialog::WaitingForApiDialog(wxWindow *parent, const std::string &title, const std::string &msg)
    : wxDialog(parent, wxID_ANY, title) {

     wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

     wxGauge* gauge = new wxGauge(this, wxID_ANY, 100);
     gauge->Pulse();
     text = new wxStaticText(this, wxID_ANY, msg);

     sizer->Add(gauge, 1, wxALL|wxEXPAND, 5);
     sizer->Add(text, 1, wxALL|wxEXPAND, 5);

     this->SetSizer(sizer);

     Bind(wxEVT_CLOSE_WINDOW, &WaitingForApiDialog::OnClose, this);
}

void WaitingForApiDialog::SetMessage(const std::string &msg) {
    text->SetLabel(msg);
}

void WaitingForApiDialog::Close() {
    if (!closed) {
        closed = true;
        wxDialog::Close(true);
    }
}

void WaitingForApiDialog::OnClose(wxCloseEvent &) {
    closed = true;
    EndDialog(wxID_OK);
}
