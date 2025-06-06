//
// Created by sigsegv on 7/24/24.
//

#include <sfmbasisapi/fhir/medication.h>
#include <sfmbasisapi/fhir/medstatement.h>
#include <sfmbasisapi/fhir/composition.h>
#include <sfmbasisapi/fhir/person.h>
#include <sfmbasisapi/fhir/allergy.h>
#include <sfmbasisapi/nhnfhir/SfmBandaPrescription.h>
#include <jjwtid/Jwt.h>
#include <jjwtid/JwtPart.h>
#include "MedBundleData.h"
#include "PrescriptionChangesService.h"
#include "PrescriptionData.h"
#include "MerchData.h"
#include "Uuid.h"
#include <sstream>

class MedBundleDataException : public std::exception {
private:
    std::string error;
public:
    MedBundleDataException(const std::string &error) : error(error) {}
    const char * what() const noexcept override;
};

const char *MedBundleDataException::what() const noexcept {
    return error.c_str();
}

std::shared_ptr<Fhir> MedBundleData::GetByUrl(const std::shared_ptr<FhirBundle> &bundle, std::string url) {
    std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
    for (const auto &entry : bundle->GetEntries()) {
        auto entryUrl = entry.GetFullUrl();
        std::transform(entryUrl.cbegin(), entryUrl.cend(), entryUrl.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
        if (entryUrl == url) {
            return entry.GetResource();
        }
    }
    return {};
}
std::vector<FhirBundleEntry> MedBundleData::GetPractitioners(const std::shared_ptr<FhirBundle> &bundle) {
    std::vector<FhirBundleEntry> practitioners{};
    for (const auto &entry : bundle->GetEntries()) {
        auto resource = entry.GetResource();
        auto practitioner = std::dynamic_pointer_cast<FhirPractitioner>(resource);

        if (practitioner) {
            practitioners.emplace_back(entry);
        }
    }
    return practitioners;
}

std::vector<std::string> MedBundleData::GetRenewals(const std::shared_ptr<FhirBundle> &bundle) {
    std::vector<std::string> renewals{};
    for (const auto &entry : bundle->GetEntries()) {
        auto resource = entry.GetResource();
        auto medicationStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(resource);

        if (medicationStatement && PrescriptionChangesService::IsRenewedWithoutChangesAssumingIsEprescription(*medicationStatement)) {
            auto reseptId = PrescriptionChangesService::GetPreviousPrescriptionId(*medicationStatement);
            if (!reseptId.empty()) {
                renewals.emplace_back(reseptId);
            }
        }
    }
    return renewals;
}

PrescriberRef MedBundleData::GetPrescriber(const std::shared_ptr<FhirBundle> &bundle, const std::string &helseidIdToken) {
    Jwt jwt{helseidIdToken};
    JwtPart jwtBody{jwt.GetUnverifiedBody()};
    std::string pid = jwtBody.GetString("helseid://claims/identity/pid");
    std::string hpr = jwtBody.GetString("helseid://claims/hpr/hpr_number");
    std::string name = jwtBody.GetString("name");
    std::string given_name = jwtBody.GetString("given_name");
    std::string middle_name = jwtBody.GetString("middle_name");
    std::string family_name = jwtBody.GetString("family_name");
    std::string dateOfBirth{};
    bool female{true};
    if (pid.size() == 11) {
        std::string sd = pid.substr(0, 2);
        std::string sm = pid.substr(2, 2);
        std::string sy2 = pid.substr(4, 2);
        std::string sc = pid.substr(6, 1);
        std::string sg = pid.substr(8, 1);
        std::size_t ccd;
        std::size_t ccm;
        std::size_t ccy;
        std::size_t ccc;
        std::size_t ccg;
        auto d = std::stoi(sd, &ccd);
        auto m = std::stoi(sm, &ccm);
        auto y = std::stoi(sy2, &ccy);
        auto c = std::stoi(sc, &ccc);
        auto g = std::stoi(sg, &ccg);
        if (ccd == 2 && ccm == 2 && ccy == 2 && ccc == 1 && ccg == 1 && y >= 0 && m > 0 && d > 0) {
            if (c <= 4) {
                if (y < 40) {
                    y += 2000;
                } else {
                    y += 1900;
                }
            } else if (c == 8) {
                y += 2000;
            } else if (c < 8) {
                if (y < 55) {
                    y += 2000;
                } else {
                    y += 1800;
                }
            } else {
                if (y < 40) {
                    y += 1900;
                } else {
                    y += 2000;
                }
            }
        }
        if ((g & 1) == 1) {
            female = false;
        }
        std::stringstream dob{};
        dob << y << "-";
        if (m < 10) {
            dob << "0";
        }
        dob << m << "-";
        if ( d < 10) {
            dob << "0";
        }
        dob << d;
        dateOfBirth = dob.str();
    }
    if (!bundle) {
        return {};
    }
    for (const auto &entry : bundle->GetEntries()) {
        auto resource = entry.GetResource();
        auto practitioner = std::dynamic_pointer_cast<FhirPractitioner>(resource);

        if (practitioner &&
            practitioner->IsActive()) {
            std::string ppid{};
            std::string phpr{};
            for (const auto &identifier : practitioner->GetIdentifiers()) {
                if (identifier.GetSystem() == "urn:oid:2.16.578.1.12.4.1.4.1") {
                    ppid = identifier.GetValue();
                } else if (identifier.GetSystem() == "urn:oid:2.16.578.1.12.4.1.4.4") {
                    phpr = identifier.GetValue();
                }
            }
            bool matching{false};
            if (!pid.empty()) {
                if (!hpr.empty()) {
                    if (ppid == pid && phpr == hpr) {
                        matching = true;
                    }
                } else {
                    if (ppid == pid) {
                        matching = true;
                    }
                }
            } else if (!hpr.empty()) {
                if (phpr == hpr) {
                    matching = true;
                }
            }
            if (matching) {
                return {.uuid = entry.GetFullUrl(), .name = practitioner->GetDisplay()};
            }
        }
    }
    std::shared_ptr<FhirPractitioner> practitioner = std::make_shared<FhirPractitioner>();
    practitioner->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner");
    practitioner->SetId(Uuid::RandomUuidString());
    practitioner->SetStatus(FhirStatus::ACTIVE);
    {
        std::vector<FhirIdentifier> identifiers{};
        if (!hpr.empty()) {
            identifiers.emplace_back(FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/HPR", "HPR-nummer", ""), "official", "urn:oid:2.16.578.1.12.4.1.4.4", hpr);
        }
        if (!pid.empty()) {
            identifiers.emplace_back(FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/FNR", "FNR-nummer", ""), "official", "urn:oid:2.16.578.1.12.4.1.4.1", pid);
        }
        practitioner->SetIdentifiers(identifiers);
    }
    practitioner->SetActive(true);
    practitioner->SetBirthDate(dateOfBirth);
    practitioner->SetGender(female ? "female" : "male");
    {
        std::vector<FhirName> setName{};
        std::string name{given_name};
        if (!name.empty()) {
            if (!middle_name.empty()) {
                name.append(" ");
                name.append(middle_name);
            }
        } else {
            name = middle_name;
        }
        setName.emplace_back("official", family_name, name);
        practitioner->SetName(setName);
    }
    std::string fullUrl{"urn:uuid:"};
    fullUrl.append(practitioner->GetId());
    FhirBundleEntry bundleEntry{fullUrl, practitioner};
    bundle->AddEntry(bundleEntry);
    return {.uuid = fullUrl, .name = practitioner->GetDisplay()};
}

std::shared_ptr<Fhir> MedBundleData::GetByUrl(const std::string &url) {
    return GetByUrl(medBundle, url);
}

PrescriberRef MedBundleData::GetPrescriber(const std::string &helseidIdToken) const {
    return GetPrescriber(medBundle, helseidIdToken);
}

FhirReference MedBundleData::GetSubjectRef() const {
    std::string pid{};
    auto patientInformation = this->patientInformation;
    if (patientInformation.GetPatientIdType() == PatientIdType::FODSELSNUMMER) {
        pid = patientInformation.GetPatientId();
    }
    auto bundle = medBundle;
    if (!bundle) {
        return {};
    }
    if (!pid.empty()) {
        for (const auto &entry: bundle->GetEntries()) {
            auto resource = entry.GetResource();
            auto patient = std::dynamic_pointer_cast<FhirPatient>(resource);

            if (patient &&
                patient->IsActive()) {
                std::string ppid{};
                for (const auto &identifier: patient->GetIdentifiers()) {
                    if (identifier.GetSystem() == "urn:oid:2.16.578.1.12.4.1.4.1") {
                        ppid = identifier.GetValue();
                    }
                }
                if (ppid == pid) {
                    return FhirReference(entry.GetFullUrl(), "http://ehelse.no/fhir/StructureDefinition/sfm-Patient", patient->GetDisplay());
                }
            }
        }
    }
    std::shared_ptr<FhirPatient> patient = std::make_shared<FhirPatient>();
    patient->SetProfile("http://ehelse.no/fhir/StructureDefinition/sfm-Patient");
    patient->SetId(Uuid::RandomUuidString());
    patient->SetStatus(FhirStatus::NOT_SET);
    {
        std::vector<FhirIdentifier> identifiers{};
        if (!pid.empty()) {
            identifiers.emplace_back(FhirCodeableConcept("http://hl7.no/fhir/NamingSystem/FNR", "FNR-nummer", ""), "official", "urn:oid:2.16.578.1.12.4.1.4.1", pid);
        }
        patient->SetIdentifiers(identifiers);
    }
    patient->SetActive(true);
    patient->SetBirthDate(patientInformation.GetDateOfBirth());
    patient->SetGender(patientInformation.GetGender() == PersonGender::FEMALE ? "female" : "male");
    {
        std::vector<FhirName> setName{};
        setName.emplace_back("official", patientInformation.GetFamilyName(), patientInformation.GetGivenName());
        patient->SetName(setName);
    }
    std::string fullUrl{"urn:uuid:"};
    fullUrl.append(patient->GetId());
    FhirBundleEntry bundleEntry{fullUrl, patient};
    bundle->AddEntry(bundleEntry);
    return FhirReference(fullUrl, "http://ehelse.no/fhir/StructureDefinition/sfm-Patient", patient->GetDisplay());
}

std::vector<FhirBundleEntry> MedBundleData::GetPractitioners() const {
    return medBundle ? GetPractitioners(medBundle) : std::vector<FhirBundleEntry>();
}

void MedBundleData::InsertNonexistingMedicationsFrom(const std::shared_ptr<FhirBundle> &otherBundle) {
    std::vector<std::string> urls{};
    {
        auto entries = medBundle->GetEntries();
        for (const auto &entry : entries) {
            auto url = entry.GetFullUrl();
            urls.emplace_back(url);
        }
    }
    std::vector<FhirBundleEntry> add{};
    {
        auto entries = otherBundle->GetEntries();
        for (const auto &entry: entries) {
            auto resource = std::dynamic_pointer_cast<FhirMedication>(entry.GetResource());
            if (!resource) {
                continue;
            }
            auto url = entry.GetFullUrl();
            bool found{false};
            for (const auto &u: urls) {
                if (u == url) {
                    found = true;
                    break;
                }
            }
            if (found) {
                continue;
            }
            urls.emplace_back(url);
            add.emplace_back(entry);
        }
    }
    if (!add.empty()) {
        auto entries = medBundle->GetEntries();
        for (const auto &entry: add) {
            entries.emplace_back(entry);
        }
        medBundle->SetEntries(entries);
    }
}

void MedBundleData::InsertNonexistingMedicationPrescriptionsFrom(const std::shared_ptr<FhirBundle> &otherBundle, const std::string &helseidIdToken) {
    std::string previousPrescriberRef{};
    {
        auto prescriberRef = GetPrescriber(otherBundle, helseidIdToken);
        previousPrescriberRef = prescriberRef.uuid;
    }
    auto previousPractitioners = GetPractitioners(otherBundle);
    {
        auto iterator = previousPractitioners.begin();
        while (iterator != previousPractitioners.end()) {
            const auto &entry = *iterator;
            if (entry.GetFullUrl() == previousPrescriberRef) {
                iterator = previousPractitioners.erase(iterator);
            } else {
                ++iterator;
            }
        }
    }
    std::vector<std::string> urls{};
    std::vector<std::string> prescriptionIds{};
    {
        auto entries = medBundle->GetEntries();
        for (const auto &entry : entries) {
            auto url = entry.GetFullUrl();
            urls.emplace_back(url);
            auto resource = std::dynamic_pointer_cast<FhirMedicationStatement>(entry.GetResource());
            if (!resource) {
                continue;
            }
            std::string prescriptionId{};
            prescriptionId = PrescriptionChangesService::GetPrescriptionId(*resource);
            if (!prescriptionId.empty()) {
                prescriptionIds.emplace_back(prescriptionId);
            }
        }
    }
    std::vector<FhirBundleEntry> add{};
    std::vector<FhirReference> medicationSectionReferencesInTheOther{};
    {
        auto entries = otherBundle->GetEntries();
        for (const auto &entry: entries) {
            auto entryResource = entry.GetResource();
            auto resource = std::dynamic_pointer_cast<FhirMedicationStatement>(entryResource);
            if (!resource) {
                auto compositionResource = std::dynamic_pointer_cast<FhirComposition>(entryResource);
                if (compositionResource) {
                    auto sections = compositionResource->GetSections();
                    for (auto &section: sections) {
                        auto sectionCoding = section.GetCode().GetCoding();
                        if (sectionCoding.empty() || sectionCoding[0].GetCode() != "sectionMedication") {
                            continue;
                        }
                        medicationSectionReferencesInTheOther = section.GetEntries();
                    }
                }
                continue;
            }
            auto url = entry.GetFullUrl();
            {
                bool found{false};
                for (const auto &u: urls) {
                    if (u == url) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    continue;
                }
            }
            {
                std::string prescriptionId{};
                if (PrescriptionChangesService::IsRenewedWithoutChanges(*resource)) {
                    prescriptionId = PrescriptionChangesService::GetPreviousPrescriptionId(*resource);
                } else {
                    prescriptionId = PrescriptionChangesService::GetPrescriptionId(*resource);
                }
                bool found{false};
                for (const auto &pid: prescriptionIds) {
                    if (pid == prescriptionId) {
                        found = true;
                        break;
                    }
                }
                if (found) {
                    continue;
                }
            }
            urls.emplace_back(url);
            add.emplace_back(entry);
        }
    }
    if (!add.empty()) {
        FhirReference prescriberRef{};
        {
            auto prescriberRefData = GetPrescriber(helseidIdToken);
            prescriberRef = FhirReference(prescriberRefData.uuid, "http://ehelse.no/fhir/StructureDefinition/sfm-Practitioner", prescriberRefData.name);
        }
        auto subjectRef = GetSubjectRef();
        std::vector<std::string> practiotionerIds{};
        {
            auto practitioners = GetPractitioners(medBundle);
            for (const auto &practitioner : practitioners) {
                practiotionerIds.emplace_back(practitioner.GetFullUrl());
            }
        }
        auto entries = medBundle->GetEntries();
        for (const auto &prevPrac : previousPractitioners) {
            auto id = prevPrac.GetFullUrl();
            bool found{false};
            for (const auto exId : practiotionerIds) {
                if (exId == id) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                practiotionerIds.emplace_back(id);
                entries.emplace_back(prevPrac);
            }
        }
        for (const auto &entry: add) {
            auto resource = std::dynamic_pointer_cast<FhirMedicationStatement>(entry.GetResource());
            if (resource) {
                auto extensions = resource->GetExtensions();
                for (const auto &extension : extensions) {
                    auto url = extension->GetUrl();
                    std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) {return std::tolower(ch);});
                    if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reginfo") {
                        auto extensions = extension->GetExtensions();
                        for (const auto &extension : extensions) {
                            auto url = extension->GetUrl();
                            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) {return std::tolower(ch);});
                            if (url == "provider") {
                                auto refExt = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                                if (refExt) {
                                    auto ref = std::dynamic_pointer_cast<FhirReference>(refExt->GetValue());
                                    if (ref) {
                                        if (ref->GetReference() == previousPrescriberRef) {
                                            refExt->SetValue(std::make_shared<FhirReference>(prescriberRef));
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                resource->SetSubject(subjectRef);
            }
            entries.emplace_back(entry);
        }
        for (const auto &entry : entries) {
            auto resource = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
            if (!resource) {
                continue;
            }
            auto sections = resource->GetSections();
            for (auto &section : sections) {
                auto sectionCoding = section.GetCode().GetCoding();
                if (sectionCoding.empty() || sectionCoding[0].GetCode() != "sectionMedication") {
                    continue;
                }
                auto medicationSectionEntries = section.GetEntries();
                for (const auto &medicationSectionEntry : medicationSectionReferencesInTheOther) {
                    bool found{false};
                    for (const auto &added : add) {
                        auto url = added.GetFullUrl();
                        if (url == medicationSectionEntry.GetReference()) {
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        medicationSectionEntries.emplace_back(medicationSectionEntry);
                    }
                }
                section.SetEntries(medicationSectionEntries);
            }
            resource->SetSections(sections);
        }
        medBundle->SetEntries(entries);
    }
}

void MedBundleData::ReplayRenewals(const std::shared_ptr<FhirBundle> &otherBundle) {
    auto renewals = GetRenewals(otherBundle);
    auto entries = medBundle->GetEntries();
    for (const auto &entry : entries) {
        auto medicationStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(entry.GetResource());
        if (medicationStatement) {
            std::string prescriptionId = PrescriptionChangesService::GetPrescriptionId(*medicationStatement);
            if (prescriptionId.empty()) {
                continue;
            }
            bool renewed{false};
            for (const auto &id : renewals) {
                if (prescriptionId == id) {
                    renewed = true;
                    break;
                }
            }
            if (renewed) {
                PrescriptionChangesService::Renew(*medicationStatement);
            }
        }
    }
}

struct RenewStatement {
    std::shared_ptr<FhirMedicationStatement> renewStatement{};
    std::string renewUrl{};
};

struct RenewBasic {
    std::shared_ptr<FhirBasic> renewBasic{};
    std::string renewUrl{};
};

class MedicationSectionInList {
private:
    std::vector<FhirCompositionSection> sections;
    FhirCompositionSection *medicationSection{nullptr};
public:
    MedicationSectionInList() : sections(), medicationSection(nullptr) {}
    MedicationSectionInList(const std::vector<FhirCompositionSection> &sections) : sections(sections) {
        FindMedicationSection();
    }
    constexpr MedicationSectionInList(const MedicationSectionInList &cp) : sections(cp.sections) {
        FindMedicationSection();
    }
    constexpr MedicationSectionInList(MedicationSectionInList &&mv) noexcept : sections(std::move(mv.sections)) {
        ForceFindMedicationSection();
    }
    constexpr MedicationSectionInList &operator =(const MedicationSectionInList &cp) {
        if (&cp == this) {
            return *this;
        }
        sections = cp.sections;
        FindMedicationSection();
        return *this;
    }
    constexpr MedicationSectionInList &operator =(MedicationSectionInList &&mv) noexcept {
        if (&mv == this) {
            return *this;
        }
        sections = std::move(mv.sections);
        ForceFindMedicationSection();
        return *this;
    }
    constexpr void FindMedicationSection() {
        medicationSection = nullptr;
        for (auto &section : this->sections) {
            auto coding = section.GetCode().GetCoding();
            if (coding.size() == 1 && coding[0].GetCode() == "sectionMedication") {
                if (medicationSection != nullptr) {
                    throw MedBundleDataException("Duplicate medication section");
                }
                medicationSection = &section;
            }
        }
    }
    constexpr void ForceFindMedicationSection() noexcept {
        medicationSection = nullptr;
        for (auto &section : this->sections) {
            auto coding = section.GetCode().GetCoding();
            if (coding.size() == 1 && coding[0].GetCode() == "sectionMedication") {
                medicationSection = &section;
            }
        }
    }
    constexpr operator bool () const {
        return medicationSection != nullptr;
    }
    [[nodiscard]] std::vector<FhirReference> GetEntries() const {
        if (medicationSection == nullptr) {
            return {};
        }
        return medicationSection->GetEntries();
    }
    [[nodiscard]] constexpr std::vector<FhirCompositionSection> GetSections() const {
        return sections;
    }
    void ClearEmptyReason() {
        if (medicationSection == nullptr) {
            return;
        }
        medicationSection->SetTextStatus("generated");
        medicationSection->SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\">List of medications</xhtml:div>");
        medicationSection->SetEmptyReason({});
    }
    void SetEntries(const std::vector<FhirReference> &entries) {
        medicationSection->SetEntries(entries);
    }
};

class OtherPrescriptionsSectionInList {
private:
    std::vector<FhirCompositionSection> sections;
    FhirCompositionSection *otherPrescriptionsSection{nullptr};
public:
    OtherPrescriptionsSectionInList() : sections(), otherPrescriptionsSection(nullptr) {}
    OtherPrescriptionsSectionInList(const std::vector<FhirCompositionSection> &sections) : sections(sections) {
        FindOtherPrescriptionsSection();
    }
    constexpr OtherPrescriptionsSectionInList(const OtherPrescriptionsSectionInList &cp) : sections(cp.sections) {
        FindOtherPrescriptionsSection();
    }
    constexpr OtherPrescriptionsSectionInList(OtherPrescriptionsSectionInList &&mv) noexcept : sections(std::move(mv.sections)) {
        ForceFindOtherPrescriptionsSection();
    }
    constexpr OtherPrescriptionsSectionInList &operator =(const OtherPrescriptionsSectionInList &cp) {
        if (&cp == this) {
            return *this;
        }
        sections = cp.sections;
        FindOtherPrescriptionsSection();
        return *this;
    }
    constexpr OtherPrescriptionsSectionInList &operator =(OtherPrescriptionsSectionInList &&mv) noexcept {
        if (&mv == this) {
            return *this;
        }
        sections = std::move(mv.sections);
        ForceFindOtherPrescriptionsSection();
        return *this;
    }
    constexpr void FindOtherPrescriptionsSection() {
        otherPrescriptionsSection = nullptr;
        for (auto &section : this->sections) {
            auto coding = section.GetCode().GetCoding();
            if (coding.size() == 1 && coding[0].GetCode() == "sectionOtherPrescriptions") {
                if (otherPrescriptionsSection != nullptr) {
                    throw MedBundleDataException("Duplicate other prescriptions section");
                }
                otherPrescriptionsSection = &section;
            }
        }
    }
    constexpr void ForceFindOtherPrescriptionsSection() noexcept {
        otherPrescriptionsSection = nullptr;
        for (auto &section : this->sections) {
            auto coding = section.GetCode().GetCoding();
            if (coding.size() == 1 && coding[0].GetCode() == "sectionOtherPrescriptions") {
                otherPrescriptionsSection = &section;
            }
        }
    }
    constexpr operator bool () const {
        return otherPrescriptionsSection != nullptr;
    }
    [[nodiscard]] std::vector<FhirReference> GetEntries() const {
        if (otherPrescriptionsSection == nullptr) {
            return {};
        }
        return otherPrescriptionsSection->GetEntries();
    }
    [[nodiscard]] constexpr std::vector<FhirCompositionSection> GetSections() const {
        return sections;
    }
    void ClearEmptyReason() {
        if (otherPrescriptionsSection == nullptr) {
            return;
        }
        otherPrescriptionsSection->SetTextStatus("generated");
        otherPrescriptionsSection->SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\">List of medications</xhtml:div>");
        otherPrescriptionsSection->SetEmptyReason({});
    }
    void SetEntries(const std::vector<FhirReference> &entries) {
        otherPrescriptionsSection->SetEntries(entries);
    }
};

class AllergySectionInList {
private:
    std::vector<FhirCompositionSection> sections;
    FhirCompositionSection *allergySection{nullptr};
public:
    AllergySectionInList() : sections(), allergySection(nullptr) {}
    AllergySectionInList(const std::vector<FhirCompositionSection> &sections) : sections(sections) {
        FindAllergySection();
    }
    constexpr AllergySectionInList(const AllergySectionInList &cp) : sections(cp.sections) {
        FindAllergySection();
    }
    constexpr AllergySectionInList(AllergySectionInList &&mv) noexcept : sections(std::move(mv.sections)) {
        ForceFindAllergySection();
    }
    constexpr AllergySectionInList &operator =(const AllergySectionInList &cp) {
        if (&cp == this) {
            return *this;
        }
        sections = cp.sections;
        FindAllergySection();
        return *this;
    }
    constexpr AllergySectionInList &operator =(AllergySectionInList &&mv) noexcept {
        if (&mv == this) {
            return *this;
        }
        sections = std::move(mv.sections);
        ForceFindAllergySection();
        return *this;
    }
    constexpr void FindAllergySection() {
        allergySection = nullptr;
        for (auto &section : this->sections) {
            auto coding = section.GetCode().GetCoding();
            if (coding.size() == 1 && coding[0].GetCode() == "sectionAllergies") {
                if (allergySection != nullptr) {
                    throw MedBundleDataException("Duplicate allergy section");
                }
                allergySection = &section;
            }
        }
    }
    constexpr void ForceFindAllergySection() noexcept {
        allergySection = nullptr;
        for (auto &section : this->sections) {
            auto coding = section.GetCode().GetCoding();
            if (coding.size() == 1 && coding[0].GetCode() == "sectionAllergies") {
                allergySection = &section;
            }
        }
    }
    constexpr operator bool () const {
        return allergySection != nullptr;
    }
    [[nodiscard]] std::vector<FhirReference> GetEntries() const {
        if (allergySection == nullptr) {
            return {};
        }
        return allergySection->GetEntries();
    }
    [[nodiscard]] constexpr std::vector<FhirCompositionSection> GetSections() const {
        return sections;
    }
    void ClearEmptyReason() {
        if (allergySection == nullptr) {
            return;
        }
        allergySection->SetTextStatus("generated");
        allergySection->SetTextXhtml("<xhtml:div xmlns:xhtml=\"http://www.w3.org/1999/xhtml\">List of medications</xhtml:div>");
        allergySection->SetEmptyReason({});
    }
    void SetEntries(const std::vector<FhirReference> &entries) {
        allergySection->SetEntries(entries);
    }
};

void MedBundleData::Prescribe(const std::vector<FhirBundleEntry> &medicament, const PrescriptionData &prescriptionData, const std::string &renewPrescriptionId, const std::string &pllId) {
    std::shared_ptr<FhirComposition> composition{};
    std::vector<RenewStatement> renewStatements{};
    for (const auto &entry : medBundle->GetEntries()) {
        if (!renewPrescriptionId.empty() || !pllId.empty()) {
            auto medicationStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(entry.GetResource());
            if (medicationStatement) {
                auto identifiers = medicationStatement->GetIdentifiers();
                if (!renewPrescriptionId.empty() && std::find_if(identifiers.cbegin(), identifiers.cend(), [&renewPrescriptionId] (const FhirIdentifier &identifier) -> bool {
                    auto type = identifier.GetType().GetText();
                    std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                    return (type == "reseptid" && identifier.GetValue() == renewPrescriptionId);
                }) != identifiers.cend()) {
                    RenewStatement renewStatement{.renewStatement = medicationStatement, .renewUrl = entry.GetFullUrl()};
                    renewStatements.emplace_back(renewStatement);
                } else if (!pllId.empty() && std::find_if(identifiers.cbegin(), identifiers.cend(), [&pllId] (const FhirIdentifier &identifier) -> bool {
                    auto type = identifier.GetType().GetText();
                    std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                    return (type == "pll" && identifier.GetValue() == pllId);
                }) != identifiers.cend()) {
                    RenewStatement renewStatement{.renewStatement = medicationStatement, .renewUrl = entry.GetFullUrl()};
                    renewStatements.emplace_back(renewStatement);
                }
            }
        }
        auto compositionObject = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
        if (compositionObject) {
            if (composition) {
                throw MedBundleDataException("Duplicate composition objects in bundle");
            }
            composition = compositionObject;
        }
    }
    if (!composition) {
        throw MedBundleDataException("Missing composition object in bundle");
    }
    {
        auto profile = composition->GetProfile();
        if (profile.size() != 1 ||
            profile[0] != "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationComposition") {
            throw MedBundleDataException("Composition with wrong profile");
        }
    }
    MedicationSectionInList medicationSectionInList{composition->GetSections()};
    if (!medicationSectionInList) {
        throw MedBundleDataException("No medication section");
    }
    auto entries = medicationSectionInList.GetEntries();
    FhirReference *renewReference{nullptr};
    std::shared_ptr<FhirMedicationStatement> renewMedicationStatement{};
    std::string renewFullUrl{};
    if (!renewPrescriptionId.empty() || !pllId.empty()) {
        for (auto &potRenewStatement: renewStatements) {
            bool found{false};
            for (auto &entry: entries) {
                if (potRenewStatement.renewUrl == entry.GetReference()) {
                    if (renewReference != nullptr) {
                        throw MedBundleDataException("Multiple references found");
                    }
                    renewReference = &entry;
                    renewFullUrl = potRenewStatement.renewUrl;
                    renewMedicationStatement = potRenewStatement.renewStatement;
                }
            }
        }
        if (renewReference == nullptr) {
            throw MedBundleDataException("Prescription to renew is not head of chain");
        }
    }
    std::shared_ptr<FhirMedicationStatement> medicationStatement = std::make_shared<FhirMedicationStatement>(prescriptionData.ToFhir());
    if (!medicament.empty()) {
        auto lastMedicament = medicament.back();
        auto medicationProfile = lastMedicament.GetResource()->GetProfile();
        FhirReference medicationReference{lastMedicament.GetFullUrl(), medicationProfile.size() == 1 ? medicationProfile[0] : "", lastMedicament.GetResource()->GetDisplay()};
        medicationStatement->SetMedicationReference(medicationReference);
    }
    std::string medicationStatementFullUrl{"urn:uuid:"};
    medicationStatementFullUrl.append(medicationStatement->GetId());
    FhirBundleEntry medicationStatementEntry{medicationStatementFullUrl, medicationStatement};
    if (renewMedicationStatement) {
        std::shared_ptr<FhirExtension> reseptAmendment{};
        for (const auto &extension : medicationStatement->GetExtensions()) {
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return std::tolower(ch); });
            if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
                if (reseptAmendment) {
                    throw MedBundleDataException("Multiple resept amendment");
                }
                reseptAmendment = extension;
            }
        }
        if (!reseptAmendment) {
            throw MedBundleDataException("No resept amendment for new prescription");
        }
        std::shared_ptr<FhirExtension> renewReseptAmendment{};
        for (const auto &extension : renewMedicationStatement->GetExtensions()) {
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return std::tolower(ch); });
            if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
                if (renewReseptAmendment) {
                    throw MedBundleDataException("Multiple resept amendment");
                }
                renewReseptAmendment = extension;
            }
        }
        if (!renewReseptAmendment) {
            throw MedBundleDataException("No resept amendment for prescription to renew");
        }
        std::string previousPrescriptionId{};
        std::string previousPllId{};
        std::vector<FhirIdentifier> oldIdentifiers;
        {
            // TODO - pllId from baseOn-chain
            oldIdentifiers = renewMedicationStatement->GetIdentifiers();
            auto iterator = oldIdentifiers.begin();
            while (iterator != oldIdentifiers.end()) {
                const auto &identifier = *iterator;
                auto type = identifier.GetType().GetText();
                std::transform(type.cbegin(), type.cend(), type.begin(), [](char ch) { return std::tolower(ch); });
                if (type == "reseptid") {
                    previousPrescriptionId = identifier.GetValue();
                } else if (type == "pll") {
                    previousPllId = identifier.GetValue();
                    iterator = oldIdentifiers.erase(iterator);
                    continue;
                }
                ++iterator;
            }
            if (previousPrescriptionId.empty() && !previousPllId.empty()) {
                renewMedicationStatement->SetIdentifiers(oldIdentifiers);
                auto identifiers = medicationStatement->GetIdentifiers();
                identifiers.insert(identifiers.begin(), FhirIdentifier(FhirCodeableConcept("PLL"), "usual", previousPllId));
                medicationStatement->SetIdentifiers(identifiers);
            }
        }
        bool recallPrevious{!previousPrescriptionId.empty()};
        if (recallPrevious) {
            for (const auto &extension: renewReseptAmendment->GetExtensions()) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [](char ch) -> char { return std::tolower(ch); });
                if (url == "recallinfo") {
                    recallPrevious = false;
                    break;
                }
            }
        }
        if (recallPrevious) {
            FhirCodeableConcept recallCode{"urn:oid:2.16.578.1.12.4.1.1.7500", "3", "Fornying med endring"};
            auto recallInfoExt = std::make_shared<FhirExtension>("recallinfo");
            recallInfoExt->AddExtension(
                    std::make_shared<FhirValueExtension>("recallId",
                                                         std::make_shared<FhirString>(previousPrescriptionId)));
            recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("recallcode",
                                                                             std::make_shared<FhirCodeableConceptValue>(
                                                                                     recallCode)));
            recallInfoExt->AddExtension(
                    std::make_shared<FhirValueExtension>("text", std::make_shared<FhirString>("Forny med endring")));
            recallInfoExt->AddExtension(
                    std::make_shared<FhirValueExtension>("notsent", std::make_shared<FhirBooleanValue>(true)));
            reseptAmendment->AddExtension(recallInfoExt);
        }
    }
    for (const auto &medicamentEntry : medicament) {
        medBundle->AddEntry(medicamentEntry);
    }
    medBundle->AddEntry(medicationStatementEntry);
    if (renewReference == nullptr) {
        if (entries.empty()) {
            medicationSectionInList.ClearEmptyReason();
        }
        entries.emplace_back(medicationStatementFullUrl,
                             "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement",
                             medicationStatement->GetDisplay());
    } else {
        *renewReference = {medicationStatementFullUrl,
                           "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement",
                           medicationStatement->GetDisplay()};
        renewMedicationStatement->SetPartOf({*renewReference});
        medicationStatement->SetBasedOn({{renewFullUrl,
                                         "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement",
                                         renewMedicationStatement->GetDisplay()}});
    }
    medicationSectionInList.SetEntries(entries);
    composition->SetSections(medicationSectionInList.GetSections());
}

