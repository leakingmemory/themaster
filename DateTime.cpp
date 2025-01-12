//
// Created by sigsegv on 5/24/24.
//

#include <time.h>
#include "DateTime.h"
#include "DateOnly.h"
#include "TimeConst.h"
#include <sstream>
#include <array>
#include <cstdint>
#include <chrono>
#ifdef WIN32
#include <windows.h>
#endif

#ifdef WIN32
#define timegm _mkgmtime
#endif

constexpr int Digits10(unsigned int n) {
    if (n == 0) {
        return 1;
    }
    int d = 0;
    while (n > 0) {
        n /= 10;
        ++d;
    }
    return d;
}

constexpr int NPow10(int p) {
    int res = 1;
    for (int i = 0; i < p; i++) {
        res *= 10;
    }
    return res;
}

static_assert(NPow10(0) == 1);
static_assert(NPow10(1) == 10);
static_assert(NPow10(2) == 100);

template <int S, int P, int N> constexpr void NumToString(std::array<char, S> &res, int off, std::array<char, P> prefix, unsigned int year) {
    for (int i = 0; i < P; i++) {
        if ((off + i) < S) {
            res[off + i] = prefix[i];
        }
    }
    for (int i = 0; i < N; i++) {
        if ((off + P + i) < S) {
            auto digit = (year / NPow10(N - i - 1)) % 10;
            res[off + P + i] = '0' + digit;
        }
    }
}

template <int P, int N> constexpr std::array<char, P + N> NumToString(std::array<char, P> prefix, unsigned int year) {
    std::array<char, P + N> result{};
    NumToString<P+N,P,N>(result, 0, prefix, year);
    return result;
}

static_assert(Digits10(2014) == 4);
static_assert(NumToString<1, 4>({(char)'+'}, 2014) == std::array<char,5>({'+', '2', '0', '1', '4'}));

constexpr std::array<char, 10> DateToIso8601(int year, int month, int day) {
    std::array<char,10> result{};
    int off = 0;
    if (year >= 0) {
        NumToString<10, 0, 4>(result, off, std::array<char,0>(), year);
        off += 4;
    } else {
        int dy = Digits10(0 - year);
        NumToString<10, 1, 3>(result, off, {'-'}, year);
        off += 4;
    }
    if (month < 0) {
        month = 0;
    } else if (month > 99) {
        month = 99;
    }
    NumToString<10, 1, 2>(result, off, {'-'}, month);
    off += 3;
    if (day < 0) {
        day = 0;
    } else if (day > 99) {
        day = 99;
    }
    NumToString<10, 1, 2>(result, off, {'-'}, day);
    return result;
}

static_assert(DateToIso8601(2024, 05, 27) == std::array<char, 10>({'2', '0', '2', '4', '-', '0', '5', '-', '2', '7'}));

constexpr std::array<char, 16> TimeToIso8601(int hour, int minutes, int seconds, uint32_t us) {
    std::array<char,16> result{};
    if (hour < 0) {
        hour = 0;
    } else if (hour > 99) {
        hour = 99;
    }
    if (minutes < 0) {
        minutes = 0;
    } else if (minutes > 99) {
        minutes = 99;
    }
    if (seconds < 0) {
        seconds = 0;
    } else if (seconds > 99) {
        seconds = 99;
    }
    if (us > 999999) {
        us = 999999;
    }
    us *= 10;
    NumToString<16,0,2>(result, 0, std::array<char,0>(), hour);
    NumToString<16,1,2>(result, 2, std::array<char,1>({':'}), minutes);
    NumToString<16,1,2>(result, 5, std::array<char,1>({':'}), seconds);
    NumToString<16,1,7>(result, 8, std::array<char,1>({'.'}), us);
    return result;
}

static_assert(TimeToIso8601(19, 48, 12, 123456) == std::array<char,16>({'1', '9', ':', '4', '8', ':', '1', '2', '.', '1', '2', '3', '4', '5', '6', '0'}));

