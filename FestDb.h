//
// Created by sigsegv on 3/18/24.
//

#ifndef DRWHATSNOT_FESTDB_H
#define DRWHATSNOT_FESTDB_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <mutex>
#include <medfest/FestVectors.h>
#include <medfest/Struct/Packed/FestUuid.h>

class FestDeserializer;
struct FestDbContainer;
struct FestDbQuota;
class LegemiddelVirkestoff;
class LegemiddelMerkevare;
class Legemiddelpakning;
class VirkestoffMedStyrke;
class Virkestoff;
class POppfLegemiddelMerkevare;
class POppfLegemiddelVirkestoff;
class POppfLegemiddelpakning;
class PLegemiddelMerkevare;
class PLegemiddelpakning;
class PLegemiddelVirkestoff;
class PLegemiddel;
class PReseptgyldighet;
class OppfRefusjon;
class OppfLegemiddelMerkevare;
class OppfLegemiddelVirkestoff;
class OppfLegemiddelpakning;
class OppfLegemiddeldose;
class OppfKodeverk;
class OppfMedForbrMatr;
class OppfNaringsmiddel;
class Element;

template <class T> struct FestModified {
    T previous{};
    T latest{};
};

template <class T> struct FestDiff {
    std::vector<T> removed{};
    std::vector<T> added{};
    std::vector<FestModified<T>> modified{};
};

struct FestDbContainer {
    std::unique_ptr<FestVectors> festVectors{};
    std::map<FestUuid,FestUuid> virkestoffMedStyrkeToVirkestoff{};
    std::mutex mtx{};
};