void MedBundleData::Prescribe(const MerchData &merchData, const std::string &renewPrescriptionId) {
    std::shared_ptr<FhirComposition> composition{};
    std::vector<RenewBasic> renewBasics{};
    for (const auto &entry : medBundle->GetEntries()) {
        if (!renewPrescriptionId.empty()) {
            auto fhirBasic = std::dynamic_pointer_cast<FhirBasic>(entry.GetResource());
            if (fhirBasic) {
                auto identifiers = fhirBasic->GetIdentifiers();
                if (std::find_if(identifiers.cbegin(), identifiers.cend(), [&renewPrescriptionId] (const FhirIdentifier &identifier) -> bool {
                    auto type = identifier.GetType().GetText();
                    std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                    return (type == "reseptid" && identifier.GetValue() == renewPrescriptionId);
                }) != identifiers.cend()) {
                    RenewBasic renewBasic{.renewBasic = fhirBasic, .renewUrl = entry.GetFullUrl()};
                    renewBasics.emplace_back(renewBasic);
                }
            }
        }
        auto compositionObject = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
        if (compositionObject) {
            if (composition) {
                throw MedBundleDataException("Duplicate composition objects in bundle");
            }
            composition = compositionObject;
        }
    }
    if (!composition) {
        throw MedBundleDataException("Missing composition object in bundle");
    }
    {
        auto profile = composition->GetProfile();
        if (profile.size() != 1 ||
            profile[0] != "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationComposition") {
            throw MedBundleDataException("Composition with wrong profile");
        }
    }
    OtherPrescriptionsSectionInList otherPrescriptionSectionInList{composition->GetSections()};
    if (!otherPrescriptionSectionInList) {
        throw MedBundleDataException("No other prescriptions section");
    }

    auto entries = otherPrescriptionSectionInList.GetEntries();
    FhirReference *renewReference{nullptr};
    std::shared_ptr<FhirBasic> renewBasic{};
    std::string renewFullUrl{};
    if (!renewPrescriptionId.empty()) {
        for (auto &potRenewBasic: renewBasics) {
            bool found{false};
            for (auto &entry: entries) {
                if (potRenewBasic.renewUrl == entry.GetReference()) {
                    if (renewReference != nullptr) {
                        throw MedBundleDataException("Multiple references found");
                    }
                    renewReference = &entry;
                    renewFullUrl = potRenewBasic.renewUrl;
                    renewBasic = potRenewBasic.renewBasic;
                }
            }
        }
        if (renewReference == nullptr) {
            throw MedBundleDataException("Prescription to renew is not head of chain");
        }
    }
    std::shared_ptr<FhirBasic> fhirBasic = std::make_shared<SfmBandaPrescription>(merchData.ToFhir());
    std::string fhirBasicFullUrl{"urn:uuid:"};
    fhirBasicFullUrl.append(fhirBasic->GetId());
    FhirBundleEntry fhirBasicEntry{fhirBasicFullUrl, fhirBasic};
    if (renewBasic) {
        std::shared_ptr<FhirExtension> reseptAmendment{};
        for (const auto &extension : fhirBasic->GetExtensions()) {
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return std::tolower(ch); });
            if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
                if (reseptAmendment) {
                    throw MedBundleDataException("Multiple resept amendment");
                }
                reseptAmendment = extension;
            }
        }
        if (!reseptAmendment) {
            throw MedBundleDataException("No resept amendment for new prescription");
        }
        std::shared_ptr<FhirExtension> renewReseptAmendment{};
        for (const auto &extension : renewBasic->GetExtensions()) {
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return std::tolower(ch); });
            if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
                if (renewReseptAmendment) {
                    throw MedBundleDataException("Multiple resept amendment");
                }
                renewReseptAmendment = extension;
            }
        }
        if (!renewReseptAmendment) {
            throw MedBundleDataException("No resept amendment for prescription to renew");
        }
        for (const auto &extension : renewReseptAmendment->GetExtensions()) {
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return std::tolower(ch); });
            if (url == "recallinfo") {
                throw MedBundleDataException("The prescription is already recalled");
            }
        }
        FhirCodeableConcept recallCode{"urn:oid:2.16.578.1.12.4.1.1.7500", "3", "Fornying med endring"};
        auto recallInfoExt = std::make_shared<FhirExtension>("recallinfo");
        recallInfoExt->AddExtension(
                std::make_shared<FhirValueExtension>("recallId", std::make_shared<FhirString>(renewPrescriptionId)));
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("recallcode", std::make_shared<FhirCodeableConceptValue>(recallCode)));
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("text", std::make_shared<FhirString>("Forny med endring")));
        recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("notsent", std::make_shared<FhirBooleanValue>(true)));
        reseptAmendment->AddExtension(recallInfoExt);
    }
    medBundle->AddEntry(fhirBasicEntry);
    if (renewReference == nullptr) {
        if (entries.empty()) {
            otherPrescriptionSectionInList.ClearEmptyReason();
        }
        entries.emplace_back(fhirBasicFullUrl,
                             "http://ehelse.no/fhir/StructureDefinition/sfm-BandaPrescription",
                             fhirBasic->GetDisplay());
    } else {
        *renewReference = {fhirBasicFullUrl,
                           "http://ehelse.no/fhir/StructureDefinition/sfm-BandaPrescription",
                           fhirBasic->GetDisplay()};
    }
    otherPrescriptionSectionInList.SetEntries(entries);
    composition->SetSections(otherPrescriptionSectionInList.GetSections());
}

