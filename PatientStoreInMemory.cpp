//
// Created by sigsegv on 12/13/23.
//

#include "PatientStoreInMemory.h"

std::vector<PatientInformation> PatientStoreInMemory::FindPatients(const std::string &searchBy) {
    std::vector<PatientInformation> matches;
    for (const auto &p : patients) {
        auto patientId = p.GetPatientId();
        if (patientId.find(searchBy) != std::string::npos) {
            matches.emplace_back(p);
            continue;
        }
        auto familyName = p.GetFamilyName();
        if (familyName.find(searchBy) != std::string::npos) {
            matches.emplace_back(p);
            continue;
        }
        auto givenName = p.GetGivenName();
        if (givenName.find(searchBy) != std::string::npos) {
            matches.emplace_back(p);
            continue;
        }
        auto commaFam = familyName;
        commaFam.append(", ");
        commaFam.append(givenName);
        if (commaFam.find(searchBy) != std::string::npos) {
            matches.emplace_back(p);
            continue;
        }
        {
            auto g = givenName;
            givenName.append(" ");
            givenName.append(familyName);
            if (givenName.find(searchBy) != std::string::npos) {
                matches.emplace_back(p);
                continue;
            }
            familyName.append(" ");
            familyName.append(g);
        }
        if (familyName.find(searchBy) != std::string::npos) {
            matches.emplace_back(p);
            continue;
        }
    }
    return matches;
}

void PatientStoreInMemory::AddPatient(const PatientInformation &pi) {
    patients.emplace_back(pi);
}
