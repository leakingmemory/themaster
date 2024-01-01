//
// Created by sigsegv on 1/1/24.
//

#include "Bignum.h"
#include <openssl/bn.h>

Bignum::Bignum(const Bignum &cp) : bn(cp.bn != nullptr ? BN_dup(cp.bn) : nullptr) {
}

Bignum::Bignum(Bignum &&mv) : bn(mv.bn) {
    if (this != &mv) {
        mv.bn = nullptr;
    }
}

Bignum &Bignum::operator=(const Bignum &cp) {
    if (this != &cp) {
        if (bn != nullptr) {
            BN_free(bn);
        }
        if (cp.bn != nullptr) {
            bn = BN_dup(cp.bn);
        } else {
            bn = nullptr;
        }
    }
    return *this;
}

Bignum &Bignum::operator=(Bignum &&mv) {
    if (this != &mv) {
        if (bn != nullptr) {
            BN_free(bn);
        }
        if (mv.bn != nullptr) {
            bn = mv.bn;
            mv.bn = nullptr;
        } else {
            bn = nullptr;
        }
    }
    return *this;
}

Bignum &Bignum::operator=(const std::string &raw) {
    if (bn != nullptr) {
        BN_free(bn);
    }
    bn = BN_bin2bn((const unsigned char *) raw.data(), raw.size(), NULL);
    return *this;
}

Bignum::operator std::string() const {
    if (bn != nullptr) {
        std::string raw{};
        raw.resize(BN_num_bytes(bn));
        BN_bn2bin(bn, (unsigned char *) raw.data());
        return raw;
    } else {
        return "";
    }
}

Bignum::~Bignum() noexcept {
    if (bn != nullptr) {
        BN_free(bn);
        bn = nullptr;
    }
}
