//
// Created by sigsegv on 11/4/25.
//

#ifndef THEMASTER_CLIENTASSERTION_H
#define THEMASTER_CLIENTASSERTION_H

#include <string>
#include <jjwtid/OidcTokenRequest.h>

class ClientAssertion {
private:
    std::string url;
    std::string clientId;
    std::string jwk;
    HelseidMultiTenantInfo helseidMultiTenantInfo{};
public:
    ClientAssertion(const std::string &audienceUrl, const std::string &clientId, const std::string &jwk,
                    const HelseidMultiTenantInfo &multiTenantInfo = {});
    std::string ToString() const;
};


#endif //THEMASTER_CLIENTASSERTION_H