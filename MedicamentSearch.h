//
// Created by sigsegv on 5/28/25.
//

#ifndef THEMASTER_MEDICAMENTSEARCH_H
#define THEMASTER_MEDICAMENTSEARCH_H

#include <memory>
#include <functional>
#include <map>
#include <vector>
#include "Lazy.h"
#include "MedicamentSearch.h"
#include <medfest/Struct/Decoded/LegemiddelVirkestoff.h>
#include <medfest/Struct/Decoded/LegemiddelMerkevare.h>
#include <medfest/Struct/Decoded/Legemiddelpakning.h>
#include <medfest/Struct/Packed/PReseptgyldighet.h>

class FestDb;
class FestUuid;

enum class FindMedicamentSelections {
    ALL = 0,
    TWO_OR_MORE_PRESCRIPTION_VALIDITY = 1
};

struct MedicamentSearchResult {
    std::vector<LegemiddelVirkestoff> legemiddelVirkestoffList{};
    std::vector<LegemiddelMerkevare> legemiddelMerkevareList{};
    std::vector<Legemiddelpakning> legemiddelpakningList{};
};

class MedicamentSearch {
private:
    std::shared_ptr<FestDb> festDb;
    Lazy<std::function<std::map<FestUuid,std::vector<PReseptgyldighet>> ()>> merkevareToPrescriptionValidity;
    Lazy<std::function<std::vector<FestUuid> ()>> merkevareWithTwoOrMoreReseptgyldighet;
    Lazy<std::function<std::vector<FestUuid> ()>> legemiddelVirkestoffWithTwoOrMoreReseptgyldighet;
    Lazy<std::function<std::vector<FestUuid> ()>> legemiddelpakningWithTwoOrMoreReseptgyldighet;
private:
    std::map<FestUuid, std::vector<PReseptgyldighet>> CreateMerkevareToPrescriptionValidity() const;
    std::vector<FestUuid> FindMerkevareWithTwoOrMoreReseptgyldighet();
    std::vector<FestUuid> FindLegemiddelVirkestoffWithTwoOrMoreReseptgyldighet();
    std::vector<FestUuid> FindLegemiddelpakningWithTwoOrMoreReseptgyldighet();
public:
    MedicamentSearch(const std::shared_ptr<FestDb> &festDb);
    std::shared_ptr<MedicamentSearchResult> PerformSearch(const std::string &term, FindMedicamentSelections selection);
};


#endif //THEMASTER_MEDICAMENTSEARCH_H
