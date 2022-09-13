// Index: 0
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 这些都会跑出异常
function f1(r, p, noPrint) {
    console.log('round: ', r, noPrint ? '' : p);

    try {
        console.log(1 instanceof p);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f1('1', undefined);
f1('2', null);
f1('3', NaN);
f1('4', Infinity);
f1('5', -Infinity);
f1('6', 0);
f1('7', 1.0);
f1('8', true);
f1('9', '');
f1('9a', '0');
f1('9b', '123'.charAt(1));
f1('10', g, true);
f1('11', obj1, true);
f1('12', obj2, true);
f1('13', /a/);
f1('14', Symbol());
/* OUTPUT
round:  1 undefined
TypeError: Right-hand side of 'instanceof' is not an object
round:  2 null
TypeError: Right-hand side of 'instanceof' is not an object
round:  3 NaN
TypeError: Right-hand side of 'instanceof' is not an object
round:  4 Infinity
TypeError: Right-hand side of 'instanceof' is not an object
round:  5 -Infinity
TypeError: Right-hand side of 'instanceof' is not an object
round:  6 0
TypeError: Right-hand side of 'instanceof' is not an object
round:  7 1
TypeError: Right-hand side of 'instanceof' is not an object
round:  8 true
TypeError: Right-hand side of 'instanceof' is not an object
round:  9 
TypeError: Right-hand side of 'instanceof' is not an object
round:  9a 0
TypeError: Right-hand side of 'instanceof' is not an object
round:  9b 2
TypeError: Right-hand side of 'instanceof' is not an object
round:  10 
false
round:  11 
TypeError: Right-hand side of 'instanceof' is not callable
round:  12 
TypeError: Right-hand side of 'instanceof' is not callable
round:  13 /a/
TypeError: Right-hand side of 'instanceof' is not callable
round:  14 Symbol()
TypeError: Right-hand side of 'instanceof' is not an object
*/


// Index: 1
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

function f(r, p) {
    console.log('== Round: ', r, p.name);

    console.log('1', undefined instanceof p);
    console.log('2', null instanceof p);
    console.log('3', NaN instanceof p);
    console.log('4', Infinity instanceof p);
    console.log('5', -Infinity instanceof p);
    console.log('6', 0 instanceof p);
    console.log('7', 1.0 instanceof p);
    console.log('8', true instanceof p);
    console.log('9', '' instanceof p);
    console.log('9a', '0' instanceof p);
    console.log('9b', '123'.charAt(1) instanceof p);
    console.log('10', g instanceof p);
    console.log('11', obj1 instanceof p);
    console.log('12', obj2 instanceof p);
    console.log('13', /a/ instanceof p);
    console.log('14', Symbol() instanceof p);
}
f(1, Number);
f(2, String);
f(3, Boolean);
f(4, Function);
f(5, Object);
f(6, RegExp);
f(7, Symbol);
/* OUTPUT
== Round:  1 Number
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
9a false
9b false
10 false
11 false
12 false
13 false
14 false
== Round:  2 String
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
9a false
9b false
10 false
11 false
12 false
13 false
14 false
== Round:  3 Boolean
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
9a false
9b false
10 false
11 false
12 false
13 false
14 false
== Round:  4 Function
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
9a false
9b false
10 true
11 false
12 false
13 false
14 false
== Round:  5 Object
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
9a false
9b false
10 true
11 true
12 true
13 true
14 false
== Round:  6 RegExp
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
9a false
9b false
10 false
11 false
12 false
13 true
14 false
== Round:  7 Symbol
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
9 false
9a false
9b false
10 false
11 false
12 false
13 false
14 false
*/


// Index: 2
// From: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/instanceof
function f() {
    // defining constructors
    function C() {}
    function D() {}

    const o = new C();

    // true, because: Object.getPrototypeOf(o) === C.prototype
    console.log(o instanceof C);

    // false, because D.prototype is nowhere in o's prototype chain
    console.log(o instanceof D);

    console.log(o instanceof Object); // true, because:
    console.log(C.prototype instanceof Object); // true

    // Re-assign `constructor.prototype`: you should
    // rarely do this in practice.
    C.prototype = {};
    const o2 = new C();

    console.log(o2 instanceof C); // true

    // false, because C.prototype is nowhere in
    // o's prototype chain anymore
    console.log(o instanceof C);

    D.prototype = new C(); // add C to [[Prototype]] linkage of D
    const o3 = new D();
    console.log(o3 instanceof D); // true
    console.log(o3 instanceof C); // true since C.prototype is now in o3's prototype chain
}
f();
/* OUTPUT
true
false
true
true
true
false
true
true
*/


// Index: 3
// From: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/instanceof
// function f() {
//     class A {}
//     class B extends A {}

//     const o1 = new A();
//     // true, because Object.getPrototypeOf(o1) === A.prototype
//     o1 instanceof A;
//     // false, because B.prototype is nowhere in o1's prototype chain
//     o1 instanceof B;

//     const o2 = new B();
//     // true, because Object.getPrototypeOf(Object.getPrototypeOf(o2)) === A.prototype
//     o2 instanceof A;
//     // true, because Object.getPrototypeOf(o2) === B.prototype
//     o2 instanceof B;
// }

// From: https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/instanceof
function g() { }
function f(r, p) {
    console.log('== Round:', r, p);

    console.log('1', p instanceof Boolean);
    console.log('2', p instanceof Number);
    console.log('3', p instanceof String);
    console.log('4', p instanceof Object);
    console.log('5', p instanceof Array);
    console.log('6', p instanceof Function);
    console.log('7', p instanceof RegExp);
    console.log('8', p instanceof Symbol);
}
f(1, new Boolean(true));
f(2, new Number(1));
f(3, new String('abc'));
f(4, []);
f(5, {});
f(6, g);
f(7, new g());
f(8, /a/i);
f(9, Symbol());
f(10, true);
f(11, 12);
f(12, 'xy');
/* OUTPUT
== Round: 1 true
1 true
2 false
3 false
4 true
5 false
6 false
7 false
8 false
== Round: 2 1
1 false
2 true
3 false
4 true
5 false
6 false
7 false
8 false
== Round: 3 abc
1 false
2 false
3 true
4 true
5 false
6 false
7 false
8 false
== Round: 4 
1 false
2 false
3 false
4 true
5 true
6 false
7 false
8 false
== Round: 5 {  }
1 false
2 false
3 false
4 true
5 false
6 false
7 false
8 false
== Round: 6 function g() { }
1 false
2 false
3 false
4 true
5 false
6 true
7 false
8 false
== Round: 7 {  }
1 false
2 false
3 false
4 true
5 false
6 false
7 false
8 false
== Round: 8 /a/i
1 false
2 false
3 false
4 true
5 false
6 false
7 true
8 false
== Round: 9 Symbol()
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
== Round: 10 true
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
== Round: 11 12
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
== Round: 12 xy
1 false
2 false
3 false
4 false
5 false
6 false
7 false
8 false
*/

