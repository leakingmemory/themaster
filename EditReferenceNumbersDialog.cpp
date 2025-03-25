//
// Created by sigsegv on 3/18/25.
//

#include "EditReferenceNumbersDialog.h"
#include "TheMasterIds.h"

EditReferenceNumbersDialog::EditReferenceNumbersDialog(wxWindow *parent, std::vector<std::string> refNumbers) : wxDialog(parent, wxID_ANY, wxT("Edit reference numbers")){
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto splitSizer = new wxBoxSizer(wxHORIZONTAL);
    this->refNumbers = new wxListView(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLC_REPORT | wxWANTS_CHARS);
    this->refNumbers->AppendColumn(wxT("Reference number"));
    {
        int i = 0;
        for (const auto &refNumber : refNumbers) {
            this->refNumbers->InsertItem(i++, wxString::FromUTF8(refNumber));
        }
    }
    this->refNumbers->Bind(wxEVT_CONTEXT_MENU, &EditReferenceNumbersDialog::OnContextMenu, this, wxID_ANY);
    this->refNumbers->Bind(wxEVT_CHAR_HOOK, &EditReferenceNumbersDialog::OnCharListEvent, this, wxID_ANY);
    splitSizer->Add(this->refNumbers, 1, wxEXPAND | wxALL, 5);
    auto modSizer = new wxBoxSizer(wxVERTICAL);
    modSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Add reference number:")), 0, wxEXPAND | wxALL, 5);
    addRef = new wxTextCtrl(this, wxID_ANY);
    addRef->Bind(wxEVT_TEXT, &EditReferenceNumbersDialog::OnEditAddReferenceNumber, this, wxID_ANY);
    addRef->Bind(wxEVT_CHAR_HOOK, &EditReferenceNumbersDialog::OnCharEventEdit, this, wxID_ANY);
    modSizer->Add(addRef, 0, wxEXPAND | wxALL, 5);
    addButton = new wxButton(this, wxID_ANY, wxT("Add"));
    addButton->Bind(wxEVT_BUTTON, &EditReferenceNumbersDialog::OnAddReferenceNumber, this, wxID_ANY);
    addButton->Enable(false);
    modSizer->Add(addButton, 0, wxEXPAND | wxALL, 5);
    splitSizer->Add(modSizer, 0, wxEXPAND | wxALL, 5);
    sizer->Add(splitSizer, 0, wxEXPAND | wxALL, 5);
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto okButton = new wxButton(this, wxID_OK, wxT("OK"));
    auto cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
    buttonsSizer->Add(okButton, 0, wxALL, 5);
    buttonsSizer->Add(cancelButton, 0, wxALL, 5);
    sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);
    SetSizerAndFit(sizer);
    Bind(wxEVT_MENU, &EditReferenceNumbersDialog::OnRemoveReferenceNumber, this, TheMaster_EditRefDialogRemoveReferenceNumber_Id);
}

bool EditReferenceNumbersDialog::HasReferenceNumber(const wxString &wxRn) const {
    auto itemCount = refNumbers->GetItemCount();
    for (decltype(itemCount) i = 0; i < itemCount; i++) {
        auto item = refNumbers->GetItemText(i);
        if (wxRn == item) {
            return true;
        }
    }
    return false;
}

bool EditReferenceNumbersDialog::HasReferenceNumber(const std::string &rn) const {
    wxString wxRn = wxString::FromUTF8(rn);
    return HasReferenceNumber(wxRn);
}

std::vector<std::string> EditReferenceNumbersDialog::GetReferenceNumbers() const {
    std::vector<std::string> result{};
    auto itemCount = refNumbers->GetItemCount();
    for (decltype(itemCount) i = 0; i < itemCount; i++) {
        auto item = refNumbers->GetItemText(i);
        result.emplace_back(item.ToStdString());
    }
    return result;
}

void EditReferenceNumbersDialog::OnEditAddReferenceNumber(const wxCommandEvent &e) {
    auto suggestedRefNum = addRef->GetValue();
    if (suggestedRefNum.empty()) {
        addButton->Enable(false);
        return;
    }
    addButton->Enable(!HasReferenceNumber(suggestedRefNum));
}

void EditReferenceNumbersDialog::OnCharEventEdit(wxKeyEvent &e) {
    if (e.GetKeyCode() == WXK_RETURN || e.GetKeyCode() == WXK_NUMPAD_ENTER) {
        if (addButton->IsEnabled()) {
            OnAddReferenceNumber({});
        }
        return;
    }
    e.Skip();
}

void EditReferenceNumbersDialog::OnCharListEvent(wxKeyEvent &e) {
    if (e.GetKeyCode() == WXK_DELETE) {
        OnRemoveReferenceNumber({});
        return;
    }
    e.Skip();
}

void EditReferenceNumbersDialog::OnAddReferenceNumber(const wxCommandEvent &e) {
    refNumbers->InsertItem(refNumbers->GetItemCount(), addRef->GetValue());
    addButton->Enable(false);
}

void EditReferenceNumbersDialog::OnContextMenu(const wxContextMenuEvent &e) {
    if (refNumbers->GetSelectedItemCount() <= 0) {
        return;
    }
    wxMenu menu{wxT("Reference numbers")};
    menu.Append(TheMaster_EditRefDialogRemoveReferenceNumber_Id, wxT("Remove"));
    PopupMenu(&menu);
}

void EditReferenceNumbersDialog::OnRemoveReferenceNumber(const wxCommandEvent &e) {
    if (refNumbers->GetSelectedItemCount() <= 0) {
        return;
    }
    auto numItems = refNumbers->GetItemCount();
    std::vector<int> rowsToDelete{};
    auto refNum = refNumbers->GetFirstSelected();
    while (refNum >= 0 && refNum < numItems) {
        rowsToDelete.emplace_back(refNum);
        refNum = refNumbers->GetNextSelected(refNum);
    }
    std::sort(rowsToDelete.begin(), rowsToDelete.end());
    auto iterator = rowsToDelete.end();
    while (iterator != rowsToDelete.begin()) {
        iterator--;
        refNumbers->DeleteItem(*iterator);
    }
}