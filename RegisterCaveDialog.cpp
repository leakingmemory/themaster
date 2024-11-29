//
// Created by sigsegv on 11/18/24.
//

#include "RegisterCaveDialog.h"
#include "MedicamentVisitor.h"
#include "MedicalCodedValue.h"
#include "FestDb.h"
#include "DateTime.h"
#include <sfmbasisapi/fhir/value.h>
#include <sfmbasisapi/fhir/allergy.h>
#include <medfest/Struct/Packed/FestUuid.h>
#include <medfest/Struct/Decoded/VirkestoffMedStyrke.h>
#include <medfest/Struct/Decoded/Virkestoff.h>
#include <wx/listctrl.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

class GetCaveCodings : public MedicamentVisitorBase {
private:
    std::shared_ptr<FestDb> festDb;
    std::vector<CaveCoding> codings{};
public:
    GetCaveCodings(const std::shared_ptr<FestDb> &festDb, const LegemiddelCore &legemiddelCore);
    void Substances(std::vector<FestUuid> &substIds);
    void Visit(const LegemiddelMerkevare &);
    void Visit(const LegemiddelVirkestoff &);
    void Visit(const Legemiddelpakning &);
    explicit constexpr operator const std::vector<CaveCoding> &() const {
        return codings;
    }
};

GetCaveCodings::GetCaveCodings(const std::shared_ptr<FestDb> &festDb, const LegemiddelCore &legemiddelCore) : festDb(festDb) {
    MedicamentVisitorBase::Visit<void>(legemiddelCore, *this);
}

std::vector<FestUuid> GetVirkestoffer(const std::shared_ptr<FestDb> &festDb, const Legemiddel &legemiddel) {
    std::vector<FestUuid> substIds{};
    for (const auto &subst : legemiddel.GetSortertVirkestoffMedStyrke()) {
        FestUuid uuid{subst};
        auto substance = festDb->GetVirkestoffMedStyrke(uuid);
        if (substance.GetRefVirkestoff().empty()) {
            continue;
        }
        FestUuid virkestoffId{substance.GetRefVirkestoff()};
        if (std::find(substIds.cbegin(), substIds.cend(), virkestoffId) == substIds.cend()) {
            substIds.emplace_back(virkestoffId);
        }
    }
    return substIds;
}

void GetCaveCodings::Substances(std::vector<FestUuid> &substIds) {
    std::vector<Virkestoff> virkestoffer{};
    auto iterator = substIds.begin();
    while (iterator != substIds.end()) {
        auto virkestoffId = *iterator;
        ++iterator;
        auto virkestoff = festDb->GetVirkestoff(virkestoffId);
        virkestoffer.emplace_back(virkestoff);
        for (const auto &substId : virkestoff.GetRefVirkestoff()) {
            FestUuid virkestoffId{substId};
            if (std::find(substIds.cbegin(), substIds.cend(), virkestoffId) == substIds.cend()) {
                iterator = substIds.insert(iterator, virkestoffId);
            }
        }
    }
    for (const auto &virkestoff : virkestoffer) {
        FhirCoding coding{"http://nhn.no/kj/fhir/CodeSystem/ActiveSubstance", virkestoff.GetId(), virkestoff.GetNavn()};
        codings.emplace_back(coding, virkestoff.GetNavn(), "Substance");
    }
}

void GetCaveCodings::Visit(const LegemiddelMerkevare &merkevare) {
    {
        FhirCoding coding{"http://nhn.no/kj/fhir/CodeSystem/DrugTradeName", merkevare.GetId(), merkevare.GetVarenavn()};
        codings.emplace_back(coding, merkevare.GetNavnFormStyrke(), "Drug trade name");
    }
    {
        auto atc = merkevare.GetAtc();
        FhirCoding coding{"http://nhn.no/kj/fhir/CodeSystem/ATC", atc.GetValue(), atc.GetDistinguishedName()};
        codings.emplace_back(coding, atc.GetDistinguishedName(), "ATC");
    }
    std::vector<FestUuid> substIds = GetVirkestoffer(festDb, static_cast<const Legemiddel &>(merkevare));
    for (const auto &subst : merkevare.GetSortertVirkestoffUtenStyrke()) {
        FestUuid virkestoffId{subst};
        if (std::find(substIds.cbegin(), substIds.cend(), virkestoffId) == substIds.cend()) {
            substIds.emplace_back(virkestoffId);
        }
    }
    Substances(substIds);
}

