//
// Created by sigsegv on 5/2/24.
//

#include "PrescriptionDetailsDialog.h"
#include <wx/listctrl.h>
#include <sfmbasisapi/fhir/medstatement.h>
#include <sfmbasisapi/fhir/fhirbasic.h>
#include <sfmbasisapi/fhir/extension.h>
#include "EreseptdosingDialog.h"
#include <sstream>

template <class T> concept PrescriptionDetailsDialogEntryRequirements = requires (const T &entry) {
    { entry.GetDisplay() } -> std::convertible_to<std::string>;
    { entry.GetExtensions() } -> std::convertible_to<std::vector<std::shared_ptr<FhirExtension>>>;
    { entry.GetIdentifiers() } -> std::convertible_to<std::vector<FhirIdentifier>>;
};

template <class T> concept PrescriptionDetailsDialogEntryHasDosage = requires (const T &entry) {
    { entry.GetDosage() } -> std::convertible_to<std::vector<FhirDosage>>;
};

template <class T> concept PrescriptionDetailsDialogEntryHasGetEffectiveDateTime = requires (const T &entry) {
    { entry.GetEffectiveDateTime() } -> std::convertible_to<std::string>;
};

template <class T> struct GetDosageFromFhir {
    GetDosageFromFhir(const T &) {
    }
    operator std::vector<FhirDosage> () const {
        return {};
    }
};

template <PrescriptionDetailsDialogEntryHasDosage T> struct GetDosageFromFhir<T> {
    std::vector<FhirDosage> result;
    GetDosageFromFhir(const T &obj) : result(obj.GetDosage()) {
    }
    operator std::vector<FhirDosage> () const {
        return result;
    }
};

template <class T> struct GetEffectiveDateTimeFromFhir {
    GetEffectiveDateTimeFromFhir(const T &) {
    }
    operator std::string () const {
        return "";
    }
};

template <PrescriptionDetailsDialogEntryHasGetEffectiveDateTime T> struct GetEffectiveDateTimeFromFhir<T> {
    std::string result;
    GetEffectiveDateTimeFromFhir(const T &obj) : result(obj.GetEffectiveDateTime()) {}
    operator std::string () const {
        return result;
    }
};

template <PrescriptionDetailsDialogEntryRequirements T> class PrescriptionDetailsDialogEntryImpl : public PrescriptionDetailsDialogEntry {
private:
    std::shared_ptr<const T> ref{};
public:
    PrescriptionDetailsDialogEntryImpl(const std::shared_ptr<const T> &ref) : ref(ref) {}
    PrescriptionDetailsDialogEntryImpl(std::shared_ptr<const T> &&ref) : ref(std::move(ref)) {}
    std::string GetDisplay() const override {
        return ref->GetDisplay();
    }
    [[nodiscard]] std::vector<FhirDosage> GetDosage() const override {
        return GetDosageFromFhir(*ref);
    }
    [[nodiscard]] std::vector<std::shared_ptr<FhirExtension>> GetExtensions() const override {
        return ref->GetExtensions();
    }
    [[nodiscard]] std::vector<FhirIdentifier> GetIdentifiers() const override {
        return ref->GetIdentifiers();
    }
    [[nodiscard]] std::string GetEffectiveDateTime() const override {
        return GetEffectiveDateTimeFromFhir(*ref);
    }
};

template <PrescriptionDetailsDialogEntryRequirements T> std::vector<std::shared_ptr<PrescriptionDetailsDialogEntry>> GetInterface(const std::vector<std::shared_ptr<const T>> &input) {
    std::vector<std::shared_ptr<PrescriptionDetailsDialogEntry>> output{};
    for (const auto &ref : input) {
        output.emplace_back(std::make_shared<PrescriptionDetailsDialogEntryImpl<T>>(ref));
    }
    return output;
}

