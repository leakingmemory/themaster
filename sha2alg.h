//
// Created by sigsegv on 8/4/22.
//

#ifndef JEOKERNEL_SHA2ALG_H
#define JEOKERNEL_SHA2ALG_H

#include <cstdint>
#include <string>
#include <bit>

class sha2alg {
public:
    typedef uint32_t round_constants_type[64];
    typedef uint8_t chunk_type[64];
    typedef uint8_t result_type[64];
    typedef uint32_t hash_values_type[8];
private:
    uint32_t w[64];
protected:
    round_constants_type k;
    hash_values_type hv;
private:
    uint64_t mlength_bits;
protected:
    constexpr sha2alg() : w(), k(), hv(), mlength_bits(0) {}
private:
    template <typename T> static constexpr T rightrotate(T val, unsigned int rot){
        const unsigned int mask = (8 * sizeof(val)) - 1;
        return (val >> (rot & mask)) | (val << ((0-rot) & mask));
    }
    constexpr void Digest(const uint32_t data[16]) {
        static_assert(rightrotate<uint16_t>(0x101, 1) == 0x8080);
        static_assert(rightrotate<uint32_t>(0x10001, 1) == 0x80008000UL);
        static_assert(rightrotate<uint64_t>(0x10001, 1) == 0x8000000000008000ULL);

        for (int i = 0; i < 16; i++) {
            if (std::endian::native == std::endian::big) {
                w[i] = data[i];
            } else {
                w[i] = std::byteswap(data[i]);
            }
        }
        for (int i = 16; i < 64; i++) {
            uint32_t s0 = rightrotate(w[i-15], 7) ^ rightrotate(w[i-15], 18) ^ (w[i-15] >> 3);
            uint32_t s1 = rightrotate(w[i-2], 17) ^ rightrotate(w[i-2], 19) ^ (w[i-2] >> 10);
            w[i] = w[i-16] + s0 + w[i-7] + s1;
        }
        auto a = hv[0];
        auto b = hv[1];
        auto c = hv[2];
        auto d = hv[3];
        auto e = hv[4];
        auto f = hv[5];
        auto g = hv[6];
        auto h = hv[7];
        for (int i = 0; i < 64; i++) {
            uint32_t s1 = rightrotate(e, 6) ^ rightrotate(e, 11) ^ rightrotate(e, 25);
            uint32_t ch = (e & f) ^ ((~e) & g);
            uint32_t temp1 = h + s1 + ch + k[i] + w[i];
            uint32_t s0 = rightrotate(a, 2) ^ rightrotate(a, 13) ^ rightrotate(a, 22);
            uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
            uint32_t temp2 = s0 + maj;

            h = g;
            g = f;
            f = e;
            e = d + temp1;
            d = c;
            c = b;
            b = a;
            a = temp1 + temp2;
        }

        hv[0] += a;
        hv[1] += b;
        hv[2] += c;
        hv[3] += d;
        hv[4] += e;
        hv[5] += f;
        hv[6] += g;
        hv[7] += h;
    }
public:
    constexpr void Consume(const uint32_t data[16]) {
        mlength_bits += 16*32;
        Digest(data);
    }
    constexpr void Consume(const chunk_type data) {
        uint32_t d[16];
        for (int i = 0; i < 16; i++) {
            uint32_t tmp = data[(i << 2) + 0];
            tmp |= ((uint32_t)data[(i << 2) + 1]) << 8;
            tmp |= ((uint32_t)data[(i << 2) + 2]) << 16;
            tmp |= ((uint32_t)data[(i << 2) + 3]) << 24;
            if (std::endian::native == std::endian::big) {
                tmp = std::byteswap(tmp);
            }
            d[i] = tmp;
        }
        Consume(d);
    }
    constexpr void Final(const uint8_t *c_data, size_t len) {
        if (len > 64) {
            return;
        }
        uint32_t data[16];
        for (size_t i = 0; i < 16; i++) {
            data[i] = 0;
        }

        for (size_t i = 0; i < len; i++) {
            uint32_t exp = c_data[i];
            exp = exp << ((i&3)*8); // TODO - big endian systems
            data[i>>2] |= exp;
        }
        mlength_bits += 8*len;
        if (len <= (64-1-8)) {
            uint32_t exp = 0x80;
            exp = exp << ((len&3)*8); // TODO - big endian systems
            data[len>>2] |= exp;
            ++len;
            len = (64-8);
            uint64_t flen{mlength_bits};
            data[len>>2] = std::byteswap((uint32_t) ((flen >> 32) & 0xFFFFFFFF)); // TODO - big endian systems
            data[(len>>2)+1] = std::byteswap((uint32_t) (flen & 0xFFFFFFFF)); // TODO - likewise
            Digest(data);
        } else {
            bool endInFirst = len < 64;
            if (endInFirst) {
                uint32_t exp = 0x80;
                exp = exp << ((len&3)*8); // TODO - big endian systems
                data[len>>2] |= exp;
                len = 64;
            }
            Digest(data);
            for (int i = 0; i < (64-8); i++) {
                data[i] = 0;
            }
            uint64_t flen{mlength_bits};
            uint64_t *rawlen = (uint64_t *) &(data[64-8]);
            if constexpr (std::endian::native == std::endian::big) {
                *rawlen = flen;
            } else {
                *rawlen = std::byteswap(flen);
            }
            Digest((uint32_t *)data);
        }
    }
    constexpr void Result(result_type &b_data) {
        uint32_t *data = (uint32_t *) &(b_data[0]);
        for (int i = 0; i < 16; i++) {
            uint32_t be{hv[i]};
            if (std::endian::native == std::endian::big) {
                data[i] = be;
            } else {
                data[i] = std::byteswap(be);
            }
        }
    }
protected:
    constexpr static void Hex(std::string &str, const uint8_t data[], size_t len) {
        for (size_t i = 0; i < len; i++) {
            std::string ss{"  "};
            char ch = (char) data[i];
            ch = (char) ((ch >> 4) & 0xF);
            if (ch < 10) {
                ch += '0';
            }
            if (ch < 0x10) {
                ch += 'a' - 10;
            }
            ss[0] = ch;
            ch = (char) data[i];
            ch = (char) (ch & 0xF);
            if (ch < 10) {
                ch += '0';
            }
            if (ch < 0x10) {
                ch += 'a' - 10;
            }
            ss[1] = ch;
            str.append(ss);
        }
    }
public:
    constexpr std::string Hex() {
        uint8_t data[64];
        Result(data);
        std::string result{};
        Hex(result, data, 64);
        return result;
    }
};

