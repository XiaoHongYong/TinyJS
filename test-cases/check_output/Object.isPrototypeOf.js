// Index: 0
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

function f1(r, p, noPrint) {
    console.log('round: ', r, noPrint ? '' : p);

    try {
        console.log(p.isPrototypeOf('x'));
        console.log(p.isPrototypeOf(1));
        console.log(p.isPrototypeOf(undefined));
        console.log(p.isPrototypeOf(null));
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
TypeError: Cannot read properties of undefined (reading 'isPrototypeOf')
round:  2 null
TypeError: Cannot read properties of null (reading 'isPrototypeOf')
round:  3 NaN
false
false
false
false
round:  4 Infinity
false
false
false
false
round:  5 -Infinity
false
false
false
false
round:  6 0
false
false
false
false
round:  7 1
false
false
false
false
round:  8 true
false
false
false
false
round:  9 
false
false
false
false
round:  9a 0
false
false
false
false
round:  9b 2
false
false
false
false
round:  10 
false
false
false
false
round:  11 
false
false
false
false
round:  12 
false
false
false
false
round:  13 /a/
false
false
false
false
round:  14 Symbol()
false
false
false
false
*/


// Index: 1
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

function f(r, p) {
    console.log('== Round: ', r, p.name);

    console.log('1', p.prototype.isPrototypeOf(undefined));
    console.log('2', p.prototype.isPrototypeOf(null));
    console.log('3', p.prototype.isPrototypeOf(NaN));
    console.log('4', p.prototype.isPrototypeOf(Infinity));
    console.log('5', p.prototype.isPrototypeOf(-Infinity));
    console.log('6', p.prototype.isPrototypeOf(0));
    console.log('7', p.prototype.isPrototypeOf(1.0));
    console.log('8', p.prototype.isPrototypeOf(true));
    console.log('9', p.prototype.isPrototypeOf(''));
    console.log('9a', p.prototype.isPrototypeOf('0'));
    console.log('9b', p.prototype.isPrototypeOf('123'.charAt(1)));
    console.log('10', p.prototype.isPrototypeOf(g));
    console.log('11', p.prototype.isPrototypeOf(obj1));
    console.log('12', p.prototype.isPrototypeOf(obj2));
    console.log('13', p.prototype.isPrototypeOf(/a/));
    console.log('14', p.prototype.isPrototypeOf(Symbol()));
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
function f() {
    function Foo() {}
    function Bar() {}
    
    Bar.prototype = Object.create(Foo.prototype);
    
    const bar = new Bar();
    
    console.log(Foo.prototype.isPrototypeOf(bar)); // expected output: true
    console.log(Bar.prototype.isPrototypeOf(bar)); // expected output: true
}
f();
/* OUTPUT
true
true
*/


// Index: 3
// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/isPrototypeOf
function f() {
    function Foo() {}
    function Bar() {}
    function Baz() {}

    Bar.prototype = Object.create(Foo.prototype);
    Baz.prototype = Object.create(Bar.prototype);

    const foo = new Foo();
    const bar = new Bar();
    const baz = new Baz();

    // prototype chains:
    // foo: Foo <- Object
    // bar: Bar <- Foo <- Object
    // baz: Baz <- Bar <- Foo <- Object
    console.log(Baz.prototype.isPrototypeOf(baz));    // true
    console.log(Baz.prototype.isPrototypeOf(bar));    // false
    console.log(Baz.prototype.isPrototypeOf(foo));    // false
    console.log(Bar.prototype.isPrototypeOf(baz));    // true
    console.log(Bar.prototype.isPrototypeOf(foo));    // false
    console.log(Foo.prototype.isPrototypeOf(baz));    // true
    console.log(Foo.prototype.isPrototypeOf(bar));    // true
    console.log(Object.prototype.isPrototypeOf(baz)); // true
}
f();
/* OUTPUT
true
false
false
true
false
true
true
true
*/

