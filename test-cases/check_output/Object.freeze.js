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


// Index: 19
// getPrototypeOf
function f() {
    var proto = {a : 'a1'};
    var o = {x : 1, 1: 2, __proto__: proto};
    var p = Object.getPrototypeOf(o);
    console.log(p === proto);
}
f();
/* OUTPUT
true
*/


// Index: 20
// getPrototypeOf
var obj2 = { tt: 'x'}
function g() { }
g.f = 'f1';

function f(r, obj, prototype, noPrint) {
    console.log('Round: ', r, noPrint ? '' : obj)

    try {
        var d = Object.getPrototypeOf(obj);
        console.log(d === prototype);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f(1, undefined);
f(2, null);
f(3, 1, Number.prototype);
f(4, false, Boolean.prototype);
f(5, NaN, Number.prototype);
f(6, Symbol(), Symbol.prototype);
f(7, 'x', String.prototype);
f(8, 'Ab', String.prototype);
f(9, g, Function.prototype);
f(10, obj2, Object.prototype, 1);
f(12, ['xy'], Array.prototype, 1);
/* OUTPUT
Round:  1 undefined
TypeError: Cannot convert undefined or null to object
Round:  2 null
TypeError: Cannot convert undefined or null to object
Round:  3 1
true
Round:  4 false
true
Round:  5 NaN
true
Round:  6 Symbol()
true
Round:  7 x
true
Round:  8 Ab
true
Round:  9 function g() { }
true
Round:  10 
true
Round:  12 
true
*/


// Index: 21
// hasOwn
function f() {
    var proto = {a : 'a1'};
    var o = {x : 1, 1: 2, __proto__: proto};
    console.log(Object.hasOwn(o, 'x'), Object.hasOwn(o, 'a'), Object.hasOwn(o, null), Object.hasOwn(o, undefined));
}
f();
/* OUTPUT
true false false false
*/


// Index: 22
// hasOwn
function f() {
    var o = {1: 2, 'undefined': 'u'};
    console.log(Object.hasOwn(o, 'undefined'), Object.hasOwn(o, undefined), Object.hasOwn(o, null), Object.hasOwn(o, 1), Object.hasOwn(o, '1'));
    console.log(Object.hasOwn(o));
}
f();
/* OUTPUT
true true false true true
true
*/


// Index: 23
// hasOwn
var obj2 = { tt: 'x'}
function g() { }
g.f = 'f1';

function f(r, obj, prop, noPrint) {
    console.log('Round: ', r, noPrint ? '' : obj)

    try {
        console.log(Object.hasOwn(obj, prop));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f(1, undefined, 'toString');
f(2, null, 'toString');
f(3, 1, 'toString');
f(4, false, 'toString');
f(5, NaN, 'toString');
f(6, Symbol(), 'toString');
f(7, 'x', 'toString');
f(71, 'x', '0');
f(72, 'x', 0);
f(73, 'x', 'length');
f(74, 'AB', '0');
f(75, 'AB', 0);
f(76, 'AB', 'length');
f(9, g, 'f', 1);
f(9, g, 'toString', 1);
f(10, obj2, 'tt', 1);
f(11, ['xy'], 'length', 1);
f(12, ['xy'], '0', 1);
f(13, ['xy'], 0, 1);
/* OUTPUT
Round:  1 undefined
TypeError: Cannot convert undefined or null to object
Round:  2 null
TypeError: Cannot convert undefined or null to object
Round:  3 1
false
Round:  4 false
false
Round:  5 NaN
false
Round:  6 Symbol()
false
Round:  7 x
false
Round:  71 x
true
Round:  72 x
true
Round:  73 x
true
Round:  74 AB
true
Round:  75 AB
true
Round:  76 AB
true
Round:  9 
true
Round:  9 
false
Round:  10 
true
Round:  11 
true
Round:  12 
true
Round:  13 
true
*/