void GetCaveCodings::Visit(const LegemiddelVirkestoff &legemiddelVirkestoff) {
    {
        auto atc = legemiddelVirkestoff.GetAtc();
        FhirCoding coding{"http://nhn.no/kj/fhir/CodeSystem/ATC", atc.GetValue(), atc.GetDistinguishedName()};
        codings.emplace_back(coding, atc.GetDistinguishedName(), "ATC");
    }
    std::vector<FestUuid> substIds = GetVirkestoffer(festDb, static_cast<const Legemiddel &>(legemiddelVirkestoff));
    Substances(substIds);
}

void GetCaveCodings::Visit(const Legemiddelpakning &pakning) {
    std::vector<Atc> atcs{};
    {
        auto atc = pakning.GetAtc();
        if (!atc.GetValue().empty()) {
            atcs.emplace_back(atc);
        }
    }
    std::vector<FestUuid> merkevareIds{};
    std::vector<FestUuid> substIds{};
    for (const auto &pi : pakning.GetPakningsinfo()) {
        auto merkevareId = pi.GetMerkevareId();
        if (merkevareId.empty()) {
            continue;
        }
        FestUuid merkevareUuid{merkevareId};
        auto merkevare = festDb->GetLegemiddelMerkevare(merkevareUuid);
        if (std::find(merkevareIds.cbegin(), merkevareIds.cend(), merkevareUuid) != merkevareIds.cend()) {
            continue;
        }
        merkevareIds.emplace_back(merkevareUuid);
        auto atc = merkevare.GetAtc();
        if (!atc.GetValue().empty() && std::find_if(atcs.cbegin(), atcs.cend(), [atc] (const Atc &a) { return atc.GetValue() == a.GetValue(); }) == atcs.cend()) {
            atcs.emplace_back(atc);
        }
        {
            FhirCoding coding{"http://nhn.no/kj/fhir/CodeSystem/DrugTradeName", merkevare.GetId(), merkevare.GetVarenavn()};
            codings.emplace_back(coding, merkevare.GetNavnFormStyrke(), "Drug trade name");
        }
        {
            auto merkevareSubst = GetVirkestoffer(festDb, static_cast<const Legemiddel &>(merkevare));
            for (const auto &subst : merkevareSubst) {
                if (std::find(substIds.cbegin(), substIds.cend(), subst) == substIds.cend()) {
                    substIds.emplace_back(subst);
                }
            }
        }
        for (const auto &subst : merkevare.GetSortertVirkestoffUtenStyrke()) {
            FestUuid virkestoffId{subst};
            if (std::find(substIds.cbegin(), substIds.cend(), virkestoffId) == substIds.cend()) {
                substIds.emplace_back(virkestoffId);
            }
        }
    }
    for (const auto &atc : atcs) {
        FhirCoding coding{"http://nhn.no/kj/fhir/CodeSystem/ATC", atc.GetValue(), atc.GetDistinguishedName()};
        codings.emplace_back(coding, atc.GetDistinguishedName(), "ATC");
    }
    Substances(substIds);
}

