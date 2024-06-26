//
// Created by sigsegv on 5/16/24.
//

#ifndef DRWHATSNOT_EXPLOREVERSIONDIALOG_H
#define DRWHATSNOT_EXPLOREVERSIONDIALOG_H

#include <memory>
#include <string>
#include <wx/wx.h>

class FestDb;
class OppfRefusjon;
class OppfLegemiddelMerkevare;
class OppfLegemiddelVirkestoff;
class OppfLegemiddelpakning;
class OppfLegemiddeldose;
class OppfKodeverk;
class Element;
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
    std::shared_ptr<std::vector<OppfRefusjon>> refusjon{};
    std::shared_ptr<std::vector<OppfLegemiddelMerkevare>> legemiddelMerkevare{};
    std::shared_ptr<std::vector<OppfLegemiddelVirkestoff>> legemiddelVirkestoff{};
    std::shared_ptr<std::vector<OppfLegemiddelpakning>> legemiddelpakning{};
    std::shared_ptr<std::vector<OppfLegemiddeldose>> legemiddeldose{};
    std::shared_ptr<std::vector<OppfKodeverk>> kodeverk{};
    std::shared_ptr<std::vector<Element>> atc{};
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
    FestExploreVersionDialog(wxWindow *parent, const std::shared_ptr<std::vector<OppfRefusjon>> &refusjon, const std::shared_ptr<std::vector<OppfLegemiddelMerkevare>> &legemiddelMerkevare, const std::shared_ptr<std::vector<OppfLegemiddelVirkestoff>> &legemiddelVirkestoff, const std::shared_ptr<std::vector<OppfLegemiddelpakning>> &legemiddelpakning, const std::shared_ptr<std::vector<OppfLegemiddeldose>> &legemiddeldose, const std::shared_ptr<std::vector<OppfKodeverk>> &kodeverk, const std::shared_ptr<std::vector<Element>> &atc, const std::string &version);
private:
    void Init();
public:
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
