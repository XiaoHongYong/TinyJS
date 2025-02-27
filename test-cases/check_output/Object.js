// Index: 0
// 测试修改 String.prototype
function f2() {
    var a = 'abc';
    Object.defineProperty(String.prototype, 'x', {
        get: function() {
            console.log('get');
        },
        set: function (v) {
            console.log('set', v);
        },
    });

    Object.defineProperty(String.prototype, 'y', {
        value: function (a) {
            console.log('y', a);
        }
    });

    console.log(a.x);
    a.x = 3;
    console.log(a.y);
}
f2();
/* OUTPUT
get
undefined
set 3
function (a) {
            console.log('y', a);
        }
*/


// Index: 1
// function 的 prototype.__proto__ 和 Object.prototype 相同
function f21() {
    function g() { this.a = 1; }

    console.log(g.prototype.__proto__.toString());
    console.log(g.prototype.__proto__ === Object.prototype);
}
f21();
/* OUTPUT
[object Object]
true
*/


// Index: 2
// 重新定义 g.prototype.__proto__
function f3() {
    function g() { this.a = 1; this.x = 5; }
    g.prototype = {
        set x(p) {
            console.log('set x via prototype: ', p);
        },
        get x() { return 'x'; },
    }

    g.prototype.__proto__ = {
        set y(p) {
            console.log('set y via prototype: ', p);
        },
        get y() { return 'y'; },
    }

    var obj = new g();
    obj.x = 2;
    obj.y = 3;

    console.log(obj.x, obj.y);
}
f3();
/* OUTPUT
set x via prototype:  5
set x via prototype:  2
set y via prototype:  3
x y
*/


// Index: 3
// 本身有属性，会优先修改本身的，再往 prototype 链上查找
function f31() {
    var proto1 = {
        set x(p) {
            console.log('set x via prototype: ', p);
        },
        get x() { return 'x'; },
    }

    proto1.__proto__ = {
        set y(p) {
            console.log('set y via prototype: ', p);
        },
        get y() { return 'y'; },
    }

    var obj = {
        x: 5,
    };
    obj.__proto__ = proto1;

    obj.x = 2;
    obj.y = 3;

    console.log(obj.x, obj.y);
}
f31();
/* OUTPUT
set y via prototype:  3
2 y
*/


// Index: 4
// proto 只有 getter，无 setter，设置 obj 属性: 不会创建新的属性
function f32() {
    var proto = {
        get y() { return 'y'; },
    }

    var obj = {
        __proto__: proto
    };

    console.log(obj.y);

    obj.y = 3;

    console.log(obj.y, proto.y);
}
f32();
/* OUTPUT
y
y y
*/


// Index: 5
// defineProperty: 如果一个描述符同时拥有 value 或 writable 和 get 或 set 键，则会产生一个异常。
function f4a(p) {
    var o = {};

    console.log(p);
    try {
        if (p == 1) {
            // TypeError: Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>
            Object.defineProperty(o, 'x', {
                get: function() {
                    console.log('get x');
                    return 'x';
                },
                writable: false,
            });
        } else if (p == 2) {
            // TypeError: Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>
            Object.defineProperty(o, 'x', {
                set: function(x) {
                    console.log('set x');
                },
                writable: false,
            });
        } else if (p == 3) {
            // TypeError: Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>
            Object.defineProperty(o, 'x', {
                get: function() {
                    console.log('get x');
                    return 'x';
                },
                value: 1,
            });
        } else if (p == 4) {
            // TypeError: Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>
            Object.defineProperty(o, 'x', {
                set: function(x) {
                    console.log('set x');
                },
                value: 2,
            });
        } else if (p == 5) {
            // TypeError: Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>
            Object.defineProperty(o, 'x', {
                set: undefined,
                value: 2,
            });
        }
    } catch (error) {
        console.log(error.name + ': ' + error.message);
    }
}
f4a(1);
f4a(2);
f4a(3);
f4a(4);
f4a(5);
/* OUTPUT
1
TypeError: Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>
2
TypeError: Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>
3
TypeError: Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>
4
TypeError: Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>
5
TypeError: Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>
*/


