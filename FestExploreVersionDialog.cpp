//
// Created by sigsegv on 5/16/24.
//

#include "FestExploreVersionDialog.h"
#include <wx/listctrl.h>
#include "FestDb.h"
#include <medfest/Struct/Decoded/OppfRefusjon.h>
#include <medfest/Struct/Decoded/OppfLegemiddelMerkevare.h>
#include <medfest/Struct/Decoded/OppfLegemiddelVirkestoff.h>
#include <medfest/Struct/Decoded/OppfLegemiddelpakning.h>
#include <medfest/Struct/Decoded/OppfLegemiddeldose.h>
#include <vector>
#include <tuple>
#include <sstream>

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
        "LegemiddelMerkevare",
        "LegemiddelVirkestoff",
        "Legemiddelpakning",
        "Legemiddeldose",
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

class FestExploreLegemiddelCoreItem : public FestExploreItem, protected LegemiddelCore, protected Oppf {
public:
    FestExploreLegemiddelCoreItem(const class Oppf &oppf, const LegemiddelCore &legemiddelCore) : LegemiddelCore(legemiddelCore), Oppf(oppf) {}
    virtual std::vector<std::tuple<std::string,std::string>> GetDetails();
    [[nodiscard]] std::string GetName() const override;
    void Show(wxPanel &panel, wxPanel &topRight) override;
    void Hide(wxPanel &panel, wxPanel &topRight) override;
};

