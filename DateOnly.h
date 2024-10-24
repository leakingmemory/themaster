//
// Created by sigsegv on 10/21/24.
//

#ifndef THEMASTER_DATEONLY_H
#define THEMASTER_DATEONLY_H

#include <cstdint>
#include <string>

class DateOnlyInvalidException : public std::exception {
public:
    DateOnlyInvalidException();
    const char * what() const noexcept override;
};

class DateOnly {
private:
    int32_t year;
    uint8_t month;
    uint8_t day;
public:
    constexpr DateOnly() : year(0), month(0), day(0) {}
    constexpr DateOnly(int32_t year, uint8_t month, uint8_t day) : year(year), month(month), day(day) {}
    DateOnly(const std::string &str);
    std::string ToString() const;
    static DateOnly Today();
    void AddDays(int days);
    void AddYears(int years);
    constexpr void SubtractDays(int days) {
        AddDays(0 - days);
    }
};


#endif //THEMASTER_DATEONLY_H
