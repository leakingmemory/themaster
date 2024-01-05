//
// Created by sigsegv on 12/31/23.
//

#include "Jwt.h"

Jwt::Jwt() : header(std::make_shared<JwtPart>()), body(std::make_shared<JwtPart>()) {
    header->Add("typ", "JWT");
}

Jwt::Jwt(const std::string &str) : header(), body() {
    auto pos1 = str.find('.');
    if (pos1 < str.size()) {
        auto part1 = str.substr(0, pos1);
        auto part2f = str.substr(pos1 + 1);
        auto pos2 = part2f.find('.');
        if (pos2 < part2f.size()) {
            unverifiedHeader = part1;
            unverifiedBody = part2f.substr(0, pos2);
            signature = part2f.substr(pos2 + 1);
        } else {
            header = std::make_shared<JwtPart>(part1);
            body = std::make_shared<JwtPart>(part2f);
        }
    } else {
        header = std::make_shared<JwtPart>();
        body = std::make_shared<JwtPart>(str);
    }
}

void Jwt::SetSignature(const std::string &sig) {
    signature = sig;
}

std::string Jwt::ToStringWithoutSignature() const {
    std::string str{header->ToBase64()};
    {
        std::string bodyStr{body->ToBase64()};
        if (!bodyStr.empty()) {
            if (!str.empty()) {
                str.append(".");
            }
            str.append(bodyStr);
        }
    }
    return str;
}

std::string Jwt::ToString() const {
    std::string str{ToStringWithoutSignature()};
    if (!signature.empty()) {
        if (!str.empty()) {
            str.append(".");
        }
        str.append(signature);
    }
    return str;
}
