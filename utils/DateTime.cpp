#include "UtilsTypes.h"
#include "DateTime.h"
#include "StringEx.h"


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
    if (DateTime::isLeapYear(year) && month > 2)
        n++;
    return n;
}

DateTime::DateTime() {
    this->day = 0;
    this->month = 0;
    this->year = 0;
    this->hour = 0;
    this->minute = 0;
    this->second = 0;
    this->ms = 0;
}

DateTime::DateTime(int year, int month, int day) {
    this->day = (uint8_t)day;
    this->month = (uint8_t)month;
    this->year = (uint16_t)year;
    this->hour = 0;
    this->minute = 0;
    this->second = 0;
    this->ms = 0;
}

DateTime::DateTime(int year, int month, int day, int hour, int minute, int second, int ms) {
    this->day = (uint8_t)day;
    this->month = (uint8_t)month;
    this->year = (uint16_t)year;
    this->hour = (uint8_t)hour;
    this->minute = (uint8_t)minute;
    this->second = (uint8_t)second;
    this->ms = (uint16_t)ms;
}

uint32_t DateTime::getTime() {
    if (month <= 0 || month > 12)
        return 0;

    uint32_t nAllDays = getDaysToYear(year) + day - 1;

    nAllDays += getDaysToMonth(year, month);

    return (nAllDays - getDaysToYear(1970)) * SECOND_IN_ONE_DAY + ((hour * 60 + minute) * 60 + second);
}

void DateTime::fromTime(uint32_t timeInMillis) {
    ms = 0;

    second = timeInMillis % 60;
    timeInMillis /= 60;

    minute = timeInMillis % 60;
    timeInMillis /= 60;

    hour = timeInMillis % 24;
    timeInMillis /= 24;

    // Now, timeInMillis is Days.
    uint32_t daysTotal = timeInMillis + getDaysToYear(1970);

    // estimate the start year.
    year = timeInMillis / 365 + 1970;
    uint32_t daysToYear = getDaysToYear(year);
    while (daysToYear <= daysTotal)
    {
        daysToYear += 365;
        if (isLeapYear(year))
            daysToYear++;
        year++;
    }
    year--;

    int dayOfYear = daysTotal - getDaysToYear(year);
    assert(dayOfYear >= 0);
    for (month = 1; month <= 12; month++)
    {
        if ((int)dayOfYear < getDaysToMonth(year, month))
            break;
    }
    month--;

    day = dayOfYear - getDaysToMonth(year, month);
    day++;
}

string DateTime::toUtcDateString()
{
    char szDate[64];
    sprintf(szDate, "%04d-%02d-%02d", year, month, day);
    return szDate;
}

string DateTime::toUtcDateTimeString()
{
    char szDate[64];
    sprintf(szDate, "%04d-%02d-%02d %02d:%02d.%02d", year, month, day, hour, minute, second);
    return szDate;
}

bool DateTime::fromString(cstr_t szDate)
{
    // year
    szDate = parseInt(szDate, year);
    if (*szDate != '-' && *szDate != '/')
        return false;
    szDate++;

    // month
    szDate = parseInt(szDate, month);
    if (*szDate != '-' && *szDate != '/')
        return false;
    szDate++;

    // day
    szDate = parseInt(szDate, day);

    while (*szDate == ' ')
        szDate++;

    if (!*szDate)
    {
        ms = hour = minute = second = 0;
        return true;
    }

    // hour
    szDate = parseInt(szDate, hour);
    if (*szDate != ':')
        return false;
    szDate++;

    // minute
    szDate = parseInt(szDate, minute);
    if (*szDate != ':' && *szDate != '.')
        return false;
    szDate++;

    // second
    parseInt(szDate, second);

    ms = 0;
    return true;
}