template <typename T, int An, int Bn> constexpr std::array<T, An + Bn> Concatenate(std::array<T, An> a, std::array<T, Bn> b) {
    std::array<T, An + Bn> result{};
    for (int i = 0; i < An; i++) {
        result[i] = a[i];
    }
    for (int i = 0; i < Bn; i++) {
        result[An + i] = b[i];
    }
    return result;
}

constexpr std::array<char, 27> DateTimeToIso8601(int year, int month, int day, int hour, int minutes, int seconds, uint32_t us) {
    return Concatenate<char,11,16>(Concatenate<char,10,1>(DateToIso8601(year, month, day), std::array<char, 1>({'T'})), TimeToIso8601(hour, minutes, seconds, us));
}

static_assert(DateTimeToIso8601(2024, 05, 27, 19, 48, 12, 123456) == std::array<char,27>({'2', '0', '2', '4', '-', '0', '5', '-', '2', '7', 'T', '1', '9', ':', '4', '8', ':', '1', '2', '.', '1', '2', '3', '4', '5', '6', '0'}));

constexpr std::array<char, 6> OffsetToIso8601(int hours, int minutes) {
    auto positive = hours >= 0;
    if (!positive) {
        hours = 0 - hours;
    }
    if (hours > 99) {
        hours = 99;
    }
    if (minutes < 0) {
        minutes = 0;
    } else if (minutes > 99) {
        minutes = 99;
    }
    std::array<char,6> result{};
    NumToString<6, 1, 2>(result, 0, std::array<char,1>({positive ? '+' : '-'}), hours);
    NumToString<6, 1, 2>(result, 3, std::array<char,1>({':'}), minutes);
    return result;
}

static_assert(OffsetToIso8601(2, 0) == std::array<char, 6>({'+', '0', '2', ':', '0', '0'}));
static_assert(OffsetToIso8601(-2, 0) == std::array<char, 6>({'-', '0', '2', ':', '0', '0'}));

constexpr std::array<char,33> DateTimeOffsetToIso8601(int year, int month, int day, int hour, int minutes, int seconds, uint32_t us, int offsetHours, int offsetMinutes) {
    return Concatenate<char,27,6>(DateTimeToIso8601(year, month, day, hour, minutes, seconds, us), OffsetToIso8601(offsetHours, offsetMinutes));
}

template <int N> constexpr void AppendToString(std::string &str, std::array<char,N> arr) {
    str.append(arr.begin(), arr.end());
}

constexpr void AppendIso8601ToString(std::string &str, int year, int month, int day, int hour, int minutes, int seconds, uint32_t us) {
    AppendToString<27>(str, DateTimeToIso8601(year, month, day, hour, minutes, seconds, us));
}

constexpr void AppendIso8601ToString(std::string &str, int year, int month, int day, int hour, int minutes, int seconds, uint32_t us, int offsetHours, int offsetMinutes) {
    AppendToString<33>(str, DateTimeOffsetToIso8601(year, month, day, hour, minutes, seconds, us, offsetHours, offsetMinutes));
}