void MedBundleData::ConvertToWithoutPrescription(const std::string &prescriptionId, bool recallPrescription) {
    if (prescriptionId.empty()) {
        throw MedBundleDataException("Missing prescription id");
    }
    std::shared_ptr<FhirComposition> composition{};
    std::vector<RenewStatement> renewStatements{};
    for (const auto &entry : medBundle->GetEntries()) {
        auto medicationStatement = std::dynamic_pointer_cast<FhirMedicationStatement>(entry.GetResource());
        if (medicationStatement) {
            auto identifiers = medicationStatement->GetIdentifiers();
            if (std::find_if(identifiers.cbegin(), identifiers.cend(), [&prescriptionId] (const FhirIdentifier &identifier) -> bool {
                auto type = identifier.GetType().GetText();
                std::transform(type.cbegin(), type.cend(), type.begin(), [] (char ch) -> char { return static_cast<char>(std::tolower(ch)); });
                return (type == "reseptid" && identifier.GetValue() == prescriptionId);
            }) != identifiers.cend()) {
                RenewStatement renewStatement{.renewStatement = medicationStatement, .renewUrl = entry.GetFullUrl()};
                renewStatements.emplace_back(renewStatement);
            }
        }
        auto compositionObject = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
        if (compositionObject) {
            if (composition) {
                throw MedBundleDataException("Duplicate composition objects in bundle");
            }
            composition = compositionObject;
        }
    }
    if (!composition) {
        throw MedBundleDataException("Missing composition object in bundle");
    }
    {
        auto profile = composition->GetProfile();
        if (profile.size() != 1 ||
            profile[0] != "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationComposition") {
            throw MedBundleDataException("Composition with wrong profile");
        }
    }
    MedicationSectionInList medicationSectionInList{composition->GetSections()};
    if (!medicationSectionInList) {
        throw MedBundleDataException("No medication section");
    }
    auto entries = medicationSectionInList.GetEntries();
    FhirReference *renewReference{nullptr};
    std::shared_ptr<FhirMedicationStatement> renewMedicationStatement{};
    std::string renewFullUrl{};
    for (auto &potRenewStatement: renewStatements) {
        bool found{false};
        for (auto &entry: entries) {
            if (potRenewStatement.renewUrl == entry.GetReference()) {
                if (renewReference != nullptr) {
                    throw MedBundleDataException("Multiple references found");
                }
                renewReference = &entry;
                renewFullUrl = potRenewStatement.renewUrl;
                renewMedicationStatement = potRenewStatement.renewStatement;
            }
        }
    }
    if (renewReference == nullptr) {
        throw MedBundleDataException("Prescription to renew is not head of chain");
    }
    if (!renewMedicationStatement) {
        throw MedBundleDataException("Prescription to renew is not head of chain");
    }
    auto recallIdentifiers = renewMedicationStatement->GetIdentifiers();
    {
        auto iterator = recallIdentifiers.begin();
        while (iterator != recallIdentifiers.end()) {
            if (iterator->GetType().GetText() == "PLL") {
                iterator = recallIdentifiers.erase(iterator);
            } else {
                ++iterator;
            }
        }
    }
    auto medicationStatement = std::make_shared<FhirMedicationStatement>(FhirMedicationStatement::ParseJson(renewMedicationStatement->ToJson()));
    auto newIdentifiers = medicationStatement->GetIdentifiers();
    bool replacing{false};
    if (recallIdentifiers.empty()) {
        recallIdentifiers = newIdentifiers;
        medicationStatement = renewMedicationStatement;
        recallPrescription = false;
        replacing = true;
    } else {
        auto iterator = newIdentifiers.begin();
        while (iterator != newIdentifiers.end()) {
            if (iterator->GetType().GetText() != "PLL") {
                iterator = newIdentifiers.erase(iterator);
            } else {
                ++iterator;
            }
        }
        if (newIdentifiers.empty()) {
            newIdentifiers.emplace_back(FhirIdentifier(FhirCodeableConcept("PLL"), "usual", Uuid::RandomUuidString()));
        }
    }
    {
        std::shared_ptr<FhirExtension> reseptAmendment{};
        for (const auto &extension : medicationStatement->GetExtensions()) {
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return std::tolower(ch); });
            if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
                if (reseptAmendment) {
                    throw MedBundleDataException("Multiple resept amendment");
                }
                reseptAmendment = extension;
            }
        }
        if (!reseptAmendment) {
            throw MedBundleDataException("No resept amendment for new prescription");
        }
        std::shared_ptr<FhirExtension> renewReseptAmendment{};
        for (const auto &extension : renewMedicationStatement->GetExtensions()) {
            auto url = extension->GetUrl();
            std::transform(url.cbegin(), url.cend(), url.begin(), [] (char ch) -> char { return std::tolower(ch); });
            if (url == "http://ehelse.no/fhir/structuredefinition/sfm-reseptamendment") {
                if (renewReseptAmendment) {
                    throw MedBundleDataException("Multiple resept amendment");
                }
                renewReseptAmendment = extension;
            }
        }
        if (!renewReseptAmendment) {
            throw MedBundleDataException("No resept amendment for prescription to renew");
        }
        if (recallPrescription) {
            for (const auto &extension: renewReseptAmendment->GetExtensions()) {
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [](char ch) -> char { return std::tolower(ch); });
                if (url == "recallinfo") {
                    throw MedBundleDataException("The prescription is already recalled");
                }
            }
            FhirCodeableConcept recallCode{"urn:oid:2.16.578.1.12.4.1.1.7500", "4", "Annen"};
            auto recallInfoExt = std::make_shared<FhirExtension>("recallinfo");
            recallInfoExt->AddExtension(std::make_shared<FhirValueExtension>("recallcode",
                                                                             std::make_shared<FhirCodeableConceptValue>(
                                                                                     recallCode)));
            recallInfoExt->AddExtension(
                    std::make_shared<FhirValueExtension>("text", std::make_shared<FhirString>("Endret til legemiddelbehandling uten resept")));
            recallInfoExt->AddExtension(
                    std::make_shared<FhirValueExtension>("notsent", std::make_shared<FhirBooleanValue>(true)));
            renewReseptAmendment->AddExtension(recallInfoExt);
        }
        std::shared_ptr<FhirValueExtension> typeResept{};
        {
            auto extensions = reseptAmendment->GetExtensions();
            auto iterator = extensions.begin();
            bool modified{false};
            while (iterator != extensions.end()) {
                const auto &extension = *iterator;
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [](char ch) -> char { return std::tolower(ch); });
                if (url == "rfstatus") {
                    iterator = extensions.erase(iterator);
                    modified = true;
                    continue;
                }
                ++iterator;
            }
            if (modified) {
                reseptAmendment->SetExtensions(extensions);
            }
        }
        {
            auto extensions = reseptAmendment->GetExtensions();
            auto iterator = extensions.begin();
            bool modified{false};
            while (iterator != extensions.end()) {
                const auto &extension = *iterator;
                auto url = extension->GetUrl();
                std::transform(url.cbegin(), url.cend(), url.begin(), [](char ch) -> char { return std::tolower(ch); });
                if (url == "typeresept") {
                    auto valueExtension = std::dynamic_pointer_cast<FhirValueExtension>(extension);
                    if (valueExtension) {
                        typeResept = valueExtension;
                        break;
                    } else {
                        iterator = extensions.erase(iterator);
                        modified = true;
                        break;
                    }
                }
                ++iterator;
            }
            auto codeableConcept = std::make_shared<FhirCodeableConceptValue>(FhirCodeableConcept("urn:oid:2.16.578.1.12.4.1.1.7491", "U", "Uten resept"));
            if (typeResept) {
                typeResept->SetValue(codeableConcept);
            } else {
                auto valueExtension = std::make_shared<FhirValueExtension>("typeresept", codeableConcept);
                typeResept = valueExtension;
                extensions.emplace_back(typeResept);
                modified = true;
            }
            if (modified) {
                reseptAmendment->SetExtensions(extensions);
            }
        }
    }
    if (!replacing) {
        renewMedicationStatement->SetIdentifiers(recallIdentifiers);
        medicationStatement->SetIdentifiers(newIdentifiers);
        medicationStatement->SetId(Uuid::RandomUuidString());
        std::string medicationStatementFullUrl{"urn:uuid:"};
        medicationStatementFullUrl.append(medicationStatement->GetId());
        FhirBundleEntry medicationStatementEntry{medicationStatementFullUrl, medicationStatement};
        medBundle->AddEntry(medicationStatementEntry);
        if (entries.empty()) {
            medicationSectionInList.ClearEmptyReason();
        }
        entries.emplace_back(medicationStatementFullUrl,
                             "http://ehelse.no/fhir/StructureDefinition/sfm-MedicationStatement",
                             medicationStatement->GetDisplay());
    } else {
        medicationStatement->SetIdentifiers(recallIdentifiers);
    }
    medicationSectionInList.SetEntries(entries);
    composition->SetSections(medicationSectionInList.GetSections());
}

