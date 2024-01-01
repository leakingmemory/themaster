//
// Created by sigsegv on 1/1/24.
//

#ifndef DRWHATSNOT_BIGNUM_H
#define DRWHATSNOT_BIGNUM_H

#include <string>

typedef struct bignum_st BIGNUM;

class Bignum {
public:
    BIGNUM *bn;
    constexpr Bignum() : bn(nullptr) {}
    Bignum(const Bignum &);
    Bignum(Bignum &&);
    Bignum &operator =(const Bignum &);
    Bignum &operator =(Bignum &&);
    Bignum &operator =(const std::string &raw);
    operator std::string() const;
    ~Bignum();
    constexpr bool empty() const {
        return bn == nullptr;
    }
};

#endif //DRWHATSNOT_BIGNUM_H
