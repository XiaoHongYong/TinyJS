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
f('19.1'.charAt(1));
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
Round:  9 undefined
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