class FestDb {
    std::shared_ptr<FestDeserializer> festDeserializer{};
    std::map<std::string,std::unique_ptr<FestDbContainer>> dbContainers{};
    std::mutex mtx{};
public:
    FestDb();
    bool IsOpen() const;
private:
    [[nodiscard]] std::map<std::string, std::unique_ptr<FestVectors>> GetFestVersionMap() const;
    [[nodiscard]] FestDbContainer &GetActiveFestDb();
    [[nodiscard]] FestDbContainer &GetFestDb(const std::string &version);
public:
    [[nodiscard]] std::vector<std::string> GetFestVersions() const;
    [[nodiscard]] std::vector<FestDbQuota> GetDbQuotas() const;
    [[nodiscard]] std::vector<LegemiddelVirkestoff> FindLegemiddelVirkestoff(const std::vector<POppfLegemiddelVirkestoff> &oppfs, const std::string &term) const;
    [[nodiscard]] std::vector<LegemiddelVirkestoff> FindLegemiddelVirkestoff(const std::string &term);
    [[nodiscard]] std::vector<LegemiddelMerkevare> FindLegemiddelMerkevare(const std::vector<POppfLegemiddelMerkevare> &oppfs, const std::string &term) const;
    [[nodiscard]] std::vector<LegemiddelMerkevare> FindLegemiddelMerkevare(const std::string &term);
    [[nodiscard]] std::vector<LegemiddelMerkevare> FindLegemiddelMerkevare(const std::vector<FestUuid> &);
    [[nodiscard]] std::vector<LegemiddelMerkevare> FindDilutionLegemiddelMerkevare();
    [[nodiscard]] std::vector<Legemiddelpakning> FindLegemiddelpakning(const std::vector<POppfLegemiddelpakning> &oppfs, const std::string &term) const;
    [[nodiscard]] std::vector<Legemiddelpakning> FindLegemiddelpakning(const std::string &term);
    [[nodiscard]] LegemiddelVirkestoff GetLegemiddelVirkestoff(FestUuid);
    [[nodiscard]] LegemiddelVirkestoff GetLegemiddelVirkestoff(const PLegemiddelVirkestoff &packed) const;
    [[nodiscard]] LegemiddelVirkestoff GetLegemiddelVirkestoffForMerkevare(FestUuid uuid);
    [[nodiscard]] LegemiddelMerkevare GetLegemiddelMerkevare(FestUuid);
    [[nodiscard]] LegemiddelMerkevare GetLegemiddelMerkevare(const PLegemiddelMerkevare &packed) const;
    [[nodiscard]] Legemiddelpakning GetLegemiddelpakning(const PLegemiddelpakning &) const;
    [[nodiscard]] Legemiddelpakning GetLegemiddelpakning(FestUuid );
    [[nodiscard]] Legemiddelpakning GetLegemiddelpakningByVarenr(const std::string &);
    [[nodiscard]] OppfKodeverk GetKodeverkById(const std::string &);
    [[nodiscard]] std::vector<Legemiddelpakning> GetLegemiddelpakningForMerkevare(FestUuid uuid);
    [[nodiscard]] VirkestoffMedStyrke GetVirkestoffMedStyrke(FestUuid uuid);
    [[nodiscard]] Virkestoff GetVirkestoff(FestUuid uuid);
    [[nodiscard]] FestUuid GetVirkestoffForVirkestoffMedStyrkeId(FestUuid virkestoffMedStyrkeId);
    [[nodiscard]] std::vector<FestUuid> GetVirkestoffMedStyrkeForVirkestoffId(FestUuid virkestoffId);
    [[nodiscard]] std::vector<POppfLegemiddelMerkevare> GetAllPLegemiddelMerkevare();
    [[nodiscard]] std::vector<POppfLegemiddelVirkestoff> GetAllPLegemiddelVirkestoff();
    [[nodiscard]] std::vector<POppfLegemiddelpakning> GetAllPLegemiddelpakning();
    bool PLegemiddelVirkestoffHasOneOfMerkevare(const PLegemiddelVirkestoff &, const std::vector<FestUuid> &merkevareId);
    bool PLegemiddelVirkestoffHasOneOfPakning(const PLegemiddelVirkestoff &, const std::vector<FestUuid> &pakningId);
    FestUuid GetLegemiddelMerkevareId(const PLegemiddelMerkevare &) const;
    std::vector<PReseptgyldighet> GetPReseptgyldighet(const PLegemiddelMerkevare &) const;
    FestUuid GetLegemiddelVirkestoffId(const PLegemiddelVirkestoff &) const;
    std::vector<FestUuid> GetRefMerkevare(const PLegemiddelVirkestoff &) const;
    std::vector<FestUuid> GetRefMerkevare(const PLegemiddelpakning &) const;
    FestUuid GetLegemiddelpakningId(const PLegemiddelpakning &) const;
    std::vector<FestUuid> GetSortertVirkestoffMedStyrke(const PLegemiddel &) const;
    std::vector<FestUuid> GetSortertVirkestoffUtenStyrke(const PLegemiddelMerkevare &) const;
    [[nodiscard]] std::vector<OppfRefusjon> GetOppfRefusjon(const std::string &festVersion);
    [[nodiscard]] std::vector<OppfLegemiddelMerkevare> GetOppfLegemiddelMerkevare(const std::string &festVersion);
    [[nodiscard]] std::vector<OppfLegemiddelVirkestoff> GetOppfLegemiddelVirkestoff(const std::string &festVersion);
    [[nodiscard]] std::vector<OppfLegemiddelpakning> GetOppfLegemiddelpakning(const std::string &festVersion);
    [[nodiscard]] std::vector<OppfLegemiddeldose> GetOppfLegemiddeldose(const std::string &festVersion);
    [[nodiscard]] std::vector<OppfKodeverk> GetOppfKodeverk(const std::string &festVersion);
    [[nodiscard]] std::vector<Element> GetKodeverkElements(const std::string &kodeverkId, const std::string &festVersion);
    [[nodiscard]] std::vector<OppfMedForbrMatr> GetOppfMedForbrMatr(const std::string &festVersion);
    [[nodiscard]] std::vector<OppfNaringsmiddel> GetOppfNaringsmiddel(const std::string &festVersion);
    [[nodiscard]] FestDiff<OppfRefusjon> GetOppfRefusjonDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion);
    [[nodiscard]] FestDiff<OppfLegemiddelMerkevare> GetOppfLegemiddelMerkevareDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion);
    [[nodiscard]] FestDiff<OppfLegemiddelVirkestoff> GetOppfLegemiddelVirkestoffDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion);
    [[nodiscard]] FestDiff<OppfLegemiddelpakning> GetOppfLegemiddelpakningDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion);
    [[nodiscard]] FestDiff<OppfLegemiddeldose> GetOppfLegemiddeldoseDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion);
    [[nodiscard]] FestDiff<OppfKodeverk> GetOppfKodeverkDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &firstVersion, const std::string &secondVersion);
    [[nodiscard]] FestDiff<Element> GetKodeverkElementsDiff(const std::function<void (int addsAndRemovesDone, int addsAndRemovesMax, int modificationsDone, int modificationsMax)> &progress, const std::string &kodeverkId, const std::string &firstVersion, const std::string &secondVersion);
};


#endif //DRWHATSNOT_FESTDB_H
