//
// Created by sigsegv on 5/24/24.
//

#ifndef THEMASTER_DATETIME_H
#define THEMASTER_DATETIME_H

#include <ctime>
#include <string>

class DateTime {
private:
    std::time_t timeInSecondsSinceEpoch;
public:
    constexpr explicit DateTime(std::time_t timeInSecondsSinceEpoch) : timeInSecondsSinceEpoch(timeInSecondsSinceEpoch) {}
    constexpr DateTime() : DateTime(0) {}
    [[nodiscard]] std::string to_iso8601() const;
};

class DateTimeOffset {
private:
    std::time_t timeInSecondsSinceEpoch;
    int32_t offsetSeconds;
public:
    constexpr DateTimeOffset(std::time_t timeInSecondsSinceEpoch, int32_t offsetSeconds) : timeInSecondsSinceEpoch(timeInSecondsSinceEpoch), offsetSeconds(offsetSeconds) {}
    explicit DateTimeOffset(std::time_t timeInSecondsSinceEpoch);
    constexpr DateTimeOffset() : DateTimeOffset(0,0) {}
    static DateTimeOffset Now();
    [[nodiscard]] std::string to_iso8601() const;
};

#endif //THEMASTER_DATETIME_H