// Index: 6
// 测试 defineProperty， setter, getter的有效性
function f4b(p) {
    console.log(p);
    var obj = {};
    try {
        if (p == 1) {
            Object.defineProperty(obj, 'x', {
                set: 'g',
            });
        } else if (p == 2) {
            Object.defineProperty(obj, 'x', {
                get: 1
            });
        } else if (p == 3) {
            Object.defineProperty(obj, 'x', {
                set: null
            });
        } else if (p == 4) {
            Object.defineProperty(obj, 'x', {
                get: null
            });
        } else if (p == 5) {
            Object.defineProperty(obj, 'x', {
                set: undefined
            });
        } else if (p == 6) {
            Object.defineProperty(obj, 'x', {
                get: undefined
            });
        }
    } catch (error) {
        console.log(error.name + ': ' + error.message);
    }
}
f4b(1);
f4b(2);
f4b(3);
f4b(4);
f4b(5);
f4b(6);
/* OUTPUT
1
TypeError: Setter must be a function: g
2
TypeError: Getter must be a function: 1
3
TypeError: Setter must be a function: null
4
TypeError: Getter must be a function: null
5
6
*/


// Index: 7
// defineProperty: 的各种属性
function f4() {
    function g() {
        this.a = 'a';
    }
    var o = new g();

    // 直接添加的 prototye 的属性是: configurable: true, enumerable: true, writable: true
    g.prototype.b = 1;

    console.log(1, Object.getOwnPropertyDescriptor(o, 'b'));

    Object.defineProperty(g.prototype, 'b', {
        value: 2,
    });
    console.log(2, o.b);

    Object.defineProperty(g.prototype, 'b', {
        value: 3,
    });
    console.log(3, o.b);
    console.log(4, Object.getOwnPropertyDescriptor(o, 'b'));

    // defineProperty 添加的 prototye 的属性是: configurable: false, enumerable: false, writable: false
    Object.defineProperty(g.prototype, 'c', {
        value: 4,
    });
    console.log(5, o.c);
    console.log(6, Object.getOwnPropertyDescriptor(o, 'b'));

    // 相同的值不会抛异常, writable 缺省为 false
    Object.defineProperty(g.prototype, 'c', {
        value: 4,
        writable: false,
    });
    console.log(7, o.c);

    // 修改不生效
    o.c = 6;
    console.log(8, o.c);

    try {
        Object.defineProperty(g.prototype, 'c', {
            value: 5,
        });
        console.log(9, o.c);
    } catch (error) {
        // 不同的值抛出异常
        // TypeError: Cannot redefine property: c
        console.log(10, error.toString());
    }

    console.log(11, delete o.a); // 可以删除
    console.log(12, delete o.b); // 可以删除
    console.log(13, delete o.c); // 无法删除

    // o.b 访问的是 prototype 中的属性，所以还在
    console.log(14, o.a, o.b, o.c);
}
f4();
/* OUTPUT
1 undefined
2 2
3 3
4 undefined
5 4
6 undefined
7 4
8 4
10 TypeError: Cannot redefine property: c
11 true
12 true
13 true
14 undefined 3 4
*/


// Index: 8
// 测试 prototype 的修改
function f8() {
    function g() {
        this.a = 1;
    }

    console.log(delete g.prototype); // false ，不能删除

    var o = new g();
    console.log(o.__proto__ === Object.prototype); // false
    console.log(o.__proto__ === g.prototype); // true

    g.prototype = 1;
    console.log(g.prototype);

    var o = new g();
    console.log(o.__proto__ === Object.prototype); // 缺省恢复为 Object.prototype

    // 抛出异常: TypeError: Cannot redefine property: prototype
    Object.defineProperty(g, 'prototype', {
        get: function() {
            console.log('get prototype');
            return proto;
        },
    });
}
f8();
/* OUTPUT
false
false
true
1
true
Uncaught TypeError: Cannot redefine property: prototype
*/


