//
// Created by sigsegv on 12/13/23.
//

#ifndef DRWHATSNOT_PATIENTSTORE_H
#define DRWHATSNOT_PATIENTSTORE_H

#include <string>
#include <vector>

enum class PatientIdType {
    NOT_SET,
    FODSELSNUMMER,
    DNUMMER
};

enum class PersonGender {
    FEMALE,
    MALE
};

class PatientInformation {
private:
    std::string patientId;
    std::string givenName;
    std::string familyName;
    std::string dateOfBirth;
    std::string postCode;
    std::string city;
    PatientIdType patientIdType;
    PersonGender gender;
public:
    // getter methods
    std::string GetPatientId() const { return patientId; }
    std::string GetGivenName() const { return givenName; }
    std::string GetFamilyName() const { return familyName; }
    std::string GetDateOfBirth() const { return dateOfBirth; }
    std::string GetPostCode() const { return postCode; }
    std::string GetCity() const { return city; }
    PatientIdType GetPatientIdType() const { return patientIdType; }
    PersonGender GetGender() const { return gender; }

    // setter methods
    void SetPatientId(const std::string& id) { patientId = id; }
    void SetGivenName(const std::string& name) { givenName = name; }
    void SetFamilyName(const std::string& name) { familyName = name; }
    void SetDateOfBirth(const std::string& dob) { dateOfBirth = dob; }
    void SetPostCode(const std::string& pcode) { postCode = pcode; }
    void SetCity(const std::string& _city) { city = _city; }
    void SetPatientIdType(PatientIdType idType) { patientIdType = idType; }
    void SetGender(PersonGender setGender) { gender = setGender; }
};

class PatientStore {
public:
    virtual ~PatientStore() = default;
    virtual std::vector<PatientInformation> FindPatients(const std::string &searchBy) = 0;
    virtual void AddPatient(const PatientInformation &) = 0;
};

#endif //DRWHATSNOT_PATIENTSTORE_H