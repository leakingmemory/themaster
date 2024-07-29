//
// Created by sigsegv on 7/24/24.
//

#include <sfmbasisapi/fhir/medication.h>
#include <sfmbasisapi/fhir/medstatement.h>
#include <sfmbasisapi/fhir/composition.h>
#include <sfmbasisapi/fhir/person.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <jjwtid/Jwt.h>
#include <jjwtid/JwtPart.h>
#include "MedBundleData.h"
#include "PrescriptionChangesService.h"

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
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        practitioner->SetId(uuidStr);
    }
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
    {
        boost::uuids::random_generator generator;
        boost::uuids::uuid randomUUID = generator();
        std::string uuidStr = boost::uuids::to_string(randomUUID);
        patient->SetId(uuidStr);
    }
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
