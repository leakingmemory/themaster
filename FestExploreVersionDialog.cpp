//
// Created by sigsegv on 5/16/24.
//

#include "FestExploreVersionDialog.h"
#include <wx/listctrl.h>
#include "FestDb.h"
#include <medfest/Struct/Decoded/OppfRefusjon.h>
#include <vector>
#include <tuple>

std::string FestExploreItem::GetName() const {
    return "No name";
}

bool FestExploreItem::FilterBy(const std::string &filter) const {
    return true;
}

void FestExploreItem::Show(wxPanel &panel, wxPanel &topRight) {
}

void FestExploreItem::Hide(wxPanel &panel, wxPanel &topRight) {
}

template <typename I, typename T> constexpr I LengthOfNullptrTerminated(T * const * const items) {
    I rv{0};
    while (items[rv] != nullptr) {
        ++rv;
    }
    return rv;
}
constexpr const char *itemTypes[] = {
        "Refusjon",
        nullptr
};
constexpr int numberOfItemTypes = LengthOfNullptrTerminated<int,const char>(&(*itemTypes));

FestExploreVersionDialog::FestExploreVersionDialog(wxWindow *parent, const std::shared_ptr<FestDb> &db, const std::string &version) :
    wxDialog(parent, wxID_ANY, wxString::FromUTF8(version)),
    db(db), version(version)
{
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    {
        auto *topSizer = new wxBoxSizer(wxHORIZONTAL);
        itemTypeSelector = new wxListView(this, wxID_ANY);
        itemTypeSelector->AppendColumn(wxT("Item type"));
        itemTypeSelector->SetColumnWidth(0, 150);
        for (std::remove_const<typeof(numberOfItemTypes)>::type pos = 0; pos < numberOfItemTypes; pos++) {
            itemTypeSelector->InsertItem(pos, wxString::FromUTF8(itemTypes[pos]));
        }
        topSizer->Add(itemTypeSelector, 3, wxALL | wxEXPAND, 5);
        auto itemListSizer = new wxBoxSizer(wxVERTICAL);
        itemFilterSelector = new wxComboBox(this, wxID_ANY);
        itemFilterSelector->SetEditable(false);
        itemListSizer->Add(itemFilterSelector, 0, wxALL | wxEXPAND, 5);
        itemListView = new wxListView(this, wxID_ANY);
        itemListView->AppendColumn(wxT("Item"));
        itemListView->SetColumnWidth(0, 200);
        itemListSizer->Add(itemListView, 1, wxALL | wxEXPAND, 5);
        topSizer->Add(itemListSizer, 4, wxALL | wxEXPAND, 5);
        topRight = new wxPanel(this, wxID_ANY);
        topSizer->Add(topRight, 8, wxALL | wxEXPAND, 5);
        itemTypeSelector->Bind(wxEVT_LIST_ITEM_SELECTED, &FestExploreVersionDialog::OnItemTypeSelection, this);
        itemTypeSelector->Bind(wxEVT_LIST_ITEM_DESELECTED, &FestExploreVersionDialog::OnItemTypeSelection, this);
        itemFilterSelector->Bind(wxEVT_COMBOBOX, &FestExploreVersionDialog::OnItemFilterSelection, this);
        itemListView->Bind(wxEVT_LIST_ITEM_SELECTED, &FestExploreVersionDialog::OnItemSelection, this);
        itemListView->Bind(wxEVT_LIST_ITEM_DESELECTED, &FestExploreVersionDialog::OnItemSelection, this);
        sizer->Add(topSizer, 1, wxALL | wxEXPAND, 5);
    }
    {
        bottom = new wxPanel(this, wxID_ANY);
        sizer->Add(bottom, 1, wxALL | wxEXPAND, 5);
    }
    SetSizerAndFit(sizer);
}

void FestExploreVersionDialog::ClearItems() {
    itemListView->ClearAll();
    itemListView->AppendColumn(wxT("Item"));
    itemListView->SetColumnWidth(0, 200);
    items.clear();
}

