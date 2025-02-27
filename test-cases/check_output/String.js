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


// Index: 10
function f() {
    s = '\x0'; console.log('#17', s);
}
/* OUTPUT
Uncaught SyntaxError: Invalid hexadecimal escape sequence
*/


// Index: 11
function f() {
    s = '\x1h'; console.log('#17', s);
}
/* OUTPUT
Uncaught SyntaxError: Invalid hexadecimal escape sequence
*/


// Index: 12
function f() {
    s = '\u{0k}'; console.log('#17', s);
}
/* OUTPUT
Uncaught SyntaxError: Invalid Unicode escape sequence
*/


// Index: 13
function f() {
    s = '\u0k'; console.log('#17', s);
}
/* OUTPUT
Uncaught SyntaxError: Invalid Unicode escape sequence
*/


// Index: 14
function f() {
    s = '\u{0'; console.log('#17', s);
}
/* OUTPUT
Uncaught SyntaxError: Invalid Unicode escape sequence
*/


// Index: 15
//// match
function f() {
    var s;
    s = '\b'; console.log('#1', s);
    s = '\f'; console.log('#2', s);
    s = '\n'; console.log('#3', s);
    s = '\r'; console.log('#4', s);
    s = '\t'; console.log('#5', s);
    s = '\v'; console.log('#6', s);
    s = '\n'; console.log('#7', s);
    s = '\0'; console.log('#8', s);
    s = '\00'; console.log('#9', s);
    s = '\000'; console.log('#10', s);
    s = '\0000'; console.log('#11', s);
    s = '\0a'; console.log('#12', s);
    s = '\377'; console.log('#13', s);
    s = '\378'; console.log('#14', s);
    s = '\376'; console.log('#15', s);
    s = '\250'; console.log('#16', s);
    s = '\x00'; console.log('#17', s);
    s = '\x01'; console.log('#18', s);
    s = '\xff'; console.log('#19', s);
    s = '\u0000'; console.log('#20', s);
    s = '\u0123'; console.log('#21', s);
    s = '\u2665'; console.log('#22', s);
    s = '\u{000000000000000000004a}'; console.log('#23', s);
    s = '\"'; console.log('#24', s);
    s = '\''; console.log('#25', s);
    s = '\\'; console.log('#26', s);
    s = '\c'; console.log('#27', s);
    s = '\d'; console.log('#28', s);
}
f();
/* OUTPUT
#1 
#2 
#3 

#4 
#5 	
#6 
#7 

#8  
#9  
#10  
#11  0
#12  a
#13 ÿ
#14 8
#15 þ
#16 ¨
#17  
#18 
#19 ÿ
#20  
#21 ģ
#22 ♥
#23 J
#24 "
#25 '
#26 \
#27 c
#28 d
*/


// Index: 16
//// match
function f() {
    const paragraph = 'The quick brown fox jumps over the lazy dog. It barked.';
    const regex = /[a-z]+/g;
    var found = paragraph.match(regex);
    console.log(found);

    found = paragraph.match(/[a-z]+/);
    console.log(found);
}
f();
/* OUTPUT
[he, quick, brown, fox, jumps, over, the, lazy, dog, t, barked]
[he, groups: undefined, index: 1, input: The quick brown fox jumps over the lazy dog. It barked.]
*/


// Index: 17
//// match
function f() {
    const paragraph = 'The quick brown fox jumps\n over the lazy dog. It barked.';
    const regex = /[a-z]+/ig;
    var found = paragraph.match(regex);
    console.log(found);

    found = paragraph.match(new RegExp('[a-z]+', 'ig'));
    console.log(found);

    found = paragraph.match(/quick(.*)?fox/i);
    console.log(found);

    found = paragraph.match(/over(.*)?lazy/i);
    console.log(found);
}
f();
/* OUTPUT
[The, quick, brown, fox, jumps, over, the, lazy, dog, It, barked]
[The, quick, brown, fox, jumps, over, the, lazy, dog, It, barked]
[quick brown fox,  brown , groups: undefined, index: 4, input: The quick brown fox jumps
 over the lazy dog. It barked.]
[over the lazy,  the , groups: undefined, index: 27, input: The quick brown fox jumps
 over the lazy dog. It barked.]
*/


