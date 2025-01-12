//
// Created by sigsegv on 11/1/24.
//

#ifndef THEMASTER_TIMECONST_H
#define THEMASTER_TIMECONST_H

#include <cstdint>
#include <string>
#include <limits>

template <typename T> static constexpr bool ParseUInt(const std::string &str, T &val) {
    val = 0;
    if (str.empty()) {
        return false;
    }
    for (auto ch : str) {
        if (ch < '0' || ch > '9') {
            val = 0;
            return false;
        }
        val *= 10;
        val += ch - '0';
    }
    return true;
}

constexpr bool IsUInteger(const std::string &str) {
    if (str.empty()) {
        return false;
    }
    for (auto ch : str) {
        if (ch < '0' || ch > '9') {
            return false;
        }
    }
    return true;
}

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
    decltype(num) dec = 1;
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

template <typename Y, typename M, typename D> static constexpr bool ParseDateStr(const std::string &str, Y &year, M &month, D &day) {
    auto iterator = str.begin();
    year = 0;
    month = 0;
    day = 0;
    if (iterator == str.end() || *iterator == '-') {
        return false;
    }
    while (iterator != str.end() && *iterator != '-') {
        auto ch = *iterator;
        if (ch < '0' || ch > '9') {
            return false;
        }
        if (year > (std::numeric_limits<typename std::remove_cvref<decltype(year)>::type>::max() / 10)) {
            return false;
        }
        year *= 10;
        auto d = ch - '0';
        auto max = std::numeric_limits<typename std::remove_cvref<decltype(year)>::type>::max() - year;
        if (d > max) {
            return false;
        }
        year += d;
        ++iterator;
    }
    if (iterator == str.end()) {
        return false;
    }
    ++iterator;
    if (iterator == str.end() || *iterator == '-') {
        return false;
    }
    while (iterator != str.end() && *iterator != '-') {
        auto ch = *iterator;
        if (ch < '0' || ch > '9') {
            return false;
        }
        if (month > 2) {
            return false;
        }
        month *= 10;
        month += ch - '0';
        ++iterator;
    }
    if (iterator == str.end()) {
        return false;
    }
    ++iterator;
    if (iterator == str.end()) {
        return false;
    }
    while (iterator != str.end() && *iterator != '-') {
        auto ch = *iterator;
        if (ch < '0' || ch > '9') {
            return false;
        }
        if (day > 3) {
            return false;
        }
        day *= 10;
        day += ch - '0';
        ++iterator;
    }
    return true;
}

constexpr std::string::size_type FindChar(const std::string &str, std::string::value_type ch, std::string::size_type start = 0) {
    if (start >= str.size()) {
        return std::string::npos;
    }
    auto iterator = str.begin() + start;
    while (iterator != str.end()) {
        if (*iterator == ch) {
            return iterator - str.begin();
        }
        ++iterator;
    }
    return std::string::npos;
}

template <typename H, typename M> static constexpr bool ParseTimeStr(const std::string &str, H &hour, M &minute) {
    auto sep = FindChar(str, ':');
    if (sep == std::string::npos) {
        hour = 0;
        minute = 0;
        return false;
    }
    if (str.find(':', sep + 1) != std::string::npos) {
        hour = 0;
        minute = 0;
        return false;
    }
    std::string hStr = str.substr(0, sep);
    std::string mStr = str.substr(sep + 1);
    if (!ParseUInt(hStr, hour)) {
        minute = 0;
        return false;
    }
    if (!ParseUInt(mStr, minute)) {
        hour = 0;
        return false;
    }
    return true;
}

template <typename H, typename M, typename S> static constexpr bool ParseTimeStr(const std::string &str, H &hour, M &minute, S &second) {
    auto sep = str.rfind(':');
    if (sep == std::string::npos) {
        hour = 0;
        minute = 0;
        second = 0;
        return false;
    }
    std::string timeStr = str.substr(0, sep);
    std::string secondStr = str.substr(sep + 1);
    if (!ParseTimeStr(timeStr, hour, minute)) {
        second = 0;
        return false;
    }
    if (!ParseUInt(secondStr, second)) {
        hour = 0;
        minute = 0;
        return false;
    }
    return true;
}

