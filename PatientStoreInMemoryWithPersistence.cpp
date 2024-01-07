//
// Created by sigsegv on 1/7/24.
//

#include "PatientStoreInMemoryWithPersistence.h"
#include "DataDirectory.h"
#include "Guard.h"
#include <nlohmann/json.hpp>
#include <sys/stat.h>

static std::vector<PatientInformation> ReadPatients() {
    std::vector<PatientInformation> patients{};
    auto patientsJson = DataDirectory::Data("themaster").ReadFile("patients.json");
    if (patientsJson.empty()) {
        return {};
    }
    auto json = nlohmann::json::parse(patientsJson);
    if (json.is_array()) {
        for (const auto &patientJson : json) {
            if (!patientJson.is_object()) {
                continue;
            }
            std::string patientId = patientJson.value("patientId", "");
            std::string givenName = patientJson.value("givenName", "");
            std::string familyName = patientJson.value("familyName", "");
            std::string dateOfBirth = patientJson.value("dateOfBirth", "");
            std::string postCode = patientJson.value("postCode", "");
            std::string city = patientJson.value("city", "");
            PatientIdType patientIdType{};
            {
                std::string patientIdTypeString = patientJson.value("patientIdType", "");
                if (patientIdTypeString == "fodselsnummer") {
                    patientIdType = PatientIdType::FODSELSNUMMER;
                } else if (patientIdTypeString == "dnummer") {
                    patientIdType = PatientIdType::DNUMMER;
                } else {
                    patientIdType = PatientIdType::NOT_SET;
                }
            }
            PersonGender gender{};
            {
                std::string genderString = patientJson.value("gender", "");
                if (genderString == "male") {
                    gender = PersonGender::MALE;
                } else {
                    gender = PersonGender::FEMALE;
                }
            }
            PatientInformation info{};
            info.SetPatientId(patientId);
            info.SetGivenName(givenName);
            info.SetFamilyName(familyName);
            info.SetDateOfBirth(dateOfBirth);
            info.SetPostCode(postCode);
            info.SetCity(city);
            info.SetPatientIdType(patientIdType);
            info.SetGender(gender);
            patients.push_back(info);
        }
    }
    return patients;
}

static void StorePatients(const std::vector<PatientInformation> &patients) {
    std::string jsonString{};
    {
        auto json = nlohmann::json::array();
        for (const auto &patient: patients) {
            auto pj = nlohmann::json::object();
            pj["patientId"] = patient.GetPatientId();
            pj["givenName"] = patient.GetGivenName();
            pj["familyName"] = patient.GetFamilyName();
            pj["dateOfBirth"] = patient.GetDateOfBirth();
            pj["postCode"] = patient.GetPostCode();
            pj["city"] = patient.GetCity();
            switch (patient.GetPatientIdType()) {
                case PatientIdType::FODSELSNUMMER:
                    pj["patientIdType"] = "fodselsnummer";
                    break;
                case PatientIdType::DNUMMER:
                    pj["patientIdType"] = "dnummer";
                    break;
                case PatientIdType::NOT_SET:
                    pj["patientIdType"] = "not_set";
                    break;
                default:
                    pj["patientIdType"] = "";
            }
            switch (patient.GetGender()) {
                case PersonGender::MALE:
                    pj["gender"] = "male";
                case PersonGender::FEMALE:
                    pj["gender"] = "female";
                default:
                    pj["gender"] = "";
            }
        }
        jsonString = json.dump();
    }
    auto prevMask = umask(00177);
    Guard resetMask{[prevMask]() { umask(prevMask); }};
    DataDirectory::Data("themaster").WriteFile("patients.json", jsonString);
}

PatientStoreInMemoryWithPersistence::PatientStoreInMemoryWithPersistence() {
    for (const auto &patient : ReadPatients()) {
        patientStore.AddPatient(patient);
    }
}

std::vector<PatientInformation> PatientStoreInMemoryWithPersistence::FindPatients(const std::string &searchBy) {
    return patientStore.FindPatients(searchBy);
}

void PatientStoreInMemoryWithPersistence::AddPatient(const PatientInformation &pi) {
    patientStore.AddPatient(pi);
    auto persistent = ReadPatients();
    persistent.push_back(pi);
    StorePatients(persistent);
}
