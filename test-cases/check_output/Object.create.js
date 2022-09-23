// Index: 0
// create 带 proto
function f() {
    var proto = {
        a: 1,
        b() {
            console.log('called proto b()');
        }
    }
    var o = Object.create(proto);
    console.log(o.a);
    o.b();
    proto.c = 2;
    console.log(o.c);

    o.d = 3;
    console.log(proto.d);
}
f();
/* OUTPUT
1
called proto b()
2
undefined
*/


// Index: 1
// create(null)
function f() {
    var o = Object.create({});
    console.log(typeof o.toString);

    o = Object.create(null);
    console.log(o.toString);

    try {
        o = Object.create();
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        o = Object.create(undefined);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        o = Object.create('x');
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
function
undefined
TypeError: Object prototype may only be an Object or null: undefined
TypeError: Object prototype may only be an Object or null: undefined
TypeError: Object prototype may only be an Object or null: x
*/


// Index: 2
// defineProperties
function f() {
    var o = {};
    Object.defineProperties(o, {
        'x': {
            value: 'x1',
        },
        'y': {
            get() { console.log('y1'); return 'y1'; }
        }
    });
    console.log(o.x, o.y);
}
f();
/* OUTPUT
y1
x1 y1
*/


// Index: 3
// create()
function f() {
    var o = Object.create({a: 1}, {
        'x': {
            value: 'x1',
        },
        'y': {
            get() { console.log('y1'); return 'y1'; }
        }
    });
    console.log(o.a, o.x, o.y);
}
f();
/* OUTPUT
y1
1 x1 y1
*/


// Index: 4
// assign
function f() {
    var org = {a : 2};
    var o = Object.assign(org, {
        'x': 'x1',
        'y'() { console.log('y1'); return 'y1'; }
    });
    console.log(o.a, o.x, o.y());
    console.log(org.a, org.x, org.y());
    console.log(o === org);
}
f();
/* OUTPUT
y1
2 x1 y1
y1
2 x1 y1
true
*/


// Index: 5
// assign
function f() {
    var org = {a : 2};
    var o = Object.assign(org);
    console.log(o.a);
}
f();
/* OUTPUT
2
*/


// Index: 6
// assign
function f() {
    const o1 = { a: 1 };
    const o2 = { b: 2 };
    const o3 = { c: 3 };
    
    const o = Object.assign(o1, o2, o3);
    console.log(o.a, o.b, o.c);
}
f();
/* OUTPUT
1 2 3
*/


// Index: 7
// assign
function f() {
    const o1 = { a: 1, b: 1, c: 1 };
    const o2 = { b: 2, c: 2 };
    const o3 = { c: 3 };
    
    const o = Object.assign({}, o1, o2, o3);
    console.log(o.a, o.b, o.c);
}
f();
/* OUTPUT
1 2 3
*/


// Index: 8
// assign
function f() {
    const obj = Object.create({ foo: 1 }, { // foo is on obj's prototype chain.
        bar: {
            value: 2  // bar is a non-enumerable property.
        },
        baz: {
            value: 3,
            enumerable: true  // baz is an own enumerable property.
        }
    });
    
    const copy = Object.assign({}, obj);
    console.log(copy); // { baz: 3 }
}
f();
/* OUTPUT
{ baz: 3 }
*/


// Index: 9
// assign
function f() {
    const o = Object.assign(1);
    console.log(typeof o, o instanceof Number);
}
f();
/* OUTPUT
object true
*/


// Index: 10
// assign
function f() {
    const o = Object.assign(true);
    console.log(typeof o, o instanceof Boolean);
}
f();
/* OUTPUT
object true
*/


// Index: 11
// assign
function f() {
    const o = Object.assign('x');
    console.log(typeof o, o instanceof String);
}
f();
/* OUTPUT
object true
*/


// Index: 12
// assign
function f() {
    const o = Object.assign('xyz');
    console.log(typeof o, o instanceof String);
}
f();
/* OUTPUT
object true
*/


// Index: 13
// assign
function f() {
    const o = Object.assign(null);
    console.log(typeof o, o instanceof Number);
}
f();
/* OUTPUT
Uncaught TypeError: Cannot convert undefined or null to object
*/


// Index: 14
// assign
function f() {
    const o = Object.assign(undefined);
    console.log(typeof o, o instanceof Number);
}
f();
/* OUTPUT
Uncaught TypeError: Cannot convert undefined or null to object
*/


// Index: 15
// entries
function f() {
    var proto = {x: 1};
    var o = { a: 2, __proto__: proto};
    for (var i in o) {
        console.log(i, o[i]);
    }
    console.log('==');
    var a = Object.entries(o);
    console.log(a.length);
    console.log(a[0]);
}
f();
/* OUTPUT
a 2
x 1
==
1
a, 2
*/


// Index: 16
// entries
var obj2 = { tt: 'x'}
function g() { }
g.f = 'f1';

function f(r, obj, noPrint) {
    console.log('Round: ', r, noPrint ? '' : obj)
    try {
        var a = Object.entries(obj);
        console.log(a.length);
        for (var item of a)
            console.log(item);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f(1, undefined);
f(2, null);
f(3, 1);
f(4, false);
f(5, NaN);
f(6, Symbol());
f(7, 'x');
f(8, 'Ab');
f(9, g);
f(10, obj2, 1);
/* OUTPUT
Round:  1 undefined
TypeError: Cannot convert undefined or null to object
Round:  2 null
TypeError: Cannot convert undefined or null to object
Round:  3 1
0
Round:  4 false
0
Round:  5 NaN
0
Round:  6 Symbol()
0
Round:  7 x
1
0, x
Round:  8 Ab
2
0, A
1, b
Round:  9 function g() { }
1
f, f1
Round:  10 
1
tt, x
*/


// Index: 17
// entries
function f() {
    var o = { a: 2};
    Object.defineProperty(o, 'x', {
        value: 'x1',
    })
    var a = Object.entries(o);
    console.log(a.length);
    console.log(a[0]);
}
f();
/* OUTPUT
1
a, 2
*/