template <typename Y, typename M, typename D, typename H, typename I, typename S, typename Htz, typename Itz> static constexpr bool ParseDateTimeOffsetStr(const std::string &str, Y &year, M &month, D &day, H &hour, I &minute, S &second, Htz &tzhours, Itz &tzminutes) {
    auto timeSep = str.find('T');
    if (timeSep <= 0) {
        year = 0;
        month = 0;
        day = 0;
        hour = 0;
        minute = 0;
        second = 0;
        tzhours = 0;
        tzminutes = 0;
        return false;
    }
    auto dateStr = str.substr(0, timeSep);
    if (!ParseDateStr(dateStr, year, month, day)) {
        hour = 0;
        minute = 0;
        second = 0;
        tzhours = 0;
        tzminutes = 0;
        return false;
    }
    if (month < 1 || month > 12 || day < 1 || day > 31) {
        year = 0;
        month = 0;
        day = 0;
        hour = 0;
        minute = 0;
        second = 0;
        tzhours = 0;
        tzminutes = 0;
        return false;
    }
    auto offsetSep = str.find('+', timeSep + 1);
    if (offsetSep == std::string::npos) {
        offsetSep = str.find('-', timeSep + 1);
    }
    if (offsetSep == std::string::npos) {
        year = 0;
        month = 0;
        day = 0;
        hour = 0;
        minute = 0;
        second = 0;
        tzhours = 0;
        tzminutes = 0;
        return false;
    }
    auto fracSep = str.find('.', timeSep + 1);
    auto timeEnd = offsetSep;
    bool hasFrac = (fracSep != std::string::npos && fracSep < timeEnd);
    if (hasFrac) {
        timeEnd = fracSep;
    }
    std::string timeStr = str.substr(timeSep + 1, timeEnd - timeSep - 1);
    if (!ParseTimeStr(timeStr, hour, minute, second)) {
        year = 0;
        month = 0;
        day = 0;
        tzhours = 0;
        tzminutes = 0;
        return false;
    }
    if (hour > 23 || minute > 59 || second > 60) {
        year = 0;
        month = 0;
        day = 0;
        hour = 0;
        minute = 0;
        second = 0;
        tzhours = 0;
        tzminutes = 0;
        return false;
    }
    if (hasFrac) {
        std::string fracStr = str.substr(fracSep + 1, offsetSep - fracSep - 1);
        if (!IsUInteger(fracStr)) {
            year = 0;
            month = 0;
            day = 0;
            hour = 0;
            minute = 0;
            second = 0;
            tzhours = 0;
            tzminutes = 0;
            return false;
        }
    }
    std::string offsetStr = str.substr(offsetSep + 1);
    if (!ParseTimeStr(offsetStr, tzhours, tzminutes)) {
        year = 0;
        month = 0;
        day = 0;
        hour = 0;
        minute = 0;
        second = 0;
        return false;
    }
    if (tzhours > 24 || tzminutes > 59) {
        year = 0;
        month = 0;
        day = 0;
        hour = 0;
        minute = 0;
        second = 0;
        tzhours = 0;
        tzminutes = 0;
        return false;
    }
    if (str[offsetSep] == '-') {
#ifdef WIN32
        Htz tzh0 = 0;
        Itz tz0 = 0;
        tzhours = tzh0 - tzhours;
        tzminutes = tz0 - tzminutes;
#else
        tzhours = static_cast<decltype(tzhours)>(0) - tzhours;
        tzminutes = static_cast<decltype(tzminutes)>(0) - tzminutes;
#endif
    }
    return true;
}

#endif //THEMASTER_TIMECONST_H
