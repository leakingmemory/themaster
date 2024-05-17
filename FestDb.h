//
// Created by sigsegv on 3/18/24.
//

#ifndef DRWHATSNOT_FESTDB_H
#define DRWHATSNOT_FESTDB_H

#include <string>
#include <vector>
#include <memory>
#include <map>

class FestDeserializer;
struct FestDbContainer;
class FestVectors;
class LegemiddelVirkestoff;
class LegemiddelMerkevare;
class Legemiddelpakning;
class FestUuid;
class POppfLegemiddelVirkestoff;
class PLegemiddelVirkestoff;
class OppfRefusjon;

class FestDb {
    std::shared_ptr<FestDeserializer> festDeserializer{};
public:
    FestDb();
    bool IsOpen() const;
private:
    [[nodiscard]] std::map<std::string, std::unique_ptr<FestVectors>> GetFestVersionMap() const;
    [[nodiscard]] FestDbContainer GetActiveFestDb() const;
    [[nodiscard]] FestDbContainer GetFestDb(const std::string &version) const;
public:
    [[nodiscard]] std::vector<std::string> GetFestVersions() const;
    [[nodiscard]] std::vector<LegemiddelVirkestoff> FindLegemiddelVirkestoff(const std::vector<POppfLegemiddelVirkestoff> &oppfs, const std::string &term) const;
    [[nodiscard]] std::vector<LegemiddelVirkestoff> FindLegemiddelVirkestoff(const std::string &term) const;
    [[nodiscard]] std::vector<LegemiddelMerkevare> FindLegemiddelMerkevare(const std::string &term) const;
    [[nodiscard]] std::vector<Legemiddelpakning> FindLegemiddelpakning(const std::string &term) const;
    [[nodiscard]] LegemiddelVirkestoff GetLegemiddelVirkestoff(FestUuid) const;
    [[nodiscard]] LegemiddelVirkestoff GetLegemiddelVirkestoff(const PLegemiddelVirkestoff &packed) const;
    [[nodiscard]] LegemiddelVirkestoff GetLegemiddelVirkestoffForMerkevare(FestUuid uuid) const;
    [[nodiscard]] LegemiddelMerkevare GetLegemiddelMerkevare(FestUuid) const;
    [[nodiscard]] Legemiddelpakning GetLegemiddelpakning(FestUuid ) const;
    [[nodiscard]] std::vector<Legemiddelpakning> GetLegemiddelpakningForMerkevare(FestUuid uuid) const;
    [[nodiscard]] FestUuid GetVirkestoffForVirkestoffMedStyrkeId(FestUuid virkestoffMedStyrkeId) const;
    [[nodiscard]] std::vector<FestUuid> GetVirkestoffMedStyrkeForVirkestoffId(FestUuid virkestoffId);
    [[nodiscard]] std::vector<POppfLegemiddelVirkestoff> GetAllPLegemiddelVirkestoff() const;
    bool PLegemiddelVirkestoffHasOneOfMerkevare(const PLegemiddelVirkestoff &, const std::vector<FestUuid> &merkevareId);
    bool PLegemiddelVirkestoffHasOneOfPakning(const PLegemiddelVirkestoff &, const std::vector<FestUuid> &pakningId);
    FestUuid GetLegemiddelVirkestoffId(const PLegemiddelVirkestoff &);
    [[nodiscard]] std::vector<OppfRefusjon> GetOppfRefusjon(const std::string &festVersion) const;
};


#endif //DRWHATSNOT_FESTDB_H
