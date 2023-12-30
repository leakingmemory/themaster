//
// Created by sigsegv on 12/25/23.
//

#ifndef DRWHATSNOT_OPENSSLRSA_H
#define DRWHATSNOT_OPENSSLRSA_H

#include <memory>
#include <string>
#include <map>

typedef struct evp_pkey_st EVP_PKEY;

class OpensslRsa {
private:
    std::unique_ptr<EVP_PKEY,void (*)(EVP_PKEY *)> rsa;
public:
    OpensslRsa();
    OpensslRsa(const OpensslRsa &) = delete;
    OpensslRsa & operator =(const OpensslRsa &) = delete;
    ~OpensslRsa();
    void GenerateRandom(int sizeKey = 2048, int exponent = 65537);
    std::map<std::string,std::string> ExportParams() const;
    void ImportParams(const std::map<std::string,std::string> &params);
    std::string ToTraditionalPrivatePem() const;
};


#endif //DRWHATSNOT_OPENSSLRSA_H
