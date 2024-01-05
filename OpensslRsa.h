//
// Created by sigsegv on 12/25/23.
//

#ifndef DRWHATSNOT_OPENSSLRSA_H
#define DRWHATSNOT_OPENSSLRSA_H

#include <memory>
#include <string>
#include <map>
#include "Bignum.h"
#include "SigningKey.h"
#include "VerificationKey.h"

typedef struct evp_pkey_st EVP_PKEY;

class OpensslRsa : public SigningKey, public VerificationKey {
private:
    std::unique_ptr<EVP_PKEY,void (*)(EVP_PKEY *)> rsa;
public:
    OpensslRsa();
    OpensslRsa(const OpensslRsa &) = delete;
    OpensslRsa(OpensslRsa &&) noexcept;
    OpensslRsa & operator =(const OpensslRsa &) = delete;
    ~OpensslRsa();
    bool operator == (const OpensslRsa &other) const;
    void GenerateRandom(int sizeKey = 2048, int exponent = 65537);
    std::map<std::string,Bignum> ExportParams() const;
    void ImportParams(const std::map<std::string,Bignum> &params);
    void ImportPublicParams(const std::map<std::string,Bignum> &params);
    std::string ToTraditionalPrivatePem() const;
    std::string ToPublicPem() const;
    std::string Sign(const std::string &content) const override;
    bool Verify(const std::string &content, const std::string &signature) const override;
};


#endif //DRWHATSNOT_OPENSSLRSA_H
