//
// Created by sigsegv on 12/13/23.
//

#ifndef DRWHATSNOT_PATIENTSTOREINMEMORY_H
#define DRWHATSNOT_PATIENTSTOREINMEMORY_H

#include "PatientStore.h"

class PatientStoreInMemory : public PatientStore {
private:
    std::vector<PatientInformation> patients{};
public:
    std::vector<PatientInformation> FindPatients(const std::string &searchBy) override;
    void AddPatient(const PatientInformation &pi) override;
};


#endif //DRWHATSNOT_PATIENTSTOREINMEMORY_H