// Index: 18
//// match
function f() {
    const paragraph = 'The \u2665 quick brown fox jumps\n over the lazy dog. It barked.';
    const regex = /[a-z]+/ig;
    var found = paragraph.match(regex);
    console.log(found);

    found = paragraph.match(new RegExp('[a-z]+', 'ig'));
    console.log(found);

    found = paragraph.match(/quick(.*)?fox/i);
    console.log(found);

    found = paragraph.match(/over(.*)?lazy/i);
    console.log(found);
}
f();
/* OUTPUT
[The, quick, brown, fox, jumps, over, the, lazy, dog, It, barked]
[The, quick, brown, fox, jumps, over, the, lazy, dog, It, barked]
[quick brown fox,  brown , groups: undefined, index: 6, input: The ♥ quick brown fox jumps
 over the lazy dog. It barked.]
[over the lazy,  the , groups: undefined, index: 29, input: The ♥ quick brown fox jumps
 over the lazy dog. It barked.]
*/


// Index: 19
//// match 其他类型的情况
function f() {
    const paragraph = 'The \u2665 quick true NaN fox jumps\n over the lazy false. It null. 1.1x';
    var found = paragraph.match();
    console.log(found);

    var found = paragraph.match(null);
    console.log(found);

    var found = paragraph.match(undefined);
    console.log(found);

    var found = paragraph.match(1);
    console.log(found);

    var found = paragraph.match('.');
    console.log(found);

    var found = paragraph.match(true);
    console.log(found);

    var found = paragraph.match(false);
    console.log(found);

    var found = paragraph.match(1.1);
    console.log(found);

    var found = paragraph.match(NaN);
    console.log(found);

    var found = paragraph.match(Symbol('fox'));
    console.log(found);
}
f();
/* OUTPUT
[, groups: undefined, index: 0, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
[null, groups: undefined, index: 56, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
[, groups: undefined, index: 0, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
[1, groups: undefined, index: 62, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
[T, groups: undefined, index: 0, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
[true, groups: undefined, index: 12, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
[false, groups: undefined, index: 46, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
[1.1, groups: undefined, index: 62, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
[NaN, groups: undefined, index: 17, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
Uncaught TypeError: Cannot convert a Symbol value to a string
*/


// Index: 20
//// match
function f() {
    const paragraph = 'The \u2665 quick \u2665 jumps\n barked.';
    const regex = /[a-z]+/ig;
    console.log('### Round 1');
    var found = paragraph.matchAll(regex);
    for (var i of found) {
        console.log(i);
    }

    console.log('### Round 2');
    found = paragraph.matchAll(new RegExp('[a-z]+', 'ig'));
    for (var i of found) {
        console.log(i);
    }

    console.log('### Round 3');
    found = paragraph.matchAll(/quick(.*)?fox/ig);
    for (var i of found) {
        console.log(i);
    }

    console.log('### Round 4');
    found = paragraph.matchAll(/over(.*)?lazy/ig);
    for (var i of found) {
        console.log(i);
    }

    console.log('### Round 5');
    found = paragraph.matchAll(/over(.*)?lazy/i);
}
f();
/* OUTPUT
### Round 1
[The, groups: undefined, index: 0, input: The ♥ quick ♥ jumps
 barked.]
[quick, groups: undefined, index: 6, input: The ♥ quick ♥ jumps
 barked.]
[jumps, groups: undefined, index: 14, input: The ♥ quick ♥ jumps
 barked.]
[barked, groups: undefined, index: 21, input: The ♥ quick ♥ jumps
 barked.]
### Round 2
[The, groups: undefined, index: 0, input: The ♥ quick ♥ jumps
 barked.]
[quick, groups: undefined, index: 6, input: The ♥ quick ♥ jumps
 barked.]
[jumps, groups: undefined, index: 14, input: The ♥ quick ♥ jumps
 barked.]
[barked, groups: undefined, index: 21, input: The ♥ quick ♥ jumps
 barked.]
### Round 3
### Round 4
### Round 5
Uncaught TypeError: String.prototype.matchAll called with a non-global RegExp argument
*/


