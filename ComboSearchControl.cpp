//
// Created by sigsegv on 3/6/25.
//

#include "ComboSearchControl.h"
#include <wx/listctrl.h>

std::vector<wxString> ComboSearchControlListProvider::GetVisibleList() const {
    auto items = GetItems();
    if (items.size() <= 100) {
        return items;
    }
    return {};
}

void ComboSearchControlListProvider::SetAutoComplete(const std::string &str) {
    autoComplete = str;
}

std::vector<wxString> ComboSearchControlPlainListProvider::GetItems() const {
    std::vector<wxString> matching{};
    wxString searchFor = wxString::FromUTF8(autoComplete);
    for (const auto &item : items) {
        if (item.find(searchFor) < item.size()) {
            matching.emplace_back(item);
        }
    }
    return matching;
}

ssize_t ComboSearchControlPlainListProvider::GetIndexOf(const wxString &searchFor) const {
    for (decltype(items.size()) i = 0; i < items.size(); ++i) {
        if (items[i] == searchFor) {
            return i;
        }
    }
    return -1;
}

void ComboSearchControlPlainListProvider::Clear() {
    items.clear();
}

void ComboSearchControlPlainListProvider::Append(const wxString &str) {
    items.emplace_back(str);
}

class ComboSearchControlPopup : public wxListView, public wxComboPopup {
private:
    wxString itemNameLabel;
    std::shared_ptr<ComboSearchControlListProvider> listProvider;
    std::vector<wxString> visibleList{};
    std::string autoComplete{};
public:
    ComboSearchControlPopup(const std::shared_ptr<ComboSearchControlListProvider> &listProvider, const wxString &itemNameLabel) : itemNameLabel(itemNameLabel), listProvider(listProvider) {}
    bool Create(wxWindow* parent) override ;
    void OnPopup() override;
    virtual wxWindow *GetControl() override;
    wxString GetStringValue() const override;
    void SetStringValue(const wxString &value) override;
    void OnComboCharEvent( wxKeyEvent& event ) override;
    void OnSelectedItem(const wxCommandEvent & event);
};

bool ComboSearchControlPopup::Create(wxWindow *parent) {
    auto success = wxListView::Create(parent,1,wxPoint(0,0),wxSize(150,100),wxLC_REPORT | wxWANTS_CHARS);
    wxListView::InsertColumn(0, itemNameLabel);
    if (success) {
        auto list = listProvider->GetVisibleList();
        int row = 0;
        for (const auto &str : list) {
            std::cout << "New insert " << str << "\n";
            wxListView::InsertItem(row++, str);
        }
        visibleList = list;
    }
    wxListView::SetSize(wxListView::GetBestSize());
    wxListView::Bind(wxEVT_CHAR, &ComboSearchControlPopup::OnComboCharEvent, this);
    wxListView::Bind(wxEVT_LIST_ITEM_SELECTED, &ComboSearchControlPopup::OnSelectedItem, this);
    return success;
}

void ComboSearchControlPopup::OnPopup() {
    autoComplete = "";
    listProvider->SetAutoComplete(autoComplete);
    auto list = listProvider->GetVisibleList();
    if (list.size() == visibleList.size()) {
        bool mismatch{false};
        for (size_t i = 0; i < list.size(); ++i) {
            if (list[i] != visibleList[i]) {
                mismatch = true;
                break;
            }
        }
        if (!mismatch) {
            return;
        }
    }
    int row = 0;
    wxListView::ClearAll();
    wxListView::InsertColumn(0, wxT("Refund code"));
    for (const auto &str : list) {
        std::cout << "Popup insert " << str << "\n";
        wxListView::InsertItem(row++, str);
    }
    wxListView::SetSize(wxListView::GetBestSize());
    visibleList = list;
}

wxWindow *ComboSearchControlPopup::GetControl() {
    return this;
}

wxString ComboSearchControlPopup::GetStringValue() const {
    if (GetSelectedItemCount() == 1) {
        auto selected = GetFirstSelected();
        if (selected >= 0 && selected < visibleList.size()) {
            std::cout << "Get string value: " << visibleList[selected] << std::endl;
            return visibleList[selected];
        }
    }
    std::cout << "Get string value: <empty>" << std::endl;
    return wxEmptyString;
}

#include <iostream>
void ComboSearchControlPopup::SetStringValue(const wxString &value) {
    std::cout << "SetStringValue: " << value << std::endl;
}

