//
// Created by sigsegv on 12/30/23.
//

#include "JwkPemRsaKey.h"
#include <nlohmann/json.hpp>
#include "Base64.h"

std::string JwkPemRsaKey::ToJwk() const {
    nlohmann::json jwkJson{};
    Base64UrlEncoding encoding{};
    jwkJson.emplace("kty", "RSA");
    jwkJson.emplace("d", encoding.Encode(d));
    jwkJson.emplace("dp", encoding.Encode(dp));
    jwkJson.emplace("dq", encoding.Encode(dq));
    jwkJson.emplace("e", encoding.Encode(e));
    jwkJson.emplace("n", encoding.Encode(n));
    jwkJson.emplace("p", encoding.Encode(p));
    jwkJson.emplace("q", encoding.Encode(q));
    jwkJson.emplace("qi", encoding.Encode(qi));
    return jwkJson.dump();
}

void JwkPemRsaKey::FromJwk(const std::string &json) {
    nlohmann::json jwkJson = nlohmann::json::parse(json);
    if (!jwkJson.contains("kty") || !jwkJson["kty"].is_string()) {
        throw std::exception();
    }
    {
        std::string kty = jwkJson["kty"];
        if (kty != "RSA") {
            throw std::exception();
        }
    }
    if (!jwkJson.contains("d") || !jwkJson["d"].is_string()) {
        throw std::exception();
    }
    if (!jwkJson.contains("dp") || !jwkJson["dp"].is_string()) {
        throw std::exception();
    }
    if (!jwkJson.contains("dq") || !jwkJson["dq"].is_string()) {
        throw std::exception();
    }
    if (!jwkJson.contains("e") || !jwkJson["e"].is_string()) {
        throw std::exception();
    }
    if (!jwkJson.contains("n") || !jwkJson["n"].is_string()) {
        throw std::exception();
    }
    if (!jwkJson.contains("p") || !jwkJson["p"].is_string()) {
        throw std::exception();
    }
    if (!jwkJson.contains("q") || !jwkJson["q"].is_string()) {
        throw std::exception();
    }
    if (!jwkJson.contains("qi") || !jwkJson["qi"].is_string()) {
        throw std::exception();
    }

    std::string d = jwkJson["d"];
    std::string dp = jwkJson["dp"];
    std::string dq = jwkJson["dq"];
    std::string e = jwkJson["e"];
    std::string n = jwkJson["n"];
    std::string p = jwkJson["p"];
    std::string q = jwkJson["q"];
    std::string qi = jwkJson["qi"];

    Base64UrlEncoding encoding{};
    this->d = encoding.Decode(d);
    this->dp = encoding.Decode(dp);
    this->dq = encoding.Decode(dq);
    this->e = encoding.Decode(e);
    this->n = encoding.Decode(n);
    this->p = encoding.Decode(p);
    this->q = encoding.Decode(q);
    this->qi = encoding.Decode(qi);
}
