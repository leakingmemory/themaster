//
// Created by sigsegv on 10/21/24.
//

#include "DateOnly.h"
#include <limits>
#include <time.h>

DateOnlyInvalidException::DateOnlyInvalidException() {}

const char *DateOnlyInvalidException::what() const noexcept {
    return "DateOnly was invalid";
}

static constexpr bool ParseStr(const std::string &str, uint32_t &year, uint8_t &month, uint8_t &day) {
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
        if (year > (std::numeric_limits<typeof(year)>::max() / 10)) {
            return false;
        }
        year *= 10;
        auto d = ch - '0';
        auto max = std::numeric_limits<typeof(year)>::max() - year;
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

static constexpr bool TestParsing(const std::string &str, int expectedYear, int expectedMonth, int expectedDay) {
    uint32_t year;
    uint8_t month;
    uint8_t day;
    ParseStr(str, year, month, day);
    return (year == expectedYear && month == expectedMonth && day == expectedDay);
}

static_assert(TestParsing("2024-10-21", 2024, 10, 21));

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

static_assert(IntToString(0) == "0");
static_assert(IntToString(1) == "1");
static_assert(IntToString(9) == "9");
static_assert(IntToString(0, 2) == "00");
static_assert(IntToString(1, 2) == "01");
static_assert(IntToString(9, 2) == "09");
static_assert(IntToString(10) == "10");
static_assert(IntToString(10, 2) == "10");
static_assert(IntToString(10, 4) == "0010");
static_assert(IntToString(2024) == "2024");
static_assert(IntToString(-1) == "-1");
static_assert(IntToString(-9) == "-9");
static_assert(IntToString(-1, 2) == "-01");
static_assert(IntToString(-9, 2) == "-09");
static_assert(IntToString(-10) == "-10");
static_assert(IntToString(-10, 2) == "-10");
static_assert(IntToString(-10, 4) == "-0010");
static_assert(IntToString(-2024) == "-2024");

static constexpr std::string DateToString(int32_t year, uint8_t month, uint8_t day) {
    std::string str{};
    str.reserve(10);
    AppendAsString(str, year, 4);
    char sep[2] = {'-', '\0'};
    str.append(sep);
    AppendAsString(str, month, 2);
    str.append(sep);
    AppendAsString(str, day, 2);
    return str;
}

static_assert(DateToString(2024, 10, 21) == "2024-10-21");
static_assert(DateToString(2024, 1, 2) == "2024-01-02");

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

static_assert(IsLeapYear(1600) == true);
static_assert(IsLeapYear(1700) == false);
static_assert(IsLeapYear(1800) == false);
static_assert(IsLeapYear(1900) == false);
static_assert(IsLeapYear(2000) == true);
static_assert(IsLeapYear(2021) == false);
static_assert(IsLeapYear(2022) == false);
static_assert(IsLeapYear(2023) == false);
static_assert(IsLeapYear(2024) == true);

template <typename T, typename M> static constexpr int GetDaysPerMonth(T year, M month) {
    --month;
    if constexpr(std::numeric_limits<M>::min() != 0) {
        if constexpr(!(std::numeric_limits<M>::min() > -128)) {
            while (month < (0 - (144 * 12))) {
                month += 144 * 12;
            }
        }
        while (month < 0) {
            month += 12;
        }
    }
    if (month >= 12) {
        month = month % 12;
    }
    if ((month % 2) == 0) {
        if (month < 7) {
            return 31;
        }
        return 30;
    } else if (month < 7) {
        if (month == 1) {
            return IsLeapYear(year) ? 29 : 28;
        }
        return 30;
    } else {
        return 31;
    }
}

static_assert(GetDaysPerMonth(2024, 1) == 31);
static_assert(GetDaysPerMonth(2024, 2) == 29);
static_assert(GetDaysPerMonth(2024, 3) == 31);
static_assert(GetDaysPerMonth(2024, 4) == 30);
static_assert(GetDaysPerMonth(2024, 5) == 31);
static_assert(GetDaysPerMonth(2024, 6) == 30);
static_assert(GetDaysPerMonth(2024, 7) == 31);
static_assert(GetDaysPerMonth(2024, 8) == 31);
static_assert(GetDaysPerMonth(2024, 9) == 30);
static_assert(GetDaysPerMonth(2024, 10) == 31);
static_assert(GetDaysPerMonth(2024, 11) == 30);
static_assert(GetDaysPerMonth(2024, 12) == 31);
static_assert(GetDaysPerMonth(2025, 2) == 28);

template <typename T> static constexpr int GetDaysPerYear(T year) {
    return IsLeapYear(year) ?  366 : 365;
}

template <typename T, typename M, typename D> static constexpr int GetRemainingDaysPerYear(T year, M month, D day) {
    if (day < 1 || month < 1 || month > 12) {
        return -1;
    }
    auto daysPerMonth = GetDaysPerMonth(year, month);
    if (day > daysPerMonth) {
        return -1;
    }
    auto total = 1 + daysPerMonth - day;
    for (auto i = month + 1; i < 13; i++) {
        total += GetDaysPerMonth(year, i);
    }
    return total;
}

static_assert(GetRemainingDaysPerYear(2024, 12, 31) == 1);
static_assert(GetRemainingDaysPerYear(2024, 12, 1) == 31);
static_assert(GetRemainingDaysPerYear(2024, 11, 30) == 32);
static_assert(GetRemainingDaysPerYear(2024, 1, 1) == 366);

template <typename T, typename M, typename D> static constexpr int GetDayOfYear(T year, M month, D day) {
    if (month < 2) {
        if (month == 1) {
            return day;
        }
        return -1;
    }
    if (month > 12) {
        return -1;
    }
    int total = 0;
    for (int i = 1; i < month; i++) {
        total += GetDaysPerMonth(year, i);
    }
    return total + day;
}

static_assert(GetDayOfYear(2024, 1, 1) == 1);
static_assert(GetDayOfYear(2024, 1, 31) == 31);
static_assert(GetDayOfYear(2024, 2, 1) == 32);
static_assert(GetDayOfYear(2024, 12, 31) == 366);

template <typename Y> static constexpr bool FromDayOfYear(Y year, int dayOfYear, int &month, int &day) {
    if (dayOfYear <= 0) {
        month = 0;
        day = 0;
        return false;
    }
    --dayOfYear;
    for (auto i = 1; i < 13; i++) {
        auto monDays = GetDaysPerMonth(year, i);
        if (dayOfYear < monDays) {
            month = i;
            day = dayOfYear + 1;
            return true;
        }
        dayOfYear -= monDays;
    }
    month = 0;
    day = 0;
    return false;
}

template <typename Y> static constexpr bool TestBidirDayOfYear(Y year, int day) {
    int month;
    int mday;
    if (!FromDayOfYear(year, day, month, mday)) {
        return false;
    }
    return GetDayOfYear(year, month, mday) == day;
}

static_assert(TestBidirDayOfYear(2024, 1));
static_assert(TestBidirDayOfYear(2024, 31));
static_assert(TestBidirDayOfYear(2024, 32));
static_assert(TestBidirDayOfYear(2024, 366));

template <typename Y> static constexpr bool TestBidirDayOfYear(Y year) {
    auto ydays = GetDaysPerYear(year);
    for (auto i = 0; i < ydays; i++) {
        if (!TestBidirDayOfYear(year, i + 1)) {
            return false;
        }
    }
    return true;
}

static_assert(TestBidirDayOfYear(2023));
static_assert(TestBidirDayOfYear(2024));

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

static constexpr bool Test400YearsAlwaysSameAmountOfDays(int from, int to) {
    for (auto i = from; i < to; i++) {
        if (daysPer400Years != GetDaysPerYears<uint32_t>(i, 400)) {
            return false;
        }
    }
    return true;
}

static_assert(Test400YearsAlwaysSameAmountOfDays(1, 100));
static_assert(Test400YearsAlwaysSameAmountOfDays(100, 200));
static_assert(Test400YearsAlwaysSameAmountOfDays(200, 300));
static_assert(Test400YearsAlwaysSameAmountOfDays(300, 400));

template <typename T, typename Y> constexpr T GetDaysFromEpoch(Y year) {
    if (year >= 1970) {
        auto years = year - 1970;
        Y n400 = years / 400;
        T days = n400;
        days = days * daysPer400Years;
        auto y = n400 * 400;
        y += 1970;
        years = years % 400;
        days += GetDaysPerYears<T>(y, years);
        return days;
    } else {
        auto years = 1970 - year;
        Y n400 = years / 400;
        T days = ((T) 0) - n400;
        days = days * daysPer400Years;
        years = years % 400;
        days -= GetDaysPerYears<T>(year, years);
        return days;
    }
}

static_assert(GetDaysFromEpoch<int32_t>(1969) == -365);
static_assert(GetDaysFromEpoch<int32_t>(1970) == 0);
static_assert(GetDaysFromEpoch<int32_t>(1971) == 365);

template <typename T, typename Y, typename M, typename D> constexpr T GetDaysFromEpoch(Y year, M month, D day) {
    T days = GetDaysFromEpoch<T>(year);
    days += GetDayOfYear(year, month, day) - 1;
    return days;
}

static_assert(GetDaysFromEpoch<int32_t>(1969, 12, 31) == -1);
static_assert(GetDaysFromEpoch<int32_t>(1970, 1, 1) == 0);
static_assert(GetDaysFromEpoch<int32_t>(1970, 1, 2) == 1);

template <typename T, typename Y, typename M, typename D> constexpr T InlineStartOfDaySecondsFromEpoch(Y year, M month, D day) {
    T tm = GetDaysFromEpoch<T>(year, month, day);
    tm = tm * (24*60*60);
    return tm;
}

static_assert(InlineStartOfDaySecondsFromEpoch<int32_t>(1969, 12, 31) == -(24*60*60));
static_assert(InlineStartOfDaySecondsFromEpoch<int32_t>(1970, 1, 1) == 0);
static_assert(InlineStartOfDaySecondsFromEpoch<int32_t>(1970, 1, 2) == (24*60*60));

DateOnly::DateOnly(const std::string &str) {
    {
        uint32_t y;
        uint8_t m;
        uint8_t d;
        if (!ParseStr(str, y, m, d)) {
            throw DateOnlyInvalidException();
        }
        if (m > 12 || m < 1 || d > 31 || d < 1) {
            throw DateOnlyInvalidException();
        }
        year = y;
        month = m;
        day = d;
    }
}

std::string DateOnly::ToString() const {
    return DateToString(year, month, day);
}

DateOnly DateOnly::Today() {
    auto secondsSinceEpochZ = time(nullptr);
    struct tm t{};
    if (localtime_r(&secondsSinceEpochZ, &t) != &t) {
        throw std::exception();
    }
    return DateOnly(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
}

static constexpr unsigned int NegativeValueAsPositive(int negativeValue) {
    if (negativeValue > 0) {
        return 0;
    }
    int64_t val = negativeValue;
    val = 0 - val;
    return static_cast<unsigned int>(val);
}

template <typename Y, typename M, typename D> static constexpr bool InlineAddDays(Y &year, M &month, D &day, int days) {
    if (days == 0) {
        return true;
    }
    if (days < 0) {
        unsigned int subtractDays = NegativeValueAsPositive(days);
        unsigned int yday = GetDayOfYear(year, month, day);
        if (subtractDays < yday) {
            yday -= subtractDays;
            int m, d;
            if (FromDayOfYear(year, yday, m, d)) {
                month = m;
                day = d;
                return true;
            } else {
                return false;
            }
        }
        subtractDays -= yday;
        --year;
        month = 12;
        day = 31;
        if (subtractDays > daysPer400Years) {
            year -= (subtractDays / daysPer400Years) * 400;
            subtractDays %= daysPer400Years;
        }
        while (subtractDays > 0) {
            yday = GetDaysPerYear(year);
            if (subtractDays < yday) {
                yday -= subtractDays;
                int m, d;
                if (FromDayOfYear(year, yday, m, d)) {
                    month = m;
                    day = d;
                    return true;
                } else {
                    return false;
                }
            }
            subtractDays -= yday;
            --year;
        }
    } else {
        auto daysRemaining = GetRemainingDaysPerYear(year, month, day);
        if (days < daysRemaining) {
            unsigned int yday = GetDayOfYear(year, month, day);
            yday += days;
            int m, d;
            if (FromDayOfYear(year, yday, m, d)) {
                month = m;
                day = d;
                return true;
            } else {
                return false;
            }
        }
        days -= daysRemaining;
        ++year;
        month = 1;
        day = 1;
        if (days > daysPer400Years) {
            year += (days / daysPer400Years) * 400;
            days %= daysPer400Years;
        }
        while (days > 0) {
            auto daysRemaining = GetDaysPerYear(year);
            if (days < daysRemaining) {
                int m, d;
                if (FromDayOfYear(year, days + 1, m, d)) {
                    month = m;
                    day = d;
                    return true;
                } else {
                    return false;
                }
            }
            days -= daysRemaining;
            ++year;
        }
    }
    return true;
}

template <typename Y, typename M, typename D> static constexpr bool TestAddDays(Y year, M month, D day, int days, Y expectedYear, M expectedMonth, D expectedDay) {
    Y y{year};
    M m{month};
    D d{day};
    if (!InlineAddDays(y, m, d, days)) {
        return false;
    }
    if (y != expectedYear || m != expectedMonth || d != expectedDay) {
        return false;
    }
    if (!InlineAddDays(y, m, d, 0 - days)) {
        return false;
    }
    return y == year && m == month && d == day;
}

static_assert(TestAddDays(2024, 10, 23, 0, 2024, 10, 23));
static_assert(TestAddDays(2024, 10, 23, 1, 2024, 10, 24));
static_assert(TestAddDays(2024, 10, 23, 8, 2024, 10, 31));
static_assert(TestAddDays(2024, 10, 23, 9, 2024, 11, 1));
static_assert(TestAddDays(2024, 10, 23, 70, 2025, 1, 1));
static_assert(TestAddDays(2024, 12, 31, 1, 2025, 1, 1));
static_assert(TestAddDays(2024, 10, 23, 103, 2025, 2, 3));
static_assert(TestAddDays(2024, 10, 23, 103 + GetDaysPerYear(2025), 2026, 2, 3));
static_assert(TestAddDays(2024, 10, 23, 103 + GetDaysPerYear(2025) + daysPer400Years, 2426, 2, 3));
static_assert(TestAddDays(2024, 10, 23, 103 + GetDaysPerYear(2025) + (2*daysPer400Years), 2826, 2, 3));

template <typename Y, typename M, typename D> static constexpr void InlineAddYears(Y &year, M month, D &day, int years) {
    year += years;
    if (month == 2 && day == 29 && !IsLeapYear(year)) {
        day = 28;
    }
}

template <typename Y, typename M, typename D> static constexpr bool TestAddYears(Y year, M month, D day, int years, Y expectedYear, M expectedMonth, D expectedDay) {
    InlineAddYears(year, month, day, years);
    return year == expectedYear && month == expectedMonth && day == expectedDay;
}

static_assert(TestAddYears(2024, 2, 29, 0, 2024, 2, 29));
static_assert(TestAddYears(2024, 2, 29, 1, 2025, 2, 28));
static_assert(TestAddYears(2024, 2, 29, 4, 2028, 2, 29));
static_assert(TestAddYears(2024, 2, 29, -1, 2023, 2, 28));
static_assert(TestAddYears(2024, 2, 29, -4, 2020, 2, 29));
static_assert(TestAddYears(2024, 10, 23, 1, 2025, 10, 23));

void DateOnly::AddDays(int days) {
    if (!InlineAddDays(year, month, day, days)) {
        throw std::exception();
    }
}

void DateOnly::AddYears(int years) {
    InlineAddYears(year, month, day, years);
}

time_t DateOnly::GetStartOfDaySecondsFromEpoch() const {
    return InlineStartOfDaySecondsFromEpoch<time_t>(year, month, day);
}

static_assert(!DateOnly());
static_assert(DateOnly(2024, 10, 30));
static_assert(!(DateOnly(2024, 10, 31) < DateOnly(2024, 10, 30)));
static_assert(!(DateOnly(2024, 10, 30) < DateOnly(2024, 10, 30)));
static_assert(DateOnly(2024, 10, 30) < DateOnly(2024, 10, 31));
static_assert(!(DateOnly(2024, 11, 30) < DateOnly(2024, 10, 30)));
static_assert(DateOnly(2024, 10, 30) < DateOnly(2024, 11, 30));
static_assert(!(DateOnly(2025, 10, 30) < DateOnly(2024, 10, 30)));
static_assert(DateOnly(2024, 10, 30) < DateOnly(2025, 10, 30));

static_assert(DateOnly(2024, 10, 31) > DateOnly(2024, 10, 30));
static_assert(!(DateOnly(2024, 10, 30) > DateOnly(2024, 10, 30)));
static_assert(!(DateOnly(2024, 10, 30) > DateOnly(2024, 10, 31)));
static_assert(DateOnly(2024, 11, 30) > DateOnly(2024, 10, 30));
static_assert(!(DateOnly(2024, 10, 30) > DateOnly(2024, 11, 30)));
static_assert(DateOnly(2025, 10, 30) > DateOnly(2024, 10, 30));
static_assert(!(DateOnly(2024, 10, 30) > DateOnly(2025, 10, 30)));

static_assert(!(DateOnly(2024, 10, 31) <= DateOnly(2024, 10, 30)));
static_assert(DateOnly(2024, 10, 30) <= DateOnly(2024, 10, 30));
static_assert(DateOnly(2024, 10, 30) <= DateOnly(2024, 10, 31));
static_assert(!(DateOnly(2024, 11, 30) <= DateOnly(2024, 10, 30)));
static_assert(DateOnly(2024, 10, 30) <= DateOnly(2024, 11, 30));
static_assert(!(DateOnly(2025, 10, 30) <= DateOnly(2024, 10, 30)));
static_assert(DateOnly(2024, 10, 30) <= DateOnly(2025, 10, 30));

static_assert(DateOnly(2024, 10, 31) >= DateOnly(2024, 10, 30));
static_assert(DateOnly(2024, 10, 30) >= DateOnly(2024, 10, 30));
static_assert(!(DateOnly(2024, 10, 30) >= DateOnly(2024, 10, 31)));
static_assert(DateOnly(2024, 11, 30) >= DateOnly(2024, 10, 30));
static_assert(!(DateOnly(2024, 10, 30) >= DateOnly(2024, 11, 30)));
static_assert(DateOnly(2025, 10, 30) >= DateOnly(2024, 10, 30));
static_assert(!(DateOnly(2024, 10, 30) >= DateOnly(2025, 10, 30)));

static_assert(DateOnly(2024, 10, 31) != DateOnly(2024, 10, 30));
static_assert(DateOnly(2024, 10, 30) == DateOnly(2024, 10, 30));
static_assert(DateOnly(2024, 10, 30) != DateOnly(2024, 10, 31));
static_assert(DateOnly(2024, 11, 30) != DateOnly(2024, 10, 30));
static_assert(DateOnly(2024, 10, 30) != DateOnly(2024, 11, 30));
static_assert(DateOnly(2025, 10, 30) != DateOnly(2024, 10, 30));
static_assert(DateOnly(2024, 10, 30) != DateOnly(2025, 10, 30));