class RefusjonskodeItem : public Refusjonskode {
public:
    RefusjonskodeItem(const Refusjonskode &refusjonskode) : Refusjonskode(refusjonskode) {
    }
    [[nodiscard]] std::string GetName() const {
        auto refusjonskode = GetRefusjonskode();
        std::string name{refusjonskode.GetValue()};
        name.append(" (");
        name.append(refusjonskode.GetDistinguishedName());
        name.append(")");
        return name;
    }
    [[nodiscard]] std::vector<std::tuple<std::string,std::string>> GetDetails() const {
        auto refusjonskode = Refusjonskode::GetRefusjonskode();
        std::string name{refusjonskode.GetValue()};
        name.append(" (");
        name.append(refusjonskode.GetDistinguishedName());
        name.append(")");
        auto underterm = Refusjonskode::GetUnderterm();
        auto undertermIterator = underterm.begin();
        std::vector<std::tuple<std::string,std::string>> details = {
                {"Name", name},
                {"Gyldig fra", Refusjonskode::GetGyldigFraDato()},
                {"Forskrives til", Refusjonskode::GetForskrivesTilDato()},
                {"Utleveres til", Refusjonskode::GetUtleveresTilDato()},
                {"Underterm", undertermIterator != underterm.end() ? *undertermIterator : ""}
        };
        if (undertermIterator != underterm.end()) {
            ++undertermIterator;
        }
        while (undertermIterator != underterm.end()) {
            std::string field{};
            std::string term{*undertermIterator};
            std::tuple<std::string,std::string> tuple = {field, term};
            details.emplace_back(tuple);
            ++undertermIterator;
        }
        auto refusjonsvilkarRefs = Refusjonskode::GetRefusjonsvilkar();
        auto refusjonsvilkarRefsIterator = refusjonsvilkarRefs.begin();
        if (refusjonsvilkarRefsIterator != refusjonsvilkarRefs.end()) {
            auto str{refusjonsvilkarRefsIterator->GetFraDato()};
            if (!str.empty()) {
                str.append(" ");
            }
            str.append(refusjonsvilkarRefsIterator->GetId());
            std::tuple<std::string,std::string> tuple = {"Refusjonsvilkar", str};
            details.emplace_back(tuple);
            ++refusjonsvilkarRefsIterator;
        }
        while (refusjonsvilkarRefsIterator != refusjonsvilkarRefs.end()) {
            auto str{refusjonsvilkarRefsIterator->GetFraDato()};
            if (!str.empty()) {
                str.append(" ");
            }
            str.append(refusjonsvilkarRefsIterator->GetId());
            std::tuple<std::string,std::string> tuple = {"", str};
            details.emplace_back(tuple);
            ++refusjonsvilkarRefsIterator;
        }
        return details;
    }
};

class FestExploreOppfRefusjonItem : public FestExploreItem, private Refusjonshjemmel, private Refusjonsgruppe, private Oppf {
private:
    std::vector<RefusjonskodeItem> refusjonskoder{};
    wxListView *refusjonskodeListView;
    wxListView *refusjonskodeDetails;
public:
    explicit FestExploreOppfRefusjonItem(const OppfRefusjon &oppfRefusjon) : Refusjonshjemmel(oppfRefusjon.GetRefusjonshjemmel()), Refusjonsgruppe(Refusjonshjemmel::GetRefusjonsgruppe()), Oppf(oppfRefusjon) {
        auto refusjonskoder = Refusjonsgruppe::GetRefusjonskode();
        for (const auto &refusjonskode : refusjonskoder) {
            this->refusjonskoder.emplace_back(refusjonskode);
        }
    }
    [[nodiscard]] std::string GetName() const override;
    static std::vector<std::string> GetFilters() {
        return {"All", "With refusjonskode with forskrives til"};
    }
    bool FilterBy(const std::string &filer) const override;
    void Show(wxPanel &panel, wxPanel &topRight) override;
    void Hide(wxPanel &panel, wxPanel &topRight) override;
    void ClearRefusjonskode();
    void ShowRefusjonskode(const RefusjonskodeItem &);
    void OnRefusjonskodeSelection(wxCommandEvent &);
};

std::string FestExploreOppfRefusjonItem::GetName() const {
    auto refusjonshjemmel = Refusjonshjemmel::GetRefusjonshjemmel();
    std::string name{refusjonshjemmel.GetDistinguishedName()};
    if (name.empty()) {
        name.append(" (");
    } else {
        name.append("(");
    }
    name.append(refusjonshjemmel.GetValue());
    name.append(")");
    return name;
}

bool FestExploreOppfRefusjonItem::FilterBy(const std::string &filter) const {
    if (filter == "With refusjonskode with forskrives til") {
        auto refusjonskoder = Refusjonsgruppe::GetRefusjonskode();
        for (const auto &refusjonskode : refusjonskoder) {
            if (!refusjonskode.GetForskrivesTilDato().empty()) {
                return true;
            }
        }
        return false;
    }
    return true;
}