RegisterCaveDialog::RegisterCaveDialog(wxWindow *parent, const std::shared_ptr<FestDb> &festDb, const LegemiddelCore &legemiddelCore, const FhirReference &recorder, const FhirReference &patient) : wxDialog(parent, wxID_ANY, wxT("Register CAVE")) {
    availableCodings = GetCaveCodings(festDb, legemiddelCore).operator const std::vector<CaveCoding> &();
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        id = boost::uuids::to_string(randomUUID);
    }
    {
        recordedDate = DateTimeOffset::Now().to_iso8601();
    }
    this->recorder = recorder;
    this->patient = patient;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "MemoryLeak"
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    codingsListView = new wxListView(this, wxID_ANY);
#pragma clang diagnostic pop
    codingsListView->AppendColumn(wxT("Substance:"));
    codingsListView->AppendColumn(wxT("Type"));
    codingsListView->SetColumnWidth(0, 300);
    {
        int rowCount = 0;
        for (const auto &coding: availableCodings) {
            auto row = rowCount++;
            codingsListView->InsertItem(row, wxString::FromUTF8(coding.display));
            codingsListView->SetItem(row, 1, wxString::FromUTF8(coding.type));
        }
    }
    codingsListView->Bind(wxEVT_LIST_ITEM_DESELECTED, &RegisterCaveDialog::ListUpdate, this);
    codingsListView->Bind(wxEVT_LIST_ITEM_SELECTED, &RegisterCaveDialog::ListUpdate, this);
    sizer->Add(codingsListView, 0, wxALL | wxEXPAND, 5);
    auto *inactiveIngredientSizer = new wxBoxSizer(wxHORIZONTAL);
    inactiveIngredient = new wxCheckBox(this, wxID_ANY, wxT("Inactive ingredient"));
    inactiveIngredientSizer->Add(inactiveIngredient, 0, wxALL | wxEXPAND, 5);
    sizer->Add(inactiveIngredientSizer, 0, wxALL | wxEXPAND, 5);
    auto *sourceOfInformationSizer = new wxBoxSizer(wxHORIZONTAL);
    sourceOfInformationSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Source of information: ")), 0, wxALL | wxEXPAND, 5);
    sourceOfInformation = new wxComboBox(this, wxID_ANY);
    sourceOfInformation->SetEditable(false);
    for (const auto &soi : MedicalCodedValue::GetCaveSourceOfInformation()) {
        sourceOfInformation->Append(wxString::FromUTF8(soi.GetDisplay()));
    }
    sourceOfInformation->Bind(wxEVT_COMBOBOX, &RegisterCaveDialog::RevalidateCommand, this);
    sourceOfInformationSizer->Add(sourceOfInformation, 0, wxALL | wxEXPAND, 5);
    sizer->Add(sourceOfInformationSizer, 0, wxALL | wxEXPAND, 5);
    auto *typeOfReactionSizer = new wxBoxSizer(wxHORIZONTAL);
    typeOfReactionSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Type of reaction: ")), 0, wxALL | wxEXPAND, 5);
    typeOfReaction = new wxComboBox(this, wxID_ANY);
    typeOfReaction->SetEditable(false);
    for (const auto &soi : MedicalCodedValue::GetCaveTypeOfReaction()) {
        typeOfReaction->Append(wxString::FromUTF8(soi.GetDisplay()));
    }
    typeOfReaction->Bind(wxEVT_COMBOBOX, &RegisterCaveDialog::RevalidateCommand, this);
    typeOfReactionSizer->Add(typeOfReaction, 0, wxALL | wxEXPAND, 5);
    sizer->Add(typeOfReactionSizer, 0, wxALL | wxEXPAND, 5);
    auto *verificationStatusSizer = new wxBoxSizer(wxHORIZONTAL);
    verificationStatusSizer->Add(new wxStaticText(this, wxID_ANY, wxT("Verification status: ")), 0, wxALL | wxEXPAND, 5);
    verificationStatus = new wxComboBox(this, wxID_ANY);
    verificationStatus->SetEditable(false);
    for (const auto &soi : MedicalCodedValue::GetCaveVerificationStatus()) {
        verificationStatus->Append(wxString::FromUTF8(soi.GetDisplay()));
    }
    verificationStatus->Bind(wxEVT_COMBOBOX, &RegisterCaveDialog::RevalidateCommand, this);
    verificationStatusSizer->Add(verificationStatus, 0, wxALL | wxEXPAND, 5);
    sizer->Add(verificationStatusSizer, 0, wxALL | wxEXPAND, 5);

    auto *buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    auto *cancelButton = new wxButton(this, wxID_CANCEL, wxT("Cancel"));
    addButton = new wxButton(this, wxID_OK, wxT("Add"));
    addButton->Enable(false);
    buttonsSizer->Add(cancelButton, 0, wxEXPAND | wxALL, 5);
    buttonsSizer->Add(addButton, 0, wxEXPAND | wxALL, 5);
    sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);

    SetSizerAndFit(sizer);
}