// Index: 21
//// matchAll 其他类型的情况
const paragraph = 'The \u2665 quick true NaN fox jumps\n over the lazy false. It null. 1.1x';
function f(r, p) {
    console.log('### Round ', r);

    var found = paragraph.matchAll(p);
    for (var i of found) {
        console.log(i);
    }
}

f(1, null);
f(2, 1);
f(3, 'x');
f(4, true);
f(5, false);
f(6, 1.1);
f(7, NaN);
f(8, Symbol('for'));
/* OUTPUT
### Round  1
[null, groups: undefined, index: 56, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
### Round  2
[1, groups: undefined, index: 62, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
[1, groups: undefined, index: 64, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
### Round  3
[x, groups: undefined, index: 23, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
[x, groups: undefined, index: 65, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
### Round  4
[true, groups: undefined, index: 12, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
### Round  5
[false, groups: undefined, index: 46, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
### Round  6
[1.1, groups: undefined, index: 62, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
### Round  7
[NaN, groups: undefined, index: 17, input: The ♥ quick true NaN fox jumps
 over the lazy false. It null. 1.1x]
### Round  8
Uncaught TypeError: Cannot convert a Symbol value to a string
*/


// Index: 22
//// padEnd
function f(r, len, pad) {
    console.log('### Round ', r, len, pad);

    var s = 'abc'.padEnd(len, pad);
    console.log(s);
}

console.log('abc'.padEnd());

f(1, undefined, undefined);
f(2, NaN, 'x');
f(3, null, 'x');
f(4, 0, 'x');
f(5, 10, undefined);
f(6, 9, null);
f(7, 8, NaN);
f(8, 8, '');
f(9, 8, 'xyz');
f(10, 9, 'xyz');
f(11, 10, 'xyz');
f(12, 11, 'xyzg');
f(13, 7, 'x');
f(14, Symbol('for'));
/* OUTPUT
abc
### Round  1 undefined undefined
abc
### Round  2 NaN x
abc
### Round  3 null x
abc
### Round  4 0 x
abc
### Round  5 10 undefined
abc       
### Round  6 9 null
abcnullnu
### Round  7 8 NaN
abcNaNNa
### Round  8 8 
abc
### Round  9 8 xyz
abcxyzxy
### Round  10 9 xyz
abcxyzxyz
### Round  11 10 xyz
abcxyzxyzx
### Round  12 11 xyzg
abcxyzgxyzg
### Round  13 7 x
abcxxxx
### Round  14 Symbol(for) undefined
Uncaught TypeError: Cannot convert a Symbol value to a number
*/


// Index: 23
//// padStart
function f(r, len, pad) {
    console.log('### Round ', r, len, pad);

    var s = 'abc'.padStart(len, pad);
    console.log(s);
}

console.log('abc'.padStart());

f(1, undefined, undefined);
f(2, NaN, 'x');
f(3, null, 'x');
f(4, 0, 'x');
f(5, 10, undefined);
f(6, 9, null);
f(7, 8, NaN);
f(8, 8, '');
f(9, 8, 'xyz');
f(10, 9, 'xyz');
f(11, 10, 'xyz');
f(12, 11, 'xyzg');
f(13, 7, 'x');
f(13, 0xffffffff, 'x');
f(14, Symbol('for'));
/* OUTPUT
abc
### Round  1 undefined undefined
abc
### Round  2 NaN x
abc
### Round  3 null x
abc
### Round  4 0 x
abc
### Round  5 10 undefined
       abc
### Round  6 9 null
nullnuabc
### Round  7 8 NaN
NaNNaabc
### Round  8 8 
abc
### Round  9 8 xyz
xyzxyabc
### Round  10 9 xyz
xyzxyzabc
### Round  11 10 xyz
xyzxyzxabc
### Round  12 11 xyzg
xyzgxyzgabc
### Round  13 7 x
xxxxabc
### Round  13 4294967295 x
Uncaught RangeError: Invalid string length
*/


