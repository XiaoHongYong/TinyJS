// Index: 0
function f() {
    console.log(0, Date.parse('a'));
    console.log(1, Date.parse('2001-01-12'));
    console.log(2, Date.parse('2001/01/12'));
    console.log(3, Date.parse(' 2001-01-12   01: 12: 32'));
    console.log(4, Date.parse('2001/01/12'));
    console.log(5, Date.parse('275760/01/12'));
    console.log(6, Date.parse('2001-01/12 24:00:00'));
    console.log(6.1, Date.parse('2001-01-12T24:00:00'));
    console.log(7, Date.parse('2001-01/12 24:01:00'));
    console.log(8, Date.parse('2001-01/12 24:00:01'));
    console.log(9, Date.parse('2001-01/12 23:60:00'));
    console.log(10, Date.parse('2001-01/12 20:00:60'));
    console.log(11, Date.parse('2001-12/31 23:59:59'));
    console.log(12, Date.parse('2001-1-1 0:0:0'));
    console.log(13, Date.parse('2001-1-0 00:00:00'));
    console.log(14, Date.parse('2001-0-1 00:00:00'));
    console.log(15, Date.parse('2001-1-1 01:02:03'));
    console.log(16, Date.parse(undefined));
    console.log(17, Date.parse(null));
    // console.log(18, Date.parse(1)); // Mon Jan 01 2001 00:00:00 GMT+0800 (China Standard Time)
    console.log(19, Date.parse({}));
    console.log(20, Date.parse('2001-01-12 a'));
    console.log(21, Date.parse('2001-01-12 1:5 a'));
}
f();
/* OUTPUT
0 NaN
1 979257600000
2 979228800000
3 979233152000
4 979228800000
5 8639978803200000
6 979315200000
6.1 979315200000
7 NaN
8 NaN
9 NaN
10 NaN
11 1009814399000
12 978278400000
13 NaN
14 NaN
15 978282123000
16 NaN
17 NaN
19 NaN
20 NaN
21 NaN
*/


