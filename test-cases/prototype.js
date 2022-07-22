
//// prototype 有先后顺序，后面对 prototype 的修改不再影响到已经创建的对象.
function f1(p) {
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
f1(1);
f1(0);


//// prototype 的多层继承
function f2() {
    function g() {
        this.x = 'x';
    }

    g.prototype = {
        a : 1,
    };

    g.prototype.__proto__ = {
        b : 2,
    };

    var obj = new g(p);
    console.log(obj.x, obj.a, obj.b);
}
f2();


function f3() {
    function g() {
        this.x = 'x';
    }

    g.prototype = String.prototype;

    var obj = new g(p);
    console.log(obj.x, obj.charCodeAt);
    console.log(obj.charCodeAt(1));
}
f3();