bool RegisterCaveDialog::IsValid() const {
    {
        auto codings = MedicalCodedValue::GetCaveVerificationStatus();
        auto selected = verificationStatus->GetSelection();
        if (selected < 0 || selected >= codings.size()) {
            return false;
        }
    }
    {
        if (codingsListView->GetSelectedItemCount() != 1) {
            return false;
        }
        auto selected = codingsListView->GetFirstSelected();
        if (selected < 0 || selected >= availableCodings.size()) {
            return false;
        }
    }
    {
        auto codings = MedicalCodedValue::GetCaveSourceOfInformation();
        auto selected = sourceOfInformation->GetSelection();
        if (selected < 0 || selected >= codings.size()) {
            return false;
        }
    }
    {
        auto codings = MedicalCodedValue::GetCaveTypeOfReaction();
        auto selected = typeOfReaction->GetSelection();
        if (selected < 0 || selected >= codings.size()) {
            return false;
        }
    }
    return true;
}

void RegisterCaveDialog::Revalidate() {
    addButton->Enable(IsValid());
}

void RegisterCaveDialog::RevalidateCommand(const wxCommandEvent &e) {
    Revalidate();
}

void RegisterCaveDialog::ListUpdate(const wxListEvent &e) {
    Revalidate();
}

std::shared_ptr<FhirAllergyIntolerance> RegisterCaveDialog::ToFhir() const {
    auto fhir = std::make_shared<FhirAllergyIntolerance>();
    fhir->SetCategories({"medication"});
    fhir->SetCriticality("high");
    fhir->SetId(id);
    fhir->SetIdentifiers({{"official", id}});
    fhir->SetProfile("http://nhn.no/kj/fhir/StructureDefinition/KjAllergyIntolerance");
    fhir->SetRecordedDate(recordedDate);
    fhir->SetRecorder(recorder);
    fhir->SetPatient(patient);
    fhir->AddExtension(std::make_shared<FhirValueExtension>("http://nhn.no/kj/fhir/StructureDefinition/KjUpdatedDateTime", std::make_shared<FhirDateTimeValue>(DateTimeOffset::Now().to_iso8601())));
    {
        auto codings = MedicalCodedValue::GetCaveVerificationStatus();
        auto selected = verificationStatus->GetSelection();
        bool active{false};
        if (selected >= 0 && selected < codings.size()) {
            auto code = codings[selected];
            if (code.GetCode() != "refuted" && code.GetCode() != "entered-in-error") {
                active = true;
            }
            fhir->SetVerificationStatus(code.ToCodeableConcept());
        }
        FhirCodeableConcept codeable{"http://terminology.hl7.org/CodeSystem/allergyintolerance-clinical", active ? "active" : "inactive", active ? "Active" : "Inactive"};
        fhir->SetClinicalStatus(codeable);
    }
    if (codingsListView->GetSelectedItemCount() == 1) {
        auto selected = codingsListView->GetFirstSelected();
        if (selected >= 0 && selected < availableCodings.size()) {
            FhirCodeableConcept codeable{{availableCodings[selected].coding}};
            if (inactiveIngredient->IsChecked()) {
                codeable.AddExtension(std::make_shared<FhirValueExtension>("http://nhn.no/kj/fhir/StructureDefinition/KjInactiveIngredient", std::make_shared<FhirBooleanValue>(true)));
            }
            fhir->SetCode(codeable);
        }
    }
    {
        auto codings = MedicalCodedValue::GetCaveSourceOfInformation();
        auto selected = sourceOfInformation->GetSelection();
        if (selected >= 0 && selected < codings.size()) {
            fhir->AddExtension(std::make_shared<FhirValueExtension>("http://nhn.no/kj/fhir/StructureDefinition/KjSourceOfInformation", std::make_shared<FhirCodingValue>(codings[selected].ToCoding())));
        }
    }
    {
        auto codings = MedicalCodedValue::GetCaveTypeOfReaction();
        auto selected = typeOfReaction->GetSelection();
        if (selected >= 0 && selected < codings.size()) {
            std::vector<FhirReaction> reactions{};
            reactions.emplace_back().SetManifestation({codings[selected].ToCodeableConcept()});
            fhir->SetReactions(reactions);
        }
    }
    return fhir;
}