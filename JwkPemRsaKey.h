//
// Created by sigsegv on 12/25/23.
//

#ifndef DRWHATSNOT_JWKPEMRSAKEY_H
#define DRWHATSNOT_JWKPEMRSAKEY_H

#include <string>

class JwkPemRsaKey {
private:
    std::string d{};
    std::string dp{};
    std::string dq{};
    std::string e{};
    std::string n{};
    std::string p{};
    std::string q{};
    std::string qi{};
public:
    constexpr JwkPemRsaKey() {}
    void GenerateRandom(int keySize = 2048, int kExp = 65537);
    std::string ToTraditionalPrivatePem() const;
    std::string ToJwk() const;
    void FromJwk(const std::string &json);
};


#endif //DRWHATSNOT_JWKPEMRSAKEY_H