void FestExploreOppfRefusjonItem::Show(wxPanel &panel, wxPanel &topRight) {
    auto *sizer = new wxBoxSizer(wxHORIZONTAL);
    auto *details = new wxListView(&panel, wxID_ANY);
    details->AppendColumn(wxT("Field"));
    details->SetColumnWidth(0, 133);
    details->AppendColumn(wxT("Value"));
    details->SetColumnWidth(1, 267);
    int rowNum = 0;
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("Oppføring"));
        details->SetItem(row, 1, wxString::FromUTF8(Oppf::GetId()));
    }
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("Tidpunkt"));
        details->SetItem(row, 1, wxString::FromUTF8(Oppf::GetTidspunkt()));
    }
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("Status"));
        auto status = Oppf::GetStatus();
        auto str{status.GetDistinguishedName()};
        str.append(" (");
        str.append(status.GetValue());
        str.append(")");
        details->SetItem(row, 1, wxString::FromUTF8(str));
    }
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("Refusjonshjemmel"));
        auto refusjonshjemmel = Refusjonshjemmel::GetRefusjonshjemmel();
        auto str{refusjonshjemmel.GetDistinguishedName()};
        str.append(" (");
        str.append(refusjonshjemmel.GetValue());
        str.append(")");
        details->SetItem(row, 1, wxString::FromUTF8(str));
    }
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("Krever varekobling"));
        details->SetItem(row, 1, Refusjonshjemmel::GetKreverVarekobling() ? wxT("Yes") : wxT("No"));
    }
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("Krever vedtak"));
        details->SetItem(row, 1, Refusjonshjemmel::GetKreverVedtak() ? wxT("Yes") : wxT("No"));
    }
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("Refusjonsgruppe"));
        details->SetItem(row, 1, wxString::FromUTF8(Refusjonsgruppe::GetId()));
    }
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("ATC"));
        auto atc = Refusjonsgruppe::GetAtc();
        std::string str{atc.GetValue()};
        str.append(" (");
        str.append(atc.GetDistinguishedName());
        str.append(")");
        details->SetItem(row, 1, wxString::FromUTF8(str));
    }
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("Gruppenummer"));
        auto gruppeNr = Refusjonsgruppe::GetGruppeNr();
        std::string str{gruppeNr.GetValue()};
        str.append(" (");
        str.append(gruppeNr.GetDistinguishedName());
        str.append(")");
        details->SetItem(row, 1, wxString::FromUTF8(str));
    }
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("Refusjonsberettiget bruk"));
        details->SetItem(row, 1, wxString::FromUTF8(Refusjonsgruppe::GetRefusjonsberettigetBruk()));
    }
    {
        auto row = rowNum++;
        details->InsertItem(row, wxT("Requires refusjonskode"));
        details->SetItem(row, 1, Refusjonsgruppe::GetKreverRefusjonskode() ? "Yes" : "No");
    }
    sizer->Add(details, 3, wxALL | wxEXPAND, 5);
    auto refVilkars = Refusjonsgruppe::GetRefVilkar();
    auto refVilkarList = new wxListView(&panel, wxID_ANY);
    refVilkarList->AppendColumn(wxT("Ref vilkår"));
    {
        int row = 0;
        for (const auto &refVilkar: refVilkars) {
            refVilkarList->InsertItem(row++, wxString::FromUTF8(refVilkar));
        }
    }
    sizer->Add(refVilkarList, 1, wxALL | wxEXPAND, 5);
    panel.SetSizerAndFit(sizer);
    auto topRightSizer = new wxBoxSizer(wxHORIZONTAL);
    refusjonskodeListView = new wxListView(&topRight, wxID_ANY);
    refusjonskodeListView->AppendColumn(wxT("Refusjonskode"));
    refusjonskodeListView->SetColumnWidth(0, 200);
    {
        int row = 0;
        for (const auto &item: refusjonskoder) {
            refusjonskodeListView->InsertItem(row++, wxString::FromUTF8(item.GetName()));
        }
    }
    refusjonskodeListView->Bind(wxEVT_LIST_ITEM_SELECTED, &FestExploreOppfRefusjonItem::OnRefusjonskodeSelection, this);
    refusjonskodeListView->Bind(wxEVT_LIST_ITEM_DESELECTED, &FestExploreOppfRefusjonItem::OnRefusjonskodeSelection, this);
    topRightSizer->Add(refusjonskodeListView, 1, wxALL | wxEXPAND, 5);
    refusjonskodeDetails = new wxListView(&topRight, wxID_ANY);
    refusjonskodeDetails->AppendColumn(wxT("Field"));
    refusjonskodeDetails->AppendColumn(wxT("Value"));
    topRightSizer->Add(refusjonskodeDetails, 1, wxALL | wxEXPAND, 5);
    topRight.SetSizerAndFit(topRightSizer);
}