// Index: 9
// prototype 的 configurable: false, writable: false. 继承的 obj 不能 set 值，可以通过 defineProperty 定义同名的属性
function f81() {
    var proto = {};

    Object.defineProperty(proto, 'x', {
        configurable: false,
        value: 1,
        writable: false,
    });

    var o = { __proto__ : proto };

    console.log(1, o.x, delete o.x); //

    o.x = 2;
    console.log(2, o.x); //

    Object.defineProperty(o, 'x', {
        configurable: true,
        value: 2,
        writable: true,
    });

    console.log(3, o.x, delete o.x, o.x); //
}
f81();
/* OUTPUT
1 1 true
2 1
3 2 true 1
*/


// Index: 10
// prototype 的 configurable: false, writable: false. 继承的 obj 不能 set 值，可以通过 defineProperty 定义同名的属性
function f82(a) {
    Object.defineProperty(arguments, '0', {
        value: 1,
        //writable : false,
    });

    // Object.defineProperty(arguments, '0', {
    //     get: function() {
    //         return 10;
    //     }
    // });

    console.log(1, a, arguments[0]);

    arguments[0] = 5;
    console.log(2, a, arguments[0]);

    // 不支持同步修改到 arguments 了
    a = 3;
    console.log(3, a);// , arguments[0]);

    arguments[0] = 6;
    console.log(4, a, arguments[0]);

    delete arguments[0];

    console.log(5, a, arguments[0]);
}
f82(0);
/* OUTPUT
1 1 1
2 5 5
3 3
4 6 6
5 6 undefined
*/


// Index: 11
// 测试 prototype 上定义的属性修改
function f9() {
    function g() {
        this.a = 1;
    }

    g.prototype = { };

    var obj = new g();

    Object.defineProperty(g.prototype, 'x', {
        value: 'x',
        writable: true,
        configurable: true,
    });
    console.log('1', obj.x);
    console.log('2', delete obj.x);
    console.log('3', obj.x);
    obj.x = 1;
    console.log('4', obj.x);

    console.log('5', delete obj.x);
    console.log('6', obj.x);
}
f9();
/* OUTPUT
1 x
2 true
3 x
4 1
5 true
6 x
*/


// Index: 12
// Object.defineProperty 删除 setter.
function f10(p) {
    function g() {
        this.a = 1;
    }

    if (p == 1) {
        Object.defineProperty(g.prototype, 'x', {
            get: function() { return 'gx'; },
            set: function (v) { console.log('set x'); },
            configurable: false,
        });
    } else if (p == 2) {
        Object.defineProperty(g.prototype, 'x', {
            value: 'px',
            // get: function() { return 'gx'; },
            // set: function (v) { },
            configurable: false,
            writable: false,    
        });
    }

    var obj = new g();
    console.log('1', delete obj.x);
    console.log('2', obj.x);

    console.log('3', delete g.prototype.x);
    console.log('4', obj.x);

    console.log('5', obj.xdxde);
}
f10(1);
f10(2);
/* OUTPUT
1 true
2 gx
3 false
4 gx
5 undefined
1 true
2 px
3 false
4 px
5 undefined
*/


// Index: 13
// 测试 Object.defineProperty 先 write prototype 上的 unconfigurable && writable 属性,
// 再 config 此属性
function f11() {
    function g() {
        this.a = 1;
    }
    Object.defineProperty(g.prototype, 'x', {
        value: 'px',
        // get: function() { return 'gx'; },
        // set: function (v) { },
        configurable: false,
        writable: false,
    });

    var obj = new g();
    console.log(obj.x);

    obj.x = 2;
    console.log(obj.x);

    Object.defineProperty(obj, 'x', {
        value: 3,
    });

    console.log(obj.x);
}
f11();
/* OUTPUT
px
px
3
*/


// Index: 14
// Object.defineProperty 先定义 getter，再定义setter
// 结论：用新的部分值覆盖旧的，但是新的未定义的，还是使用老的
function f12() {
    var o = { get x() { console.log('get x1'); return 1; }, set x(a) { console.log('set x1'); } };
    // var o = { x: 'x' };

    console.log(0, o.x);
    Object.defineProperty(o, 'x', {
        get: function() { return 'gx2'; },
        // set: function (v) { console.log('sx2'); },
        // value: 2,
        configurable: true,
    });

    o.x = 3;
    console.log('1', o.x);

    o.x = 1;

    Object.defineProperty(o, 'x', {
        set: function (v) { console.log('set x3'); },
        configurable: true,
    });

    o.x = 3;

    console.log('2', o.x);
}
f12();
/* OUTPUT
get x1
0 1
set x1
1 gx2
set x1
set x3
2 gx2
*/