class sha256 : public sha2alg {
public:
    typedef uint8_t result_type[32];
    constexpr sha256() {
        round_constants_type rc{0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
                                0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
                                0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
                                0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
                                0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
                                0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
                                0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
                                0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
        hash_values_type hv{0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};
        for (typeof(sizeof(this->k)) i = 0; i < (sizeof(this->k) / sizeof(this->k[0])); i++) {
            static_assert(sizeof(this->k[i]) == sizeof(rc[i]));
            this->k[i] = rc[i];
        }
        for (typeof(sizeof(this->hv)) i = 0; i < (sizeof(this->hv) / sizeof(this->hv[0])); i++) {
            static_assert(sizeof(this->hv[i]) == sizeof(hv[i]));
            this->hv[i] = hv[i];
        }
    }
    constexpr void Result(result_type &b_data) {
        for (int i = 0; i < 8; i++) {
            uint32_t be{hv[i]};
            b_data[(i<<2)+0] = (be >> 24) & 0xFF;
            b_data[(i<<2)+1] = (be >> 16) & 0xFF;
            b_data[(i<<2)+2] = (be >> 8) & 0xFF;
            b_data[(i<<2)+3] = (be >> 0) & 0xFF;
        }
    }
    constexpr void Hex(std::string &result) {
        uint8_t data[32];
        Result(data);
        sha2alg::Hex(result, data, 32);
    }
    std::string Hex();
    constexpr static sha256 Digest(const std::string &str) {
        sha256 alg{};
        chunk_type chunk{};
        static_assert(sizeof(chunk[0]) == 1);
        static_assert(sizeof(str[0]) == 1);
        static_assert(sizeof(chunk) > 0);
        typeof(sizeof(chunk)) v = 0;
        for (typeof(str.size()) i = 0; i < str.size(); i++) {
            chunk[v++] = str[i];
            if (v == sizeof(chunk)) {
                alg.Consume(chunk);
                v = 0;
            }
        }
        alg.Final(chunk, v);
        return alg;
    }
};


#endif //JEOKERNEL_SHA2ALG_H
