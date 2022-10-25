// Index: 0
console.log(0, 999999999999998200000);
console.log(1, 99999999999999820000012345667899001234567891234567891234567892345678);
console.log(2, 99999999999999820000012345667899001234567891234567891234567892345678e123);
console.log(3, 99999999999999820000012345667899001234567891234567891234567892345678e1234567890);
console.log(4, 2.1234567891234567891234567892345678);
console.log(5, 2.12e34567891234567891234567892345678);
console.log(6, 2.12e-34567891234567891234567892345678);
console.log(7, (-1).toFixed());
console.log(8, (0.001123).toFixed());
console.log(9, (0.001123).toFixed(5));
console.log(10, (11.11).toPrecision(1));
console.log('10.1', (1.7976931348623157e+308).toFixed(5));
function f() {
    var n = 9.999999999999999;
    for (i = 0; i < 20; i++) {
        console.log(i + 11, n, n.toPrecision(20), n.toPrecision(22), n.toExponential());
        n += 5e-16;
    }
}
f();
/* OUTPUT
0 999999999999998200000
1 9.999999999999982e+67
2 9.999999999999982e+190
3 Infinity
4 2.123456789123457
5 Infinity
6 0
7 -1
8 0
9 0.00112
10 1e+1
10.1 1.7976931348623157e+308
11 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
12 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
13 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
14 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
15 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
16 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
17 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
18 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
19 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
20 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
21 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
22 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
23 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
24 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
25 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
26 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
27 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
28 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
29 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
30 9.999999999999998 9.9999999999999982236 9.999999999999998223643 9.999999999999998e+0
*/


