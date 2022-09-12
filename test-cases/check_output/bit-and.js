// Index: 0
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f1(r, p, noPrint) {
    console.log('Round: ', r, noPrint ? '' : p);

    console.log('1', p & undefined);
    console.log('2', p & null);
    console.log('3', p & NaN);
    console.log('4', p & Infinity);
    console.log('5', p & -Infinity);
    console.log('6', p & 0);
    console.log('7', p & 0.0);
    console.log('8', p & 1.0);
    console.log('9', p & 0x1fffffffff11);
    console.log('9a', p & -0x1fffffffff11);
    console.log('9b', p & -1);
    console.log('10', p & -1.0);
    console.log('11', p & -5);
    console.log('12', p & -5.2);
    console.log('13', p & 5);
    console.log('14', p & 5000);
    console.log('15', p & 5.2);
    console.log('16', p & true);
    console.log('17', p & false);
    console.log('18', p & '');
    console.log('19', p & '0');
    console.log('20', p & '1');
    console.log('21', p & '234'.charAt(1));
    console.log('22', p & '0.0');
    console.log('23', p & '1.0');
    console.log('24', p & 'true');
    console.log('25', p & 'false');
    console.log('26', p & g);
    console.log('27', p & obj1);
    console.log('28', p & obj2);
    console.log('29', p & /a/);
    try {
        var a = Symbol();
        console.log(p & a);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
        // console.log(e);
    }
}
f1('1', undefined);
f1('2', null);
f1('3', NaN);
f1('4', Infinity);
f1('5', -Infinity);
f1('6', 0);
f1('7', 0.0);
f1('8', 1.0);
f1('9a', 0x1ffcffdfff11);
f1('9b', -0x1ffcffdfff11);
f1('9', -1);
f1('10', -2);
f1('11', -2.0);
f1('12', 2);
f1('13', 2.0);
f1('14', -1.0);
f1('15', true);
f1('16', false);
f1('17', '');
f1('18', '0');
f1('19', '1');
f1('19.1'.charAt(1));
f1('20', '0.0');
f1('21', '1.0');
f1('22', 'true');
f1('23', 'false');
f1('24', g, true);
f1('25', obj1, true);
f1('26', obj2, true);
f1('27', /a/);
//f1('28', Symbol());
/* OUTPUT
Round:  1 undefined
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  2 null
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  3 NaN
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  4 Infinity
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  5 -Infinity
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  6 0
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  7 0
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  8 1
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 1
9 1
9a 1
9b 1
10 1
11 1
12 1
13 1
14 0
15 1
16 1
17 0
18 0
19 0
20 1
21 1
22 0
23 1
24 0
25 0
26 0
27 1
28 1
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  9a 35171485089553
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 1
9 -2097391
9a 1
9b -2097391
10 -2097391
11 -2097391
12 -2097391
13 1
14 4864
15 1
16 1
17 0
18 0
19 0
20 1
21 1
22 0
23 1
24 0
25 0
26 0
27 1
28 1
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  9b -35171485089553
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 1
9 2097153
9a 239
9b 2097391
10 2097391
11 2097387
12 2097387
13 5
14 136
15 5
16 1
17 0
18 0
19 0
20 1
21 3
22 0
23 1
24 0
25 0
26 0
27 3
28 1
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  9 -1
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 1
9 -239
9a 239
9b -1
10 -1
11 -5
12 -5
13 5
14 5000
15 5
16 1
17 0
18 0
19 0
20 1
21 3
22 0
23 1
24 0
25 0
26 0
27 3
28 1
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  10 -2
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 -240
9a 238
9b -2
10 -2
11 -6
12 -6
13 4
14 5000
15 4
16 0
17 0
18 0
19 0
20 0
21 2
22 0
23 0
24 0
25 0
26 0
27 2
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  11 -2
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 -240
9a 238
9b -2
10 -2
11 -6
12 -6
13 4
14 5000
15 4
16 0
17 0
18 0
19 0
20 0
21 2
22 0
23 0
24 0
25 0
26 0
27 2
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  12 2
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 2
9b 2
10 2
11 2
12 2
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 2
22 0
23 0
24 0
25 0
26 0
27 2
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  13 2
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 2
9b 2
10 2
11 2
12 2
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 2
22 0
23 0
24 0
25 0
26 0
27 2
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  14 -1
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 1
9 -239
9a 239
9b -1
10 -1
11 -5
12 -5
13 5
14 5000
15 5
16 1
17 0
18 0
19 0
20 1
21 3
22 0
23 1
24 0
25 0
26 0
27 3
28 1
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  15 true
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 1
9 1
9a 1
9b 1
10 1
11 1
12 1
13 1
14 0
15 1
16 1
17 0
18 0
19 0
20 1
21 1
22 0
23 1
24 0
25 0
26 0
27 1
28 1
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  16 false
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  17 
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  18 0
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  19 1
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 1
9 1
9a 1
9b 1
10 1
11 1
12 1
13 1
14 0
15 1
16 1
17 0
18 0
19 0
20 1
21 1
22 0
23 1
24 0
25 0
26 0
27 1
28 1
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  9 undefined
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  20 0.0
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  21 1.0
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 1
9 1
9a 1
9b 1
10 1
11 1
12 1
13 1
14 0
15 1
16 1
17 0
18 0
19 0
20 1
21 1
22 0
23 1
24 0
25 0
26 0
27 1
28 1
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  22 true
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  23 false
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  24 
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  25 
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 1
9 1
9a 3
9b 3
10 3
11 3
12 3
13 1
14 0
15 1
16 1
17 0
18 0
19 0
20 1
21 3
22 0
23 1
24 0
25 0
26 0
27 3
28 1
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  26 
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 1
9 1
9a 1
9b 1
10 1
11 1
12 1
13 1
14 0
15 1
16 1
17 0
18 0
19 0
20 1
21 1
22 0
23 1
24 0
25 0
26 0
27 1
28 1
29 0
TypeError: Cannot convert a Symbol value to a number
Round:  27 /a/
1 0
2 0
3 0
4 0
5 0
6 0
7 0
8 0
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 0
17 0
18 0
19 0
20 0
21 0
22 0
23 0
24 0
25 0
26 0
27 0
28 0
29 0
TypeError: Cannot convert a Symbol value to a number
*/

