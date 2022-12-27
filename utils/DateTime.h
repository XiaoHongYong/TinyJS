#pragma once


int64_t getTimeInMillisecond();

class DateTime {
public:
    enum {
        SECOND_IN_ONE_DAY           = 60 * 60 * 24,
        MILLIS_IN_ONE_DAY           = 1000 * SECOND_IN_ONE_DAY,
    };

public:
    DateTime(bool localTime = true);
    DateTime(int64_t time, bool localTime = true);
    DateTime(struct tm *tm, int ms, bool localTime = true);
    DateTime(int year, int month, int day, int hour = 0, int minute = 0, int second = 0, int ms = 0, int dayOfWeek = -1, bool localTime = true);

    int year() const { return _year; }
    int month() const { return _month; }
    int day() const { return _day; }
    int hour() const { return _hour; }
    int minute() const { return _minute; }
    int second() const { return _second; }
    int dayOfWeek() const { return _dayOfWeek; }
    int ms() const { return _ms; }
    int timeZoneOffset() const { return _timeZoneOffset; }

    void setYear(int value) { _year = value; update(); }
    void setMonth(int value) { _month = value; update(); }
    void setDay(int value) { _day = value; update(); }
    void setHour(int value) { _hour = value; update(); }
    void setMinute(int value) { _minute = value; update(); }
    void setSecond(int value) { _second = value; update(); }
    void setMs(int value) { _ms = value; update(); }

    time_t getTime() const;
    void fromTime(time_t seconds);

    int64_t getTimeInMs() const;
    void fromTimeInMs(int64_t milliseconds);

    string toDateString() const;
    string toDateTimeString() const;

    cstr_t parse(cstr_t str, uint32_t length);
    bool fromString(cstr_t str, uint32_t length);

    bool isLeapYear() const { return isLeapYear(_year); }

    static bool isLeapYear(int year);

    static DateTime utcTime();
    static DateTime localTime();

    static string utcDateTimeString() { return utcTime().toDateTimeString(); }
    static string localDateTimeString() { return localTime().toDateTimeString(); }

protected:
    bool _parseEndCheck(cstr_t start, cstr_t end, cstr_t &p, bool datePart = true);
    void update();

    int                         _year;
    int                         _month, _day;
    int                         _hour;
    int                         _minute, _second;
    int                         _ms;
    int8_t                      _dayOfWeek;
    int16_t                     _timeZoneOffset;
    bool                        _isLocalTime;

};