void MedBundleData::AddCave(const std::shared_ptr<FhirAllergyIntolerance> &allergy) {
    std::shared_ptr<FhirComposition> composition{};
    for (const auto &entry : medBundle->GetEntries()) {
        auto compositionObject = std::dynamic_pointer_cast<FhirComposition>(entry.GetResource());
        if (compositionObject) {
            if (composition) {
                throw MedBundleDataException("Duplicate composition objects in bundle");
            }
            composition = compositionObject;
        }
    }
    if (!composition) {
        throw MedBundleDataException("Missing composition object in bundle");
    }
    AllergySectionInList allergySectionInList{composition->GetSections()};
    if (!allergySectionInList) {
        throw MedBundleDataException("No allergy section");
    }
    auto entries = allergySectionInList.GetEntries();
    std::string fullUrl{"urn:uuid:"};
    {
        fullUrl.append(allergy->GetId());
        FhirBundleEntry bundleEntry{fullUrl, allergy};
        medBundle->AddEntry(bundleEntry);
    }
    entries.emplace_back(fullUrl, "http://nhn.no/kj/fhir/StructureDefinition/KjAllergyIntolerance", "KjAllergyIntolerance");
    allergySectionInList.SetEntries(entries);
    composition->SetSections(allergySectionInList.GetSections());
}