// Index: 1
function f(r, n, p) {
    try {
        console.log(r + '.1', n, p, n.toExponential(p));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(r + '.2', n, p, n.toPrecision(p));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(r + '.3', n, p, n.toFixed(p));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f(0, 1, 1);
f(1, 11.11, 1);
f(2, 0.001123);
f(3, 1123.123, 5);
f(4, 1123.123, 6);
f(5, 1123.123, 7);
f(6, 1123.123, 8);
f(7, 0);
f(8, 0, 3);
f(9, -1);
f(10, -1, 4);
f(11, 1, 0);
f(12, 1, -1);
f(13, 1, 100);
f(14, 1, 101);
f(15, 999999999999998200000, 2);
f(16, Infinity, 2);
f(17, NaN, 2);
f(18, 1.7976931348623157e+308, 3);
f(19, 5e-324, 5);
/* OUTPUT
0.1 1 1 1.0e+0
0.2 1 1 1
0.3 1 1 1.0
1.1 11.11 1 1.1e+1
1.2 11.11 1 1e+1
1.3 11.11 1 11.1
2.1 0.001123 undefined 1.123e-3
2.2 0.001123 undefined 0.001123
2.3 0.001123 undefined 0
3.1 1123.123 5 1.12312e+3
3.2 1123.123 5 1123.1
3.3 1123.123 5 1123.12300
4.1 1123.123 6 1.123123e+3
4.2 1123.123 6 1123.12
4.3 1123.123 6 1123.123000
5.1 1123.123 7 1.1231230e+3
5.2 1123.123 7 1123.123
5.3 1123.123 7 1123.1230000
6.1 1123.123 8 1.12312300e+3
6.2 1123.123 8 1123.1230
6.3 1123.123 8 1123.12300000
7.1 0 undefined 0e+0
7.2 0 undefined 0
7.3 0 undefined 0
8.1 0 3 0.000e+0
8.2 0 3 0.00
8.3 0 3 0.000
9.1 -1 undefined -1e+0
9.2 -1 undefined -1
9.3 -1 undefined -1
10.1 -1 4 -1.0000e+0
10.2 -1 4 -1.000
10.3 -1 4 -1.0000
11.1 1 0 1e+0
RangeError: toPrecision() argument must be between 1 and 100
11.3 1 0 1
RangeError: toExponential() argument must be between 0 and 100
RangeError: toPrecision() argument must be between 1 and 100
RangeError: toFixed() digits argument must be between 0 and 100
13.1 1 100 1.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000e+0
13.2 1 100 1.000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
13.3 1 100 1.0000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
RangeError: toExponential() argument must be between 0 and 100
RangeError: toPrecision() argument must be between 1 and 100
RangeError: toFixed() digits argument must be between 0 and 100
15.1 999999999999998200000 2 1.00e+21
15.2 999999999999998200000 2 1.0e+21
15.3 999999999999998200000 2 999999999999998164992.00
16.1 Infinity 2 Infinity
16.2 Infinity 2 Infinity
16.3 Infinity 2 Infinity
17.1 NaN 2 NaN
17.2 NaN 2 NaN
17.3 NaN 2 NaN
18.1 1.7976931348623157e+308 3 1.798e+308
18.2 1.7976931348623157e+308 3 1.80e+308
18.3 1.7976931348623157e+308 3 1.7976931348623157e+308
19.1 5e-324 5 4.94066e-324
19.2 5e-324 5 4.9407e-324
19.3 5e-324 5 0.00000
*/


// Index: 2
function f() {
    var n = 1.234e2, i;
    for (i = 0; i < 10; i++) {
        console.log(i, n, n.toFixed(3), n.toPrecision(10), n.toPrecision(1), n.toExponential(8));
        n /= 10;
    }
}
f();
/* OUTPUT
0 123.4 123.400 123.4000000 1e+2 1.23400000e+2
1 12.34 12.340 12.34000000 1e+1 1.23400000e+1
2 1.234 1.234 1.234000000 1 1.23400000e+0
3 0.1234 0.123 0.1234000000 0.1 1.23400000e-1
4 0.01234 0.012 0.01234000000 0.01 1.23400000e-2
5 0.001234 0.001 0.001234000000 0.001 1.23400000e-3
6 0.00012340000000000002 0.000 0.0001234000000 0.0001 1.23400000e-4
7 0.000012340000000000002 0.000 0.00001234000000 0.00001 1.23400000e-5
8 0.0000012340000000000002 0.000 0.000001234000000 0.000001 1.23400000e-6
9 1.234e-7 0.000 1.234000000e-7 1e-7 1.23400000e-7
*/


// Index: 3
function f() {
    console.log(0, Number.EPSILON);
    console.log(1, Number.MAX_SAFE_INTEGER);
    console.log(2, Number.MAX_VALUE);
    console.log(3, Number.MIN_SAFE_INTEGER);
    console.log(4, Number.MIN_VALUE);
    console.log(5, Number.NEGATIVE_INFINITY);
    console.log(6, Number.NaN);
    console.log(7, Number.POSITIVE_INFINITY);
    console.log(8, Number.isFinite(Infinity));
    console.log(9, Number.isFinite(NaN));
    console.log(10, Number.isFinite(-Infinity));
    console.log(11, Number.isFinite(0));
    console.log(12, Number.isFinite(2e64));
    console.log(13, Number.isFinite('1'));
    console.log(14, Number.isFinite('0'));
    console.log(15, Number.isFinite(null));
    console.log(16, Number.isFinite(undefined));
    console.log(17, Number.isFinite(Symbol()));
    console.log(18, Number.isNaN(NaN));
    console.log(19, Number.isNaN(Infinity));
    console.log(20, Number.isNaN(-Infinity));
    console.log(21, Number.isNaN(0));
    console.log(22, Number.isNaN(2e64));
    console.log(23, Number.isNaN('1'));
    console.log(24, Number.isNaN('0'));
    console.log(25, Number.isNaN(null));
    console.log(26, Number.isNaN(undefined));
    console.log(27, Number.isNaN(Symbol()));
    console.log(28, Number.isInteger(NaN));
    console.log(29, Number.isInteger(Infinity));
    console.log(30, Number.isInteger(99999999999999999999999));
    console.log(31, Number.isInteger(-100000));
    console.log(32, Number.isInteger(2e64));
    console.log(33, Number.isInteger('1'));
    console.log(34, Number.isInteger(0.1));
    console.log(35, Number.isInteger(null));
    console.log(36, Number.isInteger(undefined));
    console.log(37, Number.isInteger(Symbol()));
    console.log(38, Number.isInteger(900719925474098.1));
    console.log(39, Number.isInteger(9007199254740992));
    console.log(40, Number.isInteger(5.000000000000001));
    console.log(41, Number.isInteger(5.0000000000000001));
    console.log(42, Number.isSafeInteger(2e64));
    console.log(43, Number.isSafeInteger('1'));
    console.log(44, Number.isSafeInteger(0.1));
    console.log(45, Number.isSafeInteger(null));
    console.log(46, Number.isSafeInteger(undefined));
    console.log(47, Number.isSafeInteger(Symbol()));
    console.log(48, Number.isSafeInteger(9007199254740990.9));
    console.log(49, Number.isSafeInteger(9007199254740992));
    console.log(50, Number.isSafeInteger(99999999999999999999999));
    console.log(51, Number.isSafeInteger(-2323009324));
    console.log(52, Number.isSafeInteger(-99999999999999999999999));
    console.log(53, Number.parseFloat('1'));
    console.log(54, Number.parseFloat(0.1));
    console.log(55, Number.parseFloat(null));
    console.log(56, Number.parseFloat(undefined));
    try {
        console.log(57, Number.parseFloat(Symbol()));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log(58, Number.parseFloat('9007199254740990.9'));
    console.log(59, Number.parseFloat('90071992 54740992'));
    console.log(60, Number.parseFloat('99999999999999999999999'));
    console.log(61, Number.parseFloat('+2.34e+6'));
    console.log(62, Number.parseFloat('-99999999999999999999999'));
    console.log(63, Number.parseFloat('0xff'));
    console.log(64, Number.parseFloat('0b1'));
    console.log(65, Number.parseFloat('NaN'));
    console.log(66, Number.parseFloat('Infinity'));
    console.log(67, Number.parseFloat('-NaN'));
    console.log(68, Number.parseFloat('-Infinity'));
    console.log(69, Number.parseFloat('-1.234e-5'));
    console.log(73, Number.parseInt('1'));
    console.log(74, Number.parseInt(0.1));
    console.log(75, Number.parseInt(null));
    console.log(76, Number.parseInt(undefined));
    try {
        console.log(57, Number.parseInt(Symbol()));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log(78, Number.parseInt('9007199254740990.9'));
    console.log(79, Number.parseInt('90071992 54740992'));
    console.log(80, Number.parseInt('99999999999999999999999'));
    console.log(81, Number.parseInt('+2.34e+6'));
    console.log(82, Number.parseInt('-99999999999999999999999'));
    console.log(83, Number.parseInt('0xff'));
    console.log(84, Number.parseInt('0b1'));
    console.log(85, Number.parseInt('NaN'));
    console.log(86, Number.parseInt('Infinity'));
    console.log(87, Number.parseInt('-NaN'));
    console.log(88, Number.parseInt('-Infinity'));
    console.log(89, Number.parseInt('-1.234e-5'));
    console.log(94, Number.parseInt(0.1, 36));
    console.log(95, Number.parseInt(null, 36));
    console.log(96, Number.parseInt(undefined, 36));
    console.log(98, Number.parseInt('9007199254740990.9', 35));
    console.log(99, Number.parseInt('90071992 54740992', 36));
    console.log(100, Number.parseInt('99999999999999999999999', 36));
    console.log(101, Number.parseInt('+2.34e+6', 36));
    console.log(102, Number.parseInt('-99999999999999999999999', 36));
    console.log(103, Number.parseInt('0xff', 36));
    console.log(104, Number.parseInt('0b1', 36));
    console.log(105, Number.parseInt('NaN', 36));
    console.log(106, Number.parseInt('Infinity', 36));
    console.log(107, Number.parseInt('-NaN', 36));
    console.log(108, Number.parseInt('-Infinity', 36));
    console.log(109, Number.parseInt('-1.234e-5', 36));
}
f();
/* OUTPUT
0 2.220446049250313e-16
1 9007199254740991
2 1.7976931348623157e+308
3 -9007199254740991
4 5e-324
5 -Infinity
6 NaN
7 Infinity
8 false
9 false
10 false
11 true
12 true
13 false
14 false
15 false
16 false
17 false
18 true
19 false
20 false
21 false
22 false
23 false
24 false
25 false
26 false
27 false
28 false
29 false
30 true
31 true
32 true
33 false
34 false
35 false
36 false
37 false
38 false
39 true
40 false
41 true
42 false
43 false
44 false
45 false
46 false
47 false
48 true
49 false
50 false
51 true
52 false
53 1
54 0.1
55 NaN
56 NaN
TypeError: Cannot convert a Symbol value to a string
58 9007199254740991
59 90071992
60 1e+23
61 2340000
62 -1e+23
63 0
64 0
65 NaN
66 Infinity
67 NaN
68 -Infinity
69 -0.00001234
73 1
74 0
75 NaN
76 NaN
TypeError: Cannot convert a Symbol value to a string
78 9007199254740990
79 90071992
80 1e+23
81 2
82 -1e+23
83 255
84 0
85 NaN
86 NaN
87 NaN
88 NaN
89 -1
94 0
95 1112745
96 86464843759093
98 1.3039804901801701e+24
99 705289292822
100 1.6037326933824684e+35
101 2
102 -1.6037326933824684e+35
103 43323
104 397
105 30191
106 1461559270678
107 -30191
108 -1461559270678
109 -1
*/

