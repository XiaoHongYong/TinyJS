#include "UtilsTypes.h"
#include "DateTime.h"
#include "StringEx.h"
#include <chrono>
#include <math.h>
#include <time.h>


using namespace std::chrono;

static const int _MonthDays[12] = {
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30
};

inline int getDaysToYear(int year) {
    year--;
    return year * 365 + year / 4 - year / 100 + year / 400;
}

inline int getDaysToMonth(int year, int month) {
    assert(month >= 1 && month <= 12);
    int n = _MonthDays[month - 1];
    if (DateTime::isLeapYear(year) && month > 2) {
        n++;
    }
    return n;
}

int64_t getTimeInMillisecond() {
    system_clock::time_point now = system_clock::now();
    auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    return now_ms.time_since_epoch().count();
}

DateTime::DateTime(bool localTime) : DateTime(getTimeInMillisecond(), localTime) {
}

tm *msToTm(int64_t timeInMs, bool isLocalTime) {
    time_t t = timeInMs / 1000;
    return isLocalTime ? localtime(&t) : gmtime(&t);
}

DateTime::DateTime(int64_t timeInMs, bool localTime) : DateTime(msToTm(timeInMs, localTime), timeInMs % 1000, localTime) {
}

DateTime::DateTime(struct tm *tm, int ms, bool localTime) : DateTime(tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, (int)ms, tm->tm_wday, localTime) {
}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int ms, int dayOfWeek, bool isLocalTime) {
    _day = day;
    _month = month;
    _year = year;
    _hour = hour;
    _minute = minute;
    _second = second;
    _ms = ms;
    _timeZoneOffset = 0;
    _isLocalTime = isLocalTime;

    if (dayOfWeek == -1) {
        auto t = getTime();
        dayOfWeek = isLocalTime ? localtime(&t)->tm_wday : gmtime(&t)->tm_wday;
    }
    _dayOfWeek = (int8_t)dayOfWeek;
}

time_t DateTime::getTime() const {
    struct tm t = {};
    t.tm_year = _year - 1900;
    t.tm_mon = _month - 1;
    t.tm_mday = _day;
    t.tm_hour = _hour;
    t.tm_min = _minute;
    t.tm_sec = _second;

#ifdef _WIN32
    auto v = std::mktime(&t);
    long seconds = 0;
    if (!_isLocalTime && _get_timezone(&seconds) == 0) {
        v -= seconds;
    }
    return v;
#else
    return _isLocalTime ? timelocal(&t) : timegm(&t);
#endif
}

void DateTime::fromTime(time_t time) {
    auto tm = _isLocalTime ? localtime(&time) : gmtime(&time);
    _year = tm->tm_year + 1900;
    _month = tm->tm_mon + 1;
    _day = tm->tm_mday;
    _hour = tm->tm_hour;
    _minute = tm->tm_min;
    _second = tm->tm_sec;
    _ms = 0;
}

int64_t DateTime::getTimeInMs() const {
    return getTime() * 1000 + _ms;
}

void DateTime::fromTimeInMs(int64_t milliseconds) {
    fromTime(milliseconds / 1000);
    _ms = milliseconds % 1000;
}

string DateTime::toDateString() const {
    char str[64];
    sprintf(str, "%04d-%02d-%02d", _year, _month, _day);
    return str;
}

string DateTime::toDateTimeString() const {
    char str[64];
    sprintf(str, "%04d-%02d-%02d %02d:%02d.%02d", _year, _month, _day, _hour, _minute, _second);
    return str;
}

bool DateTime::_parseEndCheck(cstr_t start, cstr_t end, cstr_t &p, bool datePart) {
    p = ignoreSpace(p, end);
    if (p == end) {
        update();
        return true;
    }

    if (*p == 'Z') {
        p++;
        p = ignoreSpace(p, end);
        if (p == end) {
            _isLocalTime = false;
            update();
            return true;
        } else {
            p = start;
            return true;
        }
    } else if (!datePart && (*p == '+' || *p == '-')) {
        // 时区
        // +hh:mm
        int h = 0, m = 0;
        int sign = *p == '+' ? -1 : 1;
        p++;
        p = parseInt(p, end, h);
        if (*p == ':') {
            p++;
            p = ignoreSpace(p, end);
            p = parseInt(p, end, m);
            p = ignoreSpace(p, end);
            if (p == end) {
                _isLocalTime = false;
                _hour += h * sign;
                _minute += m * sign;
                update();
                return true;
            }
        }

        // 错误格式
        p = start;
        return false;
    }

    return false;
}

