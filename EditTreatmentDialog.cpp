//
// Created by jeo on 11/13/24.
//

#include "EditTreatmentDialog.h"
#include "MedBundleData.h"
#include "DateTime.h"
#include <sfmbasisapi/fhir/person.h>
#include <sfmbasisapi/fhir/medstatement.h>

EditTreatmentDialog::EditTreatmentDialog(wxWindow *parent, const MedBundleData &medBundleData, const std::shared_ptr<FhirMedicationStatement> &medicationStatement) : wxDialog(parent, wxID_ANY, wxT("Edit treatment")), medicationStatement(medicationStatement) {
    {
        auto practitionerItems = medBundleData.GetPractitioners();
        for (const auto &practitionerItem : practitionerItems) {
            practitionerUrls.emplace_back(practitionerItem.GetFullUrl());
            auto person = std::dynamic_pointer_cast<FhirPerson>(practitionerItem.GetResource());
            std::vector<FhirName> names{};
            std::string display = person->GetDisplay();
            if (display.empty()) {
                if (person) {
                    names = person->GetName();
                }
                if (!names.empty()) {
                    display = names[0].GetDisplay();
                    if (display.empty()) {
                        display = names[0].GetGiven();
                        if (!names[0].GetFamily().empty()) {
                            if (!display.empty()) {
                                display.append(" ");
                            }
                            display.append(names[0].GetFamily());
                        }
                    }
                } else {
                    display = practitionerItem.GetFullUrl();
                }
                if (display.empty()) {
                    display = "Empty";
                }
            }
            practitionerDisplays.emplace_back(std::move(display));
        }
    }

    auto *sizer = new wxBoxSizer(wxVERTICAL);
    auto *institutedSelectSizer = new wxBoxSizer(wxHORIZONTAL);
    institutedSelectSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Instituted by: ")));
    institutedSelect = new wxComboBox(this, wxID_ANY);
    institutedSelect->SetEditable(false);
    institutedSelect->Append(wxT(""));
    for (const auto &display : practitionerDisplays) {
        institutedSelect->Append(wxString::FromUTF8(display));
    }
    institutedSelectSizer->Add(institutedSelect, 0, wxALL | wxEXPAND, 5);
    sizer->Add(institutedSelectSizer, 0, wxALL | wxEXPAND, 5);
    auto *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *cancelButton = new wxButton(this, wxID_ANY, wxT("Cancel"));
    auto *okButton = new wxButton(this, wxID_ANY, wxT("Ok"));
    buttonsSizer->Add(cancelButton, 0, wxEXPAND | wxALL, 5);
    buttonsSizer->Add(okButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(sizer);

    cancelButton->Bind(wxEVT_BUTTON, &EditTreatmentDialog::OnCancel, this);
    okButton->Bind(wxEVT_BUTTON, &EditTreatmentDialog::OnOk, this);
}

void EditTreatmentDialog::OnCancel(wxCommandEvent &e) {
    EndDialog(wxID_CANCEL);
}

struct RegInfo {
    std::shared_ptr<FhirExtension> regInfo{};
    std::shared_ptr<FhirCodeableConceptValue> status{};
    std::shared_ptr<FhirCodeableConceptValue> type{};
    std::shared_ptr<FhirReference> provider{};
    std::shared_ptr<FhirDateTimeValue> timestamp{};

    explicit operator bool() const {
        return regInfo.operator bool();
    }
};

void EditTreatmentDialog::OnOk(wxCommandEvent &e) {
    std::string fullUrl{};
    std::string display{};
    {
        auto selection = institutedSelect->GetSelection();
        if (selection > 0) {
            --selection;
            if (selection < practitionerUrls.size() && selection < practitionerDisplays.size()) {
                fullUrl = practitionerUrls[selection];
                display = practitionerDisplays[selection];
            }
        }
    }
    RegInfo institutedByExtension{};
    for (const auto &extension : medicationStatement->GetExtensions()) {
        auto url = extension->GetUrl();
        std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
        if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reginfo") {
            RegInfo regInfo{};
            for (const auto &extension : extension->GetExtensions()) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                if (!valueExtension) {
                    continue;
                }
                if (url == "status") {
                    auto codeableConcept = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                    if (codeableConcept) {
                        regInfo.status = codeableConcept;
                    }
                } else if (url == "type") {
                    auto codeableConcept = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                    if (codeableConcept) {
                        regInfo.type = codeableConcept;
                    }
                } else if (url == "provider") {
                    auto fhirReference = std::dynamic_pointer_cast<FhirReference>(valueExtension->GetValue());
                    if (fhirReference) {
                        regInfo.provider = fhirReference;
                    }
                } else if (url == "timestamp") {
                    auto dateTimeValue = std::dynamic_pointer_cast<FhirDateTimeValue>(valueExtension->GetValue());
                    if (dateTimeValue) {
                        regInfo.timestamp = dateTimeValue;
                    }
                }
            }
            if (!regInfo.type) {
                continue;
            }
            auto typeCodings = regInfo.type->GetCoding();
            if (typeCodings.size() != 1) {
                continue;
            }
            auto typeCode = typeCodings[0].GetCode();
            if (typeCode != "2") {
                continue;
            }
            if (institutedByExtension) {
                wxMessageBox(wxT("Duplicate instituted by reginfos on statement"), wxT("Invalid FHIR data"), wxICON_ERROR);
                return;
            }
            institutedByExtension = std::move(regInfo);
            institutedByExtension.regInfo = extension;
        }
    }
    if (!fullUrl.empty()) {
        if (!institutedByExtension) {
            institutedByExtension.regInfo = std::make_shared<FhirExtension>(
                    "http://ehelse.no/fhir/StructureDefinition/sfm-regInfo");
            medicationStatement->AddExtension(institutedByExtension.regInfo);
        }
        if (!institutedByExtension.status) {
            FhirCodeableConcept codeableConcept{
                    "http://ehelse.no/fhir/CodeSystem/sfm-medicationstatement-registration-status", "3", "Godkjent"};
            institutedByExtension.status = std::make_shared<FhirCodeableConceptValue>(codeableConcept);
            institutedByExtension.regInfo->AddExtension(
                    std::make_shared<FhirValueExtension>("status", institutedByExtension.status));
        }
        if (!institutedByExtension.type) {
            FhirCodeableConcept codeableConcept{"http://ehelse.no/fhir/CodeSystem/sfm-performer-roles", "2", "Instituert av"};
            institutedByExtension.status = std::make_shared<FhirCodeableConceptValue>(codeableConcept);
            institutedByExtension.regInfo->AddExtension(std::make_shared<FhirValueExtension>("type", institutedByExtension.status));
        }
        FhirReference reference{fullUrl, "http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner", display};
        if (institutedByExtension.provider) {
            *(institutedByExtension.provider) = std::move(reference);
        } else {
            institutedByExtension.provider = std::make_shared<FhirReference>(std::move(reference));
            institutedByExtension.regInfo->AddExtension(std::make_shared<FhirValueExtension>("provider", institutedByExtension.provider));
        }
        auto now = DateTimeOffset::Now();
        if (institutedByExtension.timestamp) {
            institutedByExtension.timestamp->SetDateTime(now.to_iso8601());
        } else {
            institutedByExtension.timestamp = std::make_shared<FhirDateTimeValue>(now.to_iso8601());
            institutedByExtension.regInfo->AddExtension(std::make_shared<FhirValueExtension>("timestamp", institutedByExtension.timestamp));
        }
    }
    EndDialog(wxID_OK);
}