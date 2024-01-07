//
// Created by sigsegv on 1/7/24.
//

#ifndef DRWHATSNOT_PATIENTSTOREINMEMORYWITHPERSISTENCE_H
#define DRWHATSNOT_PATIENTSTOREINMEMORYWITHPERSISTENCE_H

#include "PatientStoreInMemory.h"

class PatientStoreInMemoryWithPersistence : public PatientStore {
private:
    PatientStoreInMemory patientStore{};
public:
    PatientStoreInMemoryWithPersistence();
    std::vector<PatientInformation> FindPatients(const std::string &searchBy) override;
    void AddPatient(const PatientInformation &pi) override;
};


#endif //DRWHATSNOT_PATIENTSTOREINMEMORYWITHPERSISTENCE_H
