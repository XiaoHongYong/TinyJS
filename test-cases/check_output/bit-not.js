// Index: 0
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f() {
    console.log('1', ~undefined);
    console.log('2', ~null);
    console.log('3', ~NaN);
    console.log('4', ~Infinity);
    console.log('5', ~-Infinity);
    console.log('6', ~0);
    console.log('7', ~0.0);
    console.log('8', ~1.0);
    console.log('9', ~0x1fffffffff11);
    console.log('9a', ~-0x1fffffffff11);
    console.log('9b', ~-1);
    console.log('10', ~-1.0);
    console.log('11', ~-5);
    console.log('12', ~-5.2);
    console.log('13', ~5);
    console.log('14', ~5000);
    console.log('15', ~5.2);
    console.log('16', ~true);
    console.log('17', ~false);
    console.log('18', ~'');
    console.log('19', ~'0');
    console.log('20', ~'1');
    console.log('21', ~'234'.charAt(1));
    console.log('22', ~'0.0');
    console.log('23', ~'1.0');
    console.log('24', ~'true');
    console.log('25', ~'false');
    console.log('26', ~g);
    console.log('27', ~obj1);
    console.log('28', ~obj2);
    console.log('29', ~/a/);
    try {
        var a = Symbol();
        console.log(~a);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
        // console.log(e);
    }
}
f();
/* OUTPUT
1 -1
2 -1
3 -1
4 -1
5 -1
6 -1
7 -1
8 -2
9 238
9a -240
9b 0
10 0
11 4
12 4
13 -6
14 -5001
15 -6
16 -2
17 -1
18 -1
19 -1
20 -2
21 -4
22 -1
23 -2
24 -1
25 -1
26 -1
27 -4
28 -2
29 -1
TypeError: Cannot convert a Symbol value to a number
*/


// Index: 1
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f1(r, p, noPrint) {
    console.log('Round: ', r, noPrint ? '' : p);
    console.log(~p);
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
-1
Round:  2 null
-1
Round:  3 NaN
-1
Round:  4 Infinity
-1
Round:  5 -Infinity
-1
Round:  6 0
-1
Round:  7 0
-1
Round:  8 1
-2
Round:  9 35171485089553
2097390
Round:  9a -35171485089553
-2097392
Round:  9b -1
0
Round:  10 -2
1
Round:  11 -2
1
Round:  12 2
-3
Round:  13 2
-3
Round:  14 -1
0
Round:  15 true
-2
Round:  16 false
-1
Round:  17 
-1
Round:  18 0
-1
Round:  19 1
-2
Round:  9 undefined
-1
Round:  20 0.0
-1
Round:  21 1.0
-2
Round:  22 true
-1
Round:  23 false
-1
Round:  24 
-1
Round:  25 
-4
Round:  26 
-2
Round:  27 /a/
-1
*/

