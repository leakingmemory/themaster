//
// Created by sigsegv on 12/31/23.
//

#ifndef DRWHATSNOT_JWTPART_H
#define DRWHATSNOT_JWTPART_H

#include <map>
#include <string>
#include <memory>

class JwtPartValue;

class JwtPart : public std::map<std::string,std::shared_ptr<JwtPartValue>> {
public:
    JwtPart() : std::map<std::string,std::shared_ptr<JwtPartValue>>() {}
    JwtPart(const std::string &);
    std::string ToJson() const;
    std::string ToBase64() const;
    void Add(const std::string &name, const std::string &value);
    void Add(const std::string &name, int64_t integer);
};


#endif //DRWHATSNOT_JWTPART_H
