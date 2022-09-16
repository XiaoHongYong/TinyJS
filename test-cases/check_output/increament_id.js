// Index: 0
function f1() {
    var obj = { toString() { return '1.0'; }}
    function g() { }
    var a;

    a = undefined;        console.log(a++, a);
    a = null;             console.log(a++, a);
    a = NaN;              console.log(a++, a);
    a = 0;                console.log(a++, a);
    a = 0.0;              console.log(a++, a);
    a = 1.0;              console.log(a++, a);
    a = -1;               console.log(a++, a);
    a = -1.0;             console.log(a++, a);
    a = true;             console.log(a++, a);
    a = false;            console.log(a++, a);
    a = '';               console.log(a++, a);
    a = '0';              console.log(a++, a);
    a = '1';              console.log(a++, a);
    a = '0.0';            console.log(a++, a);
    a = '1.0';            console.log(a++, a);
    a = 'true';           console.log(a++, a);
    a = 'false';          console.log(a++, a);
    a = g;                console.log(a++, a);
    a = obj;              console.log(a++, a);
    a = /a/;              console.log(a++, a);
    try {
        a = Symbol();         console.log(a++);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f1();
/* OUTPUT
NaN NaN
0 1
NaN NaN
0 1
0 1
1 2
-1 0
-1 0
1 2
0 1
0 1
0 1
1 2
0 1
1 2
NaN NaN
NaN NaN
NaN NaN
1 2
NaN NaN
TypeError: Cannot convert a Symbol value to a number
*/


// Index: 1
function f2() {
    var obj = { toString() { return '1.0'; }}
    function g() { }
    var a;

    a = undefined;        console.log(1, a--, a);
    a = null;             console.log(2, a--, a);
    a = NaN;              console.log(3, a--, a);
    a = 0;                console.log(4, a--, a);
    a = 0.0;              console.log(5, a--, a);
    a = 1.0;              console.log(6, a--, a);
    a = -1;               console.log(7, a--, a);
    a = -1.0;             console.log(8, a--, a);
    a = true;             console.log(9, a--, a);
    a = false;            console.log(10, a--, a);
    a = '';               console.log(11, a--, a);
    a = '0';              console.log(12, a--, a);
    a = '1';              console.log(13, a--, a);
    a = '0.0';            console.log(14, a--, a);
    a = '1.0';            console.log(15, a--, a);
    a = 'true';           console.log(16, a--, a);
    a = 'false';          console.log(17, a--, a);
    a = g;                console.log(18, a--, a);
    a = obj;              console.log(19, a--, a);
    a = /a/;              console.log(20, a--, a);
    try {
        a = Symbol();         console.log(a--);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f2();
/* OUTPUT
1 NaN NaN
2 0 -1
3 NaN NaN
4 0 -1
5 0 -1
6 1 0
7 -1 -2
8 -1 -2
9 1 0
10 0 -1
11 0 -1
12 0 -1
13 1 0
14 0 -1
15 1 0
16 NaN NaN
17 NaN NaN
18 NaN NaN
19 1 0
20 NaN NaN
TypeError: Cannot convert a Symbol value to a number
*/


// Index: 2
function f3() {
    var obj = { toString() { return '1.0'; }}
    function g() { }
    var a;

    a = undefined;        console.log(--a, a);
    a = null;             console.log(--a, a);
    a = NaN;              console.log(--a, a);
    a = 0;                console.log(--a, a);
    a = 0.0;              console.log(--a, a);
    a = 1.0;              console.log(--a, a);
    a = -1;               console.log(--a, a);
    a = -1.0;             console.log(--a, a);
    a = true;             console.log(--a, a);
    a = false;            console.log(--a, a);
    a = '';               console.log(--a, a);
    a = '0';              console.log(--a, a);
    a = '1';              console.log(--a, a);
    a = '0.0';            console.log(--a, a);
    a = '1.0';            console.log(--a, a);
    a = 'true';           console.log(--a, a);
    a = 'false';          console.log(--a, a);
    a = g;                console.log(--a, a);
    a = obj;              console.log(--a, a);
    a = /a/;              console.log(--a, a);
    try {
        a = Symbol();         console.log(--a);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f3();
/* OUTPUT
NaN NaN
-1 -1
NaN NaN
-1 -1
-1 -1
0 0
-2 -2
-2 -2
0 0
-1 -1
-1 -1
-1 -1
0 0
-1 -1
0 0
NaN NaN
NaN NaN
NaN NaN
0 0
NaN NaN
TypeError: Cannot convert a Symbol value to a number
*/


// Index: 3
function f4() {
    var obj = { toString() { return '1.0'; }}
    function g() { }
    var a;

    a = undefined;        console.log(++a, a);
    a = null;             console.log(++a, a);
    a = NaN;              console.log(++a, a);
    a = 0;                console.log(++a, a);
    a = 0.0;              console.log(++a, a);
    a = 1.0;              console.log(++a, a);
    a = -1;               console.log(++a, a);
    a = -1.0;             console.log(++a, a);
    a = true;             console.log(++a, a);
    a = false;            console.log(++a, a);
    a = '';               console.log(++a, a);
    a = '0';              console.log(++a, a);
    a = '1';              console.log(++a, a);
    a = '0.0';            console.log(++a, a);
    a = '1.0';            console.log(++a, a);
    a = 'true';           console.log(++a, a);
    a = 'false';          console.log(++a, a);
    a = g;                console.log(++a, a);
    a = obj;              console.log(++a, a);
    a = /a/;              console.log(++a, a);
    try {
        a = Symbol();         console.log(++a);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f4();
/* OUTPUT
NaN NaN
1 1
NaN NaN
1 1
1 1
2 2
0 0
0 0
2 2
1 1
1 1
1 1
2 2
1 1
2 2
NaN NaN
NaN NaN
NaN NaN
2 2
NaN NaN
TypeError: Cannot convert a Symbol value to a number
*/

