//
// Created by sigsegv on 3/6/25.
//

#ifndef THEMASTER_COMBOSEARCHCONTROL_H
#define THEMASTER_COMBOSEARCHCONTROL_H

#include <wx/combo.h>
#include <vector>
#include <memory>

class ComboSearchControlListProvider {
protected:
    std::string autoComplete;
public:
    virtual std::vector<wxString> GetItems() const = 0;
    virtual ssize_t GetIndexOf(const wxString &) const = 0;
    virtual void Clear() = 0;
    virtual void Append(const wxString &) = 0;
    virtual std::vector<wxString> GetVisibleList() const;
    virtual void SetAutoComplete(const std::string &str);
};

class ComboSearchControlPlainListProvider : public ComboSearchControlListProvider {
private:
    std::vector<wxString> items{};
public:
    std::vector<wxString> GetItems() const override;
    ssize_t GetIndexOf(const wxString &) const override;
    void Clear() override;
    void Append(const wxString &) override;
};

class ComboSearchControl : public wxComboCtrl {
private:
    std::shared_ptr<ComboSearchControlListProvider> listProvider;
public:
    ComboSearchControl(wxWindow *parent, wxWindowID id, const wxString &itemNameLabel);
    ComboSearchControl(wxWindow *parent, wxWindowID id, const wxString &itemNameLabel, const std::shared_ptr<ComboSearchControlListProvider> &listProvider);
    void Init(const wxString &itemNameLabel);
    void Clear() override;
    void Append(const wxString &);
    void SetSelection(int64_t selection);
    int64_t GetSelection() const;
};


#endif //THEMASTER_COMBOSEARCHCONTROL_H