template <PrescriptionDetailsDialogEntryRequirements T> std::vector<std::shared_ptr<PrescriptionDetailsDialogEntry>> GetInterface(const std::vector<std::shared_ptr<T>> &input) {
    std::vector<std::shared_ptr<PrescriptionDetailsDialogEntry>> output{};
    for (const auto &ref : input) {
        std::shared_ptr<const T> constRef{ref};
        output.emplace_back(std::make_shared<PrescriptionDetailsDialogEntryImpl<T>>(std::move(constRef)));
    }
    return output;
}

PrescriptionDetailsDialog::PrescriptionDetailsDialog(wxWindow *parent,
                                                     const std::vector<std::shared_ptr<FhirMedicationStatement>> &statements) :
                                                     PrescriptionDetailsDialog(parent, GetInterface<FhirMedicationStatement>(statements)) {
}

PrescriptionDetailsDialog::PrescriptionDetailsDialog(wxWindow *parent,
                                                     const std::vector<std::shared_ptr<FhirBasic>> &statements) :
        PrescriptionDetailsDialog(parent, GetInterface<FhirBasic>(statements)) {
}

PrescriptionDetailsDialog::PrescriptionDetailsDialog(wxWindow *parent,
                                                     const std::vector<std::shared_ptr<PrescriptionDetailsDialogEntry>> &statements) :
        wxDialog(parent, wxID_ANY, wxT("Prescription details")), statements(statements) {
    auto *sizer = new wxBoxSizer(wxVERTICAL);
    versionsView = new wxListView(this, wxID_ANY);
    versionsView->AppendColumn(wxT(""));
    {
        int row = 0;
        for (const auto &statement: statements) {
            AddVersion(row++, statement);
        }
    }
    versionsView->Select(0);
    versionsView->Bind(wxEVT_LIST_ITEM_DESELECTED, &PrescriptionDetailsDialog::OnVersionSelect, this, wxID_ANY);
    versionsView->Bind(wxEVT_LIST_ITEM_SELECTED, &PrescriptionDetailsDialog::OnVersionSelect, this, wxID_ANY);
    sizer->Add(versionsView, 1, wxEXPAND | wxALL, 5);
    auto buttonsSizer = new wxBoxSizer(wxHORIZONTAL);
    structuredDosing = new wxButton(this, wxID_ANY, wxT("Structured dosing"));
    structuredDosing->Bind(wxEVT_BUTTON, &PrescriptionDetailsDialog::OnStructuredDosing, this, wxID_ANY);
    buttonsSizer->Add(structuredDosing, 1, wxEXPAND | wxALL, 5);
    sizer->Add(buttonsSizer, 0, wxEXPAND | wxALL, 5);
    listView = new wxListView(this, wxID_ANY);
    if (!statements.empty()) {
        DisplayStatement(statements[0]);
    } else {
        DisplayStatement({});
    }
    sizer->Add(listView, 2, wxEXPAND | wxALL, 5);
    SetSizerAndFit(sizer);
}

void PrescriptionDetailsDialog::AddVersion(int row, const std::shared_ptr<PrescriptionDetailsDialogEntry> &statement) {
    versionsView->InsertItem(row, wxString::FromUTF8(statement->GetDisplay()));
}

