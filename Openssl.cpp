//
// Created by sigsegv on 12/26/23.
//

#include "Openssl.h"
#include <memory>
#include <iostream>
#include <fcntl.h>
#include <openssl/rand.h>
#include <openssl/err.h>
#include <cstring>

template <class T> class scope_exit {
private:
    T destr;
public:
    constexpr scope_exit(T destr) : destr(destr) {}
    ~scope_exit() {
        destr();
    }
};

void Openssl::Seed(int bytes) {
    std::unique_ptr<uint8_t, void (*)(uint8_t *)> buf{(uint8_t *) malloc(bytes), [] (uint8_t *release) {free((void *) release);}};
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd < 0) {
        std::cerr << "Open failed: /dev/urandom\n";
        throw std::exception();
    }
    scope_exit fdguard{[fd] () { close(fd); }};
    int rd = read(fd, (void *) &(*buf), bytes);
    if (rd < bytes) {
        std::cerr << "Unable to get seed bytes from /dev/urandom\n";
        throw std::exception();
    }
    RAND_seed((const void *) &(*buf), bytes);
}

std::string Openssl::GetError() {
    auto errcode = ERR_get_error();
    std::string errstr{};
    errstr.resize(256, '\0');
    ERR_error_string_n(errcode, errstr.data(), errstr.size());
    {
        auto len = strlen(errstr.data());
        if (len < errstr.size()) {
            errstr.resize(len);
        }
    }
    return errstr;
}