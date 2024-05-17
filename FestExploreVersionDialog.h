//
// Created by sigsegv on 5/16/24.
//

#ifndef DRWHATSNOT_EXPLOREVERSIONDIALOG_H
#define DRWHATSNOT_EXPLOREVERSIONDIALOG_H

#include <memory>
#include <string>
#include <wx/wx.h>

class FestDb;
class wxListView;

class FestExploreItem {
public:
    virtual ~FestExploreItem() = default;
    virtual std::string GetName() const;
    virtual bool FilterBy(const std::string &filer) const;
    virtual void Show(wxPanel &panel, wxPanel &topRight);
    virtual void Hide(wxPanel &panel, wxPanel &topRight);
};

class FestExploreVersionDialog : public wxDialog {
private:
    std::shared_ptr<FestDb> db;
    std::string version;
    std::vector<std::string> itemFilters{};
    std::string itemFilter{};
    std::vector<std::shared_ptr<FestExploreItem>> items;
    std::shared_ptr<FestExploreItem> shownItem{};
    wxListView *itemTypeSelector;
    wxComboBox *itemFilterSelector;
    wxListView *itemListView;
    wxPanel *topRight;
    wxPanel *bottom;
public:
    FestExploreVersionDialog(wxWindow *parent, const std::shared_ptr<FestDb> &, const std::string &);
    void ClearItems();
    void UpdateFilters(const std::string &itemType);
    void ShowItemsWithFilter(const std::string &itemType);
    void ShowItems(const std::string &itemType);
    std::string GetSelectedItemType() const;
    void OnItemTypeSelection(wxCommandEvent &);
    void OnItemFilterSelection(wxCommandEvent &);
    void OnItemSelection(wxCommandEvent &e);
};


#endif //DRWHATSNOT_EXPLOREVERSIONDIALOG_H
