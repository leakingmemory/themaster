//
// Created by sigsegv on 12/20/23.
//

#ifndef DRWHATSNOT_HELSEIDAUTHORIZATION_H
#define DRWHATSNOT_HELSEIDAUTHORIZATION_H

#include <string>

class HelseidAuthorization {
private:
    std::string url;
    std::string clientId;
    std::string state;
    std::string verification;
public:
    constexpr HelseidAuthorization(const std::string &url, const std::string &clientId) : url(url), clientId(clientId), state() {}
    constexpr HelseidAuthorization(std::string &&url, std::string &&clientId) : url(std::move(url)), clientId(std::move(clientId)), state() {}
    std::string GetAuthorizeUrl();
    [[nodiscard]] constexpr std::string GetState() const {
        return state;
    }
    [[nodiscard]] constexpr std::string GetVerfication() const {
        return verification;
    }
};


#endif //DRWHATSNOT_HELSEIDAUTHORIZATION_H
