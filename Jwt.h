//
// Created by sigsegv on 12/31/23.
//

#ifndef DRWHATSNOT_JWT_H
#define DRWHATSNOT_JWT_H

#include "JwtPart.h"
#include <memory>

class Jwt {
private:
    std::shared_ptr<JwtPart> header;
    std::shared_ptr<JwtPart> body;
    std::string unverifiedHeader{};
    std::string unverifiedBody{};
    std::string signature{};
public:
    Jwt();
    explicit Jwt(const std::string &);
    [[nodiscard]] std::shared_ptr<JwtPart> Header() const {
        return header;
    }
    [[nodiscard]] std::shared_ptr<JwtPart> Body() const {
        return body;
    }
    [[nodiscard]] std::string GetUnverifiedHeader() const {
        return unverifiedHeader;
    }
    [[nodiscard]] std::string GetUnverifiedBody() const {
        return unverifiedBody;
    }
    void SetVerified(std::shared_ptr<JwtPart> vheader, std::shared_ptr<JwtPart> vbody) {
        header = vheader;
        body = vbody;
        unverifiedHeader = "";
        unverifiedBody = "";
    }

    [[nodiscard]] std::string GetSignature() const {
        return signature;
    }
    void SetSignature(const std::string &sig);
    [[nodiscard]] std::string ToStringWithoutSignature() const;
    [[nodiscard]] std::string ToString() const;
};


#endif //DRWHATSNOT_JWT_H
