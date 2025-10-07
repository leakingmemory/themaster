//
// Created by sigsegv on 12/20/23.
//

#ifndef DRWHATSNOT_HELSEIDAUTHORIZATION_H
#define DRWHATSNOT_HELSEIDAUTHORIZATION_H

#include <string>
#include <vector>

class HelseidAuthorization {
private:
    std::string url;
    std::string clientId;
    std::string redirectUri{"https://appredirect.radiotube.org/jeo-at-dips"};
    std::vector<std::string> scopes = {
            "openid",
            "profile",
            "offline_access",
            "helseid://scopes/hpr/hpr_number",
            "helseid://scopes/identity/network",
            "helseid://scopes/identity/pid",
            "helseid://scopes/identity/security_level",
            "nhn:kjernejournal/api",
            "e-helse:sfm.api/sfm.api"};
    std::string state;
    std::string verification;
public:
    HelseidAuthorization(const std::string &url, const std::string &clientId) : url(url), clientId(clientId), state() {}
    HelseidAuthorization(std::string &&url, std::string &&clientId) : url(std::move(url)), clientId(std::move(clientId)), state() {}
    std::string GetAuthorizeUrl();
    [[nodiscard]] std::string GetRedirectUri() const {
        return redirectUri;
    }
    [[nodiscard]] std::string GetState() const {
        return state;
    }
    [[nodiscard]] std::vector<std::string> GetScopes() const {
        return scopes;
    }
    [[nodiscard]] std::string GetVerfication() const {
        return verification;
    }
};


#endif //DRWHATSNOT_HELSEIDAUTHORIZATION_H
