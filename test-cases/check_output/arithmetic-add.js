// Index: 0
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f1(r, p, noPrint) {
    console.log('round: ', r, noPrint ? '' : p);

    console.log('1', p + undefined);
    console.log('2', p + null);
    console.log('3', p + NaN);
    console.log('4', p + Infinity);
    console.log('5', p + -Infinity);
    console.log('6', p + 0);
    console.log('7', p + 0.0);
    console.log('8', p + 1.0);
    console.log('9', p + -1);
    console.log('10', p + -1.0);
    console.log('11', p + -5);
    console.log('12', p + -5.2);
    console.log('13', p + 5);
    console.log('14', p + 5000);
    console.log('15', p + 5.2);
    console.log('16', p + true);
    console.log('17', p + false);
    console.log('18', p + '');
    console.log('19', p + '0');
    console.log('20', p + '1');
    console.log('21', p + '234'.charAt(1));
    console.log('22', p + '0.0');
    console.log('23', p + '1.0');
    console.log('24', p + 'true');
    console.log('25', p + 'false');
    console.log('26', p + g);
    console.log('27', p + obj1);
    console.log('28', p + obj2);
    console.log('29', p + /a/);
    try {
        var a = Symbol();
        console.log(p + a);
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
18 undefined
19 undefined0
20 undefined1
21 undefined3
22 undefined0.0
23 undefined1.0
24 undefinedtrue
25 undefinedfalse
26 undefinedfunction g() { }
27 undefined3.0
28 NaN
29 undefined/a/
TypeError: Cannot convert a Symbol value to a number
round:  2 null
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
12 -5.2
13 5
14 5000
15 5.2
16 1
17 0
18 null
19 null0
20 null1
21 null3
22 null0.0
23 null1.0
24 nulltrue
25 nullfalse
26 nullfunction g() { }
27 null3.0
28 1
29 null/a/
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
19 NaN0
20 NaN1
21 NaN3
22 NaN0.0
23 NaN1.0
24 NaNtrue
25 NaNfalse
26 NaNfunction g() { }
27 NaN3.0
28 NaN
29 NaN/a/
TypeError: Cannot convert a Symbol value to a number
round:  4 Infinity
1 NaN
2 Infinity
3 NaN
4 Infinity
5 NaN
6 Infinity
7 Infinity
8 Infinity
9 Infinity
10 Infinity
11 Infinity
12 Infinity
13 Infinity
14 Infinity
15 Infinity
16 Infinity
17 Infinity
18 Infinity
19 Infinity0
20 Infinity1
21 Infinity3
22 Infinity0.0
23 Infinity1.0
24 Infinitytrue
25 Infinityfalse
26 Infinityfunction g() { }
27 Infinity3.0
28 Infinity
29 Infinity/a/
TypeError: Cannot convert a Symbol value to a number
round:  5 -Infinity
1 NaN
2 -Infinity
3 NaN
4 NaN
5 -Infinity
6 -Infinity
7 -Infinity
8 -Infinity
9 -Infinity
10 -Infinity
11 -Infinity
12 -Infinity
13 -Infinity
14 -Infinity
15 -Infinity
16 -Infinity
17 -Infinity
18 -Infinity
19 -Infinity0
20 -Infinity1
21 -Infinity3
22 -Infinity0.0
23 -Infinity1.0
24 -Infinitytrue
25 -Infinityfalse
26 -Infinityfunction g() { }
27 -Infinity3.0
28 -Infinity
29 -Infinity/a/
TypeError: Cannot convert a Symbol value to a number
round:  6 0
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
12 -5.2
13 5
14 5000
15 5.2
16 1
17 0
18 0
19 00
20 01
21 03
22 00.0
23 01.0
24 0true
25 0false
26 0function g() { }
27 03.0
28 1
29 0/a/
TypeError: Cannot convert a Symbol value to a number
round:  7 0
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
12 -5.2
13 5
14 5000
15 5.2
16 1
17 0
18 0
19 00
20 01
21 03
22 00.0
23 01.0
24 0true
25 0false
26 0function g() { }
27 03.0
28 1
29 0/a/
TypeError: Cannot convert a Symbol value to a number
round:  8 1
1 NaN
2 1
3 NaN
4 Infinity
5 -Infinity
6 1
7 1
8 2
9 0
10 0
11 -4
12 -4.2
13 6
14 5001
15 6.2
16 2
17 1
18 1
19 10
20 11
21 13
22 10.0
23 11.0
24 1true
25 1false
26 1function g() { }
27 13.0
28 2
29 1/a/
TypeError: Cannot convert a Symbol value to a number
round:  9 -1
1 NaN
2 -1
3 NaN
4 Infinity
5 -Infinity
6 -1
7 -1
8 0
9 -2
10 -2
11 -6
12 -6.2
13 4
14 4999
15 4.2
16 0
17 -1
18 -1
19 -10
20 -11
21 -13
22 -10.0
23 -11.0
24 -1true
25 -1false
26 -1function g() { }
27 -13.0
28 0
29 -1/a/
TypeError: Cannot convert a Symbol value to a number
round:  10 -2
1 NaN
2 -2
3 NaN
4 Infinity
5 -Infinity
6 -2
7 -2
8 -1
9 -3
10 -3
11 -7
12 -7.2
13 3
14 4998
15 3.2
16 -1
17 -2
18 -2
19 -20
20 -21
21 -23
22 -20.0
23 -21.0
24 -2true
25 -2false
26 -2function g() { }
27 -23.0
28 -1
29 -2/a/
TypeError: Cannot convert a Symbol value to a number
round:  11 -2
1 NaN
2 -2
3 NaN
4 Infinity
5 -Infinity
6 -2
7 -2
8 -1
9 -3
10 -3
11 -7
12 -7.2
13 3
14 4998
15 3.2
16 -1
17 -2
18 -2
19 -20
20 -21
21 -23
22 -20.0
23 -21.0
24 -2true
25 -2false
26 -2function g() { }
27 -23.0
28 -1
29 -2/a/
TypeError: Cannot convert a Symbol value to a number
round:  12 2
1 NaN
2 2
3 NaN
4 Infinity
5 -Infinity
6 2
7 2
8 3
9 1
10 1
11 -3
12 -3.2
13 7
14 5002
15 7.2
16 3
17 2
18 2
19 20
20 21
21 23
22 20.0
23 21.0
24 2true
25 2false
26 2function g() { }
27 23.0
28 3
29 2/a/
TypeError: Cannot convert a Symbol value to a number
round:  13 2
1 NaN
2 2
3 NaN
4 Infinity
5 -Infinity
6 2
7 2
8 3
9 1
10 1
11 -3
12 -3.2
13 7
14 5002
15 7.2
16 3
17 2
18 2
19 20
20 21
21 23
22 20.0
23 21.0
24 2true
25 2false
26 2function g() { }
27 23.0
28 3
29 2/a/
TypeError: Cannot convert a Symbol value to a number
round:  14 -1
1 NaN
2 -1
3 NaN
4 Infinity
5 -Infinity
6 -1
7 -1
8 0
9 -2
10 -2
11 -6
12 -6.2
13 4
14 4999
15 4.2
16 0
17 -1
18 -1
19 -10
20 -11
21 -13
22 -10.0
23 -11.0
24 -1true
25 -1false
26 -1function g() { }
27 -13.0
28 0
29 -1/a/
TypeError: Cannot convert a Symbol value to a number
round:  15 true
1 NaN
2 1
3 NaN
4 Infinity
5 -Infinity
6 1
7 1
8 2
9 0
10 0
11 -4
12 -4.2
13 6
14 5001
15 6.2
16 2
17 1
18 true
19 true0
20 true1
21 true3
22 true0.0
23 true1.0
24 truetrue
25 truefalse
26 truefunction g() { }
27 true3.0
28 2
29 true/a/
TypeError: Cannot convert a Symbol value to a number
round:  16 false
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
12 -5.2
13 5
14 5000
15 5.2
16 1
17 0
18 false
19 false0
20 false1
21 false3
22 false0.0
23 false1.0
24 falsetrue
25 falsefalse
26 falsefunction g() { }
27 false3.0
28 1
29 false/a/
TypeError: Cannot convert a Symbol value to a number
round:  17 
1 undefined
2 null
3 NaN
4 Infinity
5 -Infinity
6 0
7 0
8 1
9 -1
10 -1
11 -5
12 -5.2
13 5
14 5000
15 5.2
16 true
17 false
18 
19 0
20 1
21 3
22 0.0
23 1.0
24 true
25 false
26 function g() { }
27 3.0
28 1
29 /a/
TypeError: Cannot convert a Symbol value to a string
round:  18 0
1 0undefined
2 0null
3 0NaN
4 0Infinity
5 0-Infinity
6 00
7 00
8 01
9 0-1
10 0-1
11 0-5
12 0-5.2
13 05
14 05000
15 05.2
16 0true
17 0false
18 0
19 00
20 01
21 03
22 00.0
23 01.0
24 0true
25 0false
26 0function g() { }
27 03.0
28 01
29 0/a/
TypeError: Cannot convert a Symbol value to a string
round:  19 1
1 1undefined
2 1null
3 1NaN
4 1Infinity
5 1-Infinity
6 10
7 10
8 11
9 1-1
10 1-1
11 1-5
12 1-5.2
13 15
14 15000
15 15.2
16 1true
17 1false
18 1
19 10
20 11
21 13
22 10.0
23 11.0
24 1true
25 1false
26 1function g() { }
27 13.0
28 11
29 1/a/
TypeError: Cannot convert a Symbol value to a string
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
18 undefined
19 undefined0
20 undefined1
21 undefined3
22 undefined0.0
23 undefined1.0
24 undefinedtrue
25 undefinedfalse
26 undefinedfunction g() { }
27 undefined3.0
28 NaN
29 undefined/a/
TypeError: Cannot convert a Symbol value to a number
round:  20 0.0
1 0.0undefined
2 0.0null
3 0.0NaN
4 0.0Infinity
5 0.0-Infinity
6 0.00
7 0.00
8 0.01
9 0.0-1
10 0.0-1
11 0.0-5
12 0.0-5.2
13 0.05
14 0.05000
15 0.05.2
16 0.0true
17 0.0false
18 0.0
19 0.00
20 0.01
21 0.03
22 0.00.0
23 0.01.0
24 0.0true
25 0.0false
26 0.0function g() { }
27 0.03.0
28 0.01
29 0.0/a/
TypeError: Cannot convert a Symbol value to a string
round:  21 1.0
1 1.0undefined
2 1.0null
3 1.0NaN
4 1.0Infinity
5 1.0-Infinity
6 1.00
7 1.00
8 1.01
9 1.0-1
10 1.0-1
11 1.0-5
12 1.0-5.2
13 1.05
14 1.05000
15 1.05.2
16 1.0true
17 1.0false
18 1.0
19 1.00
20 1.01
21 1.03
22 1.00.0
23 1.01.0
24 1.0true
25 1.0false
26 1.0function g() { }
27 1.03.0
28 1.01
29 1.0/a/
TypeError: Cannot convert a Symbol value to a string
round:  22 true
1 trueundefined
2 truenull
3 trueNaN
4 trueInfinity
5 true-Infinity
6 true0
7 true0
8 true1
9 true-1
10 true-1
11 true-5
12 true-5.2
13 true5
14 true5000
15 true5.2
16 truetrue
17 truefalse
18 true
19 true0
20 true1
21 true3
22 true0.0
23 true1.0
24 truetrue
25 truefalse
26 truefunction g() { }
27 true3.0
28 true1
29 true/a/
TypeError: Cannot convert a Symbol value to a string
round:  23 false
1 falseundefined
2 falsenull
3 falseNaN
4 falseInfinity
5 false-Infinity
6 false0
7 false0
8 false1
9 false-1
10 false-1
11 false-5
12 false-5.2
13 false5
14 false5000
15 false5.2
16 falsetrue
17 falsefalse
18 false
19 false0
20 false1
21 false3
22 false0.0
23 false1.0
24 falsetrue
25 falsefalse
26 falsefunction g() { }
27 false3.0
28 false1
29 false/a/
TypeError: Cannot convert a Symbol value to a string
round:  24 
1 function g() { }undefined
2 function g() { }null
3 function g() { }NaN
4 function g() { }Infinity
5 function g() { }-Infinity
6 function g() { }0
7 function g() { }0
8 function g() { }1
9 function g() { }-1
10 function g() { }-1
11 function g() { }-5
12 function g() { }-5.2
13 function g() { }5
14 function g() { }5000
15 function g() { }5.2
16 function g() { }true
17 function g() { }false
18 function g() { }
19 function g() { }0
20 function g() { }1
21 function g() { }3
22 function g() { }0.0
23 function g() { }1.0
24 function g() { }true
25 function g() { }false
26 function g() { }function g() { }
27 function g() { }3.0
28 function g() { }1
29 function g() { }/a/
TypeError: Cannot convert a Symbol value to a string
round:  25 
1 3.0undefined
2 3.0null
3 3.0NaN
4 3.0Infinity
5 3.0-Infinity
6 3.00
7 3.00
8 3.01
9 3.0-1
10 3.0-1
11 3.0-5
12 3.0-5.2
13 3.05
14 3.05000
15 3.05.2
16 3.0true
17 3.0false
18 3.0
19 3.00
20 3.01
21 3.03
22 3.00.0
23 3.01.0
24 3.0true
25 3.0false
26 3.0function g() { }
27 3.03.0
28 3.01
29 3.0/a/
TypeError: Cannot convert a Symbol value to a string
round:  26 
1 NaN
2 1
3 NaN
4 Infinity
5 -Infinity
6 1
7 1
8 2
9 0
10 0
11 -4
12 -4.2
13 6
14 5001
15 6.2
16 2
17 1
18 1
19 10
20 11
21 13
22 10.0
23 11.0
24 1true
25 1false
26 1function g() { }
27 13.0
28 2
29 1/a/
TypeError: Cannot convert a Symbol value to a number
round:  27 /a/
1 /a/undefined
2 /a/null
3 /a/NaN
4 /a/Infinity
5 /a/-Infinity
6 /a/0
7 /a/0
8 /a/1
9 /a/-1
10 /a/-1
11 /a/-5
12 /a/-5.2
13 /a/5
14 /a/5000
15 /a/5.2
16 /a/true
17 /a/false
18 /a/
19 /a/0
20 /a/1
21 /a/3
22 /a/0.0
23 /a/1.0
24 /a/true
25 /a/false
26 /a/function g() { }
27 /a/3.0
28 /a/1
29 /a//a/
TypeError: Cannot convert a Symbol value to a string
*/