std::vector<std::tuple<std::string, std::string>> FestExploreLegemiddelCoreItem::GetDetails() {
    std::string atc{};
    {
        auto atcCode = LegemiddelCore::GetAtc();
        atc = atcCode.GetValue();
        atc.append(" (");
        atc.append(atcCode.GetDistinguishedName());
        atc.append(")");
    }
    std::string form{};
    {
        auto legemiddelform = LegemiddelCore::GetLegemiddelformKort();
        form = legemiddelform.GetValue();
        form.append(" (");
        form.append(legemiddelform.GetDistinguishedName());
        form.append(")");
    }
    std::string reseptgruppe{};
    {
        auto reseptgruppeCV = LegemiddelCore::GetReseptgruppe();
        reseptgruppe = reseptgruppeCV.GetValue();
        reseptgruppe.append(" (");
        reseptgruppe.append(reseptgruppeCV.GetDistinguishedName());
        reseptgruppe.append(")");
    }
    std::vector<std::tuple<std::string, std::string>> details = {
            {"Navn form styrke", LegemiddelCore::GetNavnFormStyrke()},
            {"ATC", atc},
            {"Legemiddelform", form},
            {"Reseptgruppe", reseptgruppe},
    };
    {
        auto opioidsoknad = LegemiddelCore::GetOpioidsoknad();
        if (opioidsoknad == MaybeBoolean::MTRUE) {
            std::tuple<std::string,std::string> op = {"Opioidsøknad", "Yes"};
            details.emplace_back(op);
        } else if (opioidsoknad == MaybeBoolean::MFALSE) {
            std::tuple<std::string,std::string> op = {"Opioidsøknad", "No"};
            details.emplace_back(op);
        }
    }
    {
        auto svartTrekant = LegemiddelCore::GetSvartTrekant();
        auto str = svartTrekant.GetValue();
        if (!str.empty()) {
            str.append(" (");
            str.append(svartTrekant.GetDistinguishedName());
            str.append(")");
            std::tuple<std::string,std::string> tupl = {"Svart trekant", str};
            details.emplace_back(tupl);
        }
    }
    {
        auto typeSoknadSlv = LegemiddelCore::GetTypeSoknadSlv();
        auto str = typeSoknadSlv.GetValue();
        if (!str.empty()) {
            str.append(" (");
            str.append(typeSoknadSlv.GetDistinguishedName());
            str.append(")");
            std::tuple<std::string,std::string> tupl = {"Type søknad", str};
            details.emplace_back(tupl);
        }
    }
    {
        auto refs = LegemiddelCore::GetRefVilkar();
        auto iterator = refs.begin();
        if (iterator != refs.end()) {
            std::tuple<std::string,std::string> tupl = {"Ref vilkår", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
        while (iterator != refs.end()) {
            std::tuple<std::string,std::string> tupl = {"", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
    }
    return details;
}

std::string FestExploreLegemiddelCoreItem::GetName() const {
    return LegemiddelCore::GetNavnFormStyrke();
}

void FestExploreLegemiddelCoreItem::Show(wxPanel &panel, wxPanel &topRight) {
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
        auto detailsVec = GetDetails();
        for (const auto &detail : detailsVec) {
            auto field = std::get<0>(detail);
            auto value = std::get<1>(detail);
            auto row = rowNum++;
            details->InsertItem(row, wxString::FromUTF8(field));
            details->SetItem(row, 1, wxString::FromUTF8(value));
        }
    }
    sizer->Add(details, 3, wxALL | wxEXPAND, 5);
    panel.SetSizerAndFit(sizer);
}

void FestExploreLegemiddelCoreItem::Hide(wxPanel &panel, wxPanel &topRight) {
    panel.SetSizerAndFit(nullptr);
}

class FestExploreLegemiddelItem : public FestExploreLegemiddelCoreItem, protected Legemiddel {
public:
    FestExploreLegemiddelItem(const class Oppf &oppf, const Legemiddel &legemiddel) : FestExploreLegemiddelCoreItem(oppf, legemiddel), Legemiddel(legemiddel) {}
    std::vector<std::tuple<std::string, std::string>> GetDetails() override;
};

std::vector<std::tuple<std::string, std::string>> FestExploreLegemiddelItem::GetDetails() {
    auto details = FestExploreLegemiddelCoreItem::GetDetails();
    {
        auto sortertVirkestoffMedStyrkes = Legemiddel::GetSortertVirkestoffMedStyrke();
        auto iterator = sortertVirkestoffMedStyrkes.begin();
        if (iterator != sortertVirkestoffMedStyrkes.end()) {
            std::tuple<std::string,std::string> tupl = {"Virkestoff med styrke", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
        while (iterator != sortertVirkestoffMedStyrkes.end()) {
            std::tuple<std::string,std::string> tupl = {"", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
    }
    {
        auto administreringLegemiddel = Legemiddel::GetAdministreringLegemiddel();
        {
            auto administrasjonsvei = administreringLegemiddel.GetAdministrasjonsvei();
            std::string str{administrasjonsvei.GetValue()};
            if (!str.empty()) {
                str.append(" (");
                str.append(administrasjonsvei.GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"Administrasjonsvei", str};
                details.emplace_back(tupl);
            }
        }
        {
            auto blandingsveske = administreringLegemiddel.GetBlandingsveske();
            if (blandingsveske == MaybeBoolean::MTRUE) {
                std::tuple<std::string,std::string> tupl = {"Blandingsvæske", "Yes"};
                details.emplace_back(tupl);
            } else {
                std::tuple<std::string,std::string> tupl = {"Blandingsvæske", "No"};
                details.emplace_back(tupl);
            }
        }
        {
            auto bolus = administreringLegemiddel.GetBolus();
            std::string str{bolus.GetValue()};
            if (!str.empty()) {
                str.append(" (");
                str.append(bolus.GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"Bolus", str};
                details.emplace_back(tupl);
            }
        }
        {
            auto bruksomrader = administreringLegemiddel.GetBruksomradeEtikett();
            auto iterator = bruksomrader.begin();
            if (iterator != bruksomrader.end()) {
                std::string str{iterator->GetValue()};
                str.append(" (");
                str.append(iterator->GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"Bruksområde", str};
                details.emplace_back(tupl);
                ++iterator;
            }
            while (iterator != bruksomrader.end()) {
                std::string str{iterator->GetValue()};
                str.append(" (");
                str.append(iterator->GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"", str};
                details.emplace_back(tupl);
                ++iterator;
            }
        }
        {
            auto deling = administreringLegemiddel.GetDeling();
            std::string str{deling.GetValue()};
            if (!str.empty()) {
                str.append(" (");
                str.append(deling.GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"Deling", str};
                details.emplace_back(tupl);
            }
        }
        {
            auto enhetDosering = administreringLegemiddel.GetEnhetDosering();
            std::string str{enhetDosering.GetValue()};
            if (!str.empty()) {
                str.append(" (");
                str.append(enhetDosering.GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"Enhet dosering", str};
                details.emplace_back(tupl);
            }
        }
        {
            auto forholdsregler = administreringLegemiddel.GetForhandsregelInntak();
            auto iterator = forholdsregler.begin();
            if (iterator != forholdsregler.end()) {
                std::string str{iterator->GetValue()};
                str.append(" (");
                str.append(iterator->GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"Forholdsregel", str};
                details.emplace_back(tupl);
                ++iterator;
            }
            while (iterator != forholdsregler.end()) {
                std::string str{iterator->GetValue()};
                str.append(" (");
                str.append(iterator->GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"", str};
                details.emplace_back(tupl);
                ++iterator;
            }
        }
        {
            auto injeksjonshastighet = administreringLegemiddel.GetInjeksjonshastighetBolus();
            std::string str{injeksjonshastighet.GetValue()};
            if (!str.empty()) {
                str.append(" (");
                str.append(injeksjonshastighet.GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"Injeksjonshastighet", str};
                details.emplace_back(tupl);
            }
        }
        {
            auto kanApnes = administreringLegemiddel.GetKanApnes();
            std::string str{kanApnes.GetValue()};
            if (!str.empty()) {
                str.append(" (");
                str.append(kanApnes.GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"Kan åpnes", str};
                details.emplace_back(tupl);
            }
        }
        {
            auto kanKnuses = administreringLegemiddel.GetKanKnuses();
            std::string str{kanKnuses.GetValue()};
            if (!str.empty()) {
                str.append(" (");
                str.append(kanKnuses.GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"Kan knuses", str};
                details.emplace_back(tupl);
            }
        }
        {
            auto kortdoser = administreringLegemiddel.GetKortdose();
            auto iterator = kortdoser.begin();
            if (iterator != kortdoser.end()) {
                std::string str{iterator->GetValue()};
                str.append(" (");
                str.append(iterator->GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"Kortdose", str};
                details.emplace_back(tupl);
                ++iterator;
            }
            while (iterator != kortdoser.end()) {
                std::string str{iterator->GetValue()};
                str.append(" (");
                str.append(iterator->GetDistinguishedName());
                str.append(")");
                std::tuple<std::string,std::string> tupl = {"", str};
                details.emplace_back(tupl);
                ++iterator;
            }
        }
    }
    return details;
}

class FestExploreLegemiddelMerkevareItem : public FestExploreLegemiddelItem, protected LegemiddelMerkevare {
public:
    FestExploreLegemiddelMerkevareItem(const class Oppf &oppf, const LegemiddelMerkevare &legemiddelMerkevare) : FestExploreLegemiddelItem(oppf, legemiddelMerkevare), LegemiddelMerkevare(legemiddelMerkevare) {}
    std::vector<std::tuple<std::string, std::string>> GetDetails() override;
};

std::vector<std::tuple<std::string, std::string>> FestExploreLegemiddelMerkevareItem::GetDetails() {
    std::vector<std::tuple<std::string,std::string>> details = {
            {"Merkevare ID", LegemiddelMerkevare::GetId()}
    };
    {
        auto upstreamDetails = FestExploreLegemiddelItem::GetDetails();
        for (const auto &tupl : upstreamDetails) {
            details.emplace_back(tupl);
        }
    }
    {
        auto preparattype = LegemiddelMerkevare::GetPreparattype();
        std::string str{preparattype.GetValue()};
        str.append(" (");
        str.append(preparattype.GetDistinguishedName());
        str.append(")");
        std::tuple<std::string,std::string> tupl = {"Preparattype", str};
        details.emplace_back(tupl);
    }
    {
        std::tuple<std::string,std::string> tupl = {"Varenavn", LegemiddelMerkevare::GetVarenavn()};
        details.emplace_back(tupl);
    }
    {
        std::tuple<std::string,std::string> tupl = {"Legemiddelform", LegemiddelMerkevare::GetLegemiddelformLang()};
        details.emplace_back(tupl);
    }
    {
        std::tuple<std::string,std::string> tupl = {"Produsent", LegemiddelMerkevare::GetLegemiddelformLang()};
        details.emplace_back(tupl);
    }
    {
        auto reseptgyldighet = LegemiddelMerkevare::GetReseptgyldighet();
        auto iterator = reseptgyldighet.begin();
        if (iterator != reseptgyldighet.end()) {
            std::string str{iterator->GetVarighet()};
            str.append(" (");
            auto kjonn = iterator->GetKjonn();
            str.append(kjonn.GetValue());
            str.append(" ");
            str.append(kjonn.GetDistinguishedName());
            str.append(")");
            std::tuple<std::string,std::string> tupl = {"Reseptgyldighet", str};
            details.emplace_back(tupl);
            ++iterator;
        }
        while (iterator != reseptgyldighet.end()) {
            std::string str{iterator->GetVarighet()};
            str.append(" (");
            auto kjonn = iterator->GetKjonn();
            str.append(kjonn.GetValue());
            str.append(" ");
            str.append(kjonn.GetDistinguishedName());
            str.append(")");
            std::tuple<std::string,std::string> tupl = {"", str};
            details.emplace_back(tupl);
            ++iterator;
        }
    }
    {
        auto varseltrekant = LegemiddelMerkevare::GetVarseltrekant();
        std::string str{};
        if (varseltrekant == MaybeBoolean::MTRUE) {
            str = "Yes";
        } else if (varseltrekant == MaybeBoolean::MFALSE) {
            str = "No";
        }
        if (!str.empty()) {
            std::tuple<std::string, std::string> tupl = {"Varseltrekant", str};
            details.emplace_back(tupl);
        }
    }
    {
        auto referanseprodukt = LegemiddelMerkevare::GetReferanseprodukt();
        if (!referanseprodukt.empty()) {
            std::tuple<std::string, std::string> tupl = {"Referanseprodukt", referanseprodukt};
            details.emplace_back(tupl);
        }
    }
    {
        auto preparatomtaleavsnitt = LegemiddelMerkevare::GetPreparatomtaleavsnitt();
        auto lenke = preparatomtaleavsnitt.GetLenke();
        auto www = lenke.GetWww();
        if (!www.empty()) {
            auto overskrift = preparatomtaleavsnitt.GetAvsnittoverskrift();
            {
                std::string str{overskrift.GetValue()};
                str.append(" (");
                str.append(overskrift.GetDistinguishedName());
                str.append(")");
                std::tuple<std::string, std::string> tupl = {"Preparatomtaleavsnitt", str};
                details.emplace_back(tupl);
            }
            {
                std::tuple<std::string, std::string> tupl = {"", www};
                details.emplace_back(tupl);
            }
            {
                std::tuple<std::string, std::string> tupl = {"", lenke.GetBeskrivelse()};
                details.emplace_back(tupl);
            }
        }
    }
    {
        auto smak = LegemiddelMerkevare::GetSmak();
        std::string str{smak.GetValue()};
        if (!str.empty()) {
            str.append(" (");
            str.append(smak.GetDistinguishedName());
            str.append(")");
            std::tuple<std::string, std::string> tupl = {"Smak", str};
            details.emplace_back(tupl);
        }
    }
    {
        auto list = LegemiddelMerkevare::GetSortertVirkestoffUtenStyrke();
        auto iterator = list.begin();
        if (iterator != list.end()) {
            std::tuple<std::string,std::string> tupl = {"Virkestoff uten styrke", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
        while (iterator != list.end()) {
            std::tuple<std::string,std::string> tupl = {"", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
    }
    {
        auto vaksinestandard = LegemiddelMerkevare::GetVaksinestandard();
        std::string str{vaksinestandard.GetValue()};
        if (!str.empty()) {
            str.append(" (");
            str.append(vaksinestandard.GetDistinguishedName());
            str.append(")");
            std::tuple<std::string, std::string> tupl = {"Vaksinestandard", str};
            details.emplace_back(tupl);
        }
    }
    return details;
}

class FestExploreOppfMerkevareItem : public FestExploreLegemiddelMerkevareItem {
public:
    explicit FestExploreOppfMerkevareItem(const OppfLegemiddelMerkevare &oppfLegemiddelMerkevare) : FestExploreLegemiddelMerkevareItem(oppfLegemiddelMerkevare, oppfLegemiddelMerkevare.GetLegemiddelMerkevare()) {}
};

class FestExploreLegemiddelVirkestoffItem : public FestExploreLegemiddelItem, protected LegemiddelVirkestoff {
public:
    FestExploreLegemiddelVirkestoffItem(const class Oppf &oppf, const LegemiddelVirkestoff &legemiddelVirkestoff) : FestExploreLegemiddelItem(oppf, legemiddelVirkestoff), LegemiddelVirkestoff(legemiddelVirkestoff) {}
    std::vector<std::tuple<std::string, std::string>> GetDetails() override;
};

std::vector<std::tuple<std::string, std::string>> FestExploreLegemiddelVirkestoffItem::GetDetails() {
    std::vector<std::tuple<std::string,std::string>> details = {
            {"Legemiddelvirkestoff", LegemiddelVirkestoff::GetId()}
    };
    {
        auto upstreamDetails = FestExploreLegemiddelItem::GetDetails();
        for (const auto &tupl : upstreamDetails) {
            details.emplace_back(tupl);
        }
    }
    {
        auto refs = LegemiddelVirkestoff::GetRefLegemiddelMerkevare();
        auto iterator = refs.begin();
        if (iterator != refs.end()) {
            std::tuple<std::string,std::string> tupl = {"Ref Merkevare", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
        while (iterator != refs.end()) {
            std::tuple<std::string,std::string> tupl = {"", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
    }
    {
        auto refs = LegemiddelVirkestoff::GetRefPakning();
        auto iterator = refs.begin();
        if (iterator != refs.end()) {
            std::tuple<std::string,std::string> tupl = {"Ref Pakning", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
        while (iterator != refs.end()) {
            std::tuple<std::string,std::string> tupl = {"", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
    }
    {
        auto enhet = LegemiddelVirkestoff::GetForskrivningsenhetResept();
        std::string str{enhet.GetValue()};
        if (!str.empty()) {
            str.append(" (");
            str.append(enhet.GetDistinguishedName());
            str.append(")");
            std::tuple<std::string,std::string> tupl = {"Forskrivningsenhet", str};
            details.emplace_back(tupl);
        }
    }
    return details;
}

class FestExploreOppfVirkestoffItem : public FestExploreLegemiddelVirkestoffItem {
public:
    explicit FestExploreOppfVirkestoffItem(const OppfLegemiddelVirkestoff &oppfLegemiddelVirkestoff) : FestExploreLegemiddelVirkestoffItem(oppfLegemiddelVirkestoff, oppfLegemiddelVirkestoff.GetLegemiddelVirkestoff()) {}
};

class FestExploreLegemiddelpakningItem : public FestExploreLegemiddelCoreItem, protected Legemiddelpakning {
public:
    FestExploreLegemiddelpakningItem(const class Oppf &oppf, const Legemiddelpakning &legemiddelpakning) : FestExploreLegemiddelCoreItem(oppf, legemiddelpakning), Legemiddelpakning(legemiddelpakning) {}
    std::vector<std::tuple<std::string, std::string>> GetDetails() override;
};

std::vector<std::tuple<std::string, std::string>> FestExploreLegemiddelpakningItem::GetDetails() {
    std::vector<std::tuple<std::string,std::string>> details = {
            {"Legemiddelpakning", Legemiddelpakning::GetId()}
    };
    {
        auto upstreamDetails = FestExploreLegemiddelCoreItem::GetDetails();
        for (const auto &tupl : upstreamDetails) {
            details.emplace_back(tupl);
        }
    }
    {
        auto preparattype = Legemiddelpakning::GetPreparattype();
        std::string str{preparattype.GetValue()};
        str.append(" (");
        str.append(preparattype.GetDistinguishedName());
        str.append(")");
        std::tuple<std::string,std::string> tupl = {"Preparattype", str};
        details.emplace_back(tupl);
    }
    {
        std::tuple<std::string,std::string> tupl = {"Varenummer", Legemiddelpakning::GetVarenr()};
        details.emplace_back(tupl);
    }
    {
        auto oppbevaring = Legemiddelpakning::GetOppbevaring();
        std::string str{oppbevaring.GetValue()};
        if (!str.empty()) {
            str.append(" (");
            str.append(oppbevaring.GetDistinguishedName());
            str.append(")");
            std::tuple<std::string, std::string> tupl = {"Oppbevaring", str};
            details.emplace_back(tupl);
        }
    }
    {
        auto markedsforingsinfo = Legemiddelpakning::GetMarkedsforingsinfo();
        auto markedsforingsdato = markedsforingsinfo.GetMarkedsforingsdato();
        if (!markedsforingsdato.empty()) {
            std::tuple<std::string, std::string> tupl = {"Markedsføringsdato", markedsforingsdato};
            details.emplace_back(tupl);
        }
        auto midlUtgatt = markedsforingsinfo.GetMidlUtgattDato();
        if (!midlUtgatt.empty()) {
            std::tuple<std::string, std::string> tupl = {"Midlertidig utgått", midlUtgatt};
            details.emplace_back(tupl);
        }
        auto varenrUtgaende = markedsforingsinfo.GetVarenrUtgaende();
        if (!varenrUtgaende.empty()) {
            std::tuple<std::string, std::string> tupl = {"Varenr utgående", varenrUtgaende};
            details.emplace_back(tupl);
        }
        auto avreg = markedsforingsinfo.GetAvregDato();
        if (!avreg.empty()) {
            std::tuple<std::string, std::string> tupl = {"Avreg dato", avreg};
            details.emplace_back(tupl);
        }
    }
    {
        std::tuple<std::string,std::string> tupl = {"Ean", Legemiddelpakning::GetEan()};
        details.emplace_back(tupl);
    }
    {
        auto pakningByttegruppe = Legemiddelpakning::GetPakningByttegruppe();
        auto byttegruppe = pakningByttegruppe.GetRefByttegruppe();
        if (!byttegruppe.empty()) {
            {
                std::tuple<std::string, std::string> tupl = {"Byttegruppe", byttegruppe};
                details.emplace_back(tupl);
            }
            auto gyldigFra = pakningByttegruppe.GetGyldigFraDato();
            if (!gyldigFra.empty()) {
                std::tuple<std::string,std::string> tupl = {"Gyldig fra", gyldigFra};
                details.emplace_back(tupl);
            }
        }
    }
    {
        auto preparatomtaleavsnitt = Legemiddelpakning::GetPreparatomtaleavsnitt();
        auto lenke = preparatomtaleavsnitt.GetLenke();
        auto www = lenke.GetWww();
        if (!www.empty()) {
            auto overskrift = preparatomtaleavsnitt.GetAvsnittoverskrift();
            {
                std::string str{overskrift.GetValue()};
                str.append(" (");
                str.append(overskrift.GetDistinguishedName());
                str.append(")");
                std::tuple<std::string, std::string> tupl = {"Preparatomtaleavsnitt", str};
                details.emplace_back(tupl);
            }
            {
                std::tuple<std::string, std::string> tupl = {"", www};
                details.emplace_back(tupl);
            }
            {
                std::tuple<std::string, std::string> tupl = {"", lenke.GetBeskrivelse()};
                details.emplace_back(tupl);
            }
        }
    }
    {
        auto ikkeKonservering = Legemiddelpakning::GetIkkeKonservering();
        std::tuple<std::string,std::string> tupl = {"Ikke konservering", ikkeKonservering ? "Yes" : "No"};
        details.emplace_back(tupl);
    }
    return details;
}

class FestExploreOppfLegemiddelpakningItem : public FestExploreLegemiddelpakningItem {
public:
    explicit FestExploreOppfLegemiddelpakningItem(const OppfLegemiddelpakning &oppfLegemiddelpakning) : FestExploreLegemiddelpakningItem(oppfLegemiddelpakning, oppfLegemiddelpakning.GetLegemiddelpakning()) {}
};

class FestExploreLegemiddeldoseItem : public FestExploreLegemiddelCoreItem, protected Legemiddeldose {
public:
    FestExploreLegemiddeldoseItem(const class Oppf &oppf, const Legemiddeldose &legemiddeldose) : FestExploreLegemiddelCoreItem(oppf, legemiddeldose), Legemiddeldose(legemiddeldose) {}
    std::vector<std::tuple<std::string, std::string>> GetDetails() override;
};

std::vector<std::tuple<std::string, std::string>> FestExploreLegemiddeldoseItem::GetDetails() {
    std::vector<std::tuple<std::string,std::string>> details = {
            {"Legemiddeldose", Legemiddeldose::GetId()}
    };
    {
        auto upstreamDetails = FestExploreLegemiddelCoreItem::GetDetails();
        for (const auto &tupl : upstreamDetails) {
            details.emplace_back(tupl);
        }
    }
    {
        auto preparattype = Legemiddeldose::GetPreparattype();
        std::string str{preparattype.GetValue()};
        str.append(" (");
        str.append(preparattype.GetDistinguishedName());
        str.append(")");
        std::tuple<std::string,std::string> tupl = {"Preparattype", str};
        details.emplace_back(tupl);
    }
    {
        std::tuple<std::string,std::string> tupl = {"Lmr løpenummer", Legemiddeldose::GetLmrLopenr()};
        details.emplace_back(tupl);
    }
    {
        auto amount = Legemiddeldose::GetMengde();
        std::stringstream str{};
        str << amount.GetValue() << " " << amount.GetUnit();
        std::tuple<std::string,std::string> tupl = {"Mengde", str.str()};
        details.emplace_back(tupl);
    }
    {
        auto refs = Legemiddeldose::GetRefLegemiddelMerkevare();
        auto iterator = refs.begin();
        if (iterator != refs.end()) {
            std::tuple<std::string,std::string> tupl = {"Ref Merkevare", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
        while (iterator != refs.end()) {
            std::tuple<std::string,std::string> tupl = {"", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
    }
    {
        auto refs = Legemiddeldose::GetRefPakning();
        auto iterator = refs.begin();
        if (iterator != refs.end()) {
            std::tuple<std::string,std::string> tupl = {"Ref Pakning", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
        while (iterator != refs.end()) {
            std::tuple<std::string,std::string> tupl = {"", *iterator};
            details.emplace_back(tupl);
            ++iterator;
        }
    }
    {
        auto pakningstype = Legemiddeldose::GetPakningstype();
        std::string str{pakningstype.GetValue()};
        if (!str.empty()) {
            str.append(" (");
            str.append(pakningstype.GetDistinguishedName());
            str.append(")");
            std::tuple<std::string,std::string> tupl = {"Pakningstype", str};
            details.emplace_back(tupl);
        }
    }
    return details;
}

class FestExploreOppfLegemiddeldoseItem : public FestExploreLegemiddeldoseItem {
public:
    explicit FestExploreOppfLegemiddeldoseItem(const OppfLegemiddeldose &oppfLegemiddeldose) : FestExploreLegemiddeldoseItem(oppfLegemiddeldose, oppfLegemiddeldose.GetLegemiddeldose()) {}
};

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
    } else if (itemType == "LegemiddelMerkevare") {
        auto oppfLegemiddelMerkevares = db->GetOppfLegemiddelMerkevare(version);
        for (const auto &oppfLegemiddelMerkevare : oppfLegemiddelMerkevares) {
            items.emplace_back(std::make_shared<FestExploreOppfMerkevareItem>(oppfLegemiddelMerkevare));
        }
    } else if (itemType == "LegemiddelVirkestoff") {
        auto oppfLegemiddelVirkestoffs = db->GetOppfLegemiddelVirkestoff(version);
        for (const auto &oppfLegemiddelVirkestoff : oppfLegemiddelVirkestoffs) {
            items.emplace_back(std::make_shared<FestExploreOppfVirkestoffItem>(oppfLegemiddelVirkestoff));
        }
    } else if (itemType == "Legemiddelpakning") {
        auto oppfLegemiddelpaknings = db->GetOppfLegemiddelpakning(version);
        for (const auto &oppfLegemiddelpakning : oppfLegemiddelpaknings) {
            items.emplace_back(std::make_shared<FestExploreOppfLegemiddelpakningItem>(oppfLegemiddelpakning));
        }
    } else if (itemType == "Legemiddeldose") {
        auto oppfLegemiddeldoses = db->GetOppfLegemiddeldose(version);
        for (const auto &oppfLegemiddeldose : oppfLegemiddeldoses) {
            items.emplace_back(std::make_shared<FestExploreOppfLegemiddeldoseItem>(oppfLegemiddeldose));
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
