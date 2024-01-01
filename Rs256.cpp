//
// Created by sigsegv on 12/31/23.
//

#include "Rs256.h"
#include "Jwt.h"
#include "SigningKey.h"
#include "Base64.h"

void Rs256::Sign(Jwt &jwt) const {
    jwt.Header()->Add("alg", "RS256");
    std::string content = jwt.ToStringWithoutSignature();
    std::string signature = signingKey->Sign(content);
    Base64UrlEncoding encoding{};
    jwt.SetSignature(encoding.Encode(signature));
}