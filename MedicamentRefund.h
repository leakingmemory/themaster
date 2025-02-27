//
// Created by sigsegv on 2/14/25.
//

#ifndef THEMASTER_MEDICAMENTREFUND_H
#define THEMASTER_MEDICAMENTREFUND_H

#include "MedicalCodedValue.h"
#include <vector>

struct MedicamentRefund {
    MedicalCodedValue refund{};
    std::vector<MedicalCodedValue> codes{};
};

#endif //THEMASTER_MEDICAMENTREFUND_H
