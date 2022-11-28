// Index: 0
console.log(new Number(1));
console.log([new Number(1), 4, 5]);
/* OUTPUT
1
[Number, 4, 5]
*/


// Index: 1
function f() {
    var a = [];
    Object.defineProperty(a, 'length', {
        set: function(len) { this._len = len; },
        get: function() { return this._len; }
    })
}
f();
/* OUTPUT
Uncaught TypeError: Cannot redefine property: length
*/


// Index: 2
// at
function f() {
    var a = [1, 2];
    console.log(a.at(1), a.at(-1), a.at());
    a.__proto__ = { x : 'x1'};
    console.log(a.x);
    console.log(a[0]);
    console.log(a.at(1));
}
f();
/* OUTPUT-FIXED
2 2 1
x1
1
Uncaught TypeError: undefined is not a function
*/


// Index: 3
// at
function f(r, o) {
    console.log(r, Array.prototype.at.call(o, 0));
}
f(1, ['x']);
f(2, 1);
f(3, 'yz');
f(4, {0: 'at 0', length: "3" });
f('4-1', {0: 'at 0', length: "Infinity" });
f('4-1.1', {0: 'at 0', length: "-Infinity" });
f('4-2', {0: 'at 0', length: "NaN" });
f('4-3', {0: 'at 0', length: "abc" });
f('4-4', {0: 'at 0', length: "0xff" });
f(5, /a/);
f(6, Symbol());
f(7, true);
try {
    f(8, null);
} catch (e) {
    console.log(e.name + ': ' + e.message);
}
/* OUTPUT
1 x
2 undefined
3 y
4 at 0
4-1 at 0
4-1.1 undefined
4-2 undefined
4-3 undefined
4-4 at 0
5 undefined
6 undefined
7 undefined
TypeError: Cannot convert undefined or null to object
*/


// Index: 4
// concat
function f(r, p) {
    try {
        console.log(r, Array.prototype.concat.call(p, 2, 3));
    } catch (e) {
        console.log(r, e.name + ': ' + e.message);
    }
}
f(1, 11);
f(2, true);
f(3, 'abc');
f(4, {x: 1});
f(5, Symbol());
f(6, null);
f(7, undefined);
f(8, /a/);
f(9, ['x'])
/* OUTPUT
1 [Number, 2, 3]
2 [Boolean, 2, 3]
3 [String, 2, 3]
4 [Object, 2, 3]
5 [Symbol, 2, 3]
6 TypeError: Array.prototype.concat called on null or undefined
7 TypeError: Array.prototype.concat called on null or undefined
8 [/a/, 2, 3]
9 [x, 2, 3]
*/


// Index: 5
// concat
function f() {
    var a = [1, 2];
    a.x = 'x';
    var b = a.concat(4, 5);
    console.log(b.length, b.x, a.length);

    console.log(a.concat([4, , 5]));
}
f();
/* OUTPUT
4 undefined 2
[1, 2, 4, undefined, 5]
*/


// Index: 6
// concat
function f() {
    var a = {length: 1, 0: 'a', x: 'x1'};
    var b = Array.prototype.concat.call(a, 4, 5);
    console.log(b.length, b[0] instanceof Array);
}
f();
/* OUTPUT
3 false
*/


// Index: 7
// concat
function f() {
    var a = [];
    Object.defineProperty(a, '0', { get() { console.log('>get 0'); return 'x'}});
    var b = [1, 2].concat(a);
    console.log('=>')
    console.log(b, b[2]);
}
f();
/* OUTPUT
>get 0
=>
[1, 2, x] x
*/