void ComboSearchControlPopup::OnComboCharEvent(wxKeyEvent &event) {
    if (wxIsprint(event.GetUnicodeKey())) {
        wxChar c = event.GetUnicodeKey();
        auto wxstr = wxString::FromUTF8(autoComplete);
        auto pos = GetComboCtrl()->GetInsertionPoint();
        if (pos >= wxstr.Length() || wxIsEmpty(wxstr)) {
            wxstr.Append(c);
            pos = wxstr.Length();
        } else {
            if (pos < 0) {
                pos = 0;
            }
            wxstr.insert(pos, c);
            ++pos;
        }
        GetComboCtrl()->GetTextCtrl()->SetValue(wxstr);
        GetComboCtrl()->SetInsertionPoint(pos);
        autoComplete = wxstr.ToUTF8();
        std::cout << "OnComboCharEvent: " << autoComplete << "\n";
    } else if (event.GetKeyCode() == WXK_BACK) {
        auto wxstr = wxString::FromUTF8(autoComplete);
        auto pos = GetComboCtrl()->GetInsertionPoint();
        if (pos >= wxstr.Length()) {
            wxstr.RemoveLast();
            pos = wxstr.Length();
        } else  if (wxstr.Length() > 0){
            if (pos < 0) {
                pos = 0;
            }
            if (pos > 0) {
                --pos;
                wxstr.erase(pos, 1);
            }
        }
        GetComboCtrl()->GetTextCtrl()->SetValue(wxstr);
        GetComboCtrl()->SetInsertionPoint(pos);
        autoComplete = wxstr.ToUTF8();
        std::cout << "OnComboCharEvent: " << autoComplete << "\n";
    } else if (event.GetKeyCode() == WXK_LEFT) {
        auto pos = GetComboCtrl()->GetInsertionPoint();
        if (pos > 0) {
            --pos;
            GetComboCtrl()->SetInsertionPoint(pos);
        }
        return;
    } else if (event.GetKeyCode() == WXK_RIGHT) {
        auto wxstr = GetComboCtrl()->GetValue();
        auto pos = GetComboCtrl()->GetInsertionPoint();
        ++pos;
        if (pos <= wxstr.Length()) {
            GetComboCtrl()->SetInsertionPoint(pos);
        }
        return;
    }
    listProvider->SetAutoComplete(autoComplete);
    auto list = listProvider->GetVisibleList();
    if (list.size() == visibleList.size()) {
        bool mismatch{false};
        for (size_t i = 0; i < list.size(); ++i) {
            if (list[i] != visibleList[i]) {
                mismatch = true;
                break;
            }
        }
        if (!mismatch) {
            return;
        }
    }
    int row = 0;
    wxListView::ClearAll();
    wxListView::InsertColumn(0, wxT("Refund code"));
    for (const auto &str : list) {
        std::cout << "Popup insert " << str << "\n";
        wxListView::InsertItem(row++, str);
    }
    wxListView::SetSize(wxListView::GetBestSize());
    visibleList = list;
}

void ComboSearchControlPopup::OnSelectedItem(const wxCommandEvent & event) {
    if (wxListView::GetSelectedItemCount() == 1) {
        auto selected = wxListView::GetFirstSelected();
        if (selected >= 0 && selected < visibleList.size()) {
            auto wxstr = visibleList[selected];
            GetComboCtrl()->SetValue(wxstr);
            Dismiss();
            wxCommandEvent e{wxEVT_COMBOBOX, GetId()};
            GetComboCtrl()->GetEventHandler()->ProcessEvent(e);
        }
    }
}

ComboSearchControl::ComboSearchControl(wxWindow *parent, wxWindowID id, const wxString &itemNameLabel) : wxComboCtrl(parent, id), listProvider(std::make_shared<ComboSearchControlPlainListProvider>()) {
    Init(itemNameLabel);
}
ComboSearchControl::ComboSearchControl(wxWindow *parent, wxWindowID id, const wxString &itemNameLabel, const std::shared_ptr<ComboSearchControlListProvider> &listProvider) : wxComboCtrl(parent, id), listProvider(listProvider) {
    Init(itemNameLabel);
}

void ComboSearchControl::Init(const wxString &itemNameLabel) {
    SetPopupControl(new ComboSearchControlPopup(listProvider, itemNameLabel));
}

void ComboSearchControl::Clear() {
    listProvider->Clear();
    SetValue(wxT(""));
#pragma clang diagnostic push
#pragma ide diagnostic ignored "MemoryLeak"
    auto *e = new wxCommandEvent(wxEVT_COMBOBOX, GetId());
#pragma clang diagnostic pop
    GetEventHandler()->QueueEvent(e);
}

void ComboSearchControl::Append(const wxString &str) {
    listProvider->Append(str);
}

void ComboSearchControl::SetSelection(int64_t selection) {
}

int64_t ComboSearchControl::GetSelection() const {
    return listProvider->GetIndexOf(GetValue());
}
