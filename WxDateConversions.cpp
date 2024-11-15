//
// Created by jeo on 11/15/24.
//

#include "WxDateConversions.h"
#include <wx/datetime.h>
#include "DateOnly.h"

wxDateTime ToWxDateTime(DateOnly dateOnly) {
    auto setTime = wxDateTime::Now();
    setTime.ResetTime();
    setTime.SetMonth(wxDateTime::Jan);
    constexpr std::array<wxDateTime::Month,12> monthMap = {wxDateTime::Jan, wxDateTime::Feb, wxDateTime::Mar, wxDateTime::Apr, wxDateTime::May, wxDateTime::Jun, wxDateTime::Jul, wxDateTime::Aug, wxDateTime::Sep, wxDateTime::Oct, wxDateTime::Nov, wxDateTime::Dec};
    setTime.SetYear(dateOnly.GetYear());
    setTime.SetDay(dateOnly.GetDayOfMonth());
    setTime.SetMonth(monthMap.at(dateOnly.GetMonth() - 1));
    return setTime;
}

bool IsValidDate(wxDateTime dateTime) {
    auto year = dateTime.GetYear();
    auto month = dateTime.GetMonth();
    auto day = dateTime.GetDay();
    return year != wxDateTime::Inv_Year && month != wxDateTime::Inv_Month && day > 0 && day < 32;
}

DateOnly ToDateOnly(wxDateTime dateTime) {
    if (!IsValidDate(dateTime)) {
        return {};
    }
    auto year = dateTime.GetYear();
    auto month = dateTime.GetMonth();
    auto day = dateTime.GetDay();
    uint8_t monthNum;
    switch (month) {
        case wxDateTime::Jan:
            monthNum = 1;
            break;
        case wxDateTime::Feb:
            monthNum = 2;
            break;
        case wxDateTime::Mar:
            monthNum = 3;
            break;
        case wxDateTime::Apr:
            monthNum = 4;
            break;
        case wxDateTime::May:
            monthNum = 5;
            break;
        case wxDateTime::Jun:
            monthNum = 6;
            break;
        case wxDateTime::Jul:
            monthNum = 7;
            break;
        case wxDateTime::Aug:
            monthNum = 8;
            break;
        case wxDateTime::Sep:
            monthNum = 9;
            break;
        case wxDateTime::Oct:
            monthNum = 10;
            break;
        case wxDateTime::Nov:
            monthNum = 11;
            break;
        case wxDateTime::Dec:
            monthNum = 12;
            break;
        default:
            return {};
    }
    return {year, monthNum, static_cast<uint8_t>(day)};
}
