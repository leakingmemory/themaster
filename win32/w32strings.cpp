//
// Created by jan-e on 05.01.2025.
//

#include "w32strings.h"

w32string::w32string(const std::string &utf8str) : utf8str(utf8str) {}
w32string::w32string(std::string &&utf8str) noexcept : utf8str(std::move(utf8str)) {}

w32string::w32string(const std::wstring &ws) : utf8str(ws.size() * 4, ' ') {
    utf8str.resize(std::wcstombs(&utf8str[0], ws.c_str(), utf8str.size()));
}

w32string::operator std::string() const {
    return utf8str;
}

w32string::operator std::wstring() const {
    std::wstring ws(utf8str.size(), L' '); // Overestimate number of code points.
    ws.resize(std::mbstowcs(&ws[0], utf8str.c_str(), utf8str.size()));
    return ws;
}