// Index: 1
function f() {
    var d = new Date(1669473392136)
    console.log(1, d.getTime(), d.toISOString());

    d = new Date(3, null)
    console.log(1.1, d.getTime(), d.toISOString());

    d = new Date(1669473392136, null)
    console.log(d.toString());

    try {
        d = new Date(Symbol())
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    d = new Date('2001-01-12');
    console.log(2, d.getTime(), d.toISOString());

    d = new Date(d);
    console.log(3, d.getTime(), d.toISOString());

    d = new Date(0);
    console.log(4, d.getTime(), d.toISOString());

    d = new Date(99);
    console.log(5, d.getTime(), d.toISOString());

    d = new Date(2010);
    console.log(6, d.getTime(), d.toISOString());

    d = new Date(99, 2);
    console.log(7, d.getTime(), d.toISOString());

    d = new Date(2013, 2, 3);
    console.log(8, d.getTime(), d.toISOString());

    d = new Date(2013, 2, 3, 4);
    console.log(9, d.getTime(), d.toISOString());

    d = new Date(2013, 2, 3, 4, 5);
    console.log(10, d.getTime(), d.toISOString());

    d = new Date(2013, 2, 3, 4, 5, 6);
    console.log(11, d.getTime(), d.toISOString());

    d = new Date(2013, 2, 3, 4, 5, 6, 7);
    console.log(12, d.getTime(), d.toISOString());
}
f();
/* OUTPUT
1 1669473392136 2022-11-26T14:36:32.136Z
1.1 -2114409600000 1902-12-31T16:00:00.000Z
Invalid Date
TypeError: Cannot convert a Symbol value to a number
2 979257600000 2001-01-12T00:00:00.000Z
3 979257600000 2001-01-12T00:00:00.000Z
4 0 1970-01-01T00:00:00.000Z
5 99 1970-01-01T00:00:00.099Z
6 2010 1970-01-01T00:00:02.010Z
7 920217600000 1999-02-28T16:00:00.000Z
8 1362240000000 2013-03-02T16:00:00.000Z
9 1362254400000 2013-03-02T20:00:00.000Z
10 1362254700000 2013-03-02T20:05:00.000Z
11 1362254706000 2013-03-02T20:05:06.000Z
12 1362254706007 2013-03-02T20:05:06.007Z
*/


// Index: 2
function f() {
    var d = new Date('2001-2-3 04:05:06.7')
    console.log(1, d.getDate());
    console.log(1, d.getDay());
    console.log(2, d.getFullYear());
    console.log(3, d.getHours());
    console.log(4, d.getMilliseconds());
    console.log(5, d.getMinutes());
    console.log(6, d.getMonth());
    console.log(6, d.getSeconds());
    console.log(7, d.getTime());
    // console.log(8, d.getTimezoneOffset());
    console.log(9, d.getUTCDate());
    console.log(10, d.getUTCDay());
    console.log(11, d.getUTCFullYear());
    console.log(12, d.getUTCHours());
    console.log(13, d.getUTCMilliseconds());
    console.log(14, d.getUTCMinutes());
    console.log(15, d.getUTCMonth());
    console.log(16, d.getUTCSeconds());

    console.log(21, d.setDate(1), d.getTime());
    console.log(21, d.setFullYear(2012), d.getTime());
    console.log(22, d.setHours(3), d.getTime());
    console.log(23, d.setMilliseconds(3), d.getTime());
    console.log(24, d.setMinutes(4), d.getTime());
    console.log(25, d.setMonth(5), d.getTime());
    console.log(26, d.setSeconds(7), d.getTime());
    console.log(26, d.setTime(1669473392136), d.getTime());
    console.log(27, d.setUTCDate(7), d.getTime());
    console.log(28, d.setUTCFullYear(2008), d.getTime());
    console.log(29, d.setUTCHours(9), d.getTime());
    console.log(30, d.setUTCMilliseconds(10), d.getTime());
    console.log(31, d.setUTCMinutes(11), d.getTime());
    console.log(32, d.setUTCMonth(12), d.getTime());
    console.log(33, d.setUTCSeconds(13), d.getTime());

    console.log(34, d.toDateString());
    console.log(35, d.toGMTString());
    console.log(36, d.toISOString());
    console.log(36, d.toJSON());
    console.log(42, d.toUTCString());
}
f();
/* OUTPUT
1 3
1 6
2 2001
3 4
4 700
5 5
6 1
6 6
7 981144306700
9 2
10 5
11 2001
12 20
13 700
14 5
15 1
16 6
21 980971506700 980971506700
21 1328040306700 1328040306700
22 1328036706700 1328036706700
23 1328036706003 1328036706003
24 1328036646003 1328036646003
25 1338491046003 1338491046003
26 1338491047003 1338491047003
26 1669473392136 1669473392136
27 1667831792136 1667831792136
28 1226068592136 1226068592136
29 1226050592136 1226050592136
30 1226050592010 1226050592010
31 1226049092010 1226049092010
32 1231319492010 1231319492010
33 1231319473010 1231319473010
34 Wed Jan 07 2009
35 Wed, 07 Jan 2009 09:11:13 GMT
36 2009-01-07T09:11:13.010Z
36 2009-01-07T09:11:13.010Z
42 Wed, 07 Jan 2009 09:11:13 GMT
*/


// Index: 3
function f() {
    var d = new Date('2001-2-3 04:05:06.7')
    console.log(1, d.toLocaleDateString());
    console.log(2, d.toLocaleString());
    console.log(3, d.toLocaleTimeString());
    console.log(4, d.toString());
    console.log(5, d.toTimeString());
}
f();
/* OUTPUT-FIXED
1 2001-02-03
2 2001-02-03 04:05:06
3 04:05:06
4 Sat Feb 03 2001 04:05:06 GMT++0800 (CST)
5 04:05:06 GMT++0800 (CST)
*/