void MedBundleData::DeleteCave(const std::shared_ptr<FhirAllergyIntolerance> &allergy) {
    std::shared_ptr<FhirComposition> composition{};
    std::string entryId{};
    {
        auto entries = medBundle->GetEntries();
        auto iterator = entries.begin();
        while (iterator != entries.end()) {
            const auto &entry = *iterator;
            auto resource = entry.GetResource();
            if (resource == allergy) {
                entryId = entry.GetFullUrl();
                iterator = entries.erase(iterator);
                continue;
            }
            auto compositionObject = std::dynamic_pointer_cast<FhirComposition>(resource);
            if (compositionObject) {
                if (composition) {
                    throw MedBundleDataException("Duplicate composition objects in bundle");
                }
                composition = compositionObject;
            }
            ++iterator;
        }
        if (entryId.empty()) {
            return;
        }
        medBundle->SetEntries(entries);
    }
    if (!composition) {
        throw MedBundleDataException("Missing composition object in bundle");
    }
    AllergySectionInList allergySectionInList{composition->GetSections()};
    if (!allergySectionInList) {
        throw MedBundleDataException("No allergy section");
    }
    auto entries = allergySectionInList.GetEntries();
    auto iterator = entries.begin();
    while (iterator != entries.end()) {
        if (iterator->GetReference() == entryId) {
            iterator = entries.erase(iterator);
            break;
        }
        ++iterator;
    }
    allergySectionInList.SetEntries(entries);
    composition->SetSections(allergySectionInList.GetSections());
}
