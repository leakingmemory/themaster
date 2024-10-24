//
// Created by sigsegv on 10/15/24.
//

#include <wx/spinctrl.h>
#include "AdvancedDosingPeriodDialog.h"
#include "AdvancedDosingPeriod.h"

AdvancedDosingPeriodDialog::AdvancedDosingPeriodDialog(wxWindow *parent) : wxDialog(parent, wxID_ANY, wxT("Advanced dosing period")) {
    auto sizer = new wxBoxSizer(wxVERTICAL);
    {
        auto gs = new wxGridSizer(6);
        gs->Add(new wxStaticText(this, wxID_ANY, wxT("Morgen")), 0, wxEXPAND);
        gs->Add(new wxStaticText(this, wxID_ANY, wxT("Formiddag")), 0, wxEXPAND);
        gs->Add(new wxStaticText(this, wxID_ANY, wxT("Middag")), 0, wxEXPAND);
        gs->Add(new wxStaticText(this, wxID_ANY, wxT("Ettermiddag")), 0, wxEXPAND);
        gs->Add(new wxStaticText(this, wxID_ANY, wxT("Kveld")), 0, wxEXPAND);
        gs->Add(new wxStaticText(this, wxID_ANY, wxT("Natt")), 0, wxEXPAND);
        gs->Add(morgen = new wxSpinCtrlDouble(this), 0, wxEXPAND);
        gs->Add(formiddag = new wxSpinCtrlDouble(this), 0, wxEXPAND);
        gs->Add(middag = new wxSpinCtrlDouble(this), 0, wxEXPAND);
        gs->Add(ettermiddag = new wxSpinCtrlDouble(this), 0, wxEXPAND);
        gs->Add(kveld = new wxSpinCtrlDouble(this), 0, wxEXPAND);
        gs->Add(natt = new wxSpinCtrlDouble(this), 0, wxEXPAND);
        sizer->Add(gs, 0, wxEXPAND | wxALL, 5);
    }
    {
        auto hs = new wxBoxSizer(wxHORIZONTAL);
        hs->Add(new wxStaticText(this, wxID_ANY, wxT("Duration (days): ")), 0, wxEXPAND | wxALL, 5);
        hs->Add(duration = new wxSpinCtrl(this), 0, wxEXPAND | wxALL, 5);
        sizer->Add(hs, 0, wxEXPAND | wxALL, 5);
    }
    {
        auto hs = new wxBoxSizer(wxHORIZONTAL);
        hs->Add(ok = new wxButton(this, wxID_OK, wxT("Ok")), 0, wxEXPAND | wxALL, 5);
        hs->Add(new wxButton(this, wxID_CANCEL, wxT("Cancel")), 0, wxEXPAND | wxALL, 5);
        sizer->Add(hs, 0, wxEXPAND | wxALL, 5);
    }
    SetSizerAndFit(sizer);
    ok->Bind(wxEVT_BUTTON, &AdvancedDosingPeriodDialog::OnOk, this, wxID_ANY);
}

void AdvancedDosingPeriodDialog::OnOk(wxCommandEvent &e) {
    dosingPeriod = std::make_shared<FixedTimeAdvancedDosingPeriod>(morgen->GetValue(), formiddag->GetValue(), middag->GetValue(), ettermiddag->GetValue(), kveld->GetValue(), natt->GetValue(), duration->GetValue());
    EndModal(wxID_OK);
}
