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

    a = undefined;        console.log(a--, a);
    a = null;             console.log(a--, a);
    a = NaN;              console.log(a--, a);
    a = 0;                console.log(a--, a);
    a = 0.0;              console.log(a--, a);
    a = 1.0;              console.log(a--, a);
    a = -1;               console.log(a--, a);
    a = -1.0;             console.log(a--, a);
    a = true;             console.log(a--, a);
    a = false;            console.log(a--, a);
    a = '';               console.log(a--, a);
    a = '0';              console.log(a--, a);
    a = '1';              console.log(a--, a);
    a = '0.0';            console.log(a--, a);
    a = '1.0';            console.log(a--, a);
    a = 'true';           console.log(a--, a);
    a = 'false';          console.log(a--, a);
    a = g;                console.log(a--, a);
    a = obj;              console.log(a--, a);
    a = /a/;              console.log(a--, a);
    try {
        a = Symbol();         console.log(a--);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f2();
/* OUTPUT
NaN NaN
0 -1
NaN NaN
0 -1
0 -1
1 0
-1 -2
-1 -2
1 0
0 -1
0 -1
0 -1
1 0
0 -1
1 0
NaN NaN
NaN NaN
NaN NaN
1 0
NaN NaN
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

