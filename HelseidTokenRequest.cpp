//
// Created by sigsegv on 12/25/23.
//

#include "HelseidTokenRequest.h"
#include "JwkPemRsaKey.h"

#include <sstream>
#include <cpprest/uri.h>
#include <jwt-cpp/jwt.h>

HelseidPostRequest HelseidTokenRequest::GetTokenRequest() const {
    std::string tokenEndpoint{};
    {
        std::stringstream sstr{};
        sstr << url;
        if (!url.ends_with("/")) {
            sstr << "/";
        }
        sstr << "oauth/token";
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
        std::string pem{};
        {
            JwkPemRsaKey rsa{};
            rsa.FromJwk(jwk);
            pem = rsa.ToTraditionalPrivatePem();
        }
        auto token = jwt::create()
                .set_issuer("app://themaster")
                .set_type("JWT")
                .set_issued_at(std::chrono::system_clock::now())
                .set_expires_at(std::chrono::system_clock::now() + std::chrono::seconds{60})
                .set_subject(clientId)
                .set_audience(tokenEndpoint)
                .sign(jwt::algorithm::rs256("", pem, "", ""));
        jwt = token;
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
