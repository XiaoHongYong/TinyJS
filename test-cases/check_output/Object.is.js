// Index: 0
// Object.is
var obj1 = { toString() { return '0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f1(r, p, noPrint) {
    console.log('round: ', r, noPrint ? '' : p);

    console.log('1', Object.is(p,  undefined));
    console.log('2', Object.is(p,  null));
    console.log('3', Object.is(p,  NaN));
    console.log('4', Object.is(p,  Infinity));
    console.log('5', Object.is(p,  -Infinity));
    console.log('6', Object.is(p,  0));
    console.log('7', Object.is(p,  0.0));
    console.log('8', Object.is(p,  1.0));
    console.log('9', Object.is(p,  -1));
    console.log('10', Object.is(p,  -1.0));
    console.log('13', Object.is(p,  5));
    console.log('16', Object.is(p,  true));
    console.log('17', Object.is(p,  false));
    console.log('18', Object.is(p,  ''));
    console.log('19', Object.is(p,  '0'));
    console.log('20', Object.is(p,  '1'));
    console.log('21', Object.is(p,  '234'.charAt(1)));
    console.log('22', Object.is(p,  '0.0'));
    console.log('23', Object.is(p,  '1.0'));
    console.log('24', Object.is(p,  'true'));
    console.log('25', Object.is(p,  'false'));
    console.log('26', Object.is(p,  g));
    console.log('27', Object.is(p,  obj1));
    console.log('28', Object.is(p,  obj2));
    console.log('29', Object.is(p,  /a/));
    console.log('29', Object.is(p,  Symbol()));
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
f1('14', -1.0);
f1('15', true);
f1('16', false);
f1('17', '');
f1('18', '0');
f1('19', '1');
f1('234'.charAt(1));
f1('20', '0.0');
f1('22', 'true');
f1('23', 'false');
f1('24', g, 1);
f1('25', obj1, 1);
f1('26', obj2, 1);
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
13 false
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
29 false
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
13 false
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
29 false
round:  3 NaN
1 false
2 false
3 true
4 false
5 false
6 false
7 false
8 false
9 false
10 false
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
round:  3 undefined
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
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
13 false
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
29 false
*/


// Index: 1
// Object.is
function f() {
    console.log(Object.is({}, {}));
    console.log(Object.is({}));
    console.log(Object.is(null));
    console.log(Object.is(undefined));
    console.log(Object.is());
}
f();
/* OUTPUT
false
false
false
true
true
*/


// Index: 2
// Object.isExtensible
function f() {
    console.log(Object.isExtensible());
    console.log(Object.isExtensible(null));
    console.log(Object.isExtensible(undefined));
    console.log(Object.isExtensible(1));
    console.log(Object.isExtensible(1.0));
    console.log(Object.isExtensible(true));
    console.log(Object.isExtensible(false));
    console.log(Object.isExtensible(''));
    console.log(Object.isExtensible('abc'.charAt(1)));
    console.log(Object.isExtensible(Symbol()));

    console.log(Object.isExtensible({}));
    console.log(Object.isExtensible(/a/));
    console.log(Object.isExtensible(String.prototype));
}
f();
/* OUTPUT
false
false
false
false
false
false
false
false
false
false
true
true
true
*/


// Index: 3
// Object.isExtensible
function f(r, o) {
    console.log('Round: ', r);
    Object.preventExtensions(o);
    console.log(Object.isExtensible(o));
}
f(1, {});
f(2, String.prototype);
f(3, []);
f(4, new Array());
f(5, new Object());
f(6, /a/);
f(7, new Number(1));
f(8, new String('x'));
/* OUTPUT
Round:  1
false
Round:  2
false
Round:  3
false
Round:  4
false
Round:  5
false
Round:  6
false
Round:  7
false
Round:  8
false
*/


// Index: 4
// Object.isFrozen
function f(r, o) {
    console.log('Round: ', r);

    Object.preventExtensions(o);
    console.log(Object.isFrozen(o));
}
f(1, {});
f(2, String.prototype);
f(3, []);
f(4, new Array());
f(5, new Object());
f(6, /a/);
f(7, new Number(1));
f(8, new String('x'));
/* OUTPUT
Round:  1
true
Round:  2
false
Round:  3
true
Round:  4
true
Round:  5
true
Round:  6
false
Round:  7
true
Round:  8
true
*/


// Index: 5
// Object.isFrozen: Array
function f() {
    var o = [];
    o.x = 1;
    Object.preventExtensions(o);
    console.log(Object.isFrozen(o)); // false

    var o = [];
    o[1] = 2;
    Object.preventExtensions(o);
    console.log(Object.isFrozen(o)); // false

    var o = /a/;
    Object.preventExtensions(o);
    console.log(Object.isFrozen(o)); // false，因为 lastIndex 是缺省添加的.
}
f();
/* OUTPUT
false
false
false
*/

