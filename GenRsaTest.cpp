//
// Created by sigsegv on 12/26/23.
//

#include "JwkPemRsaKey.h"
#include "Openssl.h"
#include <iostream>

int main() {
    JwkPemRsaKey rsa{};
    Openssl::Seed(256 / 8);
    rsa.GenerateRandom();
    auto jwk = rsa.ToJwk();
    std::cout << jwk << "\n";
    auto pem = rsa.ToTraditionalPrivatePem();
    std::cout << pem << "\n";
    auto pem2 = rsa.ToTraditionalPrivatePem();
    if (pem != pem2) {
        std::cout << pem2 << "\n";
        std::cerr << "Mismatch 1\n";
        return 1;
    }
    rsa.FromJwk(jwk);
    auto pem3 = rsa.ToTraditionalPrivatePem();
    if (pem != pem3) {
        std::cout << pem3 << "\n";
        std::cerr << "Mismatch reimport\n";
        return 1;
    }
}