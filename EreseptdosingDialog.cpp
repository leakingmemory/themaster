//
// Created by sigsegv on 10/18/24.
//

#include "EreseptdosingDialog.h"
#include <sfmbasisapi/fhir/extension.h>
#include <sfmbasisapi/fhir/fhir.h>
#include <sfmbasisapi/fhir/value.h>
#include <wx/listctrl.h>

EreseptdosingDialog::EreseptdosingDialog(wxWindow *parent, const std::vector<std::shared_ptr<FhirExtension>> &ereseptdosing) : wxDialog(parent, wxID_ANY, wxT("Ereseptdosing")), ereseptdosing(ereseptdosing) {
    wxBoxSizer *sizer = new wxBoxSizer(wxVERTICAL);
    info = new wxListView(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 100));
    info->AppendColumn(wxT("Start"));
    info->AppendColumn(wxT("End"));
    sizer->Add(info, 0, wxALL | wxEXPAND, 5);
    dpInfo = new wxListView(this, wxID_ANY, wxDefaultPosition, wxSize(-1, 100));
    sizer->Add(dpInfo, 0, wxALL | wxEXPAND, 5);
    SetSizerAndFit(sizer);
    int rows = 0;
    for (const auto &ed : ereseptdosing) {
        auto row = rows++;
        std::string starttm{};
        std::string endtm{};
        auto extensions = ed->GetExtensions();
        for (const auto &extension : extensions) {
            auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
            if (!valueExtension) {
                continue;
            }
            auto value = std::dynamic_pointer_cast<FhirDateValue>(valueExtension->GetValue());
            if (!value) {
                continue;
            }
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) constexpr -> char { return std::tolower(ch); });
            if (url == "starttime") {
                starttm = value->GetRawValue();
            } else if (url == "endtime") {
                endtm = value->GetRawValue();
            }
        }
        info->InsertItem(row, wxString::FromUTF8(starttm));
        info->SetItem(row, 1, wxString::FromUTF8(endtm));
    }
    if (!ereseptdosing.empty()) {
        info->Select(0);
        info->Bind(wxEVT_LIST_ITEM_DESELECTED, &EreseptdosingDialog::OnDosingPeriodSelected, this, wxID_ANY);
        info->Bind(wxEVT_LIST_ITEM_SELECTED, &EreseptdosingDialog::OnDosingPeriodSelected, this, wxID_ANY);
        DisplayEreseptdosing(ereseptdosing[0]);
    }
}

void EreseptdosingDialog::DisplayEreseptdosing(const std::shared_ptr<FhirExtension> &ereseptdosing) {
    dpInfo->ClearAll();
    dpInfo->AppendColumn(wxT("Amount"));
    dpInfo->AppendColumn(wxT("Interval"));
    dpInfo->AppendColumn(wxT("Timerange"));
    if (!ereseptdosing) {
        return;
    }
    int rows = 0;
    for (const auto &dosing : ereseptdosing->GetExtensions()) {
        {
            auto url = dosing->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return std::tolower(ch); });
            if (url == "repeatingdosage") {
                int row = rows++;
                FhirQuantity amount{};
                FhirQuantity interval{};
                FhirCoding timerange{};
                for (const auto &extension : dosing->GetExtensions()) {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (!valueExtension) {
                        continue;
                    }
                    auto url = dosing->GetUrl();
                    std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return std::tolower(ch); });
                    if (url == "amount") {
                        auto value = std::dynamic_pointer_cast<FhirQuantityValue>(valueExtension->GetValue());
                        if (value) {
                            amount = *value;
                        }
                    } else if (url == "interval") {
                        auto value = std::dynamic_pointer_cast<FhirQuantityValue>(valueExtension->GetValue());
                        if (value) {
                            interval = *value;
                        }
                    } else if (url == "timerange") {
                        auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                        if (value) {
                            auto codings = value->GetCoding();
                            if (!codings.empty()) {
                                timerange = codings[0];
                            }
                        }
                    }
                }
                {
                    std::stringstream str{};
                    str << amount.GetValue() << " " << amount.GetUnit();
                    dpInfo->InsertItem(row, wxString::FromUTF8(str.str()));
                }
                {
                    std::stringstream str{};
                    str << interval.GetValue() << " " << interval.GetUnit();
                    dpInfo->SetItem(row, 1, wxString::FromUTF8(str.str()));
                }
                {
                    dpInfo->SetItem(row, 2, wxString::FromUTF8(timerange.GetDisplay()));
                }
            }
        }
    }
}

void EreseptdosingDialog::OnDosingPeriodSelected(wxCommandEvent &e) {
    if (info->GetSelectedItemCount() != 1) {
        DisplayEreseptdosing({});
        return;
    }
    auto selected = info->GetFirstSelected();
    if (selected < 0 || selected >= ereseptdosing.size()) {
        DisplayEreseptdosing({});
        return;
    }
    DisplayEreseptdosing(ereseptdosing[selected]);
}