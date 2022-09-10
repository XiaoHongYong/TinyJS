
#pragma once

class DateTime {
public:
    uint8_t                 day, month;
    uint16_t                year;
    uint8_t                 hour;
    uint8_t                 minute, second;
    uint16_t                ms;

    enum {
        SECOND_IN_ONE_DAY = 60 * 60 * 24,
        MILLIS_IN_ONE_DAY = 1000 * SECOND_IN_ONE_DAY,
    };

public:
    DateTime();
    DateTime(int year, int month, int day);
    DateTime(int year, int month, int day, int hour, int minute, int second, int ms);

    // Returns this Date as a second value.
    // The value is the number of seconds since Jan. 1, 1970, midnight GMT.
    uint32_t getTime();
    void fromTime(uint32_t timeInMillis);

    string toUtcDateString();
    string toUtcDateTimeString();

    bool fromString(cstr_t szDate);

    bool isLeapYear() const { return isLeapYear(year); }

    static bool isLeapYear(int year);

    static DateTime getCurrentDate();

    static DateTime getLocalTime();

};
