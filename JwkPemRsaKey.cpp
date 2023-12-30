//
// Created by sigsegv on 12/25/23.
//

#include "JwkPemRsaKey.h"
#include "OpensslRsa.h"

void JwkPemRsaKey::GenerateRandom(int keySize, int kExp) {
    std::map<std::string,std::string> paramsMap{};
    {
        OpensslRsa rsa{};
        rsa.GenerateRandom(keySize, kExp);
        paramsMap = rsa.ExportParams();
    }
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
}

std::string JwkPemRsaKey::ToTraditionalPrivatePem() const {
    OpensslRsa rsa{};
    {
        std::map<std::string, std::string> params{};
        params.insert_or_assign("d", d);
        params.insert_or_assign("e", e);
        params.insert_or_assign("n", n);
        params.insert_or_assign("rsa-exponent1", dp);
        params.insert_or_assign("rsa-exponent2", dq);
        params.insert_or_assign("rsa-factor1", p);
        params.insert_or_assign("rsa-factor2", q);
        params.insert_or_assign("rsa-coefficient1", qi);
        rsa.ImportParams(params);
    }
    return rsa.ToTraditionalPrivatePem();
}