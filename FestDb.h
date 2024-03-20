//
// Created by sigsegv on 3/18/24.
//

#ifndef DRWHATSNOT_FESTDB_H
#define DRWHATSNOT_FESTDB_H

#include <string>
#include <vector>
#include <memory>

class FestDeserializer;
class LegemiddelVirkestoff;

class FestDb {
    std::shared_ptr<FestDeserializer> festDeserializer{};
public:
    FestDb();
    bool IsOpen() const;
    [[nodiscard]] std::vector<LegemiddelVirkestoff> FindLegemiddelVirkestoff(const std::string &term) const;
};


#endif //DRWHATSNOT_FESTDB_H
