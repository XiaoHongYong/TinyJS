// Index: 0
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f1(r, p, noPrint) {
    console.log('Round: ', r, noPrint ? '' : p);

    console.log('1', p >> undefined);
    console.log('2', p >> null);
    console.log('3', p >> NaN);
    console.log('4', p >> Infinity);
    console.log('5', p >> -Infinity);
    console.log('6', p >> 0);
    console.log('7', p >> 0.0);
    console.log('8', p >> 1.0);
    console.log('9', p >> 0x1fffffffff11);
    console.log('9a', p >> -0x1fffffffff11);
    console.log('9b', p >> -1);
    console.log('10', p >> -1.0);
    console.log('11', p >> -5);
    console.log('12', p >> -5.2);
    console.log('13', p >> 5);
    console.log('14', p >> 5000);
    console.log('15', p >> 5.2);
    console.log('16', p >> true);
    console.log('17', p >> false);
    console.log('18', p >> '');
    console.log('19', p >> '0');
    console.log('20', p >> '1');
    console.log('21', p >> '234'.charAt(1));
    console.log('22', p >> '0.0');
    console.log('23', p >> '1.0');
    console.log('24', p >> 'true');
    console.log('25', p >> 'false');
    console.log('26', p >> g);
    console.log('27', p >> obj1);
    console.log('28', p >> obj2);
    console.log('29', p >> /a/);
    try {
        var a = Symbol();
        console.log(p >> a);
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
f1('9', 0x1ffcffdfff11);
f1('9a', -0x1ffcffdfff11);
f1('9b', -1);
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
1 1
2 1
3 1
4 1
5 1
6 1
7 1
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
17 1
18 1
19 1
20 0
21 0
22 1
23 0
24 1
25 1
26 1
27 0
28 0
29 1
TypeError: Cannot convert a Symbol value to a number
Round:  9 35171485089553
1 -2097391
2 -2097391
3 -2097391
4 -2097391
5 -2097391
6 -2097391
7 -2097391
8 -1048696
9 -17
9a -65
9b -1
10 -1
11 -1
12 -1
13 -65544
14 -8193
15 -65544
16 -1048696
17 -2097391
18 -2097391
19 -2097391
20 -1048696
21 -262174
22 -2097391
23 -1048696
24 -2097391
25 -2097391
26 -2097391
27 -262174
28 -1048696
29 -2097391
TypeError: Cannot convert a Symbol value to a number
Round:  9a -35171485089553
1 2097391
2 2097391
3 2097391
4 2097391
5 2097391
6 2097391
7 2097391
8 1048695
9 16
9a 64
9b 0
10 0
11 0
12 0
13 65543
14 8192
15 65543
16 1048695
17 2097391
18 2097391
19 2097391
20 1048695
21 262173
22 2097391
23 1048695
24 2097391
25 2097391
26 2097391
27 262173
28 1048695
29 2097391
TypeError: Cannot convert a Symbol value to a number
Round:  9b -1
1 -1
2 -1
3 -1
4 -1
5 -1
6 -1
7 -1
8 -1
9 -1
9a -1
9b -1
10 -1
11 -1
12 -1
13 -1
14 -1
15 -1
16 -1
17 -1
18 -1
19 -1
20 -1
21 -1
22 -1
23 -1
24 -1
25 -1
26 -1
27 -1
28 -1
29 -1
TypeError: Cannot convert a Symbol value to a number
Round:  10 -2
1 -2
2 -2
3 -2
4 -2
5 -2
6 -2
7 -2
8 -1
9 -1
9a -1
9b -1
10 -1
11 -1
12 -1
13 -1
14 -1
15 -1
16 -1
17 -2
18 -2
19 -2
20 -1
21 -1
22 -2
23 -1
24 -2
25 -2
26 -2
27 -1
28 -1
29 -2
TypeError: Cannot convert a Symbol value to a number
Round:  11 -2
1 -2
2 -2
3 -2
4 -2
5 -2
6 -2
7 -2
8 -1
9 -1
9a -1
9b -1
10 -1
11 -1
12 -1
13 -1
14 -1
15 -1
16 -1
17 -2
18 -2
19 -2
20 -1
21 -1
22 -2
23 -1
24 -2
25 -2
26 -2
27 -1
28 -1
29 -2
TypeError: Cannot convert a Symbol value to a number
Round:  12 2
1 2
2 2
3 2
4 2
5 2
6 2
7 2
8 1
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 1
17 2
18 2
19 2
20 1
21 0
22 2
23 1
24 2
25 2
26 2
27 0
28 1
29 2
TypeError: Cannot convert a Symbol value to a number
Round:  13 2
1 2
2 2
3 2
4 2
5 2
6 2
7 2
8 1
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 1
17 2
18 2
19 2
20 1
21 0
22 2
23 1
24 2
25 2
26 2
27 0
28 1
29 2
TypeError: Cannot convert a Symbol value to a number
Round:  14 -1
1 -1
2 -1
3 -1
4 -1
5 -1
6 -1
7 -1
8 -1
9 -1
9a -1
9b -1
10 -1
11 -1
12 -1
13 -1
14 -1
15 -1
16 -1
17 -1
18 -1
19 -1
20 -1
21 -1
22 -1
23 -1
24 -1
25 -1
26 -1
27 -1
28 -1
29 -1
TypeError: Cannot convert a Symbol value to a number
Round:  15 true
1 1
2 1
3 1
4 1
5 1
6 1
7 1
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
17 1
18 1
19 1
20 0
21 0
22 1
23 0
24 1
25 1
26 1
27 0
28 0
29 1
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
1 1
2 1
3 1
4 1
5 1
6 1
7 1
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
17 1
18 1
19 1
20 0
21 0
22 1
23 0
24 1
25 1
26 1
27 0
28 0
29 1
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
1 1
2 1
3 1
4 1
5 1
6 1
7 1
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
17 1
18 1
19 1
20 0
21 0
22 1
23 0
24 1
25 1
26 1
27 0
28 0
29 1
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
1 3
2 3
3 3
4 3
5 3
6 3
7 3
8 1
9 0
9a 0
9b 0
10 0
11 0
12 0
13 0
14 0
15 0
16 1
17 3
18 3
19 3
20 1
21 0
22 3
23 1
24 3
25 3
26 3
27 0
28 1
29 3
TypeError: Cannot convert a Symbol value to a number
Round:  26 
1 1
2 1
3 1
4 1
5 1
6 1
7 1
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
17 1
18 1
19 1
20 0
21 0
22 1
23 0
24 1
25 1
26 1
27 0
28 0
29 1
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

