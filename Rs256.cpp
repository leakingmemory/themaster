//
// Created by sigsegv on 12/31/23.
//

#include "Rs256.h"
#include "Jwt.h"
#include "SigningKey.h"
#include "VerificationKey.h"
#include "Base64.h"

void Rs256::Sign(Jwt &jwt) const {
    jwt.Header()->Add("alg", "RS256");
    std::string content = jwt.ToStringWithoutSignature();
    std::string signature = signingKey->Sign(content);
    Base64UrlEncoding encoding{};
    jwt.SetSignature(encoding.Encode(signature));
}

bool Rs256::Verify(Jwt &jwt) const {
    std::string content = jwt.GetUnverifiedHeader();
    content.append(".");
    content.append(jwt.GetUnverifiedBody());
    Base64UrlEncoding encoding{};
    if (verificationKey->Verify(content, encoding.Decode(jwt.GetSignature()))) {
        auto header = std::make_shared<JwtPart>(jwt.GetUnverifiedHeader());
        auto body = std::make_shared<JwtPart>(jwt.GetUnverifiedBody());
        jwt.SetVerified(header, body);
        return true;
    } else {
        return false;
    }
}