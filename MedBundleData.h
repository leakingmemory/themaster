//
// Created by sigsegv on 12/18/23.
//

#ifndef DRWHATSNOT_MEDBUNDLEDATA_H
#define DRWHATSNOT_MEDBUNDLEDATA_H

#include <memory>
#include <sfmbasisapi/fhir/bundle.h>
#include <sfmbasisapi/fhir/value.h>
#include <vector>
#include <tuple>
#include "PatientStore.h"

struct PrescriberRef {
    std::string uuid;
    std::string name;
};

class MedBundleData {
public:
    PatientInformation patientInformation{};
    std::shared_ptr<FhirBundle> medBundle{};
    std::string kjHentet{};
    std::string rfHentet{};
    FhirCoding kjFeilkode{};
    FhirCoding rfM96Feilkode{};
    FhirCoding rfM912Feilkode{};
    bool kjHarLegemidler{false};
    bool kjHarLaste{false};
    bool rfHarLaste{false};
    [[nodiscard]] static std::vector<FhirBundleEntry> GetPractitioners(const std::shared_ptr<FhirBundle> &);
    [[nodiscard]] static PrescriberRef GetPrescriber(const std::shared_ptr<FhirBundle> &, const std::string &helseidIdToken);
    [[nodiscard]] PrescriberRef GetPrescriber(const std::string &helseidIdToken) const;
    [[nodiscard]] FhirReference GetSubjectRef() const ;
    void InsertNonexistingMedicationsFrom(const std::shared_ptr<FhirBundle> &otherBundle);
    void InsertNonexistingMedicationPrescriptionsFrom(const std::shared_ptr<FhirBundle> &otherBundle, const std::string &helseidIdToken);
};

#endif //DRWHATSNOT_MEDBUNDLEDATA_H
