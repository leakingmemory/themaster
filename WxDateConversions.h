//
// Created by jeo on 11/15/24.
//

#ifndef THEMASTER_WXDATECONVERSIONS_H
#define THEMASTER_WXDATECONVERSIONS_H

class wxDateTime;
class DateOnly;

wxDateTime ToWxDateTime(DateOnly dateOnly);
bool IsValidDate(wxDateTime dateTime);
DateOnly ToDateOnly(wxDateTime dateTime);

#endif //THEMASTER_WXDATECONVERSIONS_H
