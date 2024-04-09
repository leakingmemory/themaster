//
// Created by sigsegv on 3/18/24.
//

#ifndef DRWHATSNOT_FESTDB_H
#define DRWHATSNOT_FESTDB_H

#include <string>
#include <vector>
#include <memory>

class FestDeserializer;
struct FestDbContainer;
class LegemiddelVirkestoff;
class LegemiddelMerkevare;
class Legemiddelpakning;
class FestUuid;
class POppfLegemiddelVirkestoff;
class PLegemiddelVirkestoff;

class FestDb {
    std::shared_ptr<FestDeserializer> festDeserializer{};
public:
    FestDb();
    bool IsOpen() const;
private:
    FestDbContainer GetActiveFestDb() const;
public:
    [[nodiscard]] std::vector<LegemiddelVirkestoff> FindLegemiddelVirkestoff(const std::vector<POppfLegemiddelVirkestoff> &oppfs, const std::string &term) const;
    [[nodiscard]] std::vector<LegemiddelVirkestoff> FindLegemiddelVirkestoff(const std::string &term) const;
    [[nodiscard]] std::vector<LegemiddelMerkevare> FindLegemiddelMerkevare(const std::string &term) const;
    [[nodiscard]] std::vector<Legemiddelpakning> FindLegemiddelpakning(const std::string &term) const;
    [[nodiscard]] LegemiddelVirkestoff GetLegemiddelVirkestoff(FestUuid) const;
    [[nodiscard]] LegemiddelVirkestoff GetLegemiddelVirkestoff(const PLegemiddelVirkestoff &packed) const;
    [[nodiscard]] LegemiddelMerkevare GetLegemiddelMerkevare(FestUuid) const;
    [[nodiscard]] Legemiddelpakning GetLegemiddelpakning(FestUuid ) const;
    [[nodiscard]] FestUuid GetVirkestoffForVirkestoffMedStyrkeId(FestUuid virkestoffMedStyrkeId) const;
    [[nodiscard]] std::vector<FestUuid> GetVirkestoffMedStyrkeForVirkestoffId(FestUuid virkestoffId);
    [[nodiscard]] std::vector<POppfLegemiddelVirkestoff> GetAllPLegemiddelVirkestoff() const;
    bool PLegemiddelVirkestoffHasOneOfMerkevare(const PLegemiddelVirkestoff &, const std::vector<FestUuid> &merkevareId);
    bool PLegemiddelVirkestoffHasOneOfPakning(const PLegemiddelVirkestoff &, const std::vector<FestUuid> &pakningId);
    FestUuid GetLegemiddelVirkestoffId(const PLegemiddelVirkestoff &);
};


#endif //DRWHATSNOT_FESTDB_H
