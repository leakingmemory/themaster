//
// Created by sigsegv on 11/1/24.
//

#ifndef THEMASTER_TIMECONST_H
#define THEMASTER_TIMECONST_H

#include <cstdint>
#include <string>

template <typename T> static constexpr bool IsLeapYear(T year) {
    if ((year % 4) == 0) {
        if ((year % 100) == 0) {
            if ((year % 400) == 0) {
                return true;
            }
            return false;
        }
        return true;
    }
    return false;
}

template <typename T> static constexpr int GetDaysPerYear(T year) {
    return IsLeapYear(year) ?  366 : 365;
}

template <typename T, typename Y, typename N> static constexpr T GetDaysPerYears(Y year, N n) {
    if (n <= 0) {
        return 0;
    }
    T total = 0;
    for (N i = 0; i < n; i++) {
        total += GetDaysPerYear(year + i);
    }
    return total;
}

static constexpr uint32_t daysPer400Years = GetDaysPerYears<uint32_t>(0, 400);

template <typename T> static constexpr void AppendAsString(std::string &str, T num, int minDigits = 1) {
    typeof(num) dec = 1;
    for (int i = 1; (num / dec) > 9 || i < minDigits; i++) {
        dec *= 10;
    }
    while (dec > 0) {
        auto d = (num / dec) % 10;
        dec = dec / 10;
        char substr[2] = {static_cast<char>(d + '0'), '\0'};
        str.append(substr);
    }
}

static constexpr std::string IntToString(int num, int minDigits = 1) {
    std::string str{};
    if (num >= 0) {
        AppendAsString(str, static_cast<unsigned int>(num), minDigits);
    } else {
        char neg[2] = {'-', '\0'};
        str.append(neg);
        AppendAsString(str, static_cast<unsigned int>(-num), minDigits);
    }
    return str;
}

#endif //THEMASTER_TIMECONST_H
