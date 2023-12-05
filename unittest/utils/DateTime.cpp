//
//  StringView.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/1.
//

#include "utils/Utils.h"
#include "utils/DateTime.h"


#if UNIT_TEST

#include "utils/unittest.h"
#include <time.h>


TEST(DateTime, fromTime) {
    time_t timeCur = 0;
    time(&timeCur);
    tm *tmCur = gmtime(&timeCur);

    DateTime t((uint64_t)timeCur * 1000 + 1, false);

    ASSERT_EQ(tmCur->tm_year + 1900, t.year());
    ASSERT_EQ(tmCur->tm_mon + 1, t.month());
    ASSERT_EQ(tmCur->tm_mday, t.day());
    ASSERT_EQ(tmCur->tm_hour, t.hour());
    ASSERT_EQ(tmCur->tm_min, t.minute());
    ASSERT_EQ(tmCur->tm_sec, t.second());
    ASSERT_EQ(1, t.ms());
    ASSERT_EQ(timeCur - t.getTime(), 0);
    ASSERT_EQ(timeCur, t.getTime());

    {
        DateTime t((uint64_t)timeCur * 1000, true);
        ASSERT_EQ(timeCur - t.getTime(), 0);
    }

    t.setMonth(12);
    DateTime t2;
    t2.fromTime(t.getTime());
    ASSERT_EQ(t2.month(), t.month());
    ASSERT_EQ(t2.getTime(), t.getTime());

    for (int year = 1999; year < 2001; year++) {
        DateTime date(year, 1, 1);
        auto time = date.getTime();

        DateTime date2;
        date2.fromTime(time);

        ASSERT_EQ(date2.year(), year);
    }
}

static void checkFromString(cstr_t *strs, int count, int64_t expects[]) {
    for (int i = 0; i < count; i++) {
        DateTime date;

        if (date.fromString(strs[i], (uint32_t)strlen(strs[i]))) {
            ASSERT_EQ(date.getTimeInMs(), expects[i]);
        } else {
            ASSERT_EQ(0, expects[i]);
        }
    }
}


TEST(DateTime, fromString) {
    {
        // a.map(s=>Date.parse(s)).join(', ')
        cstr_t dates[] = { "2010-01-27", "1970-12/31", "1970-12/31Z", "2010-10-27", "2010- 10- 27" };
        int64_t expected[] = { 1264550400000, 31420800000, 31449600000, 1288137600000, 1288108800000 };
        checkFromString(dates, CountOf(dates), expected);
    }

    {
        cstr_t dates[] = { "2010", "1970-12", "1970/12", "2010/12Z ", "2010Z   " };
        int64_t expected[] = { 1262304000000, 28857600000, 28828800000, 1291161600000, 1262304000000 };
        checkFromString(dates, CountOf(dates), expected);
    }

    {
        cstr_t dates[] = { "2010-01-27T", "1970-12T", "1970T", "2010-01-27+00.00", "2001-13", "2001-01-32", "2001-1-0" };
        int64_t expected[] = { 0, 0, 0, 0, 0, 0, 0 };
        checkFromString(dates, CountOf(dates), expected);
    }

    {
        cstr_t dates[] = { "2010-01-27 10:12:01", "1970-12/31 10:02:15.002", "2010-10-27 10:22:59Z","2010-10-27 10:22:59.03Z" };
        int64_t expected[] = { 1264558321000, 31456935002, 1288174979000, 1288174979030 };
        checkFromString(dates, CountOf(dates), expected);
    }

    {
        cstr_t dates[] = { /*"2001-1-1 01:02.03",*/ " 2001-01-12   01: 12: 32", "2010-01-27 10:12:01+01:3", "1970-12/31 10:02:15.002-03:04" };
        int64_t expected[] = { /*978282150000, */979233152000, 1264583341000, 31496775002 };
        checkFromString(dates, CountOf(dates), expected);
    }

    {
        cstr_t dates[] = { "2010-01-27 10:12Z01+01:3", "1970-12-31 10:02:15.002-03:04X", "2001-01/12 24:00:01", "2001-01/12 24:00:00.001", "2001-01/12 24:00:00:1" };
        int64_t expected[] = { 0, 0, 0, 0, 0 };
        checkFromString(dates, CountOf(dates), expected);
    }
}

#endif // UNIT_TEST
