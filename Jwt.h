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
    std::string signature{};
public:
    Jwt();
    [[nodiscard]] std::shared_ptr<JwtPart> Header() const {
        return header;
    }
    [[nodiscard]] std::shared_ptr<JwtPart> Body() const {
        return body;
    }
    void SetSignature(const std::string &sig);
    [[nodiscard]] std::string ToStringWithoutSignature() const;
    [[nodiscard]] std::string ToString() const;
};


#endif //DRWHATSNOT_JWT_H
