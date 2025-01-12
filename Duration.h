//
// Created by sigsegv on 10/31/24.
//

#ifndef THEMASTER_DURATION_H
#define THEMASTER_DURATION_H

#include <cstdint>
#include <string>

class DurationParser;

class Duration {
    friend DurationParser;
private:
    int32_t years;
    int32_t days;
    int32_t hours;
    int8_t months;
    int8_t minutes;
    int8_t seconds;
    bool valid;
public:
    constexpr Duration() : years(0), days(0), months(0), hours(0), minutes(0), seconds(0), valid(false) {}
    static Duration FromString(const std::string &str);
    std::string ToString() const;
    constexpr explicit operator bool () const {
        return valid;
    }
    [[nodiscard]] constexpr decltype(years) GetYears() const {
        return years;
    }
    [[nodiscard]] constexpr decltype(months) GetMonths() const {
        return months;
    }
    [[nodiscard]] constexpr decltype(days) GetDays() const {
        return days;
    }
    constexpr bool HasNoTime() const {
        return hours == 0 && minutes == 0 && seconds == 0;
    }
    constexpr bool operator == (const Duration &other) const {
        if (!valid) {
            return !other.valid;
        } else if (!other.valid) {
            return false;
        }
        return years == other.years &&
            days == other.days &&
            hours == other.hours &&
            months == other.months &&
            minutes == other.minutes &&
            seconds == other.seconds;
    }
};


#endif //THEMASTER_DURATION_H
