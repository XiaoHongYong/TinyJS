// Index: 0
// Object.is
var obj1 = { toString() { return '0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f(r, p, noPrint) {
    console.log('round: ', r, noPrint ? '' : p);

    console.log('1', Object.isSealed(p));
}
f('1', undefined);
f('2', null);
f('3', NaN);
f('4', Infinity);
f('5', -Infinity);
f('6', 0);
f('7', 0.0);
f('8', 1.0);
f('9', -1);
f('14', -1.0);
f('15', true);
f('16', false);
f('17', '');
f('18', '0');
f('19', '1');
f('234'.charAt(1));
f('20', '0.0');
f('22', 'true');
f('23', 'false');
f('24', g, 1);
f('25', obj1, 1);
f('26', obj2, 1);
f('27', /a/);
f('28', Symbol());
/* OUTPUT
round:  1 undefined
1 true
round:  2 null
1 true
round:  3 NaN
1 true
round:  4 Infinity
1 true
round:  5 -Infinity
1 true
round:  6 0
1 true
round:  7 0
1 true
round:  8 1
1 true
round:  9 -1
1 true
round:  14 -1
1 true
round:  15 true
1 true
round:  16 false
1 true
round:  17 
1 true
round:  18 0
1 true
round:  19 1
1 true
round:  3 undefined
1 true
round:  20 0.0
1 true
round:  22 true
1 true
round:  23 false
1 true
round:  24 
1 false
round:  25 
1 false
round:  26 
1 false
round:  27 /a/
1 false
round:  28 Symbol()
1 true
*/


// Index: 1
// Object.isSealed
function f() {
    var o = {};
    Object.preventExtensions(o);
    console.log(Object.isSealed(o));

    o = {};
    o.x = 1;
    Object.preventExtensions(o);
    console.log(Object.isSealed(o));

    o = {};
    Object.defineProperty(o, 'x', {
        value: 1,
        configurable: false,
        writable: true,
    });
    Object.preventExtensions(o);
    console.log(Object.isSealed(o));


    o = [];
    Object.defineProperty(o, 'x', {
        value: 1,
        configurable: false,
        writable: true,
    });
    Object.preventExtensions(o);
    console.log(Object.isSealed(o));
}
f();
/* OUTPUT
true
false
true
true
*/


// Index: 2
// Object.keys
function f() {
    try {
        console.log(Object.keys(undefined));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(Object.keys(null));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log('#1', Object.keys(1));
    console.log('#2', Object.keys(1.2));
    console.log('#3', Object.keys('x'));
    console.log('#4', Object.keys(true));
    console.log('#5', Object.keys(false));
    console.log('#6', Object.keys(/a/));
    console.log('#7', Object.keys(Symbol()));
    console.log('#8', Object.keys('abc'.charAt(1)));
    console.log('#9', Object.keys({}));
    console.log('#10', Object.keys([]));
    console.log('#11', Object.keys({x:1}));
    console.log('#12', Object.keys([1, 2]));
}
f();
/* OUTPUT
TypeError: Cannot convert undefined or null to object
TypeError: Cannot convert undefined or null to object
#1 []
#2 []
#3 [0]
#4 []
#5 []
#6 []
#7 []
#8 [0]
#9 []
#10 []
#11 [x]
#12 [0, 1]
*/


// Index: 3
// Object.keys
function f() {
    var name = 'length';
    console.log('#1', (1).propertyIsEnumerable(name));
    console.log('#2', (1.2).propertyIsEnumerable(name));
    console.log('#3', ('x').propertyIsEnumerable(name));
    console.log('#4', (true).propertyIsEnumerable(name));
    console.log('#5', (false).propertyIsEnumerable(name));
    console.log('#6', (/a/).propertyIsEnumerable(name));
    console.log('#7', (Symbol()).propertyIsEnumerable(name));
    console.log('#8', ('abc'.charAt(1)).propertyIsEnumerable(name));
    console.log('#9', ({}).propertyIsEnumerable(name));
    console.log('#10', ([]).propertyIsEnumerable(name));
    console.log('#11', ({x:1}).propertyIsEnumerable(name));
    console.log('#12', ([1, 2]).propertyIsEnumerable(name));
}
f();
/* OUTPUT
#1 false
#2 false
#3 false
#4 false
#5 false
#6 false
#7 false
#8 false
#9 false
#10 false
#11 false
#12 false
*/


// Index: 4
// Object.keys
function f() {
    var a = ['a'];
    var o = { x: 1};
    Object.defineProperty(o, 'y', {value: 1, enumerable: false});
    console.log('#1', ('xy').propertyIsEnumerable('0'));
    console.log('#2', ('xy').propertyIsEnumerable(0));
    console.log('#3', ('x').propertyIsEnumerable('length'));
    console.log('#4', (a).propertyIsEnumerable('0'));
    console.log('#5', (a).propertyIsEnumerable('length'));
    console.log('#6', (o).propertyIsEnumerable('x'));
    console.log('#7', (o).propertyIsEnumerable('y'));
}
f();
/* OUTPUT
#1 true
#2 true
#3 false
#4 true
#5 false
#6 true
#7 false
*/


// Index: 5
// Object.seal
function f() {
    try {
        console.log(Object.seal(undefined));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(Object.seal(null));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log('#1', Object.seal(1));
    console.log('#2', Object.seal(1.1));
    console.log('#3', Object.seal('x'));
    console.log('#4', Object.seal(true));
    console.log('#5', Object.seal(false));
    console.log('#6', Object.seal(/a/));
    console.log('#7', Object.seal(Symbol()));
    console.log('#8', Object.seal('abc'.charAt(1)));
    console.log('#9', Object.seal({}));
    console.log('#10', Object.seal([]));
    console.log('#11', Object.seal({x:1}));
    console.log('#12', Object.seal([1, 2]));
}
f();
/* OUTPUT
undefined
null
#1 1
#2 1.1
#3 x
#4 true
#5 false
#6 /a/
#7 Symbol()
#8 b
#9 {}
#10 []
#11 {x: 1}
#12 [1, 2]
*/


// Index: 6
// Object.seal
function f() {
    var a = ['a'];
    var o = { x: 1};

    Object.seal(a);
    console.log('#1', a[0] = 'b', a[0]);
    console.log('#2', a[1] = 'c', a[1]);
    console.log('#3', a.x = 'd', a.x);

    Object.seal(o);
    console.log('#4', o[0] = 'b', o[0]);
    console.log('#5', o.x = 'd', o.x);
}
f();
/* OUTPUT
#1 b b
#2 c undefined
#3 d undefined
#4 b undefined
#5 d d
*/


// Index: 7
// Object.setPrototypeOf
function f() {
    var proto = { z : 'z1'};
    var a = ['a'];
    var o = { x: 1};

    Object.setPrototypeOf(a, proto);
    console.log('#1', a.z);

    Object.setPrototypeOf(o, proto);
    console.log('#2', o.z);
}
f();
/* OUTPUT
#1 z1
#2 z1
*/


// Index: 8
// Object.setPrototypeOf
function f() {
    var proto = { z : 'z1'};
    try {
        console.log(Object.setPrototypeOf(undefined, proto));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(Object.setPrototypeOf(null, proto));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log('#1', Object.setPrototypeOf(1, proto));
    console.log('#2', Object.setPrototypeOf(1.1, proto));
    console.log('#3', Object.setPrototypeOf('x', proto));
    console.log('#4', Object.setPrototypeOf(true, proto));
    console.log('#5', Object.setPrototypeOf(false, proto));
    console.log('#7', Object.setPrototypeOf(Symbol(), proto));
    console.log('#8', Object.setPrototypeOf('abc'.charAt(1), proto));
}
f();
/* OUTPUT
TypeError: Object.setPrototypeOf called on null or undefined
TypeError: Object.setPrototypeOf called on null or undefined
#1 1
#2 1.1
#3 x
#4 true
#5 false
#7 Symbol()
#8 b
*/


// Index: 9
// Object.setPrototypeOf
function f(r, obj, noPrint) {
    var proto = null;
    console.log('round: ', r, noPrint ? '' : obj);

    Object.setPrototypeOf(obj, proto);
    console.log('#6', Object.getPrototypeOf(obj) === proto);

    proto = {};
    Object.setPrototypeOf(obj, proto);
    console.log('#6', Object.getPrototypeOf(obj) === proto);
}
f(1, /a/);
f(2, {});
f(3, []);
f(4, {x:1});
f(5, [1, 2]);
/* OUTPUT
round:  1 /a/
#6 true
#6 true
round:  2 {}
#6 true
#6 true
round:  3 []
#6 true
#6 true
round:  4 {x: 1}
#6 true
#6 true
round:  5 [1, 2]
#6 true
#6 true
*/


// Index: 10
// Object.setPrototypeOf
function f() {
    try {
        Object.setPrototypeOf({}, undefined);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        Object.setPrototypeOf({}, 1);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        Object.setPrototypeOf({}, 'x');
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
TypeError: Object prototype may only be an Object or null: undefined
TypeError: Object prototype may only be an Object or null: 1
TypeError: Object prototype may only be an Object or null: x
*/


// Index: 11
// Object.valueOf
function f() {
    function MyNumberType(n) {
        this.number = n;
    }

    MyNumberType.prototype.valueOf = function () {
        return this.number;
    };

    MyNumberType.prototype.toString = function () {
        return this.number + 1;
    };

    const myObj = new MyNumberType(4);
    console.log(1, myObj + 3); // 7
    console.log(2, myObj == 4);
    console.log(3, myObj === 4);
    console.log(2, myObj == 5);
    console.log(3, myObj === 5);
}
f();
/* OUTPUT
1 7
2 true
3 false
2 false
3 false
*/


// Index: 12
// Object.valueOf
function f() {
    Object.prototype.valueOf = function() {
        console.log('called Object.prototype.valueOf');
        return this;
    }

    var p2 = {
        toString() {
            console.log('called toString');
            return 3;
        },
    }

    var obj = {
        __proto__: p2,
    };

    console.log(obj + 1);
}
f();
/* OUTPUT
called Object.prototype.valueOf
called toString
4
*/


// Index: 13
// Object.valueOf
function f() {
    var p1 = {
        valueOf() {
            console.log('called valueOf');
            return new Number(14);
        }
    };

    var p2 = {
        toString() {
            console.log('called toString');
            return 3;
        },
        __proto__: p1,
    }

    var obj = {
        __proto__: p2,
    };

    console.log(obj + 1);
}
f();
/* OUTPUT
called valueOf
called toString
4
*/


// Index: 14
// Object.valueOf
function f() {
    var p1 = {
        valueOf() {
            console.log('called valueOf');
            return undefined;
        }
    };

    var p2 = {
        toString() {
            console.log('called toString');
            return 1;
        },
        __proto__: p1,
    }

    var obj = {
        __proto__: p2,
    };

    console.log(obj + 1);
}
f();
/* OUTPUT
called valueOf
NaN
*/


// Index: 15
// Object.valueOf
function f() {
    console.log(1, new Number(1) + 5);
    console.log(2, new Boolean(true) + 15);
    console.log(3, new String('1') + 6);
    console.log(1, {} + 5);
}
f();
/* OUTPUT
1 6
2 16
3 16
1 [object Object]5
*/


// Index: 16
// Object.values
function f() {
    try {
        console.log(1, Object.values());
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(2, Object.values(null));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    try {
        console.log(3, Object.values(undefined));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    console.log(4, Object.values(1));
    console.log(5, Object.values(true));
    console.log(6, Object.values(false));
    console.log(7, Object.values('x'));
    console.log(8, Object.values('xy'.charAt(1)));
    console.log(9, Object.values(Symbol()));

    var o = {'a': 'a1', '1': 'b1'};
    console.log(9, Object.values(o));

    o = ['a1', 'a2', 3];
    o.x = 'x1';
    console.log(9, Object.values(o));
}
f();
/* OUTPUT
TypeError: Cannot convert undefined or null to object
TypeError: Cannot convert undefined or null to object
TypeError: Cannot convert undefined or null to object
4 []
5 []
6 []
7 [x]
8 [y]
9 []
9 [b1, a1]
9 [a1, a2, 3, x1]
*/

