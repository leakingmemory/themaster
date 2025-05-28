//
// Created by sigsegv on 12/20/23.
//

#include "HelseidAuthorization.h"
#include <jjwtid/PkceS256.h>
#include <sstream>
#include <vector>
#include <cpprest/uri.h>
#include "win32/w32strings.h"
#include "Uuid.h"

std::string HelseidAuthorization::GetAuthorizeUrl() {
    std::stringstream strurl{};
    strurl << url;
    if (url.ends_with("/")) {
        strurl << "connect/authorize?client_id=";
    } else {
        strurl << "/connect/authorize?client_id=";
    }
    strurl << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(clientId)));
    strurl << "&nonce=";
    {
        strurl << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(Uuid::RandomUuidString())));
    }
    strurl << "&redirect_uri=";
    strurl << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(redirectUri)));
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
        strurl << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(scopeStream.str())));
    }
    strurl << "&code_challenge=";
    {
        PkceS256 pkce{};
        verification = pkce.GetVerifier();
        strurl << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(pkce.GetChallenge())));
    }
    strurl << "&code_challenge_method=S256";
    strurl << "&state=";
    {
        state = Uuid::RandomUuidString();
        strurl << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(state)));
    }
    return strurl.str();
}
