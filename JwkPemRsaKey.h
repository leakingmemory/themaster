//
// Created by sigsegv on 12/25/23.
//

#ifndef DRWHATSNOT_JWKPEMRSAKEY_H
#define DRWHATSNOT_JWKPEMRSAKEY_H

#include <string>
#include <memory>
#include "Bignum.h"

class SigningKey;
class VerificationKey;
class OpensslRsa;

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
    std::shared_ptr<OpensslRsa> CreateOpensslRsa() const;
    OpensslRsa GenerateRandomInternal(int keySize = 2048, int kExp = 65537);
    void GenerateRandom(int keySize = 2048, int kExp = 65537);
    std::string ToTraditionalPrivatePem() const;
    std::string ToPublicPem() const;
    std::string ToJwk() const;
    std::string ToPublicJwk() const;
    void FromJwk(const std::string &json);
    std::shared_ptr<SigningKey> ToSigningKey() const;
    std::shared_ptr<VerificationKey> ToVerificationKey() const;
};


#endif //DRWHATSNOT_JWKPEMRSAKEY_H
