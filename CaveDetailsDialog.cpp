//
// Created by sigsegv on 11/14/24.
//

#include "CaveDetailsDialog.h"
#include <wx/listctrl.h>
#include <sfmbasisapi/fhir/allergy.h>

CaveDetailsDialog::CaveDetailsDialog(wxWindow *parent, const std::shared_ptr<FhirAllergyIntolerance> &allergy) : wxDialog(parent, wxID_ANY, wxT("Details")){
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    auto *listView = new wxListView(this);
    listView->AppendColumn(wxT(""));
    listView->AppendColumn(wxT(""));
    listView->SetColumnWidth(0, 150);
    listView->SetColumnWidth(1, 300);
    int rowCount = 0;
    auto codings = allergy->GetCode().GetCoding();
    if (!codings.empty()) {
        for (auto coding : codings) {
            {
                auto row = rowCount++;
                listView->InsertItem(row, wxT("System: "));
                listView->SetItem(row, 1, wxString::FromUTF8(coding.GetSystem()));
            }
            {
                auto row = rowCount++;
                listView->InsertItem(row, wxT("Code: "));
                listView->SetItem(row, 1, wxString::FromUTF8(coding.GetCode()));
            }
            {
                auto row = rowCount++;
                listView->InsertItem(row, wxT("Display: "));
                listView->SetItem(row, 1, wxString::FromUTF8(coding.GetDisplay()));
            }
        }
    } else {
        auto row = rowCount++;
        listView->InsertItem(row, wxT("Text: "));
        listView->SetItem(row, 1, wxString::FromUTF8(allergy->GetCode().GetText()));
    }
    {
        auto row = rowCount++;
        listView->InsertItem(row, wxT("Status: "));
        std::string status{};
        switch (allergy->GetStatus()) {
            case FhirStatus::NOT_SET:
                status = "Not set";
                break;
            case FhirStatus::ACTIVE:
                status = "Active";
                break;
            case FhirStatus::COMPLETED:
                status = "Complete";
                break;
            case FhirStatus::FINAL:
                status = "Final";
                break;
            case FhirStatus::STOPPED:
                status = "Stopped";
                break;
            default:
                status = "Unknown";
        }
        listView->SetItem(row, 1, wxString::FromUTF8(status));
    }
    for (const auto &category : allergy->GetCategories()) {
        auto row = rowCount++;
        listView->InsertItem(row, wxT("Category: "));
        listView->SetItem(row, 1, wxString::FromUTF8(category));
    }
    {
        auto row = rowCount++;
        listView->InsertItem(row, wxT("Clinical status: "));
        auto codings = allergy->GetClinicalStatus().GetCoding();
        auto clinicalStatus = !codings.empty() ? codings[0].GetDisplay() : allergy->GetClinicalStatus().GetText();
        listView->SetItem(row, 1, wxString::FromUTF8(clinicalStatus));
    }
    {
        auto row = rowCount++;
        listView->InsertItem(row, wxT("Criticality: "));
        listView->SetItem(row, 1, wxString::FromUTF8(allergy->GetCriticality()));
    }
    for (const auto &reaction : allergy->GetReactions()) {
        for (const auto &manifestation : reaction.GetManifestations()) {
            for (const auto &coding : manifestation.GetCoding()) {
                auto row = rowCount++;
                listView->InsertItem(row, wxT("Manifestation: "));
                listView->SetItem(row, 1, wxString::FromUTF8(coding.GetDisplay()));
            }
            auto text = manifestation.GetText();
            if (!text.empty()) {
                auto row = rowCount++;
                listView->InsertItem(row, wxT("Manifestation: "));
                listView->SetItem(row, 1, wxString::FromUTF8(text));
            }
        }
    }
    {
        auto row = rowCount++;
        listView->InsertItem(row, wxT("Recorded date: "));
        listView->SetItem(row, 1, wxString::FromUTF8(allergy->GetRecordedDate()));
    }
    {
        auto row = rowCount++;
        listView->InsertItem(row, wxT("Recorder: "));
        listView->SetItem(row, 1, wxString::FromUTF8(allergy->GetRecorder().GetDisplay()));
    }
    {
        for (const auto coding : allergy->GetVerificationStatus().GetCoding()) {
            auto row = rowCount++;
            listView->InsertItem(row, wxT("Verification status: "));
            listView->SetItem(row, 1, wxString::FromUTF8(coding.GetDisplay()));
        }
        auto text = allergy->GetVerificationStatus().GetText();
        if (!text.empty()) {
            auto row = rowCount++;
            listView->InsertItem(row, wxT("Verification status: "));
            listView->SetItem(row, 1, wxString::FromUTF8(text));
        }
    }
    for (const auto &extension : allergy->GetExtensions()) {
        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
        if (!valueExtension) {
            continue;
        }
        auto url = extension->GetUrl();
        std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
        if (url == "http://nhn.no/kj/fhir/structuredefinition/kjupdateddatetime") {
            auto value = std::dynamic_pointer_cast<FhirDateTimeValue>(valueExtension->GetValue());
            if (value) {
                auto row = rowCount++;
                listView->InsertItem(row, wxT("Updated: "));
                listView->SetItem(row, 1, wxString::FromUTF8(value->GetDateTime()));
            }
        } else if (url == "http://nhn.no/kj/fhir/structuredefinition/kjsourceofinformation") {
            auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
            if (value) {
                for (const auto &coding : value->GetCoding()) {
                    auto row = rowCount++;
                    listView->InsertItem(row, wxT("Source of info: "));
                    listView->SetItem(row, 1, wxString::FromUTF8(coding.GetDisplay()));
                }
                auto text = value->GetText();
                if (!text.empty()) {
                    auto row = rowCount++;
                    listView->InsertItem(row, wxT("Source of info: "));
                    listView->SetItem(row, 1, wxString::FromUTF8(text));
                }
            }
        }
    }
    sizer->Add(listView, 1, wxALL | wxEXPAND, 5);
    SetSizerAndFit(sizer);
}