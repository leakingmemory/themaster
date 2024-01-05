//
// Created by sigsegv on 12/25/23.
//

#include "JwkPemRsaKey.h"
#include "OpensslRsa.h"

std::shared_ptr<OpensslRsa> JwkPemRsaKey::CreateOpensslRsa() const {
    std::shared_ptr<OpensslRsa> rsa = std::make_shared<OpensslRsa>();
    {
        std::map<std::string, Bignum> params{};
        params.insert_or_assign("d", d);
        params.insert_or_assign("e", e);
        params.insert_or_assign("n", n);
        params.insert_or_assign("rsa-exponent1", dp);
        params.insert_or_assign("rsa-exponent2", dq);
        params.insert_or_assign("rsa-factor1", p);
        params.insert_or_assign("rsa-factor2", q);
        if (!qi.empty()) {
            params.insert_or_assign("rsa-coefficient1", qi);
        }
        rsa->ImportParams(params);
    }
    return rsa;
}

OpensslRsa JwkPemRsaKey::GenerateRandomInternal(int keySize, int kExp) {
    OpensslRsa rsa{};
    std::map<std::string,Bignum> paramsMap{};
    {
        rsa.GenerateRandom(keySize, kExp);
        paramsMap = rsa.ExportParams();
    }
    d = "";
    e = "";
    n = "";
    dp = "";
    dq = "";
    p = "";
    q = "";
    qi = "";
    for (const auto &param : paramsMap) {
        if (param.first == "d") {
            d = param.second;
        } else if (param.first == "e") {
            e = param.second;
        } else if (param.first == "n") {
            n = param.second;
        } else if (param.first == "rsa-exponent1") {
            dp = param.second;
        } else if (param.first == "rsa-exponent2") {
            dq = param.second;
        } else if (param.first == "rsa-factor1") {
            p = param.second;
        } else if (param.first == "rsa-factor2") {
            q = param.second;
        } else if (param.first == "rsa-coefficient1") {
            qi = param.second;
        }
    }
    return std::move(rsa);
}

void JwkPemRsaKey::GenerateRandom(int keySize, int kExp) {
    OpensslRsa rsa = GenerateRandomInternal();
}

std::string JwkPemRsaKey::ToTraditionalPrivatePem() const {
    OpensslRsa rsa{};
    {
        std::map<std::string, Bignum> params{};
        params.insert_or_assign("d", d);
        params.insert_or_assign("e", e);
        params.insert_or_assign("n", n);
        params.insert_or_assign("rsa-exponent1", dp);
        params.insert_or_assign("rsa-exponent2", dq);
        params.insert_or_assign("rsa-factor1", p);
        params.insert_or_assign("rsa-factor2", q);
        if (!qi.empty()) {
            params.insert_or_assign("rsa-coefficient1", qi);
        }
        rsa.ImportParams(params);
    }
    return rsa.ToTraditionalPrivatePem();
}

std::string JwkPemRsaKey::ToPublicPem() const {
    OpensslRsa rsa{};
    {
        std::map<std::string, Bignum> params{};
        params.insert_or_assign("d", d);
        params.insert_or_assign("e", e);
        params.insert_or_assign("n", n);
        params.insert_or_assign("rsa-exponent1", dp);
        params.insert_or_assign("rsa-exponent2", dq);
        params.insert_or_assign("rsa-factor1", p);
        params.insert_or_assign("rsa-factor2", q);
        if (!qi.empty()) {
            params.insert_or_assign("rsa-coefficient1", qi);
        }
        rsa.ImportParams(params);
    }
    return rsa.ToPublicPem();
}

std::shared_ptr<SigningKey> JwkPemRsaKey::ToSigningKey() const {
    return std::dynamic_pointer_cast<SigningKey>(CreateOpensslRsa());
}

std::shared_ptr<VerificationKey> JwkPemRsaKey::ToVerificationKey() const {
    std::shared_ptr<OpensslRsa> rsa = std::make_shared<OpensslRsa>();
    {
        std::map<std::string, Bignum> params{};
        params.insert_or_assign("e", e);
        params.insert_or_assign("n", n);
        rsa->ImportPublicParams(params);
    }
    return std::dynamic_pointer_cast<VerificationKey>(rsa);
}
