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

class FestDb {
    std::shared_ptr<FestDeserializer> festDeserializer{};
public:
    FestDb();
    bool IsOpen() const;
private:
    FestDbContainer GetActiveFestDb() const;
public:
    [[nodiscard]] std::vector<LegemiddelVirkestoff> FindLegemiddelVirkestoff(const std::string &term) const;
    [[nodiscard]] std::vector<LegemiddelMerkevare> FindLegemiddelMerkevare(const std::string &term) const;
    [[nodiscard]] std::vector<Legemiddelpakning> FindLegemiddelpakning(const std::string &term) const;
};


#endif //DRWHATSNOT_FESTDB_H
