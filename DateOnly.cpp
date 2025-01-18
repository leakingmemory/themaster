//
// Created by sigsegv on 10/21/24.
//

#include "DateOnly.h"
#include "TimeConst.h"
#include "Duration.h"
#include <limits>
#include <time.h>

DateOnlyInvalidException::DateOnlyInvalidException() {}

const char *DateOnlyInvalidException::what() const noexcept {
    return "DateOnly was invalid";
}

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
        int32_t y;
        uint8_t m;
        uint8_t d;
        if (!ParseDateStr(str, y, m, d)) {
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

DateOnly DateOnly::FromDateTimeOffsetString(const std::string &str) {
    int32_t y;
    uint8_t m;
    uint8_t d;
    uint8_t h;
    uint8_t i;
    uint8_t s;
    uint8_t hz;
    uint8_t mz;
    if (ParseDateTimeOffsetStr(str, y, m, d, h, i, s, hz, mz)) {
        return {y, m, d};
    }
    throw DateOnlyInvalidException();;
}

std::string DateOnly::ToString() const {
    return DateToString(year, month, day);
}

DateOnly DateOnly::Today() {
    auto secondsSinceEpochZ = time(nullptr);
    struct tm t{};
#ifdef WIN32
    if (localtime_s(&t, &secondsSinceEpochZ) != 0) {
#else
    if (localtime_r(&secondsSinceEpochZ, &t) != &t) {
#endif
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

template <typename Y, typename M, typename D, typename T> static constexpr void InlineAddMonths(Y &year, M &month, D &day, T months) {
    typename std::remove_cvref<decltype(months)>::type years = months / 12;
    months = months % 12;
    months += month - 1;
    years += months / 12;
    months = months % 12;
    year += years;
    month = months + 1;
    auto max = GetDaysPerMonth(year, month);
    if (day > max) {
        day = max;
    }
}

template <typename Y, typename M, typename D, typename T> static constexpr bool TestAddMonths(Y year, M month, D day, T months, Y expectedYear, M expectedMonth, D expectedDay) {
    InlineAddMonths(year, month, day, months);
    return year == expectedYear && month == expectedMonth && day == expectedDay;
}

static_assert(TestAddMonths(2024, 10, 31, -16, 2023, 6, 30));
static_assert(TestAddMonths(2024, 10, 31, -15, 2023, 7, 31));
static_assert(TestAddMonths(2024, 10, 31, -14, 2023, 8, 31));
static_assert(TestAddMonths(2024, 10, 31, -13, 2023, 9, 30));
static_assert(TestAddMonths(2024, 10, 31, -12, 2023, 10, 31));
static_assert(TestAddMonths(2024, 10, 31, -1, 2024, 9, 30));
static_assert(TestAddMonths(2024, 10, 31, 0, 2024, 10, 31));
static_assert(TestAddMonths(2024, 10, 31, 1, 2024, 11, 30));
static_assert(TestAddMonths(2024, 10, 31, 12, 2025, 10, 31));
static_assert(TestAddMonths(2024, 10, 31, 13, 2025, 11, 30));
static_assert(TestAddMonths(2024, 10, 31, 14, 2025, 12, 31));
static_assert(TestAddMonths(2024, 10, 31, 15, 2026, 1, 31));
static_assert(TestAddMonths(2024, 10, 31, 16, 2026, 2, 28));

void DateOnly::AddDays(int days) {
    if (!InlineAddDays(year, month, day, days)) {
        throw std::exception();
    }
}

void DateOnly::AddMonths(int months) {
    InlineAddMonths(year, month, day, months);
}

void DateOnly::AddYears(int years) {
    InlineAddYears(year, month, day, years);
}

DateOnly &DateOnly::operator+=(Duration duration) {
    if (duration) {
        InlineAddYears(year, month, day, duration.GetYears());
        InlineAddMonths(year, month, day, duration.GetMonths());
        InlineAddDays(year, month, day, duration.GetDays());
    }
    return *this;
}

time_t DateOnly::GetStartOfDaySecondsFromEpoch() const {
    return InlineStartOfDaySecondsFromEpoch<time_t>(year, month, day);
}

DateOnly operator + (DateOnly orig, Duration dur) {
    DateOnly dateOnly{orig};
    dateOnly += dur;
    return dateOnly;
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

template <typename T> static constexpr T InlineGetDays(DateOnly start, DateOnly end) {
    if (start.GetYear() == end.GetYear()) {
        return GetDayOfYear(end.GetYear(), end.GetMonth(), end.GetDayOfMonth()) -
                GetDayOfYear(start.GetYear(), start.GetMonth(), start.GetDayOfMonth());
    }
    if (start.GetYear() > end.GetYear()) {
        return 0 - InlineGetDays<T>(end, start);
    }
    T counter = GetDaysPerYear(start.GetYear()) - GetDayOfYear(start.GetYear(), start.GetMonth(), start.GetDayOfMonth());
    decltype(end.GetYear()) year = start.GetYear() + 1;
    decltype(end.GetYear()) remainingYears = end.GetYear() - year;
    counter += (remainingYears / 400) * daysPer400Years;
    remainingYears = remainingYears % 400;
    while (remainingYears > 0) {
        counter += GetDaysPerYear(year);
        ++year;
        --remainingYears;
    }
    return counter + GetDayOfYear(end.GetYear(), end.GetMonth(), end.GetDayOfMonth());
}

static_assert(InlineGetDays<int>(DateOnly(2424, 11, 9), DateOnly(2024, 11, 8)) == (0-(daysPer400Years + 1)));
static_assert(InlineGetDays<int>(DateOnly(2026, 11, 8), DateOnly(2024, 11, 8)) == -730);
static_assert(InlineGetDays<int>(DateOnly(2025, 11, 8), DateOnly(2024, 11, 8)) == -365);
static_assert(InlineGetDays<int>(DateOnly(2025, 1, 1), DateOnly(2024, 12, 31)) == -1);
static_assert(InlineGetDays<int>(DateOnly(2024, 11, 7), DateOnly(2024, 10, 31)) == -7);
static_assert(InlineGetDays<int>(DateOnly(2024, 11, 7), DateOnly(2024, 11, 7)) == 0);
static_assert(InlineGetDays<int>(DateOnly(2024, 10, 31), DateOnly(2024, 11, 7)) == 7);
static_assert(InlineGetDays<int>(DateOnly(2024, 12, 31), DateOnly(2025, 1, 1)) == 1);
static_assert(InlineGetDays<int>(DateOnly(2024, 11, 8), DateOnly(2025, 11, 8)) == 365);
static_assert(InlineGetDays<int>(DateOnly(2024, 11, 8), DateOnly(2026, 11, 8)) == 730);
static_assert(InlineGetDays<int>(DateOnly(2024, 11, 8), DateOnly(2424, 11, 9)) == (daysPer400Years + 1));

int32_t DateOnlyDiff::GetDays() const {
    return InlineGetDays<int32_t>(start, end);
}

constexpr bool DateOnlyDiff::SameDay() const {
    if (start.month == end.month && start.day == end.day) {
        return true;
    }
    if (start.month == 2 && start.day == 29) {
        if ((end.month == 2 && end.day == 28) || (end.month == 3 && end.day == 1)) {
            return true;
        }
        return false;
    }
    if (end.month == 2 && end.day == 29) {
        if ((start.month == 2 && start.day == 28) || (start.month == 3 && start.day == 1)) {
            return true;
        }
    }
    return false;
}

constexpr DateOnly DateOnlyDiff::AddW(DateOnly d1) const {
    DateOnly d0{d1.year, d1.month, d1.day};
    if (SameDay()) {
        auto ydiff = end.year - start.year;
        d0.AddYears(ydiff);
        return d0;
    }
    auto days = GetDays();
    d0.AddDays(days);
    return d0;
}

DateOnly DateOnlyDiff::operator+(DateOnly d1) const {
    return this->AddW(d1);
}
DateOnly operator + (DateOnly d1, DateOnlyDiff diff) {
    return diff + d1;
}

