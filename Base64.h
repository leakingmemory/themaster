//
// Created by sigsegv on 12/20/23.
//

#ifndef DRWHATSNOT_BASE64_H
#define DRWHATSNOT_BASE64_H

#include <string>
#include <cstdint>

template <unsigned char base> class BaseDigitEncoder {
private:
    char digits[base];
public:
    constexpr BaseDigitEncoder(const char digits[base]) noexcept : digits() {
        for (unsigned char i =0; i < base; i++) {
            this->digits[i] = digits[i];
        }
    }
    constexpr char Encode(int digit) const {
        return digits[digit];
    }
    constexpr int Decode(char ch) const {
        for (unsigned char i = 0; i < base; i++) {
            if (digits[i] == ch) {
                return i;
            }
        }
        return 0;
    }
};

class Base64EncoderImpl {
private:
    BaseDigitEncoder<64> digitEncoder;
public:
    constexpr Base64EncoderImpl(BaseDigitEncoder<64> digitEncoder) : digitEncoder(digitEncoder) {}
    [[nodiscard]] static constexpr size_t EncodingOutputSize(size_t inputSize) {
        auto tmp = inputSize << 3;
        auto inc = (tmp % 6) != 0 ? 1 : 0;
        return (tmp / 6) + inc;
    }
    template <typename bytetype> constexpr void Encode(std::string &result, const bytetype *input, size_t size) const {
        int bitoff = 0;
        uint16_t w{0};
        for (size_t i = 0; i < size;) {
            if (bitoff < 6) {
                w = w << 8;
                w |= (uint16_t) input[i++];
                bitoff += 8;
            }
            auto extract = (w >> (bitoff - 6)) & 63;
            bitoff -= 6;
            char ch = digitEncoder.Encode(extract);
            result.append({ch});
        }
        if (bitoff > 0) {
            w = w << (6 - bitoff);
            char ch = digitEncoder.Encode(w & 63);
            result.append({ch});
        }
    }
    [[nodiscard]] static constexpr size_t DecodingOutputSize(size_t inputSize) {
        return (inputSize * 6) >> 3;
    }
    template <typename bytetype_in, typename bytetype_out> constexpr void Decode(bytetype_out *result, const bytetype_in *input, size_t size_in) const {
        int bitoff = 0;
        uint16_t w{0};
        size_t o = 0;
        for (size_t i = 0; i < size_in; i++) {
            w = w << 6;
            w |= ((uint16_t) digitEncoder.Decode(input[i]));
            bitoff += 6;
            if (bitoff >= 8) {
                result[o++] = (bytetype_out) ((w >> (bitoff - 8)) & 0xFF);
                bitoff -= 8;
            }
        }
    }
    constexpr void Encode(std::string &result, const std::string &input) const {
        result.reserve(EncodingOutputSize(input.size()));
        Encode<char>(result, input.data(), input.size());
    }
    template <typename bytetype> std::string Encode(const bytetype *input, size_t size) const {
        std::string result{};
        Encode(result, input, size);
        return result;
    }
    std::string Encode(const std::string &input) const;

    constexpr void Decode(std::string &result, const std::string &input) const {
        if (&input == &result) {
            std::string tmp{};
            Decode(tmp, input);
            result = tmp;
            return;
        }
        result.resize(DecodingOutputSize(input.size()));
        Decode(result.data(), input.data(), input.size());
    }
    [[nodiscard]] std::string Decode(const std::string &input) const;
};

class Base64UrlDigits : public BaseDigitEncoder<64> {
public:
    constexpr Base64UrlDigits() : BaseDigitEncoder<64>("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_") {}
};

class Base64Digits : public BaseDigitEncoder<64> {
public:
    constexpr Base64Digits() : BaseDigitEncoder<64>("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/") {}
};

class Base64UrlEncoding : public Base64EncoderImpl {
public:
    constexpr Base64UrlEncoding() : Base64EncoderImpl(Base64UrlDigits()) {}
};

class Base64Encoding : public Base64EncoderImpl {
public:
    constexpr Base64Encoding() : Base64EncoderImpl(Base64Digits()) {}
};

#endif //DRWHATSNOT_BASE64_H
