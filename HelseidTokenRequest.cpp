//
// Created by sigsegv on 12/25/23.
//

#include "HelseidTokenRequest.h"
#include "JwkPemRsaKey.h"

#include <sstream>
#include <cpprest/uri.h>
#include "Jwt.h"
#include "Rs256.h"

HelseidPostRequest HelseidTokenRequest::GetTokenRequest() const {
    std::string tokenEndpoint{};
    {
        std::stringstream sstr{};
        sstr << url;
        if (!url.ends_with("/")) {
            sstr << "/";
        }
        sstr << "connect/token";
        tokenEndpoint = sstr.str();
    }
    std::string scopeStr{};
    {
        std::stringstream scopeStream{};
        auto iterator = scope.begin();
        if (iterator != scope.end()) {
            scopeStream << *iterator;
            ++iterator;
        }
        while (iterator != scope.end()) {
            scopeStream << " " << *iterator;
            ++iterator;
        }
        scopeStr = scopeStream.str();
    }
    std::string jwt{};
    {
        std::shared_ptr<SigningKey> signingKey{};
        {
            JwkPemRsaKey rsa{};
            rsa.FromJwk(jwk);
            signingKey = rsa.ToSigningKey();
        }
        auto iat = std::time(nullptr);
        Jwt token{};
        token.Body()->Add("iss", "app://themaster");
        token.Body()->Add("iat", iat);
        token.Body()->Add("exp", iat + 120);
        token.Body()->Add("sub", clientId);
        token.Body()->Add("aud", tokenEndpoint);
        Rs256 rs256{signingKey};
        rs256.Sign(token);
        jwt = token.ToString();
    }
    std::map<std::string,std::string> params{};
    params.insert_or_assign("client_id", clientId);
    params.insert_or_assign("client_assertion_type", "urn:ietf:params:oauth:client-assertion-type:jwt-bearer");
    params.insert_or_assign("client_assertion", jwt);
    params.insert_or_assign("grant_type", "authorization_code");
    params.insert_or_assign("redirect_uri", redirectUri);
    params.insert_or_assign("scope", scopeStr);
    params.insert_or_assign("code", code);
    params.insert_or_assign("code_verifier", codeVerifier);
    return {.url = tokenEndpoint, .params = params};
}
