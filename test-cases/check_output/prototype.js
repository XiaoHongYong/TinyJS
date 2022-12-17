// Index: 0
//// prototype 有先后顺序，后面对 prototype 的修改不再影响到已经创建的对象.
function f(p) {
    var obj = new g(p);
    console.log(obj.x, obj.a, obj.b);

    function g(p) {
        this.x = p + 1;
    }

    if (p == 1) {
        g.prototype = {
            a : 1,
        }
    } else {
        g.prototype = {
            b : 2,
        }
    }

    g.prototype.c = 3;

    console.log(obj.x, obj.a, obj.b, obj.c);

    obj = new g(p);
    console.log('new-again:', obj.x, obj.a, obj.b, obj.c);

    g.prototype.d = 4;
    console.log('d:', obj.d);
}
f(1);
f(0);
/* OUTPUT
2 undefined undefined
2 undefined undefined undefined
new-again: 2 1 undefined 3
d: 4
1 undefined undefined
1 undefined undefined undefined
new-again: 1 undefined 2 3
d: 4
*/


// Index: 1
//// prototype 的多层继承
function f() {
    function g() {
        this.x = 'x';
        this.b = 'b2';
    }

    g.prototype = {
        a : 1,
        b : 'b1',
        c() { return this.b; }
    };

    g.prototype.__proto__ = {
        d : 2,
        b: 'b0',
        c() { return this.b; }
    };

    var obj = new g();
    console.log('#1', obj.x, obj.a, obj.d, obj.c());

    console.log('#2', 'x' in obj, 'a' in obj, 'c' in obj, 'd' in obj);

    console.log('#3', obj.hasOwnProperty('x'), obj.hasOwnProperty('a'), obj.hasOwnProperty('c'), obj.hasOwnProperty('d'));

}
f();
/* OUTPUT
#1 x 1 2 b2
#2 true true true true
#3 true false false false
*/


// Index: 2
function f() {
    function g() {
        this.x = 'x';
    }

    g.prototype = String.prototype;

    var obj = new g();
    console.log(obj.x, obj.charCodeAt);
    console.log(obj.charCodeAt(1));
}
f();
/* OUTPUT-DISABLED
x function charCodeAt() { [native code] }
Uncaught TypeError: String.prototype.toString requires that 'this' be a String
*/

