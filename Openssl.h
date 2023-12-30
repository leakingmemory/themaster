//
// Created by sigsegv on 12/26/23.
//

#ifndef DRWHATSNOT_OPENSSL_H
#define DRWHATSNOT_OPENSSL_H

#include <string>

class Openssl {
public:
    static void Seed(int bytes);
    static std::string GetError();
};


#endif //DRWHATSNOT_OPENSSL_H
