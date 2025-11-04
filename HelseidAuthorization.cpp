//
// Created by sigsegv on 12/20/23.
//

#include "HelseidAuthorization.h"
#include <jjwtid/PkceS256.h>
#include <sstream>
#include <vector>
#include <iostream>
#include <cpprest/uri.h>
#include <cpprest/json.h>
#include "http_client.h"
#include "win32/w32strings.h"
#include "Uuid.h"
#include "ClientAssertion.h"

std::string HelseidAuthorization::GetAuthorizeUrl() {
    // Prepare common parameters
    PkceS256 pkce{};
    verification = pkce.GetVerifier();
    const std::string code_challenge = from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(pkce.GetChallenge())));
    state = Uuid::RandomUuidString();
    const std::string enc_state = from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(state)));
    const std::string nonce = Uuid::RandomUuidString();
    const std::string enc_nonce = from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(nonce)));

    std::stringstream scopeStream{};
    auto it = scopes.begin();
    if (it != scopes.end()) {
        scopeStream << *it;
        ++it;
    }
    while (it != scopes.end()) {
        scopeStream << " " << *it;
        ++it;
    }
    const std::string scope_str = scopeStream.str();
    const std::string enc_scope = from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(scope_str)));

    // Attempt PAR (Pushed Authorization Request)
    try {
        std::stringstream parUrl;
        parUrl << url;
        if (url.ends_with("/")) {
            parUrl << "connect/par";
        } else {
            parUrl << "/connect/par";
        }

        // Build x-www-form-urlencoded body
        std::stringstream body;
        body << "client_id=" << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(clientId)));
        body << "&redirect_uri=" << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(redirectUri)));
        body << "&response_type=code";
        body << "&response_mode=query";
        body << "&scope=" << enc_scope;
        body << "&code_challenge=" << code_challenge;
        body << "&code_challenge_method=S256";
        body << "&state=" << enc_state;
        body << "&nonce=" << enc_nonce;
        // resources
        body << "&resource=" << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32("e-helse:sfm.api")));
        body << "&resource=" << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32("nhn:kjernejournal")));
        // client authentication via private_key_jwt if JWK is provided
        if (!secretJwk.empty()) {
            try {
                web::uri u{as_wstring_on_win32(url)};
                std::string scheme = from_wstring_on_win32(u.scheme());
                std::string host = from_wstring_on_win32(u.host());
                std::string audience = scheme + "://" + host;
                int port = u.port();
                if (port > 0 && port != 80 && port != 443) {
                    audience.append(":" + std::to_string(port));
                }
                ClientAssertion ca{audience, clientId, secretJwk};
                std::string clientAssertion = ca.ToString();
                body << "&client_assertion_type=" << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32("urn:ietf:params:oauth:client-assertion-type:jwt-bearer")));
                body << "&client_assertion=" << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(clientAssertion)));
            } catch (...) {
                // If assertion building fails, proceed without it; server may still accept PAR or we will fall back
            }
        }

        web::http::client::http_client client{utility::conversions::to_string_t(parUrl.str())};
        web::http::http_request req{web::http::methods::POST};
        req.headers().add(U("Content-Type"), U("application/x-www-form-urlencoded"));
        req.set_body(utility::conversions::to_string_t(body.str()));

        auto resp = client.request(req).get();
        if (resp.status_code() >= 200 && resp.status_code() < 300) {
            auto json = resp.extract_json().get();
            auto it_req = json.as_object().find(U("request_uri"));
            if (it_req != json.as_object().end() && it_req->second.is_string()) {
                auto request_uri = it_req->second.as_string();
                // Build authorize URL using request_uri
                std::stringstream authUrl;
                authUrl << url;
                if (url.ends_with("/")) {
                    authUrl << "connect/authorize?client_id=";
                } else {
                    authUrl << "/connect/authorize?client_id=";
                }
                authUrl << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(clientId)));
                authUrl << "&request_uri=" << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(utility::conversions::to_utf8string(request_uri))));
                return authUrl.str();
            }
        } else {
            try {
                auto status = resp.status_code();
                auto bodyStr = resp.extract_string().get();
                std::string bodyUtf8 = utility::conversions::to_utf8string(bodyStr);
                std::cerr << "PAR request to '" << parUrl.str() << "' failed (" << status << ") with body: " << bodyUtf8 << std::endl;
                // Try to parse JSON error fields for better diagnostics
                try {
                    auto jsonErr = web::json::value::parse(bodyStr);
                    auto obj = jsonErr.as_object();
                    auto it_err = obj.find(U("error"));
                    auto it_desc = obj.find(U("error_description"));
                    auto it_uri = obj.find(U("error_uri"));
                    if ((it_err != obj.end() && it_err->second.is_string()) ||
                        (it_desc != obj.end() && it_desc->second.is_string()) ||
                        (it_uri != obj.end() && it_uri->second.is_string())) {
                        std::cerr << "PAR error details: ";
                        if (it_err != obj.end() && it_err->second.is_string()) {
                            std::cerr << "error=" << utility::conversions::to_utf8string(it_err->second.as_string()) << " ";
                        }
                        if (it_desc != obj.end() && it_desc->second.is_string()) {
                            std::cerr << "error_description=\"" << utility::conversions::to_utf8string(it_desc->second.as_string()) << "\" ";
                        }
                        if (it_uri != obj.end() && it_uri->second.is_string()) {
                            std::cerr << "error_uri=" << utility::conversions::to_utf8string(it_uri->second.as_string()) << " ";
                        }
                        std::cerr << std::endl;
                    }
                } catch (...) {
                    // ignore JSON parse errors
                }
                // Log relevant headers if present
                try {
                    auto &headers = resp.headers();
                    auto it_wa = headers.find(U("WWW-Authenticate"));
                    if (it_wa != headers.end()) {
                        std::cerr << "WWW-Authenticate: " << utility::conversions::to_utf8string(it_wa->second) << std::endl;
                    }
                } catch (...) {
                    // ignore header access errors
                }
            } catch (...) {
                std::cerr << "PAR request failed with non-success status, but response body could not be read." << std::endl;
            }
        }
        // If we get here, PAR failed and we will fall back
    } catch (...) {
        // Fallback to non-PAR flow below
    }

    // Fallback: traditional authorization URL
    std::stringstream strurl{};
    strurl << url;
    if (url.ends_with("/")) {
        strurl << "connect/authorize?client_id=";
    } else {
        strurl << "/connect/authorize?client_id=";
    }
    strurl << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(clientId)));
    strurl << "&nonce=" << enc_nonce;
    strurl << "&redirect_uri=";
    strurl << from_wstring_on_win32(web::uri::encode_data_string(as_wstring_on_win32(redirectUri)));
    strurl << "&response_mode=query&response_type=code&resource=e-helse%3Asfm.api&resource=nhn%3Akjernejournal&scope=";
    strurl << enc_scope;
    strurl << "&code_challenge=" << code_challenge;
    strurl << "&code_challenge_method=S256";
    strurl << "&state=" << enc_state;
    return strurl.str();
}