static bool expectDateNumber(cstr_t start, cstr_t end, int min, int max, cstr_t &p, int &n) {
    p = ignoreSpace(p, end);
    auto tmp = parseInt(p, end, n);
    if (tmp == p || n > max || n < min) {
        p = start;
        return false;
    }

    p = tmp;
    p = ignoreSpace(p, end);
    return true;
}

cstr_t DateTime::parse(cstr_t start, uint32_t length) {
    memset(this, 0, sizeof(*this));

    _month = 1;
    _day = 1;
    _isLocalTime = false;

    auto p = start;
    auto end = start + length;

    int *fields[] =   { &_year, &_month, &_day };
    int fieldsMin[] = { 0,      1,       1 };
    int fieldsMax[] = { 275760, 12,      31 };
    int exactLength[] = { 4,    7,       10};

    for (int i = 0; i < CountOf(fields); i++) {
        if (!expectDateNumber(start, end, fieldsMin[i], fieldsMax[i], p, *fields[i])) {
            return p;
        }

        if (p == end) {
            if (p - start != exactLength[i]) _isLocalTime = true;
            update();
            return p;
        }

        if (_parseEndCheck(start, end, p)) {
            return p;
        }

        if (i < CountOf(fields) - 1) {
            if ((*p != '-' && *p != '/')) {
                return start;
            }

            if (*p != '-') _isLocalTime = true;
            p++;
        }
    }

    if (*p == 'T') {
        p++;
    }
    _isLocalTime = true;

    int *timeFields[] =   { &_hour, &_minute, &_second, &_ms };
    int timeFieldsMin[] = { 0,      0,        0,        0 };
    int timeFieldsMax[] = { 24,     59,       59,       999 };

    for (int i = 0; i < CountOf(timeFields); i++) {
        if (!expectDateNumber(start, end, timeFieldsMin[i], timeFieldsMax[i], p, *timeFields[i])) {
            return p;
        }

        if (i > 0 && _hour == 24 && *timeFields[i] > 0) {
            // 非法的时间 24:x:y:z
            return start;
        }

        if (_parseEndCheck(start, end, p, false)) {
            return p;
        }

        if (i < 2) {
            if (*p != ':' && *p != '.') {
                return start;
            }
            p++;
        } else if (i == 2) {
            // 接下来是毫秒
            if (*p == '.') {
                // 按照小数处理
                p++;
                auto tmp = parseInt(p, end, _ms);
                if (tmp > p) {
                    int len = (int)(tmp - p);
                    _ms = (int)(pow(10, -len + 3) * _ms);

                    if (_hour == 24 && _ms > 0) {
                        // 格式错误
                        return start;
                    } else if (_parseEndCheck(start, end, tmp, false)) {
                        return end;
                    }
                }

                // 格式错误
                return start;
            } else if (*p != ':') {
                // 不是毫秒
                return start;
            }
            p++;
        }
    }

    return start;
}

bool DateTime::fromString(cstr_t str, uint32_t length) {
    auto p = parse(str, length);
    return p == str + length;
}

DateTime DateTime::utcTime() {
    return DateTime(::getTimeInMillisecond(), false);
}

DateTime DateTime::localTime() {
    return DateTime(::getTimeInMillisecond(), true);
}

bool DateTime::isLeapYear(int year) {
    if (year % 4 != 0) {
        return false;
    }

    if (year % 400 == 0) {
        return true;
    }
    if (year % 100 == 0) {
        return false;
    }

    return true;
}

void DateTime::update() {
    struct tm t = {};
    t.tm_year = _year - 1900;
    t.tm_mon = _month - 1;
    t.tm_mday = _day;
    t.tm_hour = _hour;
    t.tm_min = _minute;
    t.tm_sec = _second;

#ifdef _WIN32
    int64_t time = std::mktime(&t);
    long seconds = 0;
    if (_get_timezone(&seconds)) {
        time += seconds;
    }
#else
    int64_t time = _isLocalTime ? timelocal(&t) : timegm(&t);
#endif

    time = time * 1000 + _ms;

    fromTimeInMs(time);
}
