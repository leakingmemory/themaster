//
// Created by sigsegv on 12/31/23.
//

#include "JwtPart.h"
#include <nlohmann/json.hpp>
#include "Base64.h"

class JwtPartValue {
public:
    virtual ~JwtPartValue() = default;
    virtual void AddToJson(nlohmann::json &obj, const std::string &name) const = 0;
};

class JwtPartStringValue : public JwtPartValue {
private:
    std::string str;
public:
    JwtPartStringValue(const std::string &str) : str(str) {}
    void AddToJson(nlohmann::json &obj, const std::string &name) const override;
};

class JwtPartIntegerValue : public JwtPartValue {
private:
    int64_t integer;
public:
    JwtPartIntegerValue(int64_t integer) : integer(integer) {}
    void AddToJson(nlohmann::json &obj, const std::string &name) const override;
};

void JwtPartStringValue::AddToJson(nlohmann::json &obj, const std::string &name) const {
    obj.emplace(name, str);
}

void JwtPartIntegerValue::AddToJson(nlohmann::json &obj, const std::string &name) const {
    obj.emplace(name, integer);
}

JwtPart::JwtPart(const std::string &str) : JwtPart() {
    Base64UrlEncoding encoding{};
    nlohmann::json obj = nlohmann::json::parse(encoding.Decode(str));
    if (obj.is_object()) {
        for (const auto &prop: obj.items()) {
            std::string key = prop.key();
            auto value = prop.value();
            if (value.is_string()) {
                insert_or_assign(key, std::make_shared<JwtPartStringValue>(value));
            } else if (value.is_number_integer()) {
                insert_or_assign(key, std::make_shared<JwtPartIntegerValue>(value));
            }
        }
    }
}

std::string JwtPart::ToJson() const {
    nlohmann::json obj{};
    for (const auto &item : *this) {
        item.second->AddToJson(obj, item.first);
    }
    return obj.dump();
}

std::string JwtPart::ToBase64() const {
    auto json = ToJson();
    Base64UrlEncoding encoding{};
    return encoding.Encode(json);
}

void JwtPart::Add(const std::string &name, const std::string &value) {
    insert_or_assign(name, std::make_shared<JwtPartStringValue>(value));
}

void JwtPart::Add(const std::string &name, int64_t integer) {
    insert_or_assign(name, std::make_shared<JwtPartIntegerValue>(integer));
}
