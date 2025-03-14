//
// Created by sigsegv on 3/4/25.
//

#ifndef THEMASTER_ICD10_H
#define THEMASTER_ICD10_H

#include <vector>

class MedicalCodedValue;

class ICD10 {
public:
    static const std::vector<MedicalCodedValue> &GetFullCodelist();
};


#endif //THEMASTER_ICD10_H
