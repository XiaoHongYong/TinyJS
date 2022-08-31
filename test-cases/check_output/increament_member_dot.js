function f1() {
    var obj = { toString() { return '1.0'; }}
    function g() { }

    obj.a = undefined;        console.log(obj.a++, obj.a);
    obj.a = null;             console.log(obj.a++, obj.a);
    obj.a = NaN;              console.log(obj.a++, obj.a);
    obj.a = 0;                console.log(obj.a++, obj.a);
    obj.a = 0.0;              console.log(obj.a++, obj.a);
    obj.a = 1.0;              console.log(obj.a++, obj.a);
    obj.a = -1;               console.log(obj.a++, obj.a);
    obj.a = -1.0;             console.log(obj.a++, obj.a);
    obj.a = true;             console.log(obj.a++, obj.a);
    obj.a = false;            console.log(obj.a++, obj.a);
    obj.a = '';               console.log(obj.a++, obj.a);
    obj.a = '0';              console.log(obj.a++, obj.a);
    obj.a = '1';              console.log(obj.a++, obj.a);
    obj.a = '0.0';            console.log(obj.a++, obj.a);
    obj.a = '1.0';            console.log(obj.a++, obj.a);
    obj.a = 'true';           console.log(obj.a++, obj.a);
    obj.a = 'false';          console.log(obj.a++, obj.a);
    obj.a = g;                console.log(obj.a++, obj.a);
    obj.a = obj;              console.log(obj.a++, obj.a);
    obj.a = /a/;              console.log(obj.a++, obj.a);
    try {
        obj.a = Symbol();         console.log(obj.a++);
    } catch (e) {
        console.log(e);
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


function f2() {
    var obj = { toString() { return '1.0'; }}
    function g() { }

    obj.a = undefined;        console.log(obj.a--, obj.a);
    obj.a = null;             console.log(obj.a--, obj.a);
    obj.a = NaN;              console.log(obj.a--, obj.a);
    obj.a = 0;                console.log(obj.a--, obj.a);
    obj.a = 0.0;              console.log(obj.a--, obj.a);
    obj.a = 1.0;              console.log(obj.a--, obj.a);
    obj.a = -1;               console.log(obj.a--, obj.a);
    obj.a = -1.0;             console.log(obj.a--, obj.a);
    obj.a = true;             console.log(obj.a--, obj.a);
    obj.a = false;            console.log(obj.a--, obj.a);
    obj.a = '';               console.log(obj.a--, obj.a);
    obj.a = '0';              console.log(obj.a--, obj.a);
    obj.a = '1';              console.log(obj.a--, obj.a);
    obj.a = '0.0';            console.log(obj.a--, obj.a);
    obj.a = '1.0';            console.log(obj.a--, obj.a);
    obj.a = 'true';           console.log(obj.a--, obj.a);
    obj.a = 'false';          console.log(obj.a--, obj.a);
    obj.a = g;                console.log(obj.a--, obj.a);
    obj.a = obj;              console.log(obj.a--, obj.a);
    obj.a = /a/;              console.log(obj.a--, obj.a);
    try {
        obj.a = Symbol();         console.log(obj.a--);
    } catch (e) {
        console.log(e);
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


function f3() {
    var obj = { toString() { return '1.0'; }}
    function g() { }

    obj.a = undefined;        console.log(--obj.a, obj.a);
    obj.a = null;             console.log(--obj.a, obj.a);
    obj.a = NaN;              console.log(--obj.a, obj.a);
    obj.a = 0;                console.log(--obj.a, obj.a);
    obj.a = 0.0;              console.log(--obj.a, obj.a);
    obj.a = 1.0;              console.log(--obj.a, obj.a);
    obj.a = -1;               console.log(--obj.a, obj.a);
    obj.a = -1.0;             console.log(--obj.a, obj.a);
    obj.a = true;             console.log(--obj.a, obj.a);
    obj.a = false;            console.log(--obj.a, obj.a);
    obj.a = '';               console.log(--obj.a, obj.a);
    obj.a = '0';              console.log(--obj.a, obj.a);
    obj.a = '1';              console.log(--obj.a, obj.a);
    obj.a = '0.0';            console.log(--obj.a, obj.a);
    obj.a = '1.0';            console.log(--obj.a, obj.a);
    obj.a = 'true';           console.log(--obj.a, obj.a);
    obj.a = 'false';          console.log(--obj.a, obj.a);
    obj.a = g;                console.log(--obj.a, obj.a);
    obj.a = obj;              console.log(--obj.a, obj.a);
    obj.a = /a/;              console.log(--obj.a, obj.a);
    try {
        obj.a = Symbol();         console.log(--obj.a);
    } catch (e) {
        console.log(e);
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


function f4() {
    var obj = { toString() { return '1.0'; }}
    function g() { }

    obj.a = undefined;        console.log(++obj.a, obj.a);
    obj.a = null;             console.log(++obj.a, obj.a);
    obj.a = NaN;              console.log(++obj.a, obj.a);
    obj.a = 0;                console.log(++obj.a, obj.a);
    obj.a = 0.0;              console.log(++obj.a, obj.a);
    obj.a = 1.0;              console.log(++obj.a, obj.a);
    obj.a = -1;               console.log(++obj.a, obj.a);
    obj.a = -1.0;             console.log(++obj.a, obj.a);
    obj.a = true;             console.log(++obj.a, obj.a);
    obj.a = false;            console.log(++obj.a, obj.a);
    obj.a = '';               console.log(++obj.a, obj.a);
    obj.a = '0';              console.log(++obj.a, obj.a);
    obj.a = '1';              console.log(++obj.a, obj.a);
    obj.a = '0.0';            console.log(++obj.a, obj.a);
    obj.a = '1.0';            console.log(++obj.a, obj.a);
    obj.a = 'true';           console.log(++obj.a, obj.a);
    obj.a = 'false';          console.log(++obj.a, obj.a);
    obj.a = g;                console.log(++obj.a, obj.a);
    obj.a = obj;              console.log(++obj.a, obj.a);
    obj.a = /a/;              console.log(++obj.a, obj.a);
    try {
        obj.a = Symbol();         console.log(++obj.a);
    } catch (e) {
        console.log(e);
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
