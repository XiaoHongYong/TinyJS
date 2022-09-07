// Index: 0
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f1(r, p, noPrint) {
    console.log('round: ', r, noPrint ? '' : p);

    console.log('1', p === undefined);
    console.log('2', p === null);
    console.log('3', p === NaN);
    console.log('4', p === Infinity);
    console.log('5', p === -Infinity);
    console.log('6', p === 0);
    console.log('7', p === 0.0);
    console.log('8', p === 1.0);
    console.log('9', p === -1);
    console.log('10', p === -1.0);
    console.log('11', p === -5);
    console.log('12', p === -5.0);
    console.log('13', p === 5);
    console.log('14', p === 5000);
    console.log('15', p === 6.0);
    console.log('16', p === true);
    console.log('17', p === false);
    console.log('18', p === '');
    console.log('19', p === '0');
    console.log('20', p === '1');
    console.log('21', p === '234'.charAt(1));
    console.log('22', p === '0.0');
    console.log('23', p === '1.0');
    console.log('24', p === 'true');
    console.log('25', p === 'false');
    console.log('26', p === g);
    console.log('27', p === obj1);
    console.log('28', p === obj2);
    console.log('29', p === /a/);
    try {
        var a = Symbol();
        console.log(p === a);
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
f1('28', Symbol());
/* OUTPUT
round:  1 undefined
1 true
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  2 null
1 false
2 true
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  3 NaN
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  4 Infinity
1 false
2 false
3 false
4 true
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  5 -Infinity
1 false
2 false
3 false
4 false
5 true
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  6 0
1 false
2 false
3 false
4 false
5 false
6 true
7 true
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  7 0
1 false
2 false
3 false
4 false
5 false
6 true
7 true
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  8 1
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 true
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  9 -1
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 true
10 true
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  10 -2
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  11 -2
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  12 2
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  13 2
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  14 -1
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 true
10 true
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  15 true
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 true
17 false
18 false
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
false
round:  16 false
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 true
18 false
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
false
round:  17 
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
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
false
round:  18 0
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
19 true
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
false
round:  19 1
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
19 false
20 true
21 false
22 false
23 false
24 false
25 false
26 false
27 false
28 false
29 false
false
round:  9 undefined
1 true
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  20 0.0
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
19 false
20 false
21 false
22 true
23 false
24 false
25 false
26 false
27 false
28 false
29 false
false
round:  21 1.0
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
19 false
20 false
21 false
22 false
23 true
24 false
25 false
26 false
27 false
28 false
29 false
false
round:  22 true
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
19 false
20 false
21 false
22 false
23 false
24 true
25 false
26 false
27 false
28 false
29 false
false
round:  23 false
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
19 false
20 false
21 false
22 false
23 false
24 false
25 true
26 false
27 false
28 false
29 false
false
round:  24 
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
19 false
20 false
21 false
22 false
23 false
24 false
25 false
26 true
27 false
28 false
29 false
false
round:  25 
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
19 false
20 false
21 false
22 false
23 false
24 false
25 false
26 false
27 true
28 false
29 false
false
round:  26 
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
19 false
20 false
21 false
22 false
23 false
24 false
25 false
26 false
27 false
28 true
29 false
false
round:  27 /a/
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
round:  28 Symbol()
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
10 false
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
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
false
*/

