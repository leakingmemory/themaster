//
// Created by sigsegv on 8/4/22.
//

#include "sha2alg.h"

std::string sha256::Hex() {
    std::string result{};
    Hex(result);
    return result;
}

constexpr bool Sha2Assert(const std::string &input, const std::string &expect) {
    std::string result{};
    {
        auto algo = sha256::Digest(input);
        algo.Hex(result);
    }
    return result == expect;
}

constexpr bool Sha2Assert(const std::string &input, const uint8_t expect[32]) {
    uint8_t result[32];
    {
        auto algo = sha256::Digest(input);
        algo.Result(result);
    }
    for (int i = 0; i < 32; i++) {
        if (expect[i] != result[i]) {
            return false;
        }
    }
    return true;
}

static_assert(Sha2Assert("", "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"));
static_assert(Sha2Assert("test", "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08"));
static_assert(Sha2Assert("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdfsdfsdfaaaaaaadggfgaaha", "25e47f79e7ab799e45bb42029926a89bdd16894649b991eb01fd9a3b9abad542"));
static_assert(Sha2Assert("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdfsdfsdfaaaaaaadggfgaaha42", "cbf945a14c7f034dbf41314d7f20f24a9c7306d7b8559f00dd4dd4038b3e4e8c"));
static_assert(Sha2Assert("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdfsdfsdfaaaaaaadggfgaaha42aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdfsdfsdfaaaaaaadggfgaaha42~", "b31688a255ef45a38e9a14707fd854f83082b96cae5e40ffb81717e95d5db277"));

constexpr uint8_t expectedResultRFC7636[32] = {19, 211, 30, 150, 26, 26, 216, 236, 47, 22, 177, 12, 76, 152, 46,
                                    8, 118, 168, 120, 173, 109, 241, 68, 86, 110, 225, 137, 74, 203,
                                    112, 249, 195};
static_assert(Sha2Assert("dBjftJeZ4CVP-mB92K27uhbUJU1p1r_wW1gFWFOEjXk", expectedResultRFC7636));
