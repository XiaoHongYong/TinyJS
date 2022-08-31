function f1() {
    var obj = { toString() { return '1.0'; }}
    function g() { }
    var index = 'ax';

    obj[index] = undefined;        console.log(obj[index]++, obj[index]);
    obj[index] = null;             console.log(obj[index]++, obj[index]);
    obj[index] = NaN;              console.log(obj[index]++, obj[index]);
    obj[index] = 0;                console.log(obj[index]++, obj[index]);
    obj[index] = 0.0;              console.log(obj[index]++, obj[index]);
    obj[index] = 1.0;              console.log(obj[index]++, obj[index]);
    obj[index] = -1;               console.log(obj[index]++, obj[index]);
    obj[index] = -1.0;             console.log(obj[index]++, obj[index]);
    obj[index] = true;             console.log(obj[index]++, obj[index]);
    obj[index] = false;            console.log(obj[index]++, obj[index]);
    obj[index] = '';               console.log(obj[index]++, obj[index]);
    obj[index] = '0';              console.log(obj[index]++, obj[index]);
    obj[index] = '1';              console.log(obj[index]++, obj[index]);
    obj[index] = '0.0';            console.log(obj[index]++, obj[index]);
    obj[index] = '1.0';            console.log(obj[index]++, obj[index]);
    obj[index] = 'true';           console.log(obj[index]++, obj[index]);
    obj[index] = 'false';          console.log(obj[index]++, obj[index]);
    obj[index] = g;                console.log(obj[index]++, obj[index]);
    obj[index] = obj;              console.log(obj[index]++, obj[index]);
    obj[index] = /a/;              console.log(obj[index]++, obj[index]);
    try {
        obj[index] = Symbol();         console.log(obj[index]++);
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

    obj[index] = undefined;        console.log(obj[index]--, obj[index]);
    obj[index] = null;             console.log(obj[index]--, obj[index]);
    obj[index] = NaN;              console.log(obj[index]--, obj[index]);
    obj[index] = 0;                console.log(obj[index]--, obj[index]);
    obj[index] = 0.0;              console.log(obj[index]--, obj[index]);
    obj[index] = 1.0;              console.log(obj[index]--, obj[index]);
    obj[index] = -1;               console.log(obj[index]--, obj[index]);
    obj[index] = -1.0;             console.log(obj[index]--, obj[index]);
    obj[index] = true;             console.log(obj[index]--, obj[index]);
    obj[index] = false;            console.log(obj[index]--, obj[index]);
    obj[index] = '';               console.log(obj[index]--, obj[index]);
    obj[index] = '0';              console.log(obj[index]--, obj[index]);
    obj[index] = '1';              console.log(obj[index]--, obj[index]);
    obj[index] = '0.0';            console.log(obj[index]--, obj[index]);
    obj[index] = '1.0';            console.log(obj[index]--, obj[index]);
    obj[index] = 'true';           console.log(obj[index]--, obj[index]);
    obj[index] = 'false';          console.log(obj[index]--, obj[index]);
    obj[index] = g;                console.log(obj[index]--, obj[index]);
    obj[index] = obj;              console.log(obj[index]--, obj[index]);
    obj[index] = /a/;              console.log(obj[index]--, obj[index]);
    try {
        obj[index] = Symbol();         console.log(obj[index]--);
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

    obj[index] = undefined;        console.log(--obj[index], obj[index]);
    obj[index] = null;             console.log(--obj[index], obj[index]);
    obj[index] = NaN;              console.log(--obj[index], obj[index]);
    obj[index] = 0;                console.log(--obj[index], obj[index]);
    obj[index] = 0.0;              console.log(--obj[index], obj[index]);
    obj[index] = 1.0;              console.log(--obj[index], obj[index]);
    obj[index] = -1;               console.log(--obj[index], obj[index]);
    obj[index] = -1.0;             console.log(--obj[index], obj[index]);
    obj[index] = true;             console.log(--obj[index], obj[index]);
    obj[index] = false;            console.log(--obj[index], obj[index]);
    obj[index] = '';               console.log(--obj[index], obj[index]);
    obj[index] = '0';              console.log(--obj[index], obj[index]);
    obj[index] = '1';              console.log(--obj[index], obj[index]);
    obj[index] = '0.0';            console.log(--obj[index], obj[index]);
    obj[index] = '1.0';            console.log(--obj[index], obj[index]);
    obj[index] = 'true';           console.log(--obj[index], obj[index]);
    obj[index] = 'false';          console.log(--obj[index], obj[index]);
    obj[index] = g;                console.log(--obj[index], obj[index]);
    obj[index] = obj;              console.log(--obj[index], obj[index]);
    obj[index] = /a/;              console.log(--obj[index], obj[index]);
    try {
        obj[index] = Symbol();         console.log(--obj[index]);
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

    obj[index] = undefined;        console.log(++obj[index], obj[index]);
    obj[index] = null;             console.log(++obj[index], obj[index]);
    obj[index] = NaN;              console.log(++obj[index], obj[index]);
    obj[index] = 0;                console.log(++obj[index], obj[index]);
    obj[index] = 0.0;              console.log(++obj[index], obj[index]);
    obj[index] = 1.0;              console.log(++obj[index], obj[index]);
    obj[index] = -1;               console.log(++obj[index], obj[index]);
    obj[index] = -1.0;             console.log(++obj[index], obj[index]);
    obj[index] = true;             console.log(++obj[index], obj[index]);
    obj[index] = false;            console.log(++obj[index], obj[index]);
    obj[index] = '';               console.log(++obj[index], obj[index]);
    obj[index] = '0';              console.log(++obj[index], obj[index]);
    obj[index] = '1';              console.log(++obj[index], obj[index]);
    obj[index] = '0.0';            console.log(++obj[index], obj[index]);
    obj[index] = '1.0';            console.log(++obj[index], obj[index]);
    obj[index] = 'true';           console.log(++obj[index], obj[index]);
    obj[index] = 'false';          console.log(++obj[index], obj[index]);
    obj[index] = g;                console.log(++obj[index], obj[index]);
    obj[index] = obj;              console.log(++obj[index], obj[index]);
    obj[index] = /a/;              console.log(++obj[index], obj[index]);
    try {
        obj[index] = Symbol();         console.log(++obj[index]);
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
