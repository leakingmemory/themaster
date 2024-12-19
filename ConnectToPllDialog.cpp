//
// Created by sigsegv on 12/18/24.
//

#include "ConnectToPllDialog.h"
#include "MedBundleData.h"
#include <wx/listctrl.h>
#include <sfmbasisapi/fhir/medstatement.h>
#include <sfmbasisapi/fhir/composition.h>
#include <map>

ConnectToPllDialog::ConnectToPllDialog(wxWindow *parent, const MedBundleData &bundleData) : wxDialog(parent, wxID_ANY, wxT("Connect to PLL")) {
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    listView = new wxListView(this, wxID_ANY);
    listView->AppendColumn(wxT("Name"));
    listView->SetColumnWidth(0, 300);
    if (bundleData.medBundle) {
        std::map<std::string,std::shared_ptr<FhirMedicationStatement>> statementMap{};
        std::shared_ptr<FhirComposition> composition{};
        for (const auto &entry: bundleData.medBundle->GetEntries()) {
            auto resource = entry.GetResource();
            auto medstatement = std::dynamic_pointer_cast<FhirMedicationStatement>(resource);
            if (medstatement) {
                statementMap.insert_or_assign(entry.GetFullUrl(), medstatement);
                continue;
            }
            auto comp = std::dynamic_pointer_cast<FhirComposition>(resource);
            if (comp) {
                composition = comp;
            }
        }
        FhirCompositionSection medicationSection{};
        if (composition) {
            for (const auto &section: composition->GetSections()) {
                auto sectionCoding = section.GetCode().GetCoding();
                if (sectionCoding.empty() || sectionCoding[0].GetCode() != "sectionMedication") {
                    continue;
                }
                medicationSection = section;
                break;
            }
        }
        for (const auto &reference : medicationSection.GetEntries()) {
            auto iterator = statementMap.find(reference.GetReference());
            if (iterator == statementMap.end()) {
                continue;
            }
            auto statement = iterator->second;
            auto identifiers = statement->GetIdentifiers();
            if (std::find_if(identifiers.cbegin(), identifiers.cend(), [](const auto &identifier) {
                if (identifier.GetValue().empty()) {
                    return false;
                }
                auto type = identifier.GetType().GetText();
                std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                return type == "pll";
            }) == identifiers.cend()) {
                continue;
            }
            auto medicationStatementExtensions = statement->GetExtensions();
            auto reseptAmendmentIterator = std::find_if(medicationStatementExtensions.cbegin(), medicationStatementExtensions.cend(), [](const auto &extension) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                return url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment";
            });
            if (reseptAmendmentIterator == medicationStatementExtensions.cend()) {
                continue;
            }
            auto reseptAmendmentExtensions = (*reseptAmendmentIterator)->GetExtensions();
            auto rfstatusIterator = std::find_if(reseptAmendmentExtensions.cbegin(), reseptAmendmentExtensions.cend(), [](const auto &extension) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                return url == "rfstatus";
            });
            if (rfstatusIterator != reseptAmendmentExtensions.cend()) {
                auto rfstatusExtensions = (*rfstatusIterator)->GetExtensions();
                auto statusIterator = std::find_if(rfstatusExtensions.cbegin(), rfstatusExtensions.cend(), [](const auto &extension) {
                    auto url = extension->GetUrl();
                    std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                    return url == "status";
                });
                if (statusIterator != rfstatusExtensions.cend()) {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(*statusIterator);
                    if (valueExtension) {
                        auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                        if (value && !value->GetCoding().empty()) {
                            auto code = value->GetCoding()[0].GetCode();
                            if (code == "E" || code == "U") {
                                continue;
                            }
                        }
                    }
                }
            }
            auto display = statement->GetDisplay();
            listView->InsertItem(listView->GetItemCount(), wxString::FromUTF8(display));
            statements.emplace_back(statement);
        }
    }
    sizer->Add(listView, 1, wxALL | wxEXPAND, 5);
    auto *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
    okButton = new wxButton(this, wxID_OK, wxT("Connect"));
    okButton->Enable(false);
    buttonsSizer->Add(cancelButton, 0, wxEXPAND | wxALL, 5);
    buttonsSizer->Add(okButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(sizer);
    listView->Bind(wxEVT_LIST_ITEM_SELECTED, &ConnectToPllDialog::RevalidateOkToProceed, this, wxID_ANY);
}

bool ConnectToPllDialog::HasSelectedMedicationStatement() const {
    if (listView->GetSelectedItemCount() != 1) {
        return false;
    }
    auto selection = listView->GetFirstSelected();
    return selection >= 0 && selection < statements.size();
}

std::shared_ptr<FhirMedicationStatement> ConnectToPllDialog::GetSelectedMedicationStatement() const {
    if (listView->GetSelectedItemCount() != 1) {
        return {};
    }
    auto selection = listView->GetFirstSelected();
    if (selection >= 0 && selection < statements.size()) {
        return statements[selection];
    }
    return {};
}

void ConnectToPllDialog::RevalidateOkToProceed(wxCommandEvent &) {
    okButton->Enable(HasSelectedMedicationStatement());
}