void PrescriptionDetailsDialog::DisplayStatement(const std::shared_ptr<PrescriptionDetailsDialogEntry> &statement) {
    listView->ClearAll();
    listView->AppendColumn(wxT(""));
    listView->AppendColumn(wxT(""));
    int rowNum = 0;
    {
        wxString display = wxString::FromUTF8(statement ? statement->GetDisplay() : "");
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Display: "));
        listView->SetItem(row, 1, display);
    }
    wxString ePrescriptionId{};
    wxString pllId{};
    wxString typeOfUse{};
    wxString shortDose{};
    wxString dosageText{};
    wxString dssn{};
    wxString prescriptionDate{};
    wxString expirationDate{};
    wxString festUpdate{};
    wxString reit{};
    wxString ceaseTime{};
    wxString treatmentStartDate{};
    wxString treatmentStartDateExt{};
    std::string amountUnit{};
    FhirCodeableConcept rfstatus{};
    FhirCodeableConcept itemGroup{};
    FhirCodeableConcept typeOfPrescription{};
    FhirCodeableConcept ceaseReason{};
    double amount;
    double numberOfPackages{0.000};
    bool guardianTransparencyReservation{false};
    bool inDoctorsName{false};
    bool lockedPrescription{false};
    std::vector<std::shared_ptr<FhirExtension>> regInfos{};
    std::vector<std::shared_ptr<FhirExtension>> ereseptdosing{};
    if (statement) {
        auto dosageItems = statement->GetDosage();
        if (!dosageItems.empty()) {
            auto &dosage = dosageItems[0];
            dosageText = wxString::FromUTF8(dosage.GetText());
            auto extensions = dosage.GetExtensions();
            for (const auto &extension : extensions) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                if (url == "http://ehelse.no/fhir/structuredefinition/sfm-use") {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExtension) {
                        auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                        if (value) {
                            auto codings = value->GetCoding();
                            if (!codings.empty()) {
                                auto &coding = codings[0];
                                std::string str{coding.GetCode()};
                                str.append(" ");
                                str.append(coding.GetDisplay());
                                typeOfUse = wxString::FromUTF8(str);
                            }
                        }
                    }
                } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-shortdosage") {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExtension) {
                        auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                        if (value) {
                            auto codings = value->GetCoding();
                            if (!codings.empty()) {
                                auto &coding = codings[0];
                                std::string str{coding.GetCode()};
                                str.append(" ");
                                str.append(coding.GetDisplay());
                                shortDose = wxString::FromUTF8(str);
                            }
                        }
                    }
                } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-application-area") {
                    auto subexts = extension->GetExtensions();
                    for (const auto &subext : subexts) {
                        auto url = subext->GetUrl();
                        std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) { return std::tolower(ch); });
                    }
                }
            }
        }
        auto extensions = statement->GetExtensions();
        for (const auto &extension : extensions) {
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
            if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
                auto extensions = extension->GetExtensions();
                for (const auto &extension : extensions) {
                    auto url = extension->GetUrl();
                    if (url == "dssn") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirString>(valueExtension->GetValue()) : std::shared_ptr<FhirString>();
                        if (value) {
                            dssn = wxString::FromUTF8(value->GetValue());
                        }
                    } else if (url == "rfstatus") {
                        auto extensions = extension->GetExtensions();
                        for (const auto &extension : extensions) {
                            auto url = extension->GetUrl();
                            if (url == "status") {
                                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue()) : std::shared_ptr<FhirCodeableConceptValue>();
                                if (value) {
                                    rfstatus = *value;
                                }
                            }
                        }
                    } else if (url == "reseptdate") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirDateValue>(valueExtension->GetValue()) : std::shared_ptr<FhirDateValue>();
                        if (value) {
                            prescriptionDate = wxString::FromUTF8(value->GetRawValue());
                        }
                    } else if (url == "expirationdate") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirDateValue>(valueExtension->GetValue()) : std::shared_ptr<FhirDateValue>();
                        if (value) {
                            expirationDate = wxString::FromUTF8(value->GetRawValue());
                        }
                    } else if (url == "festUpdate") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirDateTimeValue>(valueExtension->GetValue()) : std::shared_ptr<FhirDateTimeValue>();
                        if (value) {
                            festUpdate = wxString::FromUTF8(value->GetDateTime());
                        }
                    } else if (url == "guardiantransparencyreservation") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirBooleanValue>(valueExtension->GetValue()) : std::shared_ptr<FhirBooleanValue>();
                        if (value) {
                            guardianTransparencyReservation = value->IsTrue();
                        }
                    } else if (url == "indoctorsname") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirBooleanValue>(valueExtension->GetValue()) : std::shared_ptr<FhirBooleanValue>();
                        if (value) {
                            inDoctorsName = value->IsTrue();
                        }
                    } else if (url == "lockedresept") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirBooleanValue>(valueExtension->GetValue()) : std::shared_ptr<FhirBooleanValue>();
                        if (value) {
                            lockedPrescription = value->IsTrue();
                        }
                    } else if (url == "reit") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirString>(valueExtension->GetValue()) : std::shared_ptr<FhirString>();
                        if (value) {
                            reit = wxString::FromUTF8(value->GetValue());
                        }
                    } else if (url == "amount") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirQuantityValue>(valueExtension->GetValue()) : std::shared_ptr<FhirQuantityValue>();
                        if (value) {
                            amount = value->GetValue();
                            amountUnit = value->GetUnit();
                        }
                    } else if (url == "numberofpackages") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirDecimalValue>(valueExtension->GetValue()) : std::shared_ptr<FhirDecimalValue>();
                        if (value) {
                            numberOfPackages = value->GetValue();
                        }
                    } else if (url == "itemgroup") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue()) : std::shared_ptr<FhirCodeableConceptValue>();
                        if (value) {
                            itemGroup = *value;
                        }
                    } else if (url == "typeresept") {
                        auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                        auto value = valueExtension.operator bool() ? std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue()) : std::shared_ptr<FhirCodeableConceptValue>();
                        if (value) {
                            typeOfPrescription = *value;
                        }
                    } else if (url == "ereseptdosing") {
                        ereseptdosing.emplace_back(extension);
                    }
                }
            } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reginfo") {
                regInfos.emplace_back(extension);
            } else if (url == "http://ehelse.no/fhir/structuredefinition/sfm-discontinuation") {
                auto extensions = extension->GetExtensions();
                for (const auto &extension : extensions) {
                    auto url = extension->GetUrl();
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExtension) {
                        auto value = valueExtension->GetValue();
                        auto dateTimeValue = std::dynamic_pointer_cast<FhirDateTimeValue>(value);
                        auto codeableConceptValue = !dateTimeValue ? std::dynamic_pointer_cast<FhirCodeableConceptValue>(value) : std::shared_ptr<FhirCodeableConceptValue>();
                        if (dateTimeValue) {
                            if (url == "timedate") {
                                ceaseTime = wxString::FromUTF8(dateTimeValue->GetDateTime());
                            }
                        } else if (codeableConceptValue) {
                            if (url == "reason") {
                                ceaseReason = *codeableConceptValue;
                            }
                        }
                    }
                }
            } else if (url == "effectivedatetime") {
                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                if (valueExtension) {
                    auto value = std::dynamic_pointer_cast<FhirDateValue>(valueExtension->GetValue());
                    if (value) {
                        treatmentStartDateExt = wxString::FromUTF8(value->GetRawValue());
                    }
                }
            }
        }
        auto identifiers = statement->GetIdentifiers();
        for (const auto &identifier : identifiers) {
            auto typeText = identifier.GetType().GetText();
            std::transform(typeText.begin(), typeText.end(), typeText.begin(), [] (char ch) { return std::tolower(ch); });
            if (typeText == "reseptid") {
                ePrescriptionId = wxString::FromUTF8(identifier.GetValue());
            } else if (typeText == "pll") {
                pllId = wxString::FromUTF8(identifier.GetValue());
            }
        }
        treatmentStartDate = wxString::FromUTF8(statement->GetEffectiveDateTime());
    }
    this->ereseptdosing = ereseptdosing;
    structuredDosing->Enable(!ereseptdosing.empty());
    {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("EPrescriptionId:"));
        listView->SetItem(row, 1, ePrescriptionId);
    }
    if (!pllId.empty()) {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("PLL:"));
        listView->SetItem(row, 1, pllId);
    }
    {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("DSSN:"));
        listView->SetItem(row, 1, dssn);
    }
    {
        wxString status{};
        auto coding = rfstatus.GetCoding();
        if (!coding.empty()) {
            status = wxString::FromUTF8(coding[0].GetDisplay());
        }
        auto row = rowNum++;
        listView->InsertItem(row, wxT("RFStatus:"));
        listView->SetItem(row, 1, status);
    }
    if (ceaseReason.IsSet()) {
        FhirCoding coding{};
        {
            auto codings = ceaseReason.GetCoding();
            if (!codings.empty()) {
                coding = codings[0];
            }
        }
        auto ceaseCode = coding.GetCode();
        auto ceaseDisplay = coding.GetDisplay();
        if (!ceaseCode.empty() || !ceaseDisplay.empty()) {
            std::string info{ceaseCode};
            if (!ceaseDisplay.empty()) {
                if (!info.empty()) {
                    info.append(" ");
                }
                info.append(ceaseDisplay);
            }
            auto row = rowNum++;
            listView->InsertItem(row, wxT("Cessation reason: "));
            listView->SetItem(row, 1, wxString::FromUTF8(info));
        }
        if (!ceaseTime.empty()) {
            auto row = rowNum++;
            listView->InsertItem(row, wxT("Cessation reason: "));
            listView->SetItem(row, 1, ceaseTime);
        }
    }
    if (statement) {
        auto dosages = statement->GetDosage();
        for (const auto &dosage : dosages) {
            auto extensions = dosage.GetExtensions();
            for (const auto &extension : extensions) {
                auto url = extension->GetUrl();
                if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-use") {
                    auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    auto value = valueExt.operator bool() ? std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExt->GetValue()) : std::shared_ptr<FhirCodeableConceptValue>();
                    if (value) {
                        std::vector<FhirCoding> codings = value->GetCoding();
                        for (const auto &coding : codings) {
                            auto text = wxString::FromUTF8(coding.GetDisplay());
                            if (!text.empty()) {
                                int row = rowNum++;
                                listView->InsertItem(row, wxT("Type of usage:"));
                                listView->SetItem(row, 1, text);
                            }
                        }
                    }
                } else if (url == "http://ehelse.no/fhir/StructureDefinition/sfm-application-area") {
                    auto extensions = extension->GetExtensions();
                    for (const auto &extension : extensions) {
                        auto url = extension->GetUrl();
                        if (url == "text") {
                            auto valueExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                            auto value = valueExt.operator bool() ? std::dynamic_pointer_cast<FhirString>(valueExt->GetValue()) : std::shared_ptr<FhirString>();
                            wxString applicationArea = value.operator bool() ? wxString::FromUTF8(value->GetValue()) : wxString();
                            if (!applicationArea.empty()) {
                                auto row = rowNum++;
                                listView->InsertItem(row, wxT("Application area:"));
                                listView->SetItem(row, 1, applicationArea);
                            }
                        }
                    }
                }
            }
            if (!shortDose.empty()) {
                auto row = rowNum++;
                listView->InsertItem(row, wxT("Short dose: "));
                listView->SetItem(row, 1, shortDose);
            }
            auto dosageText = wxString::FromUTF8(dosage.GetText());
            auto row = rowNum++;
            listView->InsertItem(row, wxT("Dosage: "));
            listView->SetItem(row, 1, dosageText);
        }
    }
    {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Prescription date: "));
        listView->SetItem(row, 1, prescriptionDate);
    }
    {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Expiration date: "));
        listView->SetItem(row, 1, expirationDate);
    }
    {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Fest update: "));
        listView->SetItem(row, 1, festUpdate);
    }
    {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Guardian transparency reservation: "));
        listView->SetItem(row, 1, guardianTransparencyReservation ? wxT("Yes") : wxT("No"));
    }
    {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Locked prescription: "));
        listView->SetItem(row, 1, lockedPrescription ? wxT("Yes") : wxT("No"));
    }
    {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("In doctors name: "));
        listView->SetItem(row, 1, inDoctorsName ? wxT("Yes") : wxT("No"));
    }
    {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Reit: "));
        listView->SetItem(row, 1, reit);
    }
    if (!amountUnit.empty()) {
        std::stringstream ss{};
        ss << amount << " " << amountUnit;
        auto amountStr = wxString::FromUTF8(ss.str());
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Amount: "));
        listView->SetItem(row, 1, amountStr);
    }
    if (numberOfPackages > 0.0001) {
        std::stringstream ss{};
        ss << numberOfPackages;
        auto numStr = wxString::FromUTF8(ss.str());
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Number of packages: "));
        listView->SetItem(row, 1, numStr);
    }
    {
        auto coding = itemGroup.GetCoding();
        if (!coding.empty()) {
            auto row = rowNum++;
            listView->InsertItem(row, wxT("Item group: "));
            listView->SetItem(row, 1, wxString::FromUTF8(coding[0].GetDisplay()));
        }
    }
    {
        auto coding = typeOfPrescription.GetCoding();
        if (!coding.empty()) {
            auto row = rowNum++;
            listView->InsertItem(row, wxT("Type of prescription: "));
            listView->SetItem(row, 1, wxString::FromUTF8(coding[0].GetDisplay()));
        }
    }
    if (!treatmentStartDate.empty()) {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Treatment start: "));
        listView->SetItem(row, 1, treatmentStartDate);
    }
    if (!treatmentStartDateExt.empty()) {
        auto row = rowNum++;
        listView->InsertItem(row, wxT("Treatment start (extension): "));
        listView->SetItem(row, 1, treatmentStartDateExt);
    }
    for (const auto &regInfo : regInfos) {
        FhirCodeableConcept status{};
        FhirCodeableConcept type{};
        FhirReference provider{};
        std::string timestamp{};
        {
            auto extensions = regInfo->GetExtensions();
            for (const auto &extension: extensions) {
                auto url = extension->GetUrl();
                auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                if (!valueExtension) {
                    continue;
                }
                if (url == "status") {
                    auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                    if (value) {
                        status = *value;
                    }
                } else if (url == "type") {
                    auto value = std::dynamic_pointer_cast<FhirCodeableConceptValue>(valueExtension->GetValue());
                    if (value) {
                        type = *value;
                    }
                } else if (url == "provider") {
                    auto value = std::dynamic_pointer_cast<FhirReference>(valueExtension->GetValue());
                    if (value) {
                        provider = *value;
                    }
                } else if (url == "timestamp") {
                    auto value = std::dynamic_pointer_cast<FhirDateTimeValue>(valueExtension->GetValue());
                    if (value) {
                        timestamp = value->GetDateTime();
                    }
                }
            }
        }
        {
            std::stringstream ss{};
            {
                auto coding = type.GetCoding();
                if (!coding.empty()) {
                    ss << coding[0].GetDisplay();
                }
            }
            {
                auto display = provider.GetDisplay();
                if (!display.empty()) {
                    if (!ss.str().empty()) {
                        ss << " ";
                    }
                    ss << display;
                }
            }
            auto row = rowNum++;
            listView->InsertItem(row, wxT("Change: "));
            listView->SetItem(row, 1, wxString::FromUTF8(ss.str()));
        }
        {
            std::stringstream ss{};
            {
                auto coding = status.GetCoding();
                if (!coding.empty()) {
                    ss << coding[0].GetDisplay();
                }
            }
            if (!timestamp.empty()) {
                if (!ss.str().empty()) {
                    ss << " ";
                }
                ss << timestamp;
            }
            auto row = rowNum++;
            listView->InsertItem(row, wxT("Status: "));
            listView->SetItem(row, 1, wxString::FromUTF8(ss.str()));
        }
    }
    listView->SetColumnWidth(0, 250);
    listView->SetColumnWidth(1, 300);
}

void PrescriptionDetailsDialog::OnVersionSelect(wxCommandEvent &e) {
    if (versionsView->GetSelectedItemCount() != 1) {
        DisplayStatement({});
        return;
    }
    auto selected = versionsView->GetFirstSelected();
    if (selected < 0 || selected >= statements.size()) {
        DisplayStatement({});
        return;
    }
    DisplayStatement(statements[selected]);
}

void PrescriptionDetailsDialog::OnStructuredDosing(wxCommandEvent &e) {
    EreseptdosingDialog ereseptdosingDialog{this, ereseptdosing};
    ereseptdosingDialog.ShowModal();
}