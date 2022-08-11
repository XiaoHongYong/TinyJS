function f1() {
    var obj = { x : 1, y : 2 };
    Object.defineProperty(obj, 'x', {
        get: function() {
            console.log('get');
        },
        set: 1
    });

    Object.defineProperty(obj, 'y', {
        value: function (a) {
            console.log('y', a);
        }
    });

    console.log(obj.x, obj.y);
}
f1();

//// 
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
    console.log(a.x);
}
f2();

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

//// 本身有属性，会优先修改本身的，再往 prototype 链上查找
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


function f4() {
    function g() {
        this.a = 'a';
    }

    g.prototype.b = 1;

    Object.defineProperty(g.prototype, 'x', {
        get: function() {
            console.log('get x');
            return 'x';
        },
        // enumerable: true,
        //writable: true,
    });

    Object.defineProperty(g.prototype, 'y', {
        set: function (v) {
            console.log('set y', v);
            this._y = v;
        },
        enumerable: true,
    });

    Object.defineProperty(g.prototype, 'z', {
        value: 'z',
        enumerable: true,
        configurable: true,
    });

    var a = new g();

    delete a.a; //// 可以删除
    delete a.x; //// 无法删除
    delete a.z; //// 无法删除

    console.log(a.x, a.z);
    a.x = 3;
    console.log(a.x);
    a.y = 2;
    console.log(a.y);
}
f4();

function f5() {
    var a = {
        a: 1,
    };

    Object.defineProperty(a, 'x', {
        get: function() {
            console.log('get x');
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

    Object.defineProperty(a, 'z', {
        value: 'z',
        enumerable: true,
        configurable: true,
    }, 4);

    // delete a.a; //// 可以删除
    // delete a.x; //// 无法删除
    // delete a.z; //// 无法删除

    console.log(a.x, a.z);
    a.x = 3;
    console.log(a.x);
    a.y = 2;
    console.log(a.y, a.z);
}
f5();


//// 测试 Array 的 Object.defineProperty
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

//// 测试 Arguments 的 Object.defineProperty
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


//// 测试 entries
function f7() {
    function g() {
        this.a = 1;
    }

    g.prototype = {
        b: 2,
    };

    Object.defineProperty(g.prototype, 'c', {
        get: function() {
            console.log('get 1');
            return 'x';
        },
    });

    var a = new g();;

    console.log(Object.entries(a));
}
f7();

