//
// Created by sigsegv on 12/31/23.
//

#include "Jwt.h"

Jwt::Jwt() : header(std::make_shared<JwtPart>()), body(std::make_shared<JwtPart>()) {
    header->Add("typ", "JWT");
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
