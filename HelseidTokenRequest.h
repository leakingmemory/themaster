//
// Created by sigsegv on 12/25/23.
//

#ifndef DRWHATSNOT_TOKENURL_H
#define DRWHATSNOT_TOKENURL_H

#include <string>
#include <map>
#include <vector>

struct HelseidPostRequest {
    std::string url{};
    std::map<std::string,std::string> params{};
};

class HelseidTokenRequest {
private:
    std::string url;
    std::string clientId;
    std::string jwk;
    std::string redirectUri;
    std::string code;
    std::vector<std::string> scope;
    std::string codeVerifier;
public:
    HelseidTokenRequest(const std::string &url, const std::string &clientId, const std::string &jwk, const std::string &redirectUri, const std::string &code, const std::vector<std::string> &scope, const std::string &codeVerifier) : url(url), clientId(clientId), jwk(jwk), redirectUri(redirectUri), code(code), scope(scope), codeVerifier(codeVerifier) {}
    HelseidPostRequest GetTokenRequest() const;
};


#endif //DRWHATSNOT_TOKENURL_H