// Index: 8
// copyWithin
function f() {
    var a = [1, 2, 3, 4, 5];
    var b = a.copyWithin(1, 3, 5);
    console.log(a);
    console.log(b);

    try {
        console.log(Array.prototype.copyWithin.call('abc', 3, 1, 2));
        console.log(Array.prototype.copyWithin.call('abc', 2, 3, 4));
        console.log(Array.prototype.copyWithin.call('abc', 1, 2, 2));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        Array.prototype.copyWithin.call('abc', 1, 2, 3);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var o = { length: 10, 1: 'x', 3: 'z'};
    console.log(Array.prototype.copyWithin.call(o, 2, 3, 4));
}
f();
/* OUTPUT
[1, 4, 5, 4, 5]
[1, 4, 5, 4, 5]
abc
abc
abc
TypeError: Cannot assign to read only property '1' of object '[object String]'
{1: x, 2: z, 3: z, length: 10}
*/


// Index: 9
// entries
function f() {
    var i;
    var o = { length: 4, 3: 'x'};
    var it = Array.prototype.entries.call(o);
    for (i of it) console.log(i);
    console.log('#1');
    for (i of it) console.log(i);

    console.log('#2');
    var o = { 3: 'x'};
    var it = Array.prototype.entries.call(o);
    for (i of it) console.log(i);

    console.log('#3');
    o = 'xbc';
    var it = Array.prototype.entries.call(o);
    for (i of it) console.log(i);

    console.log('#4');
    o = [4, 5, 6];
    for (i of o.entries()) console.log(i);
}
f();
/* OUTPUT
[0, undefined]
[1, undefined]
[2, undefined]
[3, x]
#1
#2
#3
[0, x]
[1, b]
[2, c]
#4
[0, 4]
[1, 5]
[2, 6]
*/


// Index: 10
// copyWithin
function f() {
    console.log(1, [1, 2, 3, 4, 5].copyWithin(-2));
    // [1, 2, 3, 1, 2]
    
    console.log(2, [1, 2, 3, 4, 5].copyWithin(0, 3));
    // [4, 5, 3, 4, 5]
    
    console.log(3, [1, 2, 3, 4, 5].copyWithin(0, 3, 4));
    // [4, 2, 3, 4, 5]
    
    console.log(4, [1, 2, 3, 4, 5].copyWithin(-2, -3, -1));
    // [1, 2, 3, 3, 4]

    console.log(4.1, [1, 2, 3, 4, 5].copyWithin(1, 0, 3));
    console.log(4.2, [1, 2, 3, 4, 5].copyWithin(undefined, 2, 3));

    var a = [1, , 5];
    console.log(5, a.copyWithin(1, 2));
    console.log(6, a);

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};

    console.log(7, Array.prototype.copyWithin.call(obj, 1, 3, 5));

    try {
        console.log(8, Array.prototype.copyWithin.call('abc', 1, 2, 3));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
1 [1, 2, 3, 1, 2]
2 [4, 5, 3, 4, 5]
3 [4, 2, 3, 4, 5]
4 [1, 2, 3, 3, 4]
4.1 [1, 1, 2, 3, 5]
4.2 [3, 2, 3, 4, 5]
5 [1, 5, 5]
6 [1, 5, 5]
7 {1: 3x, 3: 3x, length: 5}
TypeError: Cannot assign to read only property '1' of object '[object String]'
*/


// Index: 11
// every
function f() {
    var a = [1, , 5];
    var obj = {1: '1x', 3: '3x', length: 5};
    var s = 'xbc';
    var c = 'Y';

    console.log(a.every(function(x, i) { console.log(x, i); return x >= 1; }));

    a.push('x');
    console.log(a.every(function(x) { return x >= 1; }));

    console.log(Array.prototype.every.call(s, function(x, i) { console.log(x, i); return x >= 'b'; }));
    console.log(Array.prototype.every.call(c, function(x, i) { console.log(x, i); return x >= 'c'; }));

    console.log(Array.prototype.every.call(obj, function(x, i) { console.log(x, i); return x >= 1; }));
}
f();
/* OUTPUT
1 0
5 2
true
false
x 0
b 1
c 2
true
Y 0
false
1x 1
false
*/


// Index: 12
// fill
function f() {
    console.log(1, [1, 2, 3, 4, 5].fill(-2));
    
    console.log(2, [1, 2, 3, 4, 5].fill(-1, 3));
    
    console.log(3, [1, 2, 3, 4, 5].fill(6, 2, 4));
    
    console.log(4, [1, 2, 3, 4, 5].fill(7, -4, -2));

    console.log(4.1, [1, 2, 3, 4, 5].fill(8, 4, 3));
    console.log(4.2, [1, 2, 3, 4, 5].fill(9, undefined, 3));

    var a = [1, , 5];
    console.log(5, a.fill(0, 1, 2));
    console.log(6, a);

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};

    console.log(7, Array.prototype.fill.call(obj, 2, 3));

    try {
        console.log(8, Array.prototype.fill.call('abc', 1, 2, 3));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log(8, Array.prototype.fill.call('abc', 1, 20, 30));

    try {
        a = [];
        Object.defineProperty(a, '0', {
            value: 'x',
            writable: false
        });
        console.log(8, Array.prototype.fill.call(a, 1, 0, 3));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        a = [];
        Object.defineProperty(a, '0', {
            get: function() {}
        });
        console.log(8, Array.prototype.fill.call(a, 1, 0, 3));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        a = [1, 2];
        Object.freeze(a);
        console.log(8, Array.prototype.fill.call(a, 1, 0, 3));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
1 [-2, -2, -2, -2, -2]
2 [1, 2, 3, -1, -1]
3 [1, 2, 6, 6, 5]
4 [1, 7, 7, 4, 5]
4.1 [1, 2, 3, 4, 5]
4.2 [9, 9, 9, 4, 5]
5 [1, 0, 5]
6 [1, 0, 5]
7 {1: 1x, 2: 2x, 3: 2, 4: 2, length: 5}
TypeError: Cannot assign to read only property '2' of object '[object String]'
8 abc
TypeError: Cannot assign to read only property '0' of object '[object Array]'
TypeError: Cannot set property 0 of [object Array] which has only a getter
TypeError: Cannot assign to read only property '0' of object '[object Array]'
*/


// Index: 13
// filter
function f() {
    console.log(1, [1, 2, 3, 4, 5].filter(i => i > 3));
    try {
        console.log(2, [1, null, , undefined, 5].filter('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var a = [1, , 5];
    console.log(4, a.filter(function (e, i, a) { console.log('4#', e, i, a); }));
    console.log(5, a.filter(i => i > 3));
    console.log(6, a);

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};

    console.log(7, Array.prototype.filter.call(obj, function (e, i, a) { console.log('7#', e, i, a); }));

    console.log(8, Array.prototype.filter.call('abc', function (e, i, a) { console.log('8#', e, i, a); return e > 'a'; }));
    try {
        console.log(9, Array.prototype.filter.call('abc', 1, 20, 30));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
1 [4, 5]
TypeError: abc is not a function
4# 1 0 [1, undefined, 5]
4# 5 2 [1, undefined, 5]
4 []
5 [5]
6 [1, undefined, 5]
7# 1x 1 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 2x 2 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 3x 3 {1: 1x, 2: 2x, 3: 3x, length: 5}
7 []
8# a 0 abc
8# b 1 abc
8# c 2 abc
8 [b, c]
TypeError: 1 is not a function
*/


// Index: 14
// find
function f() {
    console.log(1, [1, 2, 3, 4, 5].find(i => i > 3));
    try {
        console.log(2, [1, null, , undefined, 5].find('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var a = [1, , 5];
    console.log(4, a.find(function (e, i, a) { console.log('4#', e, i, a); }));
    console.log(5, a.find(i => i > 3));
    console.log(6, a);

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};

    console.log(7, Array.prototype.find.call(obj, function (e, i, a) { console.log('7#', e, i, a); }));

    console.log(8, Array.prototype.find.call('abc', function (e, i, a) { console.log('8#', e, i, a); return e > 'a'; }));
}
f();
/* OUTPUT
1 4
TypeError: abc is not a function
4# 1 0 [1, undefined, 5]
4# undefined 1 [1, undefined, 5]
4# 5 2 [1, undefined, 5]
4 undefined
5 5
6 [1, undefined, 5]
7# undefined 0 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 1x 1 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 2x 2 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 3x 3 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# undefined 4 {1: 1x, 2: 2x, 3: 3x, length: 5}
7 undefined
8# a 0 abc
8# b 1 abc
8 b
*/


// Index: 15
// findIndex
function f() {
    console.log(1, [1, 2, 3, 4, 5].findIndex(i => i > 3));
    try {
        console.log(2, [1, null, , undefined, 5].findIndex('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var a = [1, , 5];
    console.log(4, a.findIndex(function (e, i, a) { console.log('4#', e, i, a); }));
    console.log(5, a.findIndex(i => i > 3));
    console.log(6, a);

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};

    console.log(7, Array.prototype.findIndex.call(obj, function (e, i, a) { console.log('7#', e, i, a); return e == '2x'; }));

    console.log(8, Array.prototype.findIndex.call('abc', function (e, i, a) { console.log('8#', e, i, a); return e > 'a'; }));
}
f();
/* OUTPUT
1 3
TypeError: abc is not a function
4# 1 0 [1, undefined, 5]
4# undefined 1 [1, undefined, 5]
4# 5 2 [1, undefined, 5]
4 -1
5 2
6 [1, undefined, 5]
7# undefined 0 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 1x 1 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 2x 2 {1: 1x, 2: 2x, 3: 3x, length: 5}
7 2
8# a 0 abc
8# b 1 abc
8 1
*/


// Index: 16
// findLast
function f() {
    console.log(1, [1, 2, 3, 4, 5].findLast(i => i == 3));
    try {
        console.log(2, [1, null, , undefined, 5].findLast('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var a = [1, , 5];
    console.log(4, a.findLast(function (e, i, a) { console.log('4#', e, i, a); }));
    console.log(5, a.findLast(i => i > 3));
    console.log(6, a);

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};

    console.log(7, Array.prototype.findLast.call(obj, function (e, i, a) { console.log('7#', e, i, a); }));

    console.log(8, Array.prototype.findLast.call('abc', function (e, i, a) { console.log('8#', e, i, a); return e == 'b'; }));
}
f();
/* OUTPUT
1 3
TypeError: abc is not a function
4# 5 2 [1, undefined, 5]
4# undefined 1 [1, undefined, 5]
4# 1 0 [1, undefined, 5]
4 undefined
5 5
6 [1, undefined, 5]
7# undefined 4 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 3x 3 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 2x 2 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 1x 1 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# undefined 0 {1: 1x, 2: 2x, 3: 3x, length: 5}
7 undefined
8# c 2 abc
8# b 1 abc
8 b
*/


// Index: 17
// findLastIndex
function f() {
    console.log(1, [1, 2, 3, 4, 5].findLastIndex(i => i == 3));
    try {
        console.log(2, [1, null, , undefined, 5].findLastIndex('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var a = [1, , 5];
    console.log(4, a.findLastIndex(function (e, i, a) { console.log('4#', e, i, a); }));
    console.log(5, a.findLastIndex(i => i > 3));
    console.log(6, a);

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};

    console.log(7, Array.prototype.findLastIndex.call(obj, function (e, i, a) { console.log('7#', e, i, a); return e == '2x'; }));

    console.log(8, Array.prototype.findLastIndex.call('abc', function (e, i, a) { console.log('8#', e, i, a); return e == 'b'; }));
}
f();
/* OUTPUT
1 2
TypeError: abc is not a function
4# 5 2 [1, undefined, 5]
4# undefined 1 [1, undefined, 5]
4# 1 0 [1, undefined, 5]
4 -1
5 2
6 [1, undefined, 5]
7# undefined 4 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 3x 3 {1: 1x, 2: 2x, 3: 3x, length: 5}
7# 2x 2 {1: 1x, 2: 2x, 3: 3x, length: 5}
7 2
8# c 2 abc
8# b 1 abc
8 1
*/


// Index: 18
// flat
function f() {
    console.log(1, [1, 2, 3, 4, 5].flat(i => i == 3));
    try {
        console.log(2, [1, null, , undefined, 5].flat('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var a = [1, , 5];
    console.log(4, a.flat(function (e, i, a) { console.log('4#', e, i, a); }));
    console.log(5, a.flat(i => i > 3));
    console.log(6, a);

    var obj1 = {1: '1x', 2: '2x', 3: '3x', length: 5};
    var obj2 = {1: '1y', 2: ['2y1', '2y2'], 3: obj1, length: 5};

    console.log(7, Array.prototype.flat.call(obj2, 3));

    console.log(8, Array.prototype.flat.call('abc', function (e, i, a) { console.log('8#', e, i, a); return e == 'b'; }));

    // 递归的 array
    var a1 = [1, 2, []];
    var a2 = a1[2];
    a2.push(a1, 3, 4);
    console.log(a1.flat(10));
}
f();
/* OUTPUT
1 [1, 2, 3, 4, 5]
2 [1, null, undefined, 5]
4 [1, 5]
5 [1, 5]
6 [1, undefined, 5]
7 [1y, 2y1, 2y2, Object]
8 [a, b, c]
[1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, Array(3), 3, 4, 3, 4, 3, 4, 3, 4, 3, 4]
*/


// Index: 19
// flatMap
function f() {
    console.log(1, [1, 2, , 4, 5].flatMap(i => i * 3));
    console.log(1.1, [1, , 4, 5].flatMap(i => [i, i * 2, '#']));
    console.log(1.2, [1, , [4, 5]].flatMap(i => [i, i * 2, '#']));

    try {
        console.log(2, [1, null, , undefined, 5].flatMap('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var a = [1, , 5];
    console.log(4, a.flatMap(function (e, i, a) { console.log('4#', e, i, a); return e + 'x'; }));
    console.log(6, a);

    var obj1 = {1: '1x', 2: '2x', 3: '3x', length: 5};
    var obj2 = {1: '1y', 2: ['2y1', '2y2'], 3: obj1, length: 4};

    console.log(7, Array.prototype.flatMap.call(obj2, function(e, i, a) { console.log('7#', e, i, a); return e + 'x'; }));

    console.log(8, Array.prototype.flatMap.call('abc', function (e, i, a) { console.log('8#', e, i, a); return e == 'b'; }));

    // 递归的 array
    var a1 = [1, 2, []];
    var a2 = a1[2];
    a2.push(a1, 3, 4);

    try {
        console.log(a1.flatMap(10));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
1 [3, 6, 12, 15]
1.1 [1, 2, #, 4, 8, #, 5, 10, #]
1.2 [1, 2, #, Array(2), NaN, #]
TypeError: flatMap mapper function is not callable
4# 1 0 [1, undefined, 5]
4# 5 2 [1, undefined, 5]
4 [1x, 5x]
6 [1, undefined, 5]
7# 1y 1 {1: 1y, 2: Array(2), 3: Object, length: 4}
7# [2y1, 2y2] 2 {1: 1y, 2: Array(2), 3: Object, length: 4}
7# {1: 1x, 2: 2x, 3: 3x, length: 5} 3 {1: 1y, 2: Array(2), 3: Object, length: 4}
7 [1yx, 2y1,2y2x, [object Object]x]
8# a 0 abc
8# b 1 abc
8# c 2 abc
8 [false, true, false]
TypeError: flatMap mapper function is not callable
*/


// Index: 20
// forEach
function f() {
    console.log(1, [1, 2, , 4, 5].forEach(i => console.log('1#', i)));
    try {
        console.log(2, [1, null, , undefined, 5].forEach('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var a = [1, , 5];
    console.log(4, a.forEach(function (e, i, a) { console.log('4#', e, i, a); return e + 'x'; }));

    var obj1 = {1: '1x', 2: '2x', 3: '3x', length: 5};
    var obj2 = {1: '1y', 2: ['2y1', '2y2'], 3: obj1, length: 5};

    console.log(7, Array.prototype.forEach.call(obj2, function(e, i, a) { console.log('7#', e, i, a); return e + 'x'; }));

    console.log(8, Array.prototype.forEach.call('abc', function (e, i, a) { console.log('8#', e, i, a); return e == 'b'; }));
}
f();
/* OUTPUT
1# 1
1# 2
1# 4
1# 5
1 undefined
TypeError: abc is not a function
4# 1 0 [1, undefined, 5]
4# 5 2 [1, undefined, 5]
4 undefined
7# 1y 1 {1: 1y, 2: Array(2), 3: Object, length: 5}
7# [2y1, 2y2] 2 {1: 1y, 2: Array(2), 3: Object, length: 5}
7# {1: 1x, 2: 2x, 3: 3x, length: 5} 3 {1: 1y, 2: Array(2), 3: Object, length: 5}
7 undefined
8# a 0 abc
8# b 1 abc
8# c 2 abc
8 undefined
*/


// Index: 21
// includes
function f() {
    console.log(1, [1, 2, , 4, 5].includes(4));
    console.log(2, [1, null, , undefined, NaN, 5].includes(undefined));
    console.log(3, [1, null, , undefined, NaN, 5].includes(NaN));

    var a = [1, , 5];
    console.log(4, a.includes(undefined));
    console.log(5, a.includes('1'));

    var obj1 = {1: '1x', 2: '2x', 3: '3x', length: 5};
    var obj2 = {1: '1y', 2: ['2y1', '2y2'], 3: obj1, length: 5};

    console.log(7, Array.prototype.includes.call(obj2, '1y'));
    console.log(8, Array.prototype.includes.call('abc', 'b'));
    console.log(8, Array.prototype.includes.call(1, 1));
}
f();
/* OUTPUT
1 true
2 true
3 true
4 true
5 false
7 true
8 true
8 false
*/


// Index: 22
// indexOf
function f() {
    console.log(1, [1, 2, , 4, 5].indexOf(2));
    console.log(1.1, [1, 2, , 4, 5].indexOf(2, 3));
    console.log(1.2, [1, 2, , 4, 5].indexOf(5, -2));
    console.log(2, [1, null, , undefined, NaN, 5].indexOf(undefined));
    console.log(3, [1, null, , undefined, NaN, 5].indexOf(NaN));

    var a = [1, , 5];
    console.log(4, a.indexOf(undefined));
    console.log(5, a.indexOf('1'));

    var obj1 = {1: '1x', 2: '2x', 3: '3x', length: 5};
    var obj2 = {1: '1y', 2: ['2y1', '2y2'], 3: obj1, length: 5};

    console.log(7, Array.prototype.indexOf.call(obj2, '1y'));

    console.log(8, Array.prototype.indexOf.call('abc', 'a'));
}
f();
/* OUTPUT
1 1
1.1 -1
1.2 4
2 3
3 -1
4 -1
5 -1
7 1
8 0
*/


// Index: 23
// keys
function f() {
    for (var i of [,1, 2,].keys()) { console.log('1#', i); }

    var obj1 = {1: '1x', 2: '2x', 3: '3x', length: 5};
    var obj2 = {1: '1y', 2: ['2y1', '2y2'], 3: obj1, length: 5};

    var it = Array.prototype.keys.call(obj2);
    for (var i of it) {
        console.log('2#', i);
    }

    it = Array.prototype.keys.call('abc');
    for (var i of it) {
        console.log('3#', i);
    }

    var it = Array.prototype.keys.call(2);
    for (var i of it) {
        console.log('2#', i);
    }
}
f();
/* OUTPUT
1# 0
1# 1
1# 2
2# 0
2# 1
2# 2
2# 3
2# 4
3# 0
3# 1
3# 2
*/


// Index: 24
// lastIndexOf
function f() {
    console.log(1, [1, 2, , 4, 5].lastIndexOf(4));
    console.log(1.1, [1, 2, , 4, 5].lastIndexOf(4, 2));
    console.log(1.2, [1, 2, , 4, 5].lastIndexOf(4, 4));
    console.log(1.3, [1, 2, , 4, 5].lastIndexOf(4, -1));
    console.log(2, [1, null, , undefined, NaN, 5].lastIndexOf(undefined));
    console.log(3, [1, null, , undefined, NaN, 5].lastIndexOf(NaN));

    var a = [1, , 5];
    console.log(4, a.lastIndexOf(undefined));
    console.log(5, a.lastIndexOf('1'));

    var obj1 = {1: '1x', 2: '2x', 3: '3x', length: 5};
    var obj2 = {1: '1y', 2: ['2y1', '2y2'], 3: obj1, length: 5};

    console.log(7, Array.prototype.lastIndexOf.call(obj2, '1y'));

    console.log(8, Array.prototype.lastIndexOf.call('abc', 'a'));
}
f();
/* OUTPUT
1 3
1.1 -1
1.2 3
1.3 3
2 3
3 -1
4 -1
5 -1
7 1
8 0
*/


// Index: 25
// map
function f() {
    console.log(1, [1, 2, , 4, 5].map(i => (console.log('1#', i), i * 2)));
    console.log(1.1, [1, , [4, 5]].map(i => [i, i * 2, '#']));
    try {
        console.log(2, [1, null, , undefined, 5].map('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var a = [1, , 5];
    console.log(4, a.map(function (e, i, a) { console.log('4#', e, i, a); return e + 'x'; }));

    var obj1 = {1: '1x', 2: '2x', 3: '3x', length: 5};
    var obj2 = {1: '1y', 2: ['2y1', '2y2'], 3: obj1, length: 4};

    console.log(7, Array.prototype.map.call(obj2, function(e, i, a) { console.log('7#', e, i, a); return e + 'x'; }));

    console.log(8, Array.prototype.map.call('abc', function (e, i, a) { console.log('8#', e, i, a); return e == 'b'; }));
}
f();
/* OUTPUT
1# 1
1# 2
1# 4
1# 5
1 [2, 4, undefined, 8, 10]
1.1 [Array(3), undefined, Array(3)]
TypeError: abc is not a function
4# 1 0 [1, undefined, 5]
4# 5 2 [1, undefined, 5]
4 [1x, undefined, 5x]
7# 1y 1 {1: 1y, 2: Array(2), 3: Object, length: 4}
7# [2y1, 2y2] 2 {1: 1y, 2: Array(2), 3: Object, length: 4}
7# {1: 1x, 2: 2x, 3: 3x, length: 5} 3 {1: 1y, 2: Array(2), 3: Object, length: 4}
7 [undefined, 1yx, 2y1,2y2x, [object Object]x]
8# a 0 abc
8# b 1 abc
8# c 2 abc
8 [false, true, false]
*/


// Index: 26
// pop
function f() {
    var a = [1, 2];
    console.log(1, a.push(3));
    console.log(2, a);
    console.log(3, a.pop(), a);
    console.log(4, a.pop(), a);
    console.log(5, a.pop(), a);
    console.log(6, a.pop(), a);

    try {
        console.log(7, Array.prototype.pop.call('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(8, Array.prototype.push.call('abc', 'd'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log(8, Array.prototype.push.call(1, 'd'));

    var obj = {1: '1x', length: 2};
    console.log(9, Array.prototype.push.call(obj, '2x'), obj);
    console.log(10, Array.prototype.pop.call(obj), obj);
    console.log(11, Array.prototype.pop.call(obj), obj);
    console.log(12, Array.prototype.pop.call(obj), obj);
    console.log(13, Array.prototype.pop.call(obj), obj);

    var obj = {};
    console.log(14, Array.prototype.push.call(obj, '2x'), obj);

    var obj = {1: '1x'};
    console.log(15, Array.prototype.push.call(obj, '2x'), obj);

    var obj = new Number(1);
    console.log(14, Array.prototype.push.call(obj, '2x'), obj);

}
f();
/* OUTPUT
1 3
2 [1, 2, 3]
3 3 [1, 2]
4 2 [1]
5 1 []
6 undefined []
TypeError: Cannot delete property '2' of [object String]
TypeError: Cannot assign to read only property 'length' of object '[object String]'
8 1
9 3 {1: 1x, 2: 2x, length: 3}
10 2x {1: 1x, length: 2}
11 1x {length: 1}
12 undefined {length: 0}
13 undefined {length: 0}
14 1 {0: 2x, length: 1}
15 1 {0: 2x, 1: 1x, length: 1}
14 1 1
*/


// Index: 27
function f() {
    var obj = { length: 1};
    Object.defineProperty(obj, '0', {
        value: 'x',
        writable: false
    });
    try {
        console.log(16, Array.prototype.pop.call(obj), obj);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var obj = { length: 1};
    Object.defineProperty(obj, '0', {
        value: 'x',
        configurable: false
    });
    try {
        console.log(17, Array.prototype.pop.call(obj), obj);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        a = [];
        Object.preventExtensions(a);
        console.log(18, a.push(1));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        a = {};
        Object.preventExtensions(a);
        console.log(18, Array.prototype.push.call(a, 1));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT-FIXED
TypeError: Cannot delete property '0' of [object Object]
TypeError: Cannot delete property '0' of [object Object]
TypeError: Cannot add property 0, object is not extensible
TypeError: Cannot add property 0, object is not extensible
*/


// Index: 28
// reduce
function f() {
    var a = [1, 2];
    console.log(1, [1, 2].reduce((accumulator, currentValue, index) => (console.log('@1', currentValue, index), accumulator + currentValue), 5));
    console.log(2, [1, 2].reduce((accumulator, currentValue, index) => (console.log('@2', currentValue, index), accumulator + currentValue)));
    try {
        console.log(3, [].reduce((accumulator, currentValue) => (console.log('@3', 'x'), accumulator + currentValue)));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log(3.1, [].reduce((accumulator, currentValue) => (console.log('@3', 'x'), accumulator + currentValue), 5));
    console.log(4, [1].reduce((accumulator, currentValue) => (console.log('@4', 'x'), accumulator + currentValue)));
    console.log(5, [1, , 2].reduce((accumulator, currentValue) => (console.log('@5', 'x'), accumulator + currentValue)));
    console.log(6, [1, undefined, 2].reduce((accumulator, currentValue) => (console.log('@6', 'x'), accumulator + currentValue)));
    console.log(7, [, , , 2, 3].reduce((accumulator, currentValue) => (console.log('@7', 'x'), accumulator + currentValue)));
    try {
        console.log(8, [, , , ].reduce((accumulator, currentValue) => (console.log('@8', 'x'), accumulator + currentValue)));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        console.log(9, Array.prototype.reduce.call('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(10, Array.prototype.reduce.call('abc', 'd'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    console.log(11, Array.prototype.reduce.call('d', (accumulator, currentValue) => (console.log('@11', accumulator + currentValue))));

    var obj = {1: '1x', 2: '2x', length: 3};
    console.log(12, Array.prototype.reduce.call(obj, (accumulator, currentValue) => (console.log('@12', accumulator + currentValue))));
}
f();
/* OUTPUT
@1 1 0
@1 2 1
1 8
@2 2 1
2 3
TypeError: Reduce of empty array with no initial value
3.1 5
4 1
@5 x
5 3
@6 x
@6 x
6 NaN
@7 x
7 5
TypeError: Reduce of empty array with no initial value
TypeError: undefined is not a function
TypeError: d is not a function
11 d
@12 1x2x
12 undefined
*/


// Index: 29
// reduceRight
function f() {
    var a = [1, 2];
    console.log(1, [1, 2].reduceRight((accumulator, currentValue, index) => (console.log('@1', currentValue, index), accumulator + currentValue), 5));
    console.log(2, [1, 2].reduceRight((accumulator, currentValue, index) => (console.log('@2', currentValue, index), accumulator + currentValue)));
    try {
        console.log(3, [].reduceRight((accumulator, currentValue) => (console.log('@3', 'x'), accumulator + currentValue)));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log(3.1, [].reduceRight((accumulator, currentValue) => (console.log('@3', 'x'), accumulator + currentValue), 5));
    console.log(4, [1].reduceRight((accumulator, currentValue) => (console.log('@4', 'x'), accumulator + currentValue)));
    console.log(5, [1, , 2, ,].reduceRight((accumulator, currentValue) => (console.log('@5', 'x'), accumulator + currentValue)));
    console.log(6, [1, undefined, 2].reduceRight((accumulator, currentValue) => (console.log('@6', 'x'), accumulator + currentValue)));
    console.log(7, [, , , 2, 3,,].reduceRight((accumulator, currentValue) => (console.log('@7', 'x'), accumulator + currentValue)));
    try {
        console.log(8, [, , , ].reduceRight((accumulator, currentValue) => (console.log('@8', 'x'), accumulator + currentValue)));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        console.log(9, Array.prototype.reduceRight.call('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(10, Array.prototype.reduceRight.call('abc', 'd'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    console.log(11, Array.prototype.reduceRight.call('d', (accumulator, currentValue) => (console.log('@11', accumulator + currentValue))));

    var obj = {1: '1x', 2: '2x', length: 3};
    console.log(11, Array.prototype.reduceRight.call(obj, (accumulator, currentValue) => (console.log('@12', accumulator + currentValue))));
}
f();
/* OUTPUT
@1 2 1
@1 1 0
1 8
@2 1 0
2 3
TypeError: Reduce of empty array with no initial value
3.1 5
4 1
@5 x
5 3
@6 x
@6 x
6 NaN
@7 x
7 5
TypeError: Reduce of empty array with no initial value
TypeError: undefined is not a function
TypeError: d is not a function
11 d
@12 2x1x
11 undefined
*/


// Index: 30
// reverse
function f() {
    console.log(1, [1, 2].reverse());
    console.log(2, [1].reverse());
    console.log(3, [,1].reverse());
    console.log(4, [,1,,].reverse());

    var a = [];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    a.reverse();
    console.log(5, a[0], a[1], a[2]);

    console.log(9, Array.prototype.reverse.call(''));

    try {
        console.log(9, Array.prototype.reverse.call('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log(10, Array.prototype.reverse.call(1));

    var obj = {1: '1x', 2: '2x', length: 3, others: 'y'};
    console.log(11, Array.prototype.reverse.call(obj));


    var a = {length: 3};
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    console.log(12, Array.prototype.reverse.call(a), a[0], a[1], a[2]);
}
f();
/* OUTPUT-FIXED
1 [2, 1]
2 [1]
3 [1, undefined]
4 [undefined, 1, undefined]
get 0 x
get 2 b
set 0:  b
set 2:  x
get 0 b
get 2 x
5 b undefined x
9 
TypeError: Cannot assign to read only property '0' of object '[object String]'
10 1
11 {0: 2x, 1: 1x, length: 3, others: y}
get 0 x
get 2 b
set 0:  b
set 2:  x
get 0 b
get 2 x
get 2 x
get 0 b
12 {0: b, 2: x, _0: b, _2: x, length: 3} b undefined x
*/


// Index: 31
// shift
function f() {
    var a = [1, 2];
    console.log(1, a.shift(), a);
    console.log(2, [].shift(), a);
    console.log(3, [, 1].shift(), a);

    a = {}
    console.log(4, Array.prototype.shift.call(a), a);

    try {
        console.log(5, Array.prototype.shift.call(''));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        console.log(7, Array.prototype.shift.call('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(8, Array.prototype.shift.call(1));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log(8, Array.prototype.shift.call(1));

    var obj = {1: '1x', length: 2};
    console.log(9, Array.prototype.shift.call(obj), obj);

    console.log('#10');

    var a = [];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    try {
        console.log(10, a.shift());
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    console.log('#11');

    // 有 configurable，无异常
    var a = [];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {configurable: true, get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {configurable: true, get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    console.log(11, Array.prototype.shift.call(a));

    console.log('#12');

    // 不会删除元素，无异常
    var a = [,,,3];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 1, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    console.log(11, Array.prototype.shift.call(a));

    console.log('#13');

    // 异常: 无 setter
    var a = [,,,3];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 1, {get: function() { console.log('get 0', this._0); return this._0; }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }})
    try {
        console.log(11, Array.prototype.shift.call(a));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    console.log('#13.5');

    // 异常: 无 setter
    var a = [,,,3];
    Object.defineProperty(a, 1, {value: 'x', writable: false})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }})
    try {
        console.log(11, Array.prototype.shift.call(a));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    console.log('#13.7');

    // 正常: 无 getter
    var a = [,,,3];
    Object.defineProperty(a, 1, {set: function(p) { console.log('set 1', p); this._1 = p; }, configurable: true})
    try {
        console.log(11, Array.prototype.shift.call(a));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    console.log('#14');

    // 有 configurable，无异常
    var a = {length: 3};
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {configurable: true, get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {configurable: true, get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    console.log(11, Array.prototype.shift.call(a));

    console.log('#15');

    // 不会删除元素，无异常
    var a = {3: '3x', length: 4};
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 1, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    console.log(11, Array.prototype.shift.call(a));
}
f();
/* OUTPUT
1 1 [2]
2 undefined [2]
3 undefined [2]
4 undefined {length: 0}
TypeError: Cannot assign to read only property 'length' of object '[object String]'
TypeError: Cannot assign to read only property '0' of object '[object String]'
8 undefined
8 undefined
9 undefined {0: 1x, length: 1}
#10
get 0 x
TypeError: Cannot delete property '0' of [object Array]
#11
get 0 x
get 2 b
11 x
#12
get 0 x
get 2 b
set 0:  b
set 2:  3
11 undefined
#13
get 0 x
get 2 b
TypeError: Cannot set property 1 of [object Array] which has only a getter
#13.5
get 2 undefined
TypeError: Cannot assign to read only property '1' of object '[object Array]'
#13.7
11 undefined
#14
get 0 x
get 2 b
11 x
#15
get 0 x
get 2 b
set 0:  b
set 2:  3x
11 undefined
*/


// Index: 32
function f() {
    // 异常
    var a = {length: 3};
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    try {
        console.log(11, Array.prototype.shift.call(a));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT-FIXED
get 0 x
TypeError: Cannot delete property '0' of [object Object]
*/


// Index: 33
// slice
function f() {
    // console.log(1, [1, 2, , 4, 5].slice(1));
    // console.log(1.1, [1, 2, , 4, 5].slice(1, 0));
    // console.log(1.2, [1, 2, , 4, 5].slice(1, 4));
    // console.log(1.3, [1, 2, , 4, 5].slice(1, -1));
    // console.log(1.4, [1, 2, , 4, 5].slice(1, Infinity));
    // console.log(2, [1, null, , undefined, NaN, 5].slice(undefined));
    // console.log(3, [1, null, , undefined, NaN, 5].slice(NaN));

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};

    console.log(4, Array.prototype.slice.call(obj, 2));
    console.log(5, Array.prototype.slice.call(1, 2));

    console.log(6, Array.prototype.slice.call('abcde', '-2'));
}
f();
/* OUTPUT
4 [2x, 3x, undefined]
5 []
6 [d, e]
*/


// Index: 34
// some
function f() {
    var a = [1, , 5];
    var obj = {1: '1x', 3: '3x', length: 5};
    var s = 'xbc';
    var c = 'Y';

    console.log(a.some(function(x, i) { console.log(x, i); return x >= 5; }));
    console.log(a.some(function(x, i) { console.log(x, i); return x >= 10; }));

    console.log(Array.prototype.some.call(s, function(x, i) { console.log(x, i); return x >= 'c'; }));
    console.log(Array.prototype.some.call(s, function(x, i) { console.log(x, i); return x > 'c'; }));
    console.log(Array.prototype.some.call(c, function(x, i) { console.log(x, i); return x >= 'y'; }));
    console.log(Array.prototype.some.call(c, function(x, i) { console.log(x, i); return x > 'y'; }));

    console.log(Array.prototype.some.call(obj, function(x, i) { console.log(x, i); return x === '1x'; }));
    console.log(Array.prototype.some.call(obj, function(x, i) { console.log(x, i); return x === '2x'; }));
}
f();
/* OUTPUT
1 0
5 2
true
1 0
5 2
false
x 0
true
x 0
true
Y 0
false
Y 0
false
1x 1
true
1x 1
3x 3
false
*/


// Index: 35
// sort
function f() {
    var a = [2, 1, 4];
    console.log(1, a.sort(), a);
    console.log(1.1, [ 3, 1, 11, true, 'true', true, 's', NaN, 'G'].sort());
    console.log(1.3, ['Z', '[', '\\', ']', '^', '_', '`', '[object Object]', {}, '[object Object]'].sort());
    console.log(2, [].sort());
    console.log(3, [undefined, , 2, 1,,].sort());

    console.log(3.1, [undefined, , 2, 1,,].sort(function (a, b) { console.log(a, b); return a - b; }));

    a = {}
    console.log(4, Array.prototype.sort.call(a), a);

    console.log(5, Array.prototype.sort.call(true));
    console.log(5, Array.prototype.sort.call(1));
    console.log(5, Array.prototype.sort.call(''));

    try {
        console.log(7, Array.prototype.sort.call('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var obj = {1: 5, 2: 4, length: 4};
    console.log(9, Array.prototype.sort.call(obj), obj);


    console.log('#10');

    var a = [];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    try {
        console.log(10, a.sort());
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    console.log('#11');

    // 有 configurable，无异常
    var a = [];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {configurable: true, get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {configurable: true, get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    Array.prototype.sort.call(a);
    console.log(11, a[0], a[1], a[2]);

    console.log('#12');

    // 不会删除元素，无异常
    var a = [,,,3];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 1, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    Array.prototype.sort.call(a);
    console.log(12, a[0], a[1], a[2]);

    console.log('#13');

    // 异常: 无 setter
    var a = [,,,3];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 1, {get: function() { console.log('get 0', this._0); return this._0; }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }})
    try {
        Array.prototype.sort.call(a);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    console.log('#14');

    // 有 configurable，无异常
    var a = {length: 3};
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {configurable: true, get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {configurable: true, get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    Array.prototype.sort.call(a);
    console.log(14, a[0], a[1], a[2]);

    console.log('#15');

    // 不会删除元素，无异常
    var a = {3: '3x', length: 4};
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 1, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    Array.prototype.sort.call(a);
    console.log(15, a[0], a[1], a[2], a[3]);
}
f();
/* OUTPUT
1 [1, 2, 4] [1, 2, 4]
1.1 [1, 11, 3, G, NaN, s, true, true, true]
1.3 [Z, [, [object Object], Object, [object Object], \, ], ^, _, `]
2 []
3 [1, 2, undefined, undefined, undefined]
1 2
3.1 [1, 2, undefined, undefined, undefined]
4 {} {}
5 true
5 1
5 
TypeError: Cannot assign to read only property '0' of object '[object String]'
9 {0: 4, 1: 5, length: 4} {0: 4, 1: 5, length: 4}
#10
get 0 x
get 2 b
set 0:  b
TypeError: Cannot delete property '2' of [object Array]
#11
get 0 x
get 2 b
set 0:  b
get 0 b
11 b x undefined
#12
get 0 x
get 2 b
set 0:  b
set 2:  x
get 0 b
get 2 x
12 3 b x
#13
get 0 x
get 2 b
TypeError: Cannot set property 1 of [object Array] which has only a getter
#14
get 0 x
get 2 b
set 0:  b
get 0 b
14 b x undefined
#15
get 0 x
get 2 b
set 0:  b
set 2:  x
get 0 b
get 2 x
15 3x b x undefined
*/


// Index: 36
function f() {
    // 异常
    var a = {length: 3};
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    try {
        Array.prototype.sort.call(a);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT-FIXED
get 0 x
get 2 b
set 0:  b
TypeError: Cannot delete property '2' of [object Object]
*/


// Index: 37
// splice
function f() {
    var a;
    console.log(1, (a = [1, 2, , 4, 5]).splice(1), a);
    console.log(1.1, (a = [1, 2, , 4, 5]).splice(1, 0), a);
    console.log(1.2, (a = [1, 2, , 4, 5]).splice(1, 2), a);
    console.log(1.3, (a = [1, 2, , 4, 5]).splice(1, -1), a);
    console.log(1.4, (a = [1, 2, , 4, 5]).splice(1, 1, 'x', 'y'), a);
    console.log(1.5, (a = [1, 2, , 4, 5]).splice(1, 2, 'x', 'y'), a);
    console.log(1.6, (a = [1, 2, , 4, 5]).splice(1, 2, 'x'), a);
    console.log(1.7, (a = [1, 2, , 4, 5]).splice(1, 2, 'x', 'y', 'z'), a);

    console.log('#1');
    var a = [,,,3];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 1, {get: function() { console.log('get 1', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 1: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    console.log(a.splice(0, 1));
    try {
        console.log(a.splice(0, 1));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    console.log('#2');
    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};
    console.log(2, Array.prototype.splice.call(obj, 0, 1));

    console.log('#3');
    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};
    console.log(3, Array.prototype.splice.call(obj, 0, 1, 'x'));

    console.log('#4');
    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};
    console.log(4, Array.prototype.splice.call(obj, 0, 1, 'x', 'y'));

    console.log('#5');
    console.log(5, Array.prototype.splice.call(1, 2));

    console.log('#6');
    function g(r, a, b, c) {
        console.log('#' + r);
        try {
            Array.prototype.splice.call('abcdef', a, b, c);
        } catch (e) {
            console.log(e.name + ': ' + e.message);
        }
    }
    g(10, 0, 1, 2);
    g(11, 1, 0, 2);
    g(12, 1, 0);
    g(13, 6, 0);
}
f();
/* OUTPUT
1 [2, undefined, 4, 5] [1]
1.1 [] [1, 2, undefined, 4, 5]
1.2 [2, undefined] [1, 4, 5]
1.3 [] [1, 2, undefined, 4, 5]
1.4 [2] [1, x, y, undefined, 4, 5]
1.5 [2, undefined] [1, x, y, 4, 5]
1.6 [2, undefined] [1, x, 4, 5]
1.7 [2, undefined] [1, x, y, z, 4, 5]
#1
get 1 x
get 2 b
set 1:  b
set 2:  3
[undefined]
get 1 b
get 2 3
set 1:  3
TypeError: Cannot delete property '2' of [object Array]
#2
2 [undefined]
#3
3 [undefined]
#4
4 [undefined]
#5
5 []
#6
#10
TypeError: Cannot assign to read only property '0' of object '[object String]'
#11
TypeError: Cannot assign to read only property '5' of object '[object String]'
#12
TypeError: Cannot assign to read only property '5' of object '[object String]'
#13
TypeError: Cannot assign to read only property 'length' of object '[object String]'
*/


// Index: 38
// unshift
function f() {
    var a;
    console.log(1, (a = [1, 2, , 4, 5]).unshift(), a);
    console.log(1.1, (a = [1, 2, , 4, 5]).unshift(1), a);
    console.log(1.2, (a = [1, 2, , 4, 5]).unshift(1, 2), a);
    console.log(1.3, (a = [1, 2, , 4, 5]).unshift(1, 2, undefined), a);

    var a = [,,,3];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 1, {get: function() { console.log('get 1', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 1: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})

    try {
        console.log(a.unshift(0, 1), a);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};
    console.log(4, Array.prototype.unshift.call(obj));

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};
    console.log(4, Array.prototype.unshift.call(obj, 0));

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};
    console.log(4, Array.prototype.unshift.call(obj, 0, 1, 'x', 'y'));

    console.log(5, Array.prototype.unshift.call(1, 2));

    console.log(6, Array.prototype.unshift.call(obj = {}), obj);

    try {
        Array.prototype.unshift.call('abcdef');
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        Array.prototype.unshift.call('abcdef', 1);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
1 5 [1, 2, undefined, 4, 5]
1.1 6 [1, 1, 2, undefined, 4, 5]
1.2 7 [1, 2, 1, 2, undefined, 4, 5]
1.3 8 [1, 2, undefined, 1, 2, undefined, 4, 5]
get 2 b
get 1 x
TypeError: Cannot delete property '2' of [object Array]
4 5
4 6
4 9
5 1
6 0 {length: 0}
TypeError: Cannot assign to read only property 'length' of object '[object String]'
TypeError: Cannot assign to read only property '5' of object '[object String]'
*/


// Index: 39
// values
function f() {
    function g(r, it) {
        console.log('round:', r);
        for (var value of it) {
            console.log(r, value);
        }
    }

    var array1 = ['a', 'b', , 'c'];
    var it = array1.values();
    g('#1', it);

    it = Array.prototype.values.call('def');
    g('#2', it);

    it = Array.prototype.values.call('g');
    g('#3', it);

    it = Array.prototype.values.call(1);
    g('#4', it);

    var obj = {1: '1x', 2: '2x', 3: '3x', length: 5};
    it = Array.prototype.values.call(obj);
    g('#5', it);

    it = Array.prototype.values.call({});
    g('#6', it);
}
f();
/* OUTPUT
round: #1
#1 a
#1 b
#1 undefined
#1 c
round: #2
#2 d
#2 e
#2 f
round: #3
#3 g
round: #4
round: #5
#5 undefined
#5 1x
#5 2x
#5 3x
#5 undefined
round: #6
*/


// Index: 40
console.log(1, Array.isArray([]));
console.log(2, Array.isArray([1]));
console.log(3, Array.isArray(new Array()));
console.log(4, Array.isArray(new Array("a", "b", "c", "d")));
console.log(5, Array.isArray(new Array(3)));
// Little known fact: Array.prototype itself is an array:
console.log(6, Array.isArray(Array.prototype));

// all following calls return false
console.log(11, Array.isArray());
console.log(12, Array.isArray({}));
console.log(13, Array.isArray(null));
console.log(14, Array.isArray(undefined));
console.log(15, Array.isArray(17));
console.log(16, Array.isArray("Array"));
console.log(17, Array.isArray(true));
console.log(18, Array.isArray(false));
// console.log(19, Array.isArray(new Uint8Array(32)));
// This is not an array, because it was not created using the
// array literal syntax or the Array constructor
console.log(20, Array.isArray({ __proto__: Array.prototype }));
/* OUTPUT
1 true
2 true
3 true
4 true
5 true
6 true
11 false
12 false
13 false
14 false
15 false
16 false
17 false
18 false
20 false
*/


// Index: 41
// from
function f() {
    console.log(1, Array.from([1, 2, 3]));
    console.log(2, Array.from('abc'));

    console.log(3, Array.from([1, 2, 3], undefined));

    try {
        console.log(Array.from('abc', null));
    } catch (e) {
        console.log(4, e.name + ': ' + e.message);
    }

    console.log(5, Array.from([1,, 2, 3], (x) => x + x));

    console.log(6, Array.from({length: 5}, (v, i) => i));

    console.log(Array.from.call({}, { length: 1, 0: "foo" })); // [ 'foo' ]
}
f();
/* OUTPUT
1 [1, 2, 3]
2 [a, b, c]
3 [1, 2, 3]
4 TypeError: null is not a function
5 [2, NaN, 4, 6]
6 [0, 1, 2, 3, 4]
[foo]
*/


// Index: 42
// Array.of
function f() {
    console.log(Array.of());
    console.log(Array.of(1, 3, 5));
    console.log(Array.of('xy'));
    console.log(Array.of(undefined, NaN, null));
}
f();
/* OUTPUT
[]
[1, 3, 5]
[xy]
[undefined, NaN, null]
*/

