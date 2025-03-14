//
// Created by sigsegv on 3/4/25.
//

#include "ICD10.h"
#include "MedicalCodedValue.h"
#include <ICD-10-codeset.gen.h>

static std::vector<MedicalCodedValue> GetICD10Codelist() {
    std::vector<MedicalCodedValue> cl{};
    for (size_t i = 0; i < __ICD_10_codeset_csv_size; i++) {
        auto &val = __ICD_10_codeset_csv[i];
        cl.emplace_back("2.16.578.1.12.4.1.1.7110", val.col0, val.col1, val.col2);
    }
    return cl;
}

static const std::vector<MedicalCodedValue> icd10Full = GetICD10Codelist();

const std::vector<MedicalCodedValue> &ICD10::GetFullCodelist() {
    return icd10Full;
}