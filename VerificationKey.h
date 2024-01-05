//
// Created by sigsegv on 1/1/24.
//

#ifndef DRWHATSNOT_VERIFICATIONKEY_H
#define DRWHATSNOT_VERIFICATIONKEY_H

#include <string>

class VerificationKey {
public:
    virtual bool Verify(const std::string &content, const std::string &signature) const = 0;
};

#endif //DRWHATSNOT_VERIFICATIONKEY_H