// Index: 24
//// repeat
function f(r, len, noPrint) {
    console.log('### Round ', r, noPrint ? '' : len);

    try {
        var s = 'abc'.repeat(len);
        console.log(s);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}

console.log(''.repeat(1000));

f(1, undefined);
f(2, NaN);
f(3, null);
f(4, 0);
f(5, -1);
f(6, 9);
f(7, 1);
f(8, '2');
f(9, { toString() { return '3'}}, 1);
f(10, function() {}, 1);
f(14, Symbol());
/* OUTPUT
### Round  1 undefined

### Round  2 NaN

### Round  3 null

### Round  4 0

### Round  5 -1
RangeError: Invalid count value: -1
### Round  6 9
abcabcabcabcabcabcabcabcabc
### Round  7 1
abc
### Round  8 2
abcabc
### Round  9 
abcabcabc
### Round  10 

### Round  14 Symbol()
TypeError: Cannot convert a Symbol value to a number
*/


// Index: 25
//// replace
function f(r, pattern, replacement, noPrint) {
    console.log('### Round ', r, noPrint ? '' : pattern + ', ' + replacement);

    try {
        var s = '-undefined=abc'.replace(pattern, replacement);
        console.log(s);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}

console.log(''.replace('x'));
f(1, undefined, null);
f(2, 'ab', 'xy');
f(3, '-', '$`');
f(3.1, 'd', '$`');
f('3.2', 'i', '$\'');
f('3.3', 'c', '$\'');
f(4, 'def', '$&');
f(5, 'de', '$$');
f(6, 'd', '$');
f(6, 'd', '$x');
f(7, 1);
f(8, '2');
f(9, { toString() { return '3'}}, 1);
f(10, function() {}, 1);
f(14, Symbol());
/* OUTPUT
### Round  1 undefined, null
-null=abc
### Round  2 ab, xy
-undefined=xyc
### Round  3 -, $`
undefined=abc
### Round  3.1 d, $`
-un-unefined=abc
### Round  3.2 i, $'
-undefned=abcned=abc
### Round  3.3 c, $'
-undefined=ab
### Round  4 def, $&
-undefined=abc
### Round  5 de, $$
-un$fined=abc
### Round  6 d, $
-un$efined=abc
### Round  6 d, $x
-un$xefined=abc
### Round  7 1, undefined
-undefined=abc
### Round  8 2, undefined
-undefined=abc
### Round  9 3, 1
-undefined=abc
### Round  10 function() {}, 1
-undefined=abc
Uncaught TypeError: Cannot convert a Symbol value to a string
*/


// Index: 26
var a = 1;
// let dateRegexp = /(?<year>[0-9]{4})-(?<month>[0-9]{2})-(?<day>[0-9]{2})/;
// let str = "2019-04-30";
// let groups = str.match(dateRegexp).groups;
// console.log(groups.year); // 2019
// console.log(groups.month); // 04
// console.log(groups.day); // 30
/* OUTPUT

*/


// Index: 27
// replace: string
function f() {
    let dateRegexp = /([0-9]{4})-([0-9]{2})-([0-9]{2})/;
    let str = "2019-04-19";
    console.log(str.replace(dateRegexp, '$0:$1/$2/$3/$4/$10/$100/$20/$30'));
}
f();
/* OUTPUT
$0:2019/04/19/$4/20190/201900/040/190
*/


// Index: 28
// replace: function
function f() {
    function replacer(match, p1, p2, p3, offset, string, groups) {
        // p1 is non-digits, p2 digits, and p3 non-alphanumerics
        console.log(match);
        console.log(p1);
        console.log(p2);
        console.log(p3);
        console.log(offset);
        console.log(string);
        return p1 + '-' + p2 + '-' + p3;
    }
    const newString = '\u2665abc12345#$*%'.replace(/([a-z]+)(\d*)([^\w]*)/, replacer);
    console.log(newString);
}
f();
/* OUTPUT
abc12345#$*%
abc
12345
#$*%
1
♥abc12345#$*%
♥abc-12345-#$*%
*/


// Index: 29
// replaceAll: string
function f() {
    let p = /(\w+): (\d+)/g;
    let str = "abc: 1, def: 22";
    console.log(str.replaceAll(p, '$0:$1/$2/$3/$4/$10/$100/$20/$30'));
}
f();
/* OUTPUT
$0:abc/1/$3/$4/abc0/abc00/10/$30, $0:def/22/$3/$4/def0/def00/220/$30
*/


// Index: 30
// replaceAll: function
function f() {
    function replacer(match, p1, p2, p3, offset, string, groups) {
        // p1 is non-digits, p2 digits, and p3 non-alphanumerics
        console.log(match);
        console.log(p1);
        console.log(p2);
        console.log(p3);
        console.log(offset);
        console.log(string);
        return p1 + '-' + p2 + '-' + p3;
    }
    const newString = '\u2665abc12345#$*% def789@@'.replaceAll(/([a-z]+)(\d*)([^\w]*)/g, replacer);
    console.log(newString);
}
f();
/* OUTPUT
abc12345#$*% 
abc
12345
#$*% 
1
♥abc12345#$*% def789@@
def789@@
def
789
@@
14
♥abc12345#$*% def789@@
♥abc-12345-#$*% def-789-@@
*/


// Index: 31
// replaceAll: string
function f() {
    let p = '(\\w+): (\\d+)';
    let str = "abc: 1, abc,def: 22";
    console.log(str.replaceAll(p, '$0:$1/$2/$3/$4/$10/$100/$20/$30'));

    console.log(str.replaceAll('abc', '$0:$1/$2/$3/$4/$10/$100/$20/$30'));

    console.log(str.replaceAll(/abc/, '$0')); // throw exception
}
f();
/* OUTPUT
abc: 1, abc,def: 22
$0:$1/$2/$3/$4/$10/$100/$20/$30: 1, $0:$1/$2/$3/$4/$10/$100/$20/$30,def: 22
Uncaught TypeError: String.prototype.replaceAll called with a non-global RegExp argument
*/


// Index: 32
// search
function f() {
    let str = "abc: 1, abc,def: 22";
    console.log(str.search(/a/));
    console.log(str.search(/a/g));
    console.log(str.search(/:/));
    console.log(str.search(/(\d)+/));
    console.log(str.search('@@'));
    console.log(str.search());
}
f();
/* OUTPUT
0
0
3
5
-1
0
*/


// Index: 33
// slice
function f() {
    let str = "abcdefghijklmnopqrst";
    console.log(1, str.slice(/a/));
    console.log(2, str.slice(NaN, 1));
    console.log(3, str.slice(NaN, Infinity));
    console.log(4, str.slice(true));
    console.log(5, str.slice('3'));
    console.log(6, str.slice());

    console.log(7, str.slice(undefined, -1));
    console.log(8, str.slice(-2, -1));
    console.log(9, str.slice(-2, 0));
    console.log(10, str.slice(50, 51));

    str = 'X';
    console.log(11, str.slice(0, -1));
    console.log(12, str.slice(0, 1));
    console.log(13, str.slice(-1, 1));
    console.log(14, str.slice(50, 51));
    console.log(15, str.slice());

    str = 'X-\u2665-D, \u2665-X';
    console.log(16, str.slice(4, 8));

    str.slice(Symbol());
}
f();
/* OUTPUT
1 abcdefghijklmnopqrst
2 a
3 abcdefghijklmnopqrst
4 bcdefghijklmnopqrst
5 defghijklmnopqrst
6 abcdefghijklmnopqrst
7 abcdefghijklmnopqrs
8 s
9 
10 
11 
12 X
13 X
14 
15 X
16 D, ♥
Uncaught TypeError: Cannot convert a Symbol value to a number
*/


// Index: 34
// substring
function f() {
    let str = "abcdefghijklmnopqrst";
    console.log(1, str.substring(/a/));
    console.log(2, str.substring(NaN, 1));
    console.log(3, str.substring(NaN, Infinity));
    console.log(4, str.substring(true));
    console.log(5, str.substring('3'));
    console.log(6, str.substring());

    console.log(7, str.substring(undefined, -1));
    console.log(8, str.substring(-2, -1));
    console.log(9, str.substring(-2, 2));
    console.log(10, str.substring(50, 51));

    str = 'X';
    console.log(11, str.substring(2, -1));
    console.log(12, str.substring(3, 1));
    console.log(13, str.substring(-1, 1));
    console.log(14, str.substring(50, 51));
    console.log(15, str.substring());

    str = 'X-\u2665-D, \u2665-X';
    console.log(16, str.substring(4, 8));

    str.substring(Symbol());
}
f();
/* OUTPUT
1 abcdefghijklmnopqrst
2 a
3 abcdefghijklmnopqrst
4 bcdefghijklmnopqrst
5 defghijklmnopqrst
6 abcdefghijklmnopqrst
7 
8 
9 ab
10 
11 X
12 
13 X
14 
15 X
16 D, ♥
Uncaught TypeError: Cannot convert a Symbol value to a number
*/


// Index: 35
// substr
function f() {
    let str = "abcdefghijklmnopqrst";
    console.log(1, str.substr(/a/));
    console.log(2, str.substr(NaN, 1));
    console.log(3, str.substr(NaN, Infinity));
    console.log(4, str.substr(true));
    console.log(5, str.substr('3'));
    console.log(6, str.substr());

    console.log(7, str.substr(undefined, -1));
    console.log(8, str.substr(-1, 3));
    console.log(9, str.substr(-2, 2));
    console.log(10, str.substr(50, 51));

    str = 'X';
    console.log(11, str.substr(2, -1));
    console.log(12, str.substr(3, 1));
    console.log(13, str.substr(-1, 1));
    console.log(14, str.substr(50, 51));
    console.log(15, str.substr());

    str = 'X-\u2665-D, \u2665-X';
    console.log(16, str.substr(4, 4));

    str.substr(Symbol());
}
f();
/* OUTPUT
1 abcdefghijklmnopqrst
2 a
3 abcdefghijklmnopqrst
4 bcdefghijklmnopqrst
5 defghijklmnopqrst
6 abcdefghijklmnopqrst
7 
8 t
9 st
10 
11 
12 
13 X
14 
15 X
16 D, ♥
Uncaught TypeError: Cannot convert a Symbol value to a number
*/


// Index: 36
// split
function f() {
    let str = "abcdefg";
    console.log(1, str.split());
    // console.log(2, str.split(undefined, Infinity));
    console.log(3, str.split(undefined, NaN));
    console.log(4, str.split('', 1));
    console.log(4, str.split('', 2.5));
    console.log(5, str.split('', 5));
    console.log(6, str.split(''));
    console.log(6, str.split('', -1));

    str = 'babcbdb'
    console.log(7, str.split(/b/));
    console.log(8, str.split(/b/, 1));
    console.log(9, str.split(/b/, 3));
    console.log(10, str.split(/b/, -10));

    console.log(11, str.split(/b/));
    console.log(12, str.split(/b/, 1));
    console.log(13, str.split(/b/, 3));
    console.log(14, str.split(/b/, -10));

    str = 'X';
    console.log(15, str.split('X'));
    console.log(15, str.split('x'));

    str.split(Symbol());
}
f();
/* OUTPUT
1 [abcdefg]
3 []
4 [a]
4 [a, b]
5 [a, b, c, d, e]
6 [a, b, c, d, e, f, g]
6 [a, b, c, d, e, f, g]
7 [, a, c, d, ]
8 []
9 [, a, c]
10 [, a, c, d, ]
11 [, a, c, d, ]
12 []
13 [, a, c]
14 [, a, c, d, ]
15 [, ]
15 [X]
Uncaught TypeError: Cannot convert a Symbol value to a string
*/


// Index: 37
// startsWith
function f() {
    let str = "abcdefg";
    console.log(1, str.split());
    // console.log(2, str.split(undefined, Infinity));
    console.log(3, str.startsWith(a, NaN));
    console.log(4, str.startsWith('b', 1));
    console.log(4, str.startsWith('c', 2.5));
    console.log(5, str.startsWith('a', 5));
    console.log(6, str.startsWith(''));
    console.log(6, str.startsWith('', -1));

    str = 'babcbdb'
    console.log(7, str.startsWith('ba'));
    console.log(8, str.startsWith('ba', 1));
    console.log(9, str.startsWith('ba', 3));
    console.log(10, str.startsWith('ba', -10));
    console.log(11, 'undefined'.startsWith());

    str = 'X';
    console.log(12, str.startsWith('X', 3));
    console.log(15, str.startsWith('X'));
    console.log(16, str.startsWith('x'));

    str.split(Symbol());
}
f();
/* OUTPUT
1 [abcdefg]
Uncaught ReferenceError: a is not defined
*/


// Index: 38
// toLowerCase
function f() {
    let str = "abcdefg";
    console.log(2, str.toLowerCase(), str);
    str = 'Abc';
    console.log(3, str.toLowerCase(), str);
    str = 'ABC';
    console.log(4, str.toLowerCase(), str);
    str = 'A';
    console.log(5, str.toLowerCase(), str);
    str = 'a';
    console.log(6, str.toLowerCase(), str);
    str = '@#@#$%';
    console.log(6, str.toLowerCase(), str);

    str = 'X'
    console.log(7, str.toLowerCase());
    str = 'x';
    console.log(8, str.toLowerCase());
    str = '@'
    console.log(9, str.toLowerCase());
}
f();
/* OUTPUT
2 abcdefg abcdefg
3 abc Abc
4 abc ABC
5 a A
6 a a
6 @#@#$% @#@#$%
7 x
8 x
9 @
*/


// Index: 39
// toUpperCase
function f() {
    let str = "abcdefg";
    console.log(2, str.toUpperCase(), str);
    str = 'Abc';
    console.log(3, str.toUpperCase(), str);
    str = 'ABC';
    console.log(4, str.toUpperCase(), str);
    str = 'A';
    console.log(5, str.toUpperCase(), str);
    str = 'a';
    console.log(6, str.toUpperCase(), str);
    str = '@#@#$%';
    console.log(6, str.toUpperCase(), str);

    str = 'X'
    console.log(7, str.toUpperCase());
    str = 'x';
    console.log(8, str.toUpperCase());
    str = '@'
    console.log(9, str.toUpperCase());
}
f();
/* OUTPUT
2 ABCDEFG abcdefg
3 ABC Abc
4 ABC ABC
5 A A
6 A a
6 @#@#$% @#@#$%
7 X
8 X
9 @
*/


// Index: 40
// toString
function f() {
    let str = "abcdefg";
    console.log(2, str.toString());
    str = new String('xhy');
    console.log(3, str.toString());
    str = 'A';
    console.log(4, str.toString());
}
f();
/* OUTPUT
2 abcdefg
3 xhy
4 A
*/


// Index: 41
// trim, trimStart, trimEnd
function f() {
    let str = " \n\\trab \t\n cdefg\r\n\t";
    console.log(2, str.trim());
    console.log(3, str.trimStart());
    console.log(4, str.trimEnd());

    str = 'abc';
    console.log(5, str.trim());
    console.log(6, str.trimStart());
    console.log(7, str.trimEnd());

    str = 'x';
    console.log(8, str.trim());
    console.log(9, str.trimStart());
    console.log(10, str.trimEnd());

    str = '\t';
    console.log(11, str.trim());
    console.log(12, str.trimStart());
    console.log(13, str.trimEnd());
}
f();
/* OUTPUT
2 \trab 	
 cdefg
3 \trab 	
 cdefg
	
4  
\trab 	
 cdefg
5 abc
6 abc
7 abc
8 x
9 x
10 x
11 
12 
13
*/