std::string DateTime::to_iso8601() const {
    struct tm t{};
#ifdef WIN32
    if (gmtime_s(&t, &timeInSecondsSinceEpoch) != 0) {
#else
    if (gmtime_r(&timeInSecondsSinceEpoch, &t) != &t) {
#endif
        throw std::exception();
    }
    std::string str{};
    AppendIso8601ToString(str, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, 0);
    return str;
}

DateTimeOffset::DateTimeOffset(std::time_t timeInSecondsSinceEpoch) {
    struct tm t{};
#ifdef WIN32
    TIME_ZONE_INFORMATION tzinfo;
    GetTimeZoneInformation(&tzinfo);
    this->timeInSecondsSinceEpoch = timeInSecondsSinceEpoch - (tzinfo.Bias * 60);
    this->offsetSeconds = (0 - tzinfo.Bias) * 60;
#else
    if (localtime_r(&timeInSecondsSinceEpoch, &t) != &t) {
        throw std::exception();
    }
    this->timeInSecondsSinceEpoch = timeInSecondsSinceEpoch + t.tm_gmtoff;
    this->offsetSeconds = t.tm_gmtoff;
#endif
}

constexpr void TimeOffsets(int32_t offset, int &hours, int &minutes) {
    hours = offset / 3600;
    if (offset < 0) {
        offset = 0 - offset;
    }
    minutes = (offset / 60) % 60;
}

int32_t DateTimeOffset::OffsetForTime(time_t tim) {
#ifdef WIN32
    TIME_ZONE_INFORMATION tzinfo;
    GetTimeZoneInformation(&tzinfo);
    return (0 - tzinfo.Bias) * 60;
#else
    struct tm t{};
    if (localtime_r(&tim, &t) != &t) {
        throw std::exception();
    }
    return t.tm_gmtoff;
#endif
}

DateTimeOffset DateTimeOffset::Now() {
    auto secondsSinceEpochZ = time(nullptr);
    auto off = OffsetForTime(secondsSinceEpochZ);
    return {secondsSinceEpochZ + off, (int32_t) off};
}

template <typename F> concept OffsetForTimeFunc = requires (F f) {
    { f(std::declval<std::time_t>()) } -> std::convertible_to<int32_t>;
};

template <OffsetForTimeFunc F> constexpr DateTimeOffset InlineFromLocaltime(std::time_t timeInSecondsSinceEpoch, F offsetForTime) {
    auto off0 = offsetForTime(timeInSecondsSinceEpoch);
    auto ref0 = timeInSecondsSinceEpoch - off0;
    auto off1 = offsetForTime(ref0);
    return {timeInSecondsSinceEpoch, off1};
}

constexpr int32_t FixedPlus1(std::time_t) {
    return 3600;
}

static_assert(InlineFromLocaltime(1730381994, FixedPlus1).GetTimeInSecondsSinceEpoch() == 1730381994);
static_assert(InlineFromLocaltime(1730381994, FixedPlus1).GetOffsetSeconds() == 3600);

DateTimeOffset DateTimeOffset::FromLocaltime(std::time_t timeInSecondsSinceEpoch) {
    return InlineFromLocaltime(timeInSecondsSinceEpoch, OffsetForTime);
}

DateTimeOffset DateTimeOffset::FromDate(DateOnly dateOnly) {
    return InlineFromLocaltime(dateOnly.GetStartOfDaySecondsFromEpoch(), OffsetForTime);
}

template <typename T, typename H, typename M> constexpr T GetTimeOffset(H h, M m) {
    T off = h;
    off *= 60;
    off += m;
    return off * 60;
}

static_assert(GetTimeOffset<int>(2, 1) == 7260);
static_assert(GetTimeOffset<int>(-2, -1) == -7260);

DateTimeOffset DateTimeOffset::FromString(const std::string &str) {
    std::tm tm{};
#ifdef WIN32
    int offH, offM;
#else
    int16_t offH, offM;
#endif
    ParseDateTimeOffsetStr(str, tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, offH, offM);
    tm.tm_year -= 1900;
    --tm.tm_mon;
#ifndef WIN32
    tm.tm_zone = nullptr;
#endif
    time_t t = timegm(&tm);
    auto off = GetTimeOffset<int32_t>(offH, offM);
    return {t + off, off};
}

std::string DateTimeOffset::to_iso8601() const {
    struct tm t{};
#ifdef WIN32
    if (gmtime_s(&t, &timeInSecondsSinceEpoch) != 0) {
#else
    if (gmtime_r(&timeInSecondsSinceEpoch, &t) != &t) {
#endif
        throw std::exception();
    }
    std::string str{};
    int offHours;
    int offMinutes;
    TimeOffsets(offsetSeconds, offHours, offMinutes);
    AppendIso8601ToString(str, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec, 0, offHours, offMinutes);
    return str;
}
