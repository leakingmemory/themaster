//
// Created by sigsegv on 12/21/23.
//

#include "Base64.h"

std::string Base64EncoderImpl::Encode(const std::string &input) const {
    std::string result{};
    Encode(result, input);
    return result;
}

std::string Base64EncoderImpl::Decode(const std::string &input) const {
    std::string result{};
    Decode(result, input);
    return result;
}

constexpr bool AssertBase64Encode(const std::string &expected, const std::string &input) {
    std::string result{};
    Base64Encoding encoding{};
    encoding.Encode(result, input);
    return expected == result;
}

constexpr bool AssertBase64UrlEncode(const std::string &expected, const uint8_t *input, size_t inputSize) {
    std::string result{};
    Base64UrlEncoding encoding{};
    encoding.Encode(result, input, inputSize);
    return expected == result;
}

constexpr bool AssertBase64EncodeDecode(const std::string &input) {
    std::string result{};
    Base64Encoding encoding{};
    encoding.Encode(result, input);
    encoding.Decode(result, result);
    return input == result;
}

constexpr bool AssertBase64EncodeDecodeSizes(const std::string &input) {
    std::string result{};
    Base64Encoding encoding{};
    encoding.Encode(result, input);
    if (result.size() != encoding.EncodingOutputSize(input.size())) {
        return false;
    }
    std::string final{};
    encoding.Decode(final, result);
    return encoding.DecodingOutputSize(result.size()) == final.size();
}

static_assert(Base64Digits().Encode(('a' >> 2) & 63) == 'Y');
static_assert(Base64Digits().Encode(('a' << 4) & 63) == 'Q');

static_assert(Base64Digits().Encode(('a' >> 2) & 63) == 'Y');
static_assert(Base64Digits().Encode((('a' & 3) << 4) + (('a' >> 4) & 15)) == 'W');
static_assert(Base64Digits().Encode((('a' & 15) << 2) + (('a' >> 6) & 3)) == 'F');
static_assert(Base64Digits().Encode('a' & 63) == 'h');

static_assert(AssertBase64Encode("", ""));
static_assert(AssertBase64Encode("YQ", "a"));
static_assert(AssertBase64Encode("YWE", "aa"));
static_assert(AssertBase64Encode("YWFh", "aaa"));
static_assert(AssertBase64Encode("dGVzdA", "test"));
static_assert(AssertBase64Encode("YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhc2Rmc2Rmc2RmYWFhYWFhYWRnZ2ZnYWFoYTQyYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhc2Rmc2Rmc2RmYWFhYWFhYWRnZ2ZnYWFoYTQyfg", "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdfsdfsdfaaaaaaadggfgaaha42aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdfsdfsdfaaaaaaadggfgaaha42~"));

static_assert(AssertBase64EncodeDecode(""));
static_assert(AssertBase64EncodeDecode("a"));
static_assert(AssertBase64EncodeDecode("aa"));
static_assert(AssertBase64EncodeDecode("aaa"));
static_assert(AssertBase64EncodeDecode("test"));
static_assert(AssertBase64EncodeDecode("YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhc2Rmc2Rmc2RmYWFhYWFhYWRnZ2ZnYWFoYTQyYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhc2Rmc2Rmc2RmYWFhYWFhYWRnZ2ZnYWFoYTQyfg\", \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdfsdfsdfaaaaaaadggfgaaha42aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdfsdfsdfaaaaaaadggfgaaha42~"));
static_assert(AssertBase64EncodeDecode("\x01\xad"));
static_assert(AssertBase64EncodeDecode("\x01\xad\x0c\xf0\xdf\xdf\xef\xab\x3e\x99\x7b\xd3\x4f\x13\xcf"));

static_assert(AssertBase64EncodeDecodeSizes(""));
static_assert(AssertBase64EncodeDecodeSizes("a"));
static_assert(AssertBase64EncodeDecodeSizes("aa"));
static_assert(AssertBase64EncodeDecodeSizes("aaa"));
static_assert(AssertBase64EncodeDecodeSizes("test"));
static_assert(AssertBase64EncodeDecodeSizes("YWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhc2Rmc2Rmc2RmYWFhYWFhYWRnZ2ZnYWFoYTQyYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhYWFhc2Rmc2Rmc2RmYWFhYWFhYWRnZ2ZnYWFoYTQyfg\", \"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdfsdfsdfaaaaaaadggfgaaha42aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaasdfsdfsdfaaaaaaadggfgaaha42~"));

constexpr uint8_t valuesFromRFC7636[32] = {19, 211, 30, 150, 26, 26, 216, 236, 47, 22, 177, 12, 76, 152, 46,
                                               8, 118, 168, 120, 173, 109, 241, 68, 86, 110, 225, 137, 74, 203,
                                               112, 249, 195};
static_assert(AssertBase64UrlEncode("E9Melhoa2OwvFrEMTJguCHaoeK1t8URWbuGJSstw-cM", valuesFromRFC7636, 32));
