//
// Created by sigsegv on 11/1/24.
//

#include "TimeConst.h"

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
