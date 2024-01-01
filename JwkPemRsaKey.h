//
// Created by sigsegv on 12/25/23.
//

#ifndef DRWHATSNOT_JWKPEMRSAKEY_H
#define DRWHATSNOT_JWKPEMRSAKEY_H

#include <string>
#include <memory>
#include "Bignum.h"

class SigningKey;

class JwkPemRsaKey {
private:
    Bignum d{};
    Bignum dp{};
    Bignum dq{};
    Bignum e{};
    Bignum n{};
    Bignum p{};
    Bignum q{};
    Bignum qi{};
public:
    constexpr JwkPemRsaKey() {}
    void GenerateRandom(int keySize = 2048, int kExp = 65537);
    std::string ToTraditionalPrivatePem() const;
    std::string ToPublicPem() const;
    std::string ToJwk() const;
    std::string ToPublicJwk() const;
    void FromJwk(const std::string &json);
    std::shared_ptr<SigningKey> ToSigningKey() const;
};


#endif //DRWHATSNOT_JWKPEMRSAKEY_H
