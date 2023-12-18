//
// Created by sigsegv on 12/18/23.
//

#ifndef DRWHATSNOT_MEDBUNDLEDATA_H
#define DRWHATSNOT_MEDBUNDLEDATA_H

#include <memory>
#include <sfmbasisapi/fhir/bundle.h>
#include <sfmbasisapi/fhir/value.h>

struct MedBundleData {
    std::shared_ptr<FhirBundle> medBundle{};
    std::string kjHentet{};
    std::string rfHentet{};
    FhirCoding kjFeilkode{};
    FhirCoding rfM96Feilkode{};
    FhirCoding rfM912Feilkode{};
    bool kjHarLegemidler{false};
    bool kjHarLaste{false};
    bool rfHarLaste{false};
};

#endif //DRWHATSNOT_MEDBUNDLEDATA_H