// Index: 15
// Object.defineProperty 不带 value，和 value 为 undefined 情况
function f13() {
    var o = {  x: 1 };
    Object.defineProperty(o, 'x', {
        writable: true,
        enumerable: true,
        configurable: true,
    });

    console.log(1, o.x);

    Object.defineProperty(o, 'x', {
        writable: true,
        enumerable: true,
        configurable: true,
        value: undefined,
    });

    console.log(2, o.x);
}
f13();
/* OUTPUT
1 1
2 undefined
*/


// Index: 16
// Object.defineProperty 会不会修改之前的 writable
function f14() {
    var o = {  x: 1 };
    Object.defineProperty(o, 'x', {
    });

    console.log(Object.getOwnPropertyDescriptor(o, 'x'));

    console.log(1, o.x);

    Object.defineProperty(o, 'x', {
        writable: false,
    });

    console.log(Object.getOwnPropertyDescriptor(o, 'x'));

    console.log(2, o.x);
}
f14();
/* OUTPUT
{configurable: true, enumerable: true, value: 1, writable: true}
1 1
{configurable: true, enumerable: true, value: 1, writable: false}
2 1
*/


// Index: 17
//
// 测试 Array 的 Object.defineProperty
function f6() {
    var a = [];

    Object.defineProperty(a, 1, {
        get: function() {
            console.log('get 1');
            return 'x';
        },
        // enumerable: true,
        //writable: true,
    });

    Object.defineProperty(a, 'y', {
        set: function (v) {
            console.log('set y', v);
            this._y = v;
        },
        enumerable: true,
    });

    console.log(a[1], a.y);
    a.y = 3;
}
f6();
/* OUTPUT
get 1
x undefined
set y 3
*/


// Index: 18
// 测试 Arguments 的 Object.defineProperty
function f61(a, b, c) {
    Object.defineProperty(arguments, 1, {
        get: function() {
            console.log('get 1');
            return 'x';
        },
        // enumerable: true,
        //writable: true,
    });

    Object.defineProperty(arguments, 'y', {
        set: function (v) {
            console.log('set y', v);
            this._y = v;
        },
        enumerable: true,
    });

    console.log(arguments[1], arguments.y, b);
    arguments.y = 3;
}
f61(1, 2, 3);
/* OUTPUT
get 1
x undefined 2
set y 3
*/


// Index: 19
// 测试 prototype 的修改
function f() {
    var a = {};
    Object.preventExtensions(a);
    Object.defineProperty(a, 'x', { value: 1});
}
f();
/* OUTPUT
Uncaught TypeError: Cannot define property x, object is not extensible
*/


// Index: 20
// 测试 prototype 的修改
function f() {
    var a = {};
    Object.defineProperty(a, 'x', { value: 2});
    Object.defineProperty(a, 'x', { value: 2});
    console.log('#1');
    Object.defineProperty(a, 'x', { value: 1});
}
f();
/* OUTPUT
#1
Uncaught TypeError: Cannot redefine property: x
*/


// Index: 21
// String
function f() {
    var a = new String('abc');
    a.y = 2;

    for (var i in a) {
        console.log('#1', i, a[i]);
    }

    for (var i of a) {
        console.log('#2', i);
    }
}
f();
/* OUTPUT
#1 0 a
#1 1 b
#1 2 c
#1 y 2
#2 a
#2 b
#2 c
*/


// Index: 22
// String
function f() {
    var proto = { 'x': 1 };
    var o = { __proto__: proto };

    console.log('#1', proto.propertyIsEnumerable('x'));
    console.log('#2', o.propertyIsEnumerable('x'));
}
f();
/* OUTPUT
#1 true
#2 false
*/

