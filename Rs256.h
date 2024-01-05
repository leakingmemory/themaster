//
// Created by sigsegv on 12/31/23.
//

#ifndef DRWHATSNOT_RS256_H
#define DRWHATSNOT_RS256_H

#include <memory>

class SigningKey;
class VerificationKey;
class Jwt;

class Rs256 {
private:
    std::shared_ptr<SigningKey> signingKey;
    std::shared_ptr<VerificationKey> verificationKey;
public:
    Rs256(std::shared_ptr<SigningKey> signingKey) : signingKey(signingKey), verificationKey() {}
    Rs256(std::shared_ptr<VerificationKey> verificationKey) : signingKey(), verificationKey(verificationKey) {}
    void Sign(Jwt &jwt) const;
    bool Verify(Jwt &jwt) const;
};


#endif //DRWHATSNOT_RS256_H
