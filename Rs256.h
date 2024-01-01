//
// Created by sigsegv on 12/31/23.
//

#ifndef DRWHATSNOT_RS256_H
#define DRWHATSNOT_RS256_H

#include <memory>

class SigningKey;
class Jwt;

class Rs256 {
private:
    std::shared_ptr<SigningKey> signingKey;
public:
    Rs256(std::shared_ptr<SigningKey> signingKey) : signingKey(signingKey) {}
    void Sign(Jwt &jwt) const;
};


#endif //DRWHATSNOT_RS256_H
