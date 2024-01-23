//
// Created by sigsegv on 1/22/24.
//

#include <wx/spinctrl.h>
#include "PrescriptionDialog.h"
#include "TheMasterFrame.h"
#include <iomanip>

PrescriptionDialog::PrescriptionDialog(TheMasterFrame *frame) : wxDialog(frame, wxID_ANY, wxT("Prescription")) {
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    auto *dssnSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *dssnLabel = new wxStaticText(this, wxID_ANY, wxT("DSSN:"));
    dssnCtrl = new wxTextCtrl(this, wxID_ANY);
    dssnSizer->Add(dssnLabel, 0, wxEXPAND | wxALL, 5);
    dssnSizer->Add(dssnCtrl, 1, wxEXPAND | wxALL, 5);
    auto *numPackagesSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *numPackagesLabel = new wxStaticText(this, wxID_ANY, wxT("Num packages:"));
    numberOfPackagesCtrl = new wxSpinCtrlDouble(this, wxID_ANY);
    numPackagesSizer->Add(numPackagesLabel, 0, wxEXPAND | wxALL, 5);
    numPackagesSizer->Add(numberOfPackagesCtrl, 1, wxEXPAND | wxALL, 5);
    auto *reitSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *reitLabel = new wxStaticText(this, wxID_ANY, wxT("Reit:"));
    reitCtrl = new wxSpinCtrl(this, wxID_ANY);
    reitSizer->Add(reitLabel, 0, wxEXPAND | wxALL, 5);
    reitSizer->Add(reitCtrl, 1, wxEXPAND | wxALL, 5);
    auto *applicationAreaSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *applicationAreaLabel = new wxStaticText(this, wxID_ANY, wxT("Application:"));
    applicationAreaCtrl = new wxTextCtrl(this, wxID_ANY);
    applicationAreaSizer->Add(applicationAreaLabel, 0, wxEXPAND | wxALL, 5);
    applicationAreaSizer->Add(applicationAreaCtrl, 1, wxEXPAND | wxALL, 5);
    auto *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *cancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"));
    auto *proceedButton = new wxButton(this, wxID_ANY, wxT("Finish"));
    buttonsSizer->Add(cancelButton, 0, wxEXPAND | wxALL, 5);
    buttonsSizer->Add(proceedButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(dssnSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(numPackagesSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(reitSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(applicationAreaSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(sizer);
    cancelButton->Bind(wxEVT_BUTTON, &PrescriptionDialog::OnCancel, this);
    proceedButton->Bind(wxEVT_BUTTON, &PrescriptionDialog::OnProceed, this);
}

void PrescriptionDialog::OnCancel(wxCommandEvent &e) {
    EndDialog(wxID_CANCEL);
}

void PrescriptionDialog::OnProceed(wxCommandEvent &e) {
    std::time_t now = std::time(nullptr);
    std::tm tm{};
    localtime_r(&now, &tm);
    std::tm tmp1y = tm;
    tmp1y.tm_year++;
    if (tmp1y.tm_mon == 1 && tmp1y.tm_mday == 29) {
        tmp1y.tm_mon++;
        tmp1y.tm_mday = 1;
    }
    std::time_t nowp1y = mktime(&tmp1y);
    std::time_t endtime = nowp1y - (24 * 3600);
    std::tm endtm{};
    localtime_r(&endtime, &endtm);
    char date[11];  // Date string will have 10 chars + null terminator
    std::strftime(date, sizeof(date), "%Y-%m-%d", &tm);
    prescriptionData.reseptdate = date;
    std::strftime(date, sizeof(date), "%Y-%m-%d", &endtm);
    prescriptionData.expirationdate = date;
    prescriptionData.dssn = dssnCtrl->GetValue().ToStdString();
    auto numPkg = numberOfPackagesCtrl->GetValue();
    if (numPkg > 0.001) {
        prescriptionData.numberOfPackages = numPkg;
        prescriptionData.numberOfPackagesSet = true;
    }
    prescriptionData.reit = reitCtrl->GetValue();
    prescriptionData.applicationArea = applicationAreaCtrl->GetValue();
    prescriptionData.itemGroup = {"urn:oid:2.16.578.1.12.4.1.1.7402", "L", "Legemiddel", "Legemiddel"};
    prescriptionData.rfstatus = {"urn:oid:2.16.578.1.12.4.1.1.7408", "E", "Ekspederbar", "Ekspederbar"};
    prescriptionData.typeresept = {"urn:oid:2.16.578.1.12.4.1.1.7491", "E", "Eresept", "Eresept"};
    prescriptionData.use = {"urn:oid:2.16.578.1.12.4.1.1.9101", "1", "Fast", "Fast"};

    std::ostringstream nowStream;
    nowStream << std::put_time(&tm, "%Y-%m-%dT%H:%M:%S");

    auto tzone = localtime(&now);
    if(tzone->tm_gmtoff >= 0)
        nowStream << "+";
    else
        nowStream << "-";
    nowStream << std::setfill('0') << std::setw(2) << abs(tzone->tm_gmtoff / 3600) << ":" << std::setw(2) << abs((tzone->tm_gmtoff / 60) % 60);

    std::string nowString = nowStream.str();
    prescriptionData.lastChanged = nowString;
    EndDialog(wxID_OK);
}
