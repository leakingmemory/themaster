//
// Created by sigsegv on 12/25/23.
//

#include "OpensslRsa.h"
#include "Openssl.h"
#include <iostream>
#include <memory>
#include <vector>
#include <openssl/rsa.h>
#include <openssl/pem.h>
#include <openssl/bio.h>
#include <openssl/param_build.h>

OpensslRsa::OpensslRsa() : rsa(nullptr, [] (EVP_PKEY *) {}) {}

OpensslRsa::~OpensslRsa() = default;

static std::unique_ptr<EVP_PKEY,void (*)(EVP_PKEY *)> GeneratePkey(EVP_PKEY_CTX &pctx) {
    EVP_PKEY *pkey{nullptr};
    int res = EVP_PKEY_keygen(&pctx, &pkey);
    if (res <= 0) {
        std::cerr << Openssl::GetError() << "\n";
        throw std::exception();
    }
    std::unique_ptr<EVP_PKEY,void (*)(EVP_PKEY *)> uk{pkey, [] (EVP_PKEY *release) {EVP_PKEY_free(release);}};
    return uk;
}

static std::unique_ptr<EVP_PKEY,void (*)(EVP_PKEY *)> PkeyFromParams(EVP_PKEY_CTX &pctx, OSSL_PARAM *params) {
    EVP_PKEY *pkey{nullptr};
    int res = EVP_PKEY_fromdata(&pctx, &pkey, EVP_PKEY_KEYPAIR, params);
    if (res <= 0) {
        std::cerr << Openssl::GetError() << "\n";
        throw std::exception();
    }
    std::unique_ptr<EVP_PKEY,void (*)(EVP_PKEY *)> uk{pkey, [] (EVP_PKEY *release) {EVP_PKEY_free(release);}};
    return uk;
}

void OpensslRsa::GenerateRandom(int sizeKey, int exponent) {
    std::unique_ptr<BIGNUM, void (*)(BIGNUM *)> e = {BN_new(), [] (BIGNUM *release) { BN_free(release); }};
    BN_native2bn((unsigned char *) &exponent, sizeof(exponent), &(*e));
    std::unique_ptr<EVP_PKEY_CTX, void (*)(EVP_PKEY_CTX *)> pctx{EVP_PKEY_CTX_new_id( EVP_PKEY_RSA, NULL), [] (EVP_PKEY_CTX *ctx) {EVP_PKEY_CTX_free(ctx);}};
    EVP_PKEY_keygen_init(&(*pctx));
    rsa = std::move(GeneratePkey(*pctx));
}

std::map<std::string,std::string> OpensslRsa::ExportParams() const {
    std::map<std::string,std::string> paramsMap{};
    OSSL_PARAM *params;
    int ret = EVP_PKEY_todata(&(*rsa), EVP_PKEY_KEYPAIR, &params);
    if (ret <= 0) {
        return {};
    }
    for (auto i = 0; params[i].key != NULL; i++) {
        std::string key{params[i].key};
        std::string value{(const char *) params[i].data, params[i].data_size};
        paramsMap.insert_or_assign(key, value);
    }
    OSSL_PARAM_free(params);
    return paramsMap;
}

void OpensslRsa::ImportParams(const std::map<std::string, Bignum> &params) {
    std::unique_ptr<EVP_PKEY_CTX, void (*)(EVP_PKEY_CTX *)> pctx{EVP_PKEY_CTX_new_id( EVP_PKEY_RSA, NULL), [] (EVP_PKEY_CTX *ctx) {EVP_PKEY_CTX_free(ctx);}};
    EVP_PKEY_fromdata_init(&(*pctx));
    std::unique_ptr<OSSL_PARAM_BLD,void (*)(OSSL_PARAM_BLD *)> bld{
            OSSL_PARAM_BLD_new(),
            [](OSSL_PARAM_BLD *release) { OSSL_PARAM_BLD_free(release); }
    };
    for (const auto &param : params) {
        if (!OSSL_PARAM_BLD_push_BN(&(*bld), param.first.c_str(), param.second.bn)) {
            throw std::exception();
        }
    }
    std::unique_ptr<OSSL_PARAM,void (*)(OSSL_PARAM *)> opensslParams{nullptr, [] (OSSL_PARAM *) {}};
    {
        OSSL_PARAM *p = OSSL_PARAM_BLD_to_param(&(*bld));
        if (p == NULL) {
            throw std::exception();
        }
        opensslParams = {p, [] (OSSL_PARAM *release) {OSSL_PARAM_free(release);}};
    }
    rsa = std::move(PkeyFromParams(*pctx, &(*opensslParams)));
}

std::string OpensslRsa::ToTraditionalPrivatePem() const {
    std::string pem{};
    std::unique_ptr<BIO,void (*)(BIO*)> bio{BIO_new(BIO_s_mem()), [] (auto *release) {BIO_free(release);}};
    auto ret = PEM_write_bio_PrivateKey_traditional(&(*bio), &(*rsa), NULL, NULL, 0, NULL, NULL);
    if (ret <= 0) {
        return "";
    }
    size_t size{0};
    size = BIO_pending(&(*bio));
    pem.resize(size);
    BIO_read(&(*bio), pem.data(), size);
    return pem;
}

std::string OpensslRsa::ToPublicPem() const {
    std::string pem{};
    std::unique_ptr<BIO,void (*)(BIO*)> bio{BIO_new(BIO_s_mem()), [] (auto *release) {BIO_free(release);}};
    auto ret = PEM_write_bio_PUBKEY(&(*bio), &(*rsa));
    if (ret <= 0) {
        return "";
    }
    size_t size{0};
    size = BIO_pending(&(*bio));
    pem.resize(size);
    BIO_read(&(*bio), pem.data(), size);
    return pem;
}

std::string OpensslRsa::Sign(const std::string &content) const {
    std::unique_ptr<EVP_MD_CTX, void (*)(EVP_MD_CTX *)> ctx{
        EVP_MD_CTX_new(),
        [] (EVP_MD_CTX *release) { EVP_MD_CTX_free(release); }
    };
    if (!ctx) {
        std::cerr << Openssl::GetError() << "\n";
        throw std::exception();
    }
    if (EVP_DigestSignInit(&(*ctx), NULL, EVP_sha256(), NULL, &(*rsa)) <= 0) {
        std::cerr << Openssl::GetError() << "\n";
        throw std::exception();
    }
    size_t siglen{0};
    if (EVP_DigestSignUpdate(&(*ctx), content.data(), content.size()) <= 0) {
        std::cerr << Openssl::GetError() << "\n";
        throw std::exception();
    }
    if (EVP_DigestSignFinal(&(*ctx), NULL, &siglen) <= 0) {
        std::cerr << Openssl::GetError() << "\n";
        throw std::exception();
    }
    std::string signature{};
    signature.resize(siglen);
    if (EVP_DigestSignFinal(&(*ctx), (unsigned char *) signature.data(), &siglen) <= 0) {
        std::cerr << Openssl::GetError() << "\n";
        throw std::exception();
    }
    return signature;
}