void FestExploreOppfRefusjonItem::Hide(wxPanel &panel, wxPanel &topRight) {
    panel.SetSizerAndFit(nullptr);
    topRight.SetSizerAndFit(nullptr);
}

void FestExploreOppfRefusjonItem::ClearRefusjonskode() {
    refusjonskodeDetails->ClearAll();
    refusjonskodeDetails->AppendColumn(wxT("Field"));
    refusjonskodeDetails->AppendColumn(wxT("Value"));
}

void FestExploreOppfRefusjonItem::ShowRefusjonskode(const RefusjonskodeItem &item) {
    auto details = item.GetDetails();
    int rowNum = 0;
    for (const auto &detail : details) {
        auto field = std::get<0>(detail);
        auto value = std::get<1>(detail);
        auto row = rowNum++;
        refusjonskodeDetails->InsertItem(row, wxString::FromUTF8(field));
        refusjonskodeDetails->SetItem(row, 1, wxString::FromUTF8(value));
    }
}

void FestExploreOppfRefusjonItem::OnRefusjonskodeSelection(wxCommandEvent &) {
    auto selection = refusjonskodeListView->GetFirstSelected();
    ClearRefusjonskode();
    if (selection < 0 || selection >= refusjonskoder.size()) {
        return;
    }
    {
        auto nextSelection = refusjonskodeListView->GetNextSelected(selection);
        if (nextSelection >= 0 && nextSelection < refusjonskoder.size()) {
            return;
        }
    }
    ShowRefusjonskode(refusjonskoder[selection]);
}

void FestExploreVersionDialog::UpdateFilters(const std::string &itemType) {
    if (itemType == "Refusjon") {
        itemFilters = FestExploreOppfRefusjonItem::GetFilters();
    } else {
        itemFilters = {"All"};
    }
    itemFilterSelector->Clear();
    for (const auto &filter : itemFilters) {
        itemFilterSelector->Append(wxString::FromUTF8(filter));
    }
}

void FestExploreVersionDialog::ShowItemsWithFilter(const std::string &itemType) {
    if (itemType == "Refusjon") {
        auto oppfRefusjons = db->GetOppfRefusjon(version);
        for (const auto &oppfRefusjon : oppfRefusjons) {
            items.emplace_back(std::make_shared<FestExploreOppfRefusjonItem>(oppfRefusjon));
        }
    }
    auto selection = itemFilterSelector->GetSelection();
    if (selection >= 0 && selection < itemFilters.size()) {
        auto filter = itemFilters[selection];
        auto iterator = items.begin();
        while (iterator != items.end()) {
            if ((*iterator)->FilterBy(filter)) {
                ++iterator;
            } else {
                iterator = items.erase(iterator);
            }
        }
    }
    int row = 0;
    for (const auto &item : items) {
        itemListView->InsertItem(row++, wxString::FromUTF8(item->GetName()));
    }
}

void FestExploreVersionDialog::ShowItems(const std::string &itemType) {
    UpdateFilters(itemType);
    ShowItemsWithFilter(itemType);
}

std::string FestExploreVersionDialog::GetSelectedItemType() const {
    std::string selectedItem{};
    {
        auto selection = itemTypeSelector->GetFirstSelected();
        if (selection < 0 || selection >= numberOfItemTypes) {
            return {};
        }
        {
            auto nextSelection = itemTypeSelector->GetNextSelected(selection);
            if (nextSelection >= 0 && nextSelection < numberOfItemTypes) {
                return {};
            }
        }
        selectedItem = itemTypes[selection];
    }
    return selectedItem;
}

void FestExploreVersionDialog::OnItemTypeSelection(wxCommandEvent &) {
    ClearItems();
    std::string selectedItem = GetSelectedItemType();
    if (selectedItem.empty()) {
        return;
    }
    ShowItems(selectedItem);
}

void FestExploreVersionDialog::OnItemFilterSelection(wxCommandEvent &e) {
    ClearItems();
    std::string selectedItem = GetSelectedItemType();
    if (selectedItem.empty()) {
        return;
    }
    ShowItemsWithFilter(selectedItem);
}

void FestExploreVersionDialog::OnItemSelection(wxCommandEvent &e) {
    if (shownItem) {
        shownItem->Hide(*bottom, *topRight);
        shownItem = {};
    }
    auto selected = itemListView->GetFirstSelected();
    if (selected < 0 || selected >= items.size()) {
        return;
    }
    {
        auto nextSelected = itemListView->GetNextSelected(selected);
        if (nextSelected >= 0 && nextSelected < items.size()) {
            return;
        }
    }
    shownItem = items[selected];
    shownItem->Show(*bottom, *topRight);
}
