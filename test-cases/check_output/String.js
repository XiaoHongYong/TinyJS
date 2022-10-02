// Index: 0
//// fromCharCode 参数为字符串
function f() {
    console.log(String.fromCharCode(65, '66'));
    console.log(String.fromCharCode(65, '66', 67, 67.8, 68));

    var obj = { toString() { return 69; }};
    console.log(String.fromCharCode(obj));
}
f();
/* OUTPUT
AB
ABCCD
E
*/


// Index: 1
//// charAt
function f() {
    var s = 'abcde';
    console.log(s.charAt(0));
    console.log(s.charCodeAt(0));
    console.log(s.at(0));

    console.log(s.charAt(-1));
    console.log(s.charCodeAt(-1));
    console.log(s.at(-1));
}
f();
/* OUTPUT
a
97
a

NaN
e
*/


// Index: 2
//// charAt
function f() {
    console.log(String.fromCodePoint(0));
    console.log('a');
}
f();
/* OUTPUT
 
a
*/


// Index: 3
//// fromCharCode 参数为字符串
var obj1 = { toString() { return '63.0'; }}
var obj2 = { toString() { return 61.1; }}
function g() { }
function f(r, p, noPrint) {
    console.log('Round: ', r, noPrint ? '' : p);
    try {
        console.log(String.fromCodePoint(p));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f('0');
f('1', undefined);
f('2', null);
f('3', NaN);
f('4', Infinity);
f('5', -Infinity);
f('6', 50);
f('7', 50.0);
f('8', 51.0);
f('9', -51);
f('10', -52);
f('11', -52.0);
f('12', 52);
f('13', 52.0);
f('14', -51.0);
f('15', true);
f('16', false);
f('17', '');
f('18', '50');
f('19', '51');
f('19.1', 'xz'.charAt(1));
f('20', '50.0');
f('21', '51.0');
f('22', 'true');
f('23', 'false');
f('24', g, 1);
f('25', obj1, 1);
f('26', obj2, 1);
f('27', /a/);
f('28', Symbol());
/* OUTPUT
Round:  0 undefined
RangeError: Invalid code point NaN
Round:  1 undefined
RangeError: Invalid code point NaN
Round:  2 null
 
Round:  3 NaN
RangeError: Invalid code point NaN
Round:  4 Infinity
RangeError: Invalid code point Infinity
Round:  5 -Infinity
RangeError: Invalid code point -Infinity
Round:  6 50
2
Round:  7 50
2
Round:  8 51
3
Round:  9 -51
RangeError: Invalid code point -51
Round:  10 -52
RangeError: Invalid code point -52
Round:  11 -52
RangeError: Invalid code point -52
Round:  12 52
4
Round:  13 52
4
Round:  14 -51
RangeError: Invalid code point -51
Round:  15 true

Round:  16 false
 
Round:  17 
 
Round:  18 50
2
Round:  19 51
3
Round:  19.1 z
RangeError: Invalid code point NaN
Round:  20 50.0
2
Round:  21 51.0
3
Round:  22 true
RangeError: Invalid code point NaN
Round:  23 false
RangeError: Invalid code point NaN
Round:  24 
RangeError: Invalid code point NaN
Round:  25 
?
Round:  26 
RangeError: Invalid code point 61.1
Round:  27 /a/
RangeError: Invalid code point NaN
Round:  28 Symbol()
TypeError: Cannot convert a Symbol value to a number
*/


// Index: 4
//// charCodeAt
function f() {
    s = 'a𝌆bc'
    console.log(s.charCodeAt(0));
    console.log(s.charCodeAt(1));
    console.log(s.charCodeAt(2));
    console.log(s.charCodeAt(3));
}
f();
/* OUTPUT
97
55348
57094
98
*/


// Index: 5
//// charCodeAt
function f() {
    s = 'a𝌆bc'
    console.log(s.codePointAt(0.1));
    console.log(s.codePointAt(1));
    console.log(s.codePointAt(2));
    console.log(s.codePointAt(3));
}
f();
/* OUTPUT
97
119558
57094
98
*/


// Index: 6
//// concat
var obj1 = { toString() { return '63.0'; }}
var obj2 = { toString() { return 61.1; }}
function g() { }
function f(r, p, noPrint) {
    console.log('Round: ', r, noPrint ? '' : p);
    try {
        console.log('abc'.concat(p));
        console.log('a'.concat(p));
        console.log(''.concat(p));
        console.log(new String('abc').concat(p));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f('0');
f('1', undefined);
f('2', null);
f('3', NaN);
f('4', Infinity);
f('5', -Infinity);
f('6', 50);
f('7', 50.0);
f('8', 51.0);
f('9', -51);
f('10', -52);
f('11', -52.0);
f('12', 52);
f('13', 52.0);
f('14', -51.0);
f('15', true);
f('16', false);
f('17', '');
f('18', '50');
f('19', '51');
f('19.1', 'xz'.charAt(1));
f('20', '50.0');
f('21', '51.0');
f('22', 'true');
f('23', 'false');
f('24', g, 1);
f('25', obj1, 1);
f('26', obj2, 1);
f('26', new String('xy'), 1);
f('27', /a/);
f('28', Symbol());
/* OUTPUT
Round:  0 undefined
abcundefined
aundefined
undefined
abcundefined
Round:  1 undefined
abcundefined
aundefined
undefined
abcundefined
Round:  2 null
abcnull
anull
null
abcnull
Round:  3 NaN
abcNaN
aNaN
NaN
abcNaN
Round:  4 Infinity
abcInfinity
aInfinity
Infinity
abcInfinity
Round:  5 -Infinity
abc-Infinity
a-Infinity
-Infinity
abc-Infinity
Round:  6 50
abc50
a50
50
abc50
Round:  7 50
abc50
a50
50
abc50
Round:  8 51
abc51
a51
51
abc51
Round:  9 -51
abc-51
a-51
-51
abc-51
Round:  10 -52
abc-52
a-52
-52
abc-52
Round:  11 -52
abc-52
a-52
-52
abc-52
Round:  12 52
abc52
a52
52
abc52
Round:  13 52
abc52
a52
52
abc52
Round:  14 -51
abc-51
a-51
-51
abc-51
Round:  15 true
abctrue
atrue
true
abctrue
Round:  16 false
abcfalse
afalse
false
abcfalse
Round:  17 
abc
a

abc
Round:  18 50
abc50
a50
50
abc50
Round:  19 51
abc51
a51
51
abc51
Round:  19.1 z
abcz
az
z
abcz
Round:  20 50.0
abc50.0
a50.0
50.0
abc50.0
Round:  21 51.0
abc51.0
a51.0
51.0
abc51.0
Round:  22 true
abctrue
atrue
true
abctrue
Round:  23 false
abcfalse
afalse
false
abcfalse
Round:  24 
abcfunction g() { }
afunction g() { }
function g() { }
abcfunction g() { }
Round:  25 
abc63.0
a63.0
63.0
abc63.0
Round:  26 
abc61.1
a61.1
61.1
abc61.1
Round:  26 
abcxy
axy
xy
abcxy
Round:  27 /a/
abc/a/
a/a/
/a/
abc/a/
Round:  28 Symbol()
TypeError: Cannot convert a Symbol value to a string
*/


// Index: 7
//// endsWith
function f() {
    s = 'a𝌆bc'
    console.log(s.endsWith(''));
    console.log(s.endsWith('𝌆bc'));
    console.log(s.endsWith('c'));
    console.log(s.endsWith());

    s = 'a𝌆bc1'
    console.log(s.endsWith('1'));
    console.log(s.endsWith(1));

    s = 'a𝌆bcNaN'
    console.log(s.endsWith(NaN));

    s = 'a𝌆bcInfinity'
    console.log(s.endsWith(Infinity));
}
f();
/* OUTPUT
true
true
true
false
true
true
true
true
*/


// Index: 8
//// indexOf
function f() {
    s = 'a𝌆bc'
    console.log(1, s.indexOf(''));
    console.log(2, s.indexOf('𝌆bc'));
    console.log(3, s.indexOf('c'));
    console.log(4, s.indexOf());
    console.log('4.1', s.indexOf('𝌆', -1));
    console.log('4.2', s.indexOf('𝌆', 10));

    s = 'a𝌆bc1'
    console.log(5, s.indexOf('1'));
    console.log(6, s.indexOf(1));

    s = 'a𝌆bcNaN'
    console.log(7, s.indexOf(NaN));

    s = 'a𝌆bcInfinity'
    console.log(8, s.indexOf(Infinity));

    s = 'abcbc'
    console.log(9, s.indexOf('a', 0));
    console.log(10, s.indexOf('a', 1));
    console.log(11, s.indexOf('b', 1));
    console.log(12, s.indexOf('bc', 2));
    console.log(13, s.indexOf('b', 4));
    console.log(14, s.indexOf('b', 10));
    console.log(15, s.indexOf('b', -1));
}
f();
/* OUTPUT
1 0
2 1
3 4
4 -1
4.1 1
4.2 -1
5 5
6 5
7 5
8 5
9 0
10 -1
11 1
12 3
13 -1
14 -1
15 1
*/


// Index: 9
//// lastIndexOf
function f() {
    s = 'a𝌆bc'
    console.log(s.lastIndexOf(''));
    console.log(s.lastIndexOf('𝌆bc'));
    console.log(s.lastIndexOf('c'));
    console.log(s.lastIndexOf());

    s = 'a𝌆bc1'
    console.log(s.lastIndexOf('1'));
    console.log(s.lastIndexOf(1));

    s = 'a𝌆bcNaN'
    console.log(s.lastIndexOf(NaN));

    s = 'a𝌆bcInfinity'
    console.log(s.lastIndexOf(Infinity));

    s = 'abcbc'
    console.log(s.lastIndexOf('a', 0));
    console.log(s.lastIndexOf('a', 1));
    console.log(s.lastIndexOf('b', 1));
    console.log(s.lastIndexOf('bc', 2));
    console.log(s.lastIndexOf('b', 4));
    console.log(s.lastIndexOf('b', 10));
    console.log(s.lastIndexOf('b', -1));
}
f();
/* OUTPUT
5
1
4
-1
5
5
5
5
0
0
1
1
3
3
-1
*/

