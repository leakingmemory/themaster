//
// Created by sigsegv on 2/28/25.
//

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

template <typename T> constexpr typename std::make_signed<T>::type CapSigned(T inputVal) {
    if (inputVal < 0) {
        return static_cast<typename std::make_signed<T>::type>(inputVal);
    }
    typename std::make_unsigned<T>::type input{inputVal};
    typename std::make_unsigned<T>::type max{std::numeric_limits<typename std::make_signed<T>::type>::max()};
    if (input > max) {
        return max;
    }
    return input;
}

constexpr void decode_quotes(std::string &str) {
    auto input = str.begin();
    auto output = str.begin();
    char quoted = '\0';
    char previous = '\0';
    bool doubleQuoted = false;
    while (input != str.end()) {
        bool handled{false};
        if (quoted != '\0') {
            handled = true;
            if (*input == quoted) {
                if (previous == quoted) {
                    *output = quoted;
                    ++output;
                    doubleQuoted = true;
                }
            } else if (previous == quoted) {
                quoted = '\0';
                handled = false;
            } else {
                *output = *input;
                ++output;
            }
            if (!doubleQuoted) {
                previous = *input;
            } else {
                previous = '\0';
            }
        }
        if (handled) {
            ++input;
            continue;
        }
        if (*input == '\"' || *input == '\'') {
            quoted = *input;
            previous = '\0';
            doubleQuoted = false;
        } else {
            *output = *input;
            ++output;
        }
        ++input;
    }
    auto len = output - str.begin();
    str.resize(len);
}

constexpr bool verify_decode_q(const std::string &input, const std::string &expected) {
    std::string d{input};
    decode_quotes(d);
    return d == expected;
}

static_assert(verify_decode_q("1", "1"));
static_assert(verify_decode_q("\"1\"", "1"));
static_assert(verify_decode_q("\"\"\"1\"", "\"1"));
static_assert(verify_decode_q("'\"\"1'", "\"\"1"));
static_assert(verify_decode_q("'''1'", "'1"));

constexpr void parse_ln(std::vector<std::string> &output, const std::string &input) {
    typename std::make_signed<decltype(input.size())>::type start = 0;
    output.reserve(input.size() / 10);
    auto lim = CapSigned(input.size());
    char quoted = '\0';
    char previous = '\0';
    bool doubleQuoted{false};
    for (typename std::make_signed<decltype(input.size())>::type i = 0; i < lim; i++) {
        bool handled{false};
        if (quoted != '\0') {
            if (previous == quoted) {
                if (quoted == input[i]) {
                    doubleQuoted = !doubleQuoted;
                    handled = true;
                } else if (doubleQuoted) {
                    doubleQuoted = false;
                    handled = true;
                } else {
                    quoted = '\0';
                }
            }
            previous = input[i];
        }
        if (handled) {
            continue;
        }
        if (input[i] == '\"' || input[i] == '\'') {
            quoted = input[i];
            previous = '\0';
            doubleQuoted = false;
        } else if (input[i] == ',') {
            std::string &o = output.emplace_back();
            o.reserve(i - start);
            o.append(input.begin() + start, input.begin() + i);
            decode_quotes(o);
            start = i + 1;
        }
    }
    std::string &o = output.emplace_back();
    o.reserve(lim - start);
    o.append(input.begin() + start, input.begin() + lim);
    decode_quotes(o);
}

constexpr bool test_lnparser(const std::string &input, const std::vector<std::string> &expected) {
    std::vector<std::string> output{};
    parse_ln(output, input);
    auto iterator1 = output.begin();
    auto iterator2 = expected.begin();
    while (iterator1 != output.end()) {
        if (iterator2 == expected.end()) {
            return false;
        }
        if (*iterator1 != *iterator2) {
            return false;
        }
        ++iterator2;
        ++iterator1;
    }
    if (iterator2 != expected.end()) {
        return false;
    }
    return true;
}

static_assert(test_lnparser(",", {"", ""}));
static_assert(test_lnparser("1,2", {"1", "2"}));
static_assert(test_lnparser("\"1\",\"2\"", {"1", "2"}));
static_assert(test_lnparser("'1','2'", {"1", "2"}));
static_assert(test_lnparser("'\"1','2'", {"\"1", "2"}));
static_assert(test_lnparser("\"\"\"1\",'2'", {"\"1", "2"}));

