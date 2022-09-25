// Index: 0
// freeze
function f() {
    var o = {a : 1};
    Object.freeze(o);

    o.a = 3;
    console.log(o.a, o.b++, ++o.c);

    try {
        Object.defineProperty(o, 'a', {writable: true});
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
1 NaN NaN
TypeError: Cannot redefine property: a
*/


// Index: 1
// freeze
var obj2 = { tt: 'x'}
function g() { }
g.f = 'f1';

function f(r, obj, noPrint) {
    console.log('Round: ', r, noPrint ? '' : obj)
    try {
        Object.freeze(obj);
        obj.length = 3;
        console.log(obj, 'length');
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
f(11, globalThis, 1);
/* OUTPUT
Round:  1 undefined
TypeError: Cannot set properties of undefined (setting 'length')
Round:  2 null
TypeError: Cannot set properties of null (setting 'length')
Round:  3 1
1 length
Round:  4 false
false length
Round:  5 NaN
NaN length
Round:  6 Symbol()
Symbol() length
Round:  7 x
x length
Round:  8 Ab
Ab length
Round:  9 function g() { }
function g() { } length
Round:  10 
{ tt: x } length
Round:  11 
TypeError: Cannot freeze
*/


// Index: 2
// freeze: prototype has setter
function f() {
    var proto = {
        x: 1,
        set y(p) { console.log('set y'); }
    }
    var o = {a : 1, __proto__: proto};
    console.log(o.x);
    Object.freeze(o);
    o.x = 2;
    console.log(o.x, o.b++);
    o.y = 3;

    o.a = 3;
    console.log(o.a);
}
f();
/* OUTPUT
1
1 NaN
set y
1
*/


// Index: 3
// freeze: array
function f() {
    var proto = {
        x: 1,
        set y(p) { console.log('set y'); }
    }
    var o = [2, 4];
    o.__proto__ = proto;
    console.log(o.x);
    Object.freeze(o);
    o.x = 2;
    console.log(o.x, o.a++);
    o.y = 3;

    o[0] = 'p0';
    o[10] = 'p10';
    console.log(o[0], o[10]);
}
f();
/* OUTPUT
1
1 NaN
set y
2 undefined
*/


// Index: 4
// freeze: Array.prototype
function f() {
    var o = Array.prototype;
    o.x = 1;
    Object.freeze(o);
    o.x = 2;
    o.y = 3;
    o.toString = 'xx';

    console.log(typeof o.toString, o.y, o.x, o.a++, o.toString++, typeof o.toString);
}
f();
/* OUTPUT
function undefined 1 NaN NaN function
*/


// Index: 5
// freeze: arguments
function f() {
    var o = arguments;
    Object.freeze(o);
    o.x = 2;
    o[0] = 'p0';
    o[10] = 'p10';

    console.log(o.x, o[0], o[10], o.a++, o[1]++, o[1]);
}
f(2, 4);
/* OUTPUT
undefined 2 undefined NaN 4 4
*/


// Index: 6
// freeze: function
function f() {
    var o = function() {};
    Object.freeze(o);
    o.x = 2;
    o[0] = 'p0';

    console.log(o.x, o[0], o.y++, o.y);
}
f();
/* OUTPUT
undefined undefined NaN undefined
*/


// Index: 7
// preventExtensions
function f() {
    var proto = {
        x: 1,
        set y(p) { console.log('set y'); }
    }
    var o = {a : 1, __proto__: proto};

    console.log(o.a, o.x);
    Object.preventExtensions(o);
    o.a = 'b';
    o.x = 2;
    console.log(o.a, o.x);
    o.y = 3;
}
f();
/* OUTPUT
1 1
b 1
set y
*/


// Index: 8
// fromEntries
function f() {
    var entries = [['a', 'b'], ['1', 'd']];
    var o = Object.fromEntries(entries);
    console.log(o);
}
f();
/* OUTPUT
{ 1: d, a: b }
*/


// Index: 9
// fromEntries: exceptions
var obj2 = { tt: 'x'}
function g() { }
g.f = 'f1';

function f(r, entries, noPrint) {
    console.log('Round: ', r, noPrint ? '' : entries)

    try {
        var o = Object.fromEntries(entries);
        console.log(o);
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
f(11, globalThis, 1);
f(12, ['xy'], 1);
/* OUTPUT-FIXED
Round:  1 undefined
TypeError: undefined is not iterable
Round:  2 null
TypeError: undefined is not iterable
Round:  3 1
TypeError: number 1 is not iterable (cannot read property Symbol(Symbol.iterator))
Round:  4 false
TypeError: boolean false is not iterable (cannot read property Symbol(Symbol.iterator))
Round:  5 NaN
TypeError: number NaN is not iterable (cannot read property Symbol(Symbol.iterator))
Round:  6 Symbol()
TypeError: symbol Symbol() is not iterable (cannot read property Symbol(Symbol.iterator))
Round:  7 x
TypeError: Iterator value x is not an entry object
Round:  8 Ab
TypeError: Iterator value A is not an entry object
Round:  9 function g() { }
TypeError: function is not iterable (cannot read property Symbol(Symbol.iterator))
Round:  10 
TypeError: object is not iterable (cannot read property Symbol(Symbol.iterator))
Round:  11 
TypeError: object is not iterable (cannot read property Symbol(Symbol.iterator))
Round:  12 
TypeError: Iterator value xy is not an entry object
*/


// Index: 10
// fromEntries
function f() {
    var entries = [['a', 'b'], ['1', 'd']];
    var o = Object.fromEntries(entries);
    console.log(o);
}
f();
/* OUTPUT
{ 1: d, a: b }
*/


// Index: 11
// fromEntries
function f() {
    var entries = [[], ['1']];
    var o = Object.fromEntries(entries);
    console.log(o);
}
f();
/* OUTPUT
{ 1: undefined, undefined: undefined }
*/


// Index: 12
// getOwnPropertyDescriptor
function f() {
    var o = {x : 1};
    var d = Object.getOwnPropertyDescriptor(o, 'x');
    console.log(d);

    o = {set x(p) {}};
    var d = Object.getOwnPropertyDescriptor(o, 'x');
    console.log(d);

    var d = Object.getOwnPropertyDescriptor(o, 'y');
    console.log(d);

    Object.defineProperty(o, 'a', {
        value: '1',
        writable: false,
        enumerable: false,
    });

    var d = Object.getOwnPropertyDescriptor(o, 'a');
    console.log(d);
}
f();
/* OUTPUT-FIXED
{ value: 1, writable: true, enumerable: true, configurable: true }
{ set: x(p) {}, get: undefined, enumerable: true, configurable: true}
undefined
{ value: 1, writable: false, enumerable: false, configurable: false }
*/


// Index: 13
// getOwnPropertyDescriptors
function f() {
    var o = {x : 1};
    var d = Object.getOwnPropertyDescriptors(o);
    console.log(d);
}
f();
/* OUTPUT-FIXED
{ x: [object Object] }
*/


// Index: 14
// getOwnPropertyDescriptors
function f() {
    var proto = {a : 'a1'};
    var o = {x : 1, __proto__: proto};
    var d = Object.getOwnPropertyDescriptors(o);
    console.log(d);
}
f();
/* OUTPUT-FIXED
{ x: [object Object] }
*/


// Index: 15
// getOwnPropertyDescriptors
var obj2 = { tt: 'x'}
function g() { }
g.f = 'f1';

function f(r, obj, noPrint) {
    console.log('Round: ', r, noPrint ? '' : obj)

    try {
        var d = Object.getOwnPropertyDescriptors(obj);
        console.log(d);
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
/* OUTPUT
Round:  1 undefined
TypeError: Cannot convert undefined or null to object
Round:  2 null
TypeError: Cannot convert undefined or null to object
Round:  3 1
{  }
Round:  4 false
{  }
Round:  5 NaN
{  }
Round:  6 Symbol()
{  }
*/


// Index: 16
// getOwnPropertyNames
function f() {
    var o = {x : 1, 1: 2};
    var d = Object.getOwnPropertyNames(o);
    console.log(d);
}
f();
/* OUTPUT
1, x
*/


// Index: 17
// getOwnPropertyNames
function f() {
    var proto = {a : 'a1'};
    var o = {x : 1, 1: 2, __proto__: proto};
    var d = Object.getOwnPropertyNames(o);
    console.log(d);
}
f();
/* OUTPUT
1, x
*/


// Index: 18
// getOwnPropertyNames
var obj2 = { tt: 'x'}
function g() { }
g.f = 'f1';

function f(r, obj, noPrint) {
    console.log('Round: ', r, noPrint ? '' : obj)

    try {
        var d = Object.getOwnPropertyNames(obj);
        console.log(d);
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
/* OUTPUT
Round:  1 undefined
TypeError: Cannot convert undefined or null to object
Round:  2 null
TypeError: Cannot convert undefined or null to object
Round:  3 1

Round:  4 false

Round:  5 NaN

Round:  6 Symbol()
*/

