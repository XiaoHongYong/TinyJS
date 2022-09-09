// Index: 0
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


// Index: 4
// 仅有 setter：NaN
function f5() {
    var obj = {  set x(a) { this._x += a;}, _x : 0};
    console.log(obj.x);
    console.log(obj.x++);
    console.log(obj.x);
    console.log(obj.x += 10);
    console.log(obj.x);
}
f5();
/* OUTPUT
undefined
NaN
undefined
NaN
undefined
*/


// Index: 5
// 仅有 getter：不能修改
function f51() {
    var obj = {  get x() { return 1; }};
    console.log(obj.x);
    console.log(++obj.x);
    console.log(obj.x);
    console.log(obj.x += 10);
    console.log(obj.x);
}
f51();
/* OUTPUT
1
2
1
11
1
*/


// Index: 6
// 有 setter/getter 的情况：调用 setter
function f6() {
    var obj = { get x() { return this._x; }, set x(a) { this._x += a;}, _x : 0};
    console.log(obj.x);
    console.log(obj.x++);
    console.log(obj.x);
    console.log(obj.x += 10);
    console.log(obj.x);
}
f6();
/* OUTPUT
0
0
1
11
12
*/


// Index: 7
// writable false：不能修改，但是返回值增加了
function f7() {
    var obj = { x : 1};
    Object.defineProperty(obj, 'x', { writable: false});

    console.log(obj.x);
    console.log(++obj.x, obj.x);
}
f7();
/* OUTPUT
1
2 1
*/


// Index: 8
// __proto__ 有属性的情况：不修改 __proto__，修改 object
function f8() {
    var proto = {x: 1};

    var obj = { __proto__ : proto};

    console.log(obj.x);
    console.log(++obj.x, obj.x, proto.x);
}
f8();
/* OUTPUT
1
2 2 1
*/


// Index: 9
// __proto__ writable: false: 不能修改 obj
function f9() {
    var proto = {};
    Object.defineProperty(proto, 'x', { writable: false, value: 1});

    var obj = { __proto__ : proto};

    console.log(obj.x);
    console.log(++obj.x, obj.x, proto.x);
}
f9();
/* OUTPUT
1
2 1 1
*/


// Index: 10
// __proto__ 是 getter 的情况: 不修改 obj
function f10() {
    var proto = { _x: 1, get x() { return this._x; } };

    var obj = { __proto__ : proto};

    console.log(obj.x, proto.x);
    console.log(++obj.x, obj.x, proto.x, proto._x);
}
f10();
/* OUTPUT
1 1
2 1 1 1
*/


// Index: 11
// __proto__ 是 setter/getter 的情况: 调用 __proto__ setter，不修改 obj
function f11() {
    var proto = { _x: 1, set x(a) { this._x = a; console.log('set x:', a); }, get x() { return this._x; } };

    var obj = { __proto__ : proto};

    console.log(obj.x, proto.x);
    console.log(++obj.x, obj.x, proto.x, proto._x);
}
f11();
/* OUTPUT
1 1
set x: 2
2 2 1 1
*/