DateTime DateTime::getCurrentDate()
{
#ifdef _WIN32
    SYSTEMTIME    sysTime;

    GetSystemTime(&sysTime);

    return DateTime(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
#else
    time_t        ltime;
    tm            *ptm;

    time(&ltime);
    ptm = gmtime(&ltime);

    return DateTime(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, 0);
#endif
}

DateTime DateTime::getLocalTime()
{
#ifdef _WIN32
    SYSTEMTIME    sysTime;

    ::getLocalTime(&sysTime);

    return DateTime(sysTime.wYear, sysTime.wMonth, sysTime.wDay, sysTime.wHour, sysTime.wMinute, sysTime.wSecond, sysTime.wMilliseconds);
#else
    time_t        ltime;
    tm            *ptm;

    time(&ltime);
    ptm = localtime(&ltime);

    return DateTime(ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, 0);
#endif
}

bool DateTime::isLeapYear(int year)
{
    if (year % 4 != 0)
        return false;

    if (year % 400 == 0)
        return true;
    if (year % 100 == 0)
        return false;

    return true;
}


//////////////////////////////////////////////////////////////////////////
// CPPUnit test
#ifdef _CPPUNIT_TEST

#include <time.h>

IMPLEMENT_CPPUNIT_TEST_REG(DateTime)

class CTestCaseCBaseDate : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(CTestCaseCBaseDate);
    CPPUNIT_TEST(testFromTime);
    CPPUNIT_TEST(testFromString);
    CPPUNIT_TEST_SUITE_END();

protected:
    void testFromTime()
    {
        time_t    timeCur = 0;
        time(&timeCur);
        tm *tmCur = gmtime(&timeCur);
        DateTime t;
        t.fromTime((uint32_t)timeCur);

        ASSERT_TRUE(tmCur->tm_year + 1900 == t.year);
        ASSERT_TRUE(tmCur->tm_mon + 1 == t.month);
        ASSERT_TRUE(tmCur->tm_mday == t.day);
        ASSERT_TRUE(tmCur->tm_hour == t.hour);
        ASSERT_TRUE(tmCur->tm_min == t.minute);
        ASSERT_TRUE(tmCur->tm_sec == t.second);
        ASSERT_TRUE(0 == t.ms);
        ASSERT_TRUE(timeCur == t.getTime());

        t.month = 12;
        DateTime t2;
        t2.fromTime(t.getTime());
        ASSERT_TRUE(t2.month = t.month);
        ASSERT_TRUE(t2.getTime() == t.getTime());

        for (int year = 1999; year < 2001; year++)
        {
            DateTime    date(year, 1, 1);
            uint32_t time = date.getTime();

            DateTime    date2;
            date2.fromTime(time);

            ASSERT_TRUE(date2.year == year);
        }

        int vYear[] = { 1970, 1988, 2000, 2100 };

        for (int i = 0; i < CountOf(vYear); i++)
        {
            int year = vYear[i];
            for (int month = 1; month <= 12; month++)
            {
                int dayToMonthStart = getDaysToMonth(year, month);
                int dayToMOnthEnd;
                if (month == 12)
                    dayToMOnthEnd = dayToMonthStart + 31;
                else
                    dayToMOnthEnd = getDaysToMonth(year, month + 1);
                for (int day = 1; day <= dayToMOnthEnd - dayToMonthStart; day++)
                {
                    DateTime    date(year, month, day);
                    uint32_t time = date.getTime();

                    DateTime    date2;
                    date2.fromTime(time);

                    ASSERT_TRUE(date2.year == year);
                    ASSERT_TRUE(date2.month == month);
                    ASSERT_TRUE(date2.day == day);
                }
            }
        }
    }

    void testFromString()
    {
        cstr_t vDate[] = { "2010-01-27", "1970-12-31", "2010-10-27" };

        for (int i = 0; i < CountOf(vDate); i++)
        {
            DateTime    date;

            date.fromString(vDate[i]);
            string str = date.toUtcDateString();
            ASSERT_TRUE(str == vDate[i]);
        }

        cstr_t vDateTime[] = { "2010-01-27 10:12.01", "1970-12-31 10:02.15", "2010-10-27 10:22.59" };

        for (int i = 0; i < CountOf(vDateTime); i++)
        {
            DateTime    date;

            date.fromString(vDateTime[i]);
            string str = date.toUtcDateTimeString();
            ASSERT_TRUE(str == vDateTime[i]);
        }
    }

};

CPPUNIT_TEST_SUITE_REGISTRATION(CTestCaseCBaseDate);

#endif // _CPPUNIT_TEST
