// Index: 0
// apply
function f() {
    var o = function () {};
    console.log(o.name, o.length);

    var a = 'abc';

    o = {
        z : function () {},
        get a() { },
        set a(p) { },
        [a]: function() {},
    };
    var x = o.y = function () {};
    console.log(x.name, x.length);
    console.log(o.z.name, o.z.length);

    var d = Object.getOwnPropertyDescriptor(o, 'a');
    console.log(d.get.name, d.set.name);
    console.log(o.abc.name);
}
f();
/* OUTPUT-FIXED
o 0
 0
z 0
get a set a
[a]
*/


// Index: 1
// apply
function f() {
    var a = [];
    Object.defineProperty(a, 0, { get: function() {
        console.log('get 0');
        return '1';
    }, set: function(p) {
        console.log('set 0');
    }});
    Object.defineProperty(a, 1, { get: function() {
        console.log('get 1');
        return '2';
    }, set: function(p) {
        console.log('set 1');
    }});

    function g() {
        //console.log('g:', arguments[0]);
    }

    g.apply(null, a);
}
f();
/* OUTPUT
get 0
get 1
*/


// Index: 2
function f() {
    globalThis.xyz = 'xyz-value';
    function g() {
        console.log(this.xyz);
    }

    g();
}
f();
/* OUTPUT
xyz-value
*/


// Index: 3
// bind
function f() {
    const module = {
        x: 42,
        getX: function() {
            return this.x;
        }
    };

    const unboundGetX = module.getX;
    console.log(unboundGetX()); // The function gets invoked at the global scope
    // expected output: undefined
    
    var boundGetX = unboundGetX.bind(module);
    console.log(boundGetX.name);
    console.log(boundGetX());
    // expected output: 42

    boundGetX = boundGetX.bind();
    console.log(boundGetX.name);
    console.log(boundGetX());
    // expected output: 42

    boundGetX = unboundGetX.bind();
    console.log(boundGetX());
}
f();
/* OUTPUT
undefined
bound getX
42
bound bound getX
42
undefined
*/


// Index: 4
// bind
function f() {
    var a = ['1-1', 2];

    var at = Array.prototype.at.bind(a);
    console.log(at(0));
}
f();
/* OUTPUT
1-1
*/


// Index: 5
// bind
function f() {
    var a = Function.prototype.bind.call(Array);
    console.log(a() instanceof Array);
}
f();
/* OUTPUT
true
*/


// Index: 6
// bind
function f() {
    Function.prototype.bind.call(1);
}
f();
/* OUTPUT
Uncaught TypeError: Bind must be called on a function
*/


// Index: 7
// bind
function f() {
    const module = {
        x: 42,
        getX: function() {
            this.yy = 'yy';
            return this.x;
        }
    };

    const unboundGetX = module.getX;
    
    var boundGetX = unboundGetX.bind(module);

    var o = new boundGetX();
    console.log(o.yy);
}
f();
/* OUTPUT
yy
*/


// Index: 8
// call
function f() {
    const module = {
        x: 42,
        getX: function(p) {
            this.yy = 'yy';
            console.log('P: ', p);
            return this.x;
        }
    };

    var getX = module.getX;
    
    console.log(getX.call(module, 'p1'));
}
f();
/* OUTPUT
P:  p1
42
*/


// Index: 9
function f() {
    function isNative(Ctor) {
        console.log(typeof Ctor === 'function');
        console.log(Ctor.toString());
        return typeof Ctor === 'function' && /native code/.test(Ctor.toString());
    }

    console.log(isNative(Symbol));
    console.log(isNative(Number));
    console.log(isNative(String));
    console.log(isNative(Reflect));
}
f();
/* OUTPUT
true
function Symbol() { [native code] }
true
true
function Number() { [native code] }
true
true
function String() { [native code] }
true
false
[object Reflect]
false
*/
