//
// Created by sigsegv on 12/20/23.
//

#include "HelseidAuthorization.h"
#include <jjwtid/PkceS256.h>
#include <sstream>
#include <vector>
#include <cpprest/uri.h>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

std::string HelseidAuthorization::GetAuthorizeUrl() {
    std::stringstream strurl{};
    strurl << url;
    if (url.ends_with("/")) {
        strurl << "connect/authorize?client_id=";
    } else {
        strurl << "/connect/authorize?client_id=";
    }
    strurl << web::uri::encode_data_string(clientId);
    strurl << "&nonce=";
    {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        strurl << web::uri::encode_data_string(to_string(uuid));
    }
    strurl << "&redirect_uri=";
    strurl << web::uri::encode_data_string(redirectUri);
    strurl << "&response_mode=query&response_type=code&resource=e-helse%3Asfm.api&resource=nhn%3Akjernejournal&scope=";
    {
        std::stringstream scopeStream{};
        auto iterator = scopes.begin();
        if (iterator != scopes.end()) {
            scopeStream << *iterator;
            ++iterator;
        }
        while (iterator != scopes.end()) {
            scopeStream << " " << *iterator;
            ++iterator;
        }
        strurl << web::uri::encode_data_string(scopeStream.str());
    }
    strurl << "&code_challenge=";
    {
        PkceS256 pkce{};
        verification = pkce.GetVerifier();
        strurl << web::uri::encode_data_string(pkce.GetChallenge());
    }
    strurl << "&code_challenge_method=S256";
    strurl << "&state=";
    {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        state = to_string(uuid);
        strurl << web::uri::encode_data_string(state);
    }
    return strurl.str();
}
