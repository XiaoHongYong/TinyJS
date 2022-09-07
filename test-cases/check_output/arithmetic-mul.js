// Index: 0
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f1(r, p, noPrint) {
    console.log('round: ', r, noPrint ? '' : p);

    console.log('1', p * undefined);
    console.log('2', p * null);
    console.log('3', p * NaN);
    console.log('4', p * Infinity);
    console.log('5', p * -Infinity);
    console.log('6', p * 0);
    console.log('7', p * 0.0);
    console.log('8', p * 1.0);
    console.log('9', p * -1);
    console.log('10', p * -1.0);
    console.log('11', p * -5);
    console.log('12', p * -5.0);
    console.log('13', p * 5);
    console.log('14', p * 5000);
    console.log('15', p * 6.0);
    console.log('16', p * true);
    console.log('17', p * false);
    console.log('18', p * '');
    console.log('19', p * '0');
    console.log('20', p * '1');
    console.log('21', p * '234'.charAt(1));
    console.log('22', p * '0.0');
    console.log('23', p * '1.0');
    console.log('24', p * 'true');
    console.log('25', p * 'false');
    console.log('26', p * g);
    console.log('27', p * obj1);
    console.log('28', p * obj2);
    console.log('29', p * /a/);
    try {
        var a = Symbol();
        console.log(p * a);
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
round:  1 undefined
1 NaN
2 NaN
3 NaN
4 NaN
5 NaN
6 NaN
7 NaN
8 NaN
9 NaN
10 NaN
11 NaN
12 NaN
13 NaN
14 NaN
15 NaN
16 NaN
17 NaN
18 NaN
19 NaN
20 NaN
21 NaN
22 NaN
23 NaN
24 NaN
25 NaN
26 NaN
27 NaN
28 NaN
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  2 null
1 NaN
2 0
3 NaN
4 NaN
5 NaN
6 0
7 0
8 0
9 -0
10 -0
11 -0
12 -0
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
24 NaN
25 NaN
26 NaN
27 0
28 0
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  3 NaN
1 NaN
2 NaN
3 NaN
4 NaN
5 NaN
6 NaN
7 NaN
8 NaN
9 NaN
10 NaN
11 NaN
12 NaN
13 NaN
14 NaN
15 NaN
16 NaN
17 NaN
18 NaN
19 NaN
20 NaN
21 NaN
22 NaN
23 NaN
24 NaN
25 NaN
26 NaN
27 NaN
28 NaN
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  4 Infinity
1 NaN
2 NaN
3 NaN
4 Infinity
5 -Infinity
6 NaN
7 NaN
8 Infinity
9 -Infinity
10 -Infinity
11 -Infinity
12 -Infinity
13 Infinity
14 Infinity
15 Infinity
16 Infinity
17 NaN
18 NaN
19 NaN
20 Infinity
21 Infinity
22 NaN
23 Infinity
24 NaN
25 NaN
26 NaN
27 Infinity
28 Infinity
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  5 -Infinity
1 NaN
2 NaN
3 NaN
4 -Infinity
5 Infinity
6 NaN
7 NaN
8 -Infinity
9 Infinity
10 Infinity
11 Infinity
12 Infinity
13 -Infinity
14 -Infinity
15 -Infinity
16 -Infinity
17 NaN
18 NaN
19 NaN
20 -Infinity
21 -Infinity
22 NaN
23 -Infinity
24 NaN
25 NaN
26 NaN
27 -Infinity
28 -Infinity
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  6 0
1 NaN
2 0
3 NaN
4 NaN
5 NaN
6 0
7 0
8 0
9 -0
10 -0
11 -0
12 -0
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
24 NaN
25 NaN
26 NaN
27 0
28 0
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  7 0
1 NaN
2 0
3 NaN
4 NaN
5 NaN
6 0
7 0
8 0
9 -0
10 -0
11 -0
12 -0
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
24 NaN
25 NaN
26 NaN
27 0
28 0
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  8 1
1 NaN
2 0
3 NaN
4 Infinity
5 -Infinity
6 0
7 0
8 1
9 -1
10 -1
11 -5
12 -5
13 5
14 5000
15 6
16 1
17 0
18 0
19 0
20 1
21 3
22 0
23 1
24 NaN
25 NaN
26 NaN
27 3
28 1
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  9 -1
1 NaN
2 -0
3 NaN
4 -Infinity
5 Infinity
6 -0
7 -0
8 -1
9 1
10 1
11 5
12 5
13 -5
14 -5000
15 -6
16 -1
17 -0
18 -0
19 -0
20 -1
21 -3
22 -0
23 -1
24 NaN
25 NaN
26 NaN
27 -3
28 -1
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  10 -2
1 NaN
2 -0
3 NaN
4 -Infinity
5 Infinity
6 -0
7 -0
8 -2
9 2
10 2
11 10
12 10
13 -10
14 -10000
15 -12
16 -2
17 -0
18 -0
19 -0
20 -2
21 -6
22 -0
23 -2
24 NaN
25 NaN
26 NaN
27 -6
28 -2
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  11 -2
1 NaN
2 -0
3 NaN
4 -Infinity
5 Infinity
6 -0
7 -0
8 -2
9 2
10 2
11 10
12 10
13 -10
14 -10000
15 -12
16 -2
17 -0
18 -0
19 -0
20 -2
21 -6
22 -0
23 -2
24 NaN
25 NaN
26 NaN
27 -6
28 -2
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  12 2
1 NaN
2 0
3 NaN
4 Infinity
5 -Infinity
6 0
7 0
8 2
9 -2
10 -2
11 -10
12 -10
13 10
14 10000
15 12
16 2
17 0
18 0
19 0
20 2
21 6
22 0
23 2
24 NaN
25 NaN
26 NaN
27 6
28 2
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  13 2
1 NaN
2 0
3 NaN
4 Infinity
5 -Infinity
6 0
7 0
8 2
9 -2
10 -2
11 -10
12 -10
13 10
14 10000
15 12
16 2
17 0
18 0
19 0
20 2
21 6
22 0
23 2
24 NaN
25 NaN
26 NaN
27 6
28 2
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  14 -1
1 NaN
2 -0
3 NaN
4 -Infinity
5 Infinity
6 -0
7 -0
8 -1
9 1
10 1
11 5
12 5
13 -5
14 -5000
15 -6
16 -1
17 -0
18 -0
19 -0
20 -1
21 -3
22 -0
23 -1
24 NaN
25 NaN
26 NaN
27 -3
28 -1
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  15 true
1 NaN
2 0
3 NaN
4 Infinity
5 -Infinity
6 0
7 0
8 1
9 -1
10 -1
11 -5
12 -5
13 5
14 5000
15 6
16 1
17 0
18 0
19 0
20 1
21 3
22 0
23 1
24 NaN
25 NaN
26 NaN
27 3
28 1
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  16 false
1 NaN
2 0
3 NaN
4 NaN
5 NaN
6 0
7 0
8 0
9 -0
10 -0
11 -0
12 -0
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
24 NaN
25 NaN
26 NaN
27 0
28 0
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  17 
1 NaN
2 0
3 NaN
4 NaN
5 NaN
6 0
7 0
8 0
9 -0
10 -0
11 -0
12 -0
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
24 NaN
25 NaN
26 NaN
27 0
28 0
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  18 0
1 NaN
2 0
3 NaN
4 NaN
5 NaN
6 0
7 0
8 0
9 -0
10 -0
11 -0
12 -0
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
24 NaN
25 NaN
26 NaN
27 0
28 0
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  19 1
1 NaN
2 0
3 NaN
4 Infinity
5 -Infinity
6 0
7 0
8 1
9 -1
10 -1
11 -5
12 -5
13 5
14 5000
15 6
16 1
17 0
18 0
19 0
20 1
21 3
22 0
23 1
24 NaN
25 NaN
26 NaN
27 3
28 1
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  9 undefined
1 NaN
2 NaN
3 NaN
4 NaN
5 NaN
6 NaN
7 NaN
8 NaN
9 NaN
10 NaN
11 NaN
12 NaN
13 NaN
14 NaN
15 NaN
16 NaN
17 NaN
18 NaN
19 NaN
20 NaN
21 NaN
22 NaN
23 NaN
24 NaN
25 NaN
26 NaN
27 NaN
28 NaN
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  20 0.0
1 NaN
2 0
3 NaN
4 NaN
5 NaN
6 0
7 0
8 0
9 -0
10 -0
11 -0
12 -0
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
24 NaN
25 NaN
26 NaN
27 0
28 0
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  21 1.0
1 NaN
2 0
3 NaN
4 Infinity
5 -Infinity
6 0
7 0
8 1
9 -1
10 -1
11 -5
12 -5
13 5
14 5000
15 6
16 1
17 0
18 0
19 0
20 1
21 3
22 0
23 1
24 NaN
25 NaN
26 NaN
27 3
28 1
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  22 true
1 NaN
2 NaN
3 NaN
4 NaN
5 NaN
6 NaN
7 NaN
8 NaN
9 NaN
10 NaN
11 NaN
12 NaN
13 NaN
14 NaN
15 NaN
16 NaN
17 NaN
18 NaN
19 NaN
20 NaN
21 NaN
22 NaN
23 NaN
24 NaN
25 NaN
26 NaN
27 NaN
28 NaN
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  23 false
1 NaN
2 NaN
3 NaN
4 NaN
5 NaN
6 NaN
7 NaN
8 NaN
9 NaN
10 NaN
11 NaN
12 NaN
13 NaN
14 NaN
15 NaN
16 NaN
17 NaN
18 NaN
19 NaN
20 NaN
21 NaN
22 NaN
23 NaN
24 NaN
25 NaN
26 NaN
27 NaN
28 NaN
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  24 
1 NaN
2 NaN
3 NaN
4 NaN
5 NaN
6 NaN
7 NaN
8 NaN
9 NaN
10 NaN
11 NaN
12 NaN
13 NaN
14 NaN
15 NaN
16 NaN
17 NaN
18 NaN
19 NaN
20 NaN
21 NaN
22 NaN
23 NaN
24 NaN
25 NaN
26 NaN
27 NaN
28 NaN
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  25 
1 NaN
2 0
3 NaN
4 Infinity
5 -Infinity
6 0
7 0
8 3
9 -3
10 -3
11 -15
12 -15
13 15
14 15000
15 18
16 3
17 0
18 0
19 0
20 3
21 9
22 0
23 3
24 NaN
25 NaN
26 NaN
27 9
28 3
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  26 
1 NaN
2 0
3 NaN
4 Infinity
5 -Infinity
6 0
7 0
8 1
9 -1
10 -1
11 -5
12 -5
13 5
14 5000
15 6
16 1
17 0
18 0
19 0
20 1
21 3
22 0
23 1
24 NaN
25 NaN
26 NaN
27 3
28 1
29 NaN
TypeError: Cannot convert a Symbol value to a number
round:  27 /a/
1 NaN
2 NaN
3 NaN
4 NaN
5 NaN
6 NaN
7 NaN
8 NaN
9 NaN
10 NaN
11 NaN
12 NaN
13 NaN
14 NaN
15 NaN
16 NaN
17 NaN
18 NaN
19 NaN
20 NaN
21 NaN
22 NaN
23 NaN
24 NaN
25 NaN
26 NaN
27 NaN
28 NaN
29 NaN
TypeError: Cannot convert a Symbol value to a number
*/

