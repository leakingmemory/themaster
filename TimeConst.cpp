//
// Created by sigsegv on 11/1/24.
//

#include "TimeConst.h"

constexpr bool TestParseUInt(const std::string &str, unsigned int expected) {
    unsigned int result;
    if (!ParseUInt(str, result)) {
        return false;
    }
    return result == expected;
}

static_assert(!TestParseUInt("", 0));
static_assert(!TestParseUInt("a", 0));
static_assert(TestParseUInt("0", 0));
static_assert(TestParseUInt("1", 1));
static_assert(TestParseUInt("9", 9));
static_assert(TestParseUInt("00", 0));
static_assert(TestParseUInt("01", 1));
static_assert(TestParseUInt("09", 9));
static_assert(TestParseUInt("10", 10));
static_assert(TestParseUInt("11", 11));
static_assert(TestParseUInt("103", 103));

static_assert(IsUInteger("124567890"));
static_assert(!IsUInteger("a24567890"));

static_assert(IsLeapYear(1600) == true);
static_assert(IsLeapYear(1700) == false);
static_assert(IsLeapYear(1800) == false);
static_assert(IsLeapYear(1900) == false);
static_assert(IsLeapYear(2000) == true);
static_assert(IsLeapYear(2021) == false);
static_assert(IsLeapYear(2022) == false);
static_assert(IsLeapYear(2023) == false);
static_assert(IsLeapYear(2024) == true);

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

static constexpr bool TestDateParsing(const std::string &str, int expectedYear, int expectedMonth, int expectedDay) {
    uint32_t year;
    uint8_t month;
    uint8_t day;
    ParseDateStr(str, year, month, day);
    return (year == expectedYear && month == expectedMonth && day == expectedDay);
}

static_assert(TestDateParsing("2024-10-21", 2024, 10, 21));

constexpr bool TestParseTime(const std::string &str, int expectedHours, int expectedMinutes) {
    int hours;
    int minutes;
    if (!ParseTimeStr(str, hours, minutes)) {
        return false;
    }
    return hours == expectedHours && minutes == expectedMinutes;
}

static_assert(TestParseTime("01:02", 1, 2));
static_assert(TestParseTime("12:34", 12, 34));

constexpr bool TestParseTime(const std::string &str, int expectedHours, int expectedMinutes, int expectedSeconds) {
    int hours;
    int minutes;
    int seconds;
    if (!ParseTimeStr(str, hours, minutes, seconds)) {
        return false;
    }
    return hours == expectedHours && minutes == expectedMinutes && seconds == expectedSeconds;
}

static_assert(TestParseTime("12:34:56", 12, 34, 56));

constexpr bool TestParseDateTimeOffsetStr(const std::string &str, int year, int month, int day, int hour, int minute, int second, int tzh, int tzm) {
    int y;
    int m;
    int d;
    int h;
    int i;
    int s;
    int th;
    int tm;
    auto success = ParseDateTimeOffsetStr(str, y, m, d, h, i, s, th, tm);
    if (!success) {
        return false;
    }
    return y == year && m == month && d == day && h == hour && i == minute && s == second && th == tzh && tm == tzm;
}

static_assert(TestParseDateTimeOffsetStr("2024-11-03T12:34:56.000+13:34", 2024, 11, 3, 12, 34, 56, 13, 34));
static_assert(TestParseDateTimeOffsetStr("2024-11-03T12:34:56.000-13:34", 2024, 11, 3, 12, 34, 56, -13, -34));