constexpr void parse_lnsep(std::vector<std::string> &lns, const std::string &input) {
    auto iterator = input.begin();
    auto start = iterator;
    char prevLnsep = '\0';
    while (iterator != input.end()) {
        auto ch = *iterator;
        if (ch == '\n' || ch == '\r') {
            if (prevLnsep == '\0' || prevLnsep == ch) {
                auto &o = lns.emplace_back();
                o.append(start, iterator);
                start = iterator + 1;
                prevLnsep = ch;
            } else {
                start = iterator + 1;
                prevLnsep = '\0';
            }
        } else {
            prevLnsep = '\0';
        }
        ++iterator;
    }
    auto &o = lns.emplace_back();
    if (start != iterator) {
        o.append(start, iterator);
    }
}

constexpr bool test_lnsep(const std::string &input, const std::vector<std::string> &expected) {
    std::vector<std::string> output{};
    parse_lnsep(output, input);
    auto iterator1 = output.begin();
    auto iterator2 = expected.begin();
    while (iterator1 != output.end()) {
        if (iterator2 == expected.end()) {
            return false;
        }
        if (*iterator1 != *iterator2) {
            return false;
        }
        ++iterator2;
        ++iterator1;
    }
    if (iterator2 != expected.end()) {
        return false;
    }
    return true;
}

static_assert(test_lnsep("", {""}));
static_assert(test_lnsep("\n", {"", ""}));
static_assert(test_lnsep("\r", {"", ""}));
static_assert(test_lnsep("\n\r", {"", ""}));
static_assert(test_lnsep("\r\n", {"", ""}));
static_assert(test_lnsep("\n\n", {"", "", ""}));
static_assert(test_lnsep("\r\r", {"", "", ""}));
static_assert(test_lnsep("\n\r\n\r", {"", "", ""}));
static_assert(test_lnsep("\r\n\r\n", {"", "", ""}));
static_assert(test_lnsep("a\nb", {"a", "b"}));
static_assert(test_lnsep("a\rb", {"a", "b"}));
static_assert(test_lnsep("a\n\rb", {"a", "b"}));
static_assert(test_lnsep("a\r\nb", {"a", "b"}));
static_assert(test_lnsep("a\nb\nc", {"a", "b", "c"}));
static_assert(test_lnsep("a\rb\rc", {"a", "b", "c"}));
static_assert(test_lnsep("a\n\rb\n\rc", {"a", "b", "c"}));
static_assert(test_lnsep("a\r\nb\r\nc", {"a", "b", "c"}));

constexpr void strtrim(std::string &str) {
    while (!str.empty() && (str[0] == ' ' || str[0] == '\t')) {
        str.erase(0, 1);
    }
    while (!str.empty() && (str[str.size() - 1] == ' ' || str[str.size() - 1] == '\t')) {
        str.resize(str.size() - 1);
    }
}

constexpr bool test_strtrim(const std::string &input, const std::string &expected) {
    std::string str{input};
    strtrim(str);
    return str == expected;
}

static_assert(test_strtrim("", ""));
static_assert(test_strtrim(" ", ""));
static_assert(test_strtrim(" a ", "a"));
static_assert(test_strtrim(" a", "a"));
static_assert(test_strtrim("a ", "a"));
static_assert(test_strtrim("a", "a"));

constexpr void ParseCsv(std::vector<std::vector<std::string>> &csv, const std::string &input) {
    std::vector<std::string> lns{};
    parse_lnsep(lns, input);
    for (auto &ln : lns) {
        strtrim(ln);
        if (ln.empty()) {
            continue;
        }
        std::vector<std::string> &cl = csv.emplace_back();
        parse_ln(cl, ln);
    }
}

constexpr void SafeName(std::string &str) {
    std::transform(str.cbegin(), str.cend(), str.begin(), [] (char ch) {
        if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')) {
            return ch;
        } else {
            return '_';
        }
    });
    if (str.empty()) {
        str = "empty";
    }
    if (str[0] <= '9' && str[0] >= '0') {
        str[0] = 'A';
    }
}

constexpr bool test_SafeName(const std::string &input, const std::string &expected) {
    std::string output{input};
    SafeName(output);
    return output == expected;
}

static_assert(test_SafeName("", "empty"));
static_assert(test_SafeName("abc", "abc"));
static_assert(test_SafeName(" abc", "_abc"));
static_assert(test_SafeName("1234", "A234"));

