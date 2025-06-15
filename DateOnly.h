//
// Created by sigsegv on 10/21/24.
//

#ifndef THEMASTER_DATEONLY_H
#define THEMASTER_DATEONLY_H

#include <cstdint>
#include <ctime>
#include <string>

class DateOnlyInvalidException : public std::exception {
public:
    DateOnlyInvalidException();
    const char * what() const noexcept override;
};

class Duration;
class DateOnlyDiff;

class DateOnly {
    friend DateOnlyDiff;
private:
    int32_t year;
    uint8_t month;
    uint8_t day;
public:
    constexpr DateOnly() : year(0), month(0), day(0) {}
    constexpr DateOnly(int32_t year, uint8_t month, uint8_t day) : year(year), month(month), day(day) {}
    DateOnly(const std::string &str);
    static DateOnly FromDateTimeOffsetString(const std::string &str);
    std::string ToString() const;
    [[nodiscard]] constexpr decltype(year) GetYear() const {
        return year;
    }
    [[nodiscard]] constexpr int GetMonth() const {
        return month;
    }
    [[nodiscard]] constexpr int GetDayOfMonth() const {
        return day;
    }
    static DateOnly Today();
    void AddDays(int days);
    void AddMonths(int months);
    void AddYears(int years);
    DateOnly &operator += (Duration);
#ifndef WIN32
    constexpr
#endif
    void SubtractDays(int days) {
        AddDays(0 - days);
    }
    time_t GetStartOfDaySecondsFromEpoch() const;
    constexpr explicit operator bool () const {
        return year != 0 && month != 0 && day != 0;
    }
    constexpr bool operator < (const DateOnly &other) const {
        if (other.year < year) {
            return false;
        } else if (other.year > year) {
            if (other.year == 0 || year == 0) {
                return false;
            }
            return true;
        } else {
            if (year == 0) {
                return false;
            }
            if (other.month < month) {
                return false;
            } else if (other.month > month) {
                if (other.month == 0 || month == 0) {
                    return false;
                }
                return true;
            } else {
                if (month == 0) {
                    return false;
                }
                if (other.day < day) {
                    return false;
                } else if (other.day > day) {
                    if (other.day == 0 || day == 0) {
                        return false;
                    }
                    return true;
                } else {
                    return false;
                }
            }
        }
    }
    constexpr bool operator > (const DateOnly &other) const {
        return other < *this;
    }
    constexpr bool operator == (const DateOnly &other) const {
        return other.year == year && other.month == month && other.day == day;
    }
    constexpr bool operator <= (const DateOnly &other) const {
        return *this == other || *this < other;
    }
    constexpr bool operator >= (const DateOnly &other) const {
        return *this == other || *this > other;
    }
};

DateOnly operator + (DateOnly, Duration);

class DateOnlyDiff {
private:
    DateOnly start;
    DateOnly end;
public:
    constexpr DateOnlyDiff(DateOnly start, DateOnly end) : start(start), end(end) {}
    int32_t GetDays() const;
private:
    constexpr bool SameDay() const;
    constexpr DateOnly AddW(DateOnly d1) const;
public:
    DateOnly operator + (DateOnly) const;
};

constexpr DateOnlyDiff operator - (DateOnly d1, DateOnly d2) {
    return {d2, d1};
}
DateOnly operator + (DateOnly d1, DateOnlyDiff diff);

#endif //THEMASTER_DATEONLY_H