constexpr void StrEscape(std::string &str) {
    auto iterator = str.begin();
    int escapeZerosCount{0};
    while (iterator != str.end()) {
        auto ch = *iterator;
        auto safe = (ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') ||
                ch == ' ' || ch == '\'' || ch == ',' || ch == '.' || ch == ';' || ch == ':';
        if (!safe) {
            *iterator = '\\';
            ++iterator;
            iterator = str.insert(iterator, static_cast<char>((static_cast<unsigned char>(ch) % 8) + '0'));
            if (ch < 8) {
                escapeZerosCount = 2;
                ++iterator;
                continue;
            }
            iterator = str.insert(iterator, static_cast<char>(((static_cast<unsigned char>(ch) / 8) % 8) + '0'));
            if (ch < 64) {
                escapeZerosCount = 1;
                iterator += 2;
                continue;
            }
            escapeZerosCount = 0;
            iterator = str.insert(iterator, static_cast<char>((static_cast<unsigned char>(ch) / 64) + '0'));
            iterator += 3;
            continue;
        }
        if (escapeZerosCount > 0 && *iterator >= '0' && *iterator <= '9') {
            iterator -= 3 - escapeZerosCount;
            while (escapeZerosCount > 0) {
                iterator = str.insert(iterator, '0');
                --escapeZerosCount;
            }
            iterator += 3;
        }
        ++iterator;
        escapeZerosCount = 0;
    }
}

constexpr bool test_StrEscape(const std::string &input, const std::string &expected) {
    std::string output{input};
    StrEscape(output);
    return output == expected;
}

static_assert(test_StrEscape("", ""));
static_assert(test_StrEscape("test", "test"));
static_assert(test_StrEscape("\1", "\\1"));
static_assert(test_StrEscape("\10", "\\10"));
static_assert(test_StrEscape("\0010", "\\0010"));
static_assert(test_StrEscape("\0100", "\\0100"));
static_assert(test_StrEscape("@", "\\100"));

int cppmain(const std::string &cmd, const std::vector<std::string> &args) {
    if (args.size() != 1) {
        std::cerr << "Usage:\n " << cmd << " <input-file>\n";
        return 1;
    }
    std::vector<std::vector<std::string>> csvData{};
    {
        std::string input{};
        {
            std::ifstream inputstream{};
            inputstream.open(args[0]);
            inputstream.seekg(0, std::ios::end);
            input.resize(inputstream.tellg());
            inputstream.seekg(0, std::ios::beg);
            inputstream.read(input.data(), static_cast<long>(input.size()));
            inputstream.close();
        }
        ParseCsv(csvData, input);
    }
    auto iterator = csvData.begin();
    if (iterator == csvData.end()) {
        return 0;
    }
    std::vector<std::string> colnames{};
    std::string name{args[0]};
    {
        auto iterator = name.end();
        while (iterator != name.begin()) {
            --iterator;
            if (*iterator == '\\' || *iterator == '/') {
                name.erase(name.begin(), iterator + 1);
                break;
            }
        }
    }
    SafeName(name);
    std::cout << "struct " << name << " {\n";
    for (const auto &title : *iterator) {
        std::string name{};
        {
            std::stringstream namess{};
            namess << "col" << colnames.size();
            name = namess.str();
        }
        std::cout << "    const char * const " << name << ";\n";
        colnames.emplace_back(name);
    }
    std::cout << "};\n\nconstexpr struct " << name << " " << "__" << name << "[] = {\n";
    ++iterator;
    bool sep{false};
    while (iterator != csvData.end()) {
        if (sep) {
            std::cout << ",\n";
        }
        std::cout << "    {";
        {
            bool sep{false};
            auto cit = (*iterator).begin();
            for (const auto &colnm: colnames) {
                if (sep) {
                    std::cout << ", ";
                }
                std::string val{};
                if (cit != (*iterator).end()) {
                    val = *cit;
                    ++cit;
                }
                StrEscape(val);
                std::cout << "." << colnm << " = \"" << val << "\"";
                sep = true;
            }
            std::cout << "}";
        }
        sep = true;
        ++iterator;
    }
    if (sep) {
        std::cout << "\n";
    }
    std::cout << "};\nconstexpr size_t " << "__" << name << "_size = " << (csvData.size() - 1) << ";\n";
    return 0;
}

int main(const int argc, const char * const * const argv) {
    std::string cmd{};
    std::vector<std::string> args{};

    if (argc > 0) {
        cmd = *argv;
    }
    for (std::remove_cv<decltype(argc)>::type i = 1; i < argc; i++) {
        args.emplace_back(argv[i]);
    }
    cppmain(cmd, args);
}