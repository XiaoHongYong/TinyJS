
function f1() {
    g(1);
    g(g=2);
    g(3); // TypeError: g is not a function

    function g(p) {
        console.log(p);
    }
}

f1();


function f2(c) {
    if (c) {
        const a = 1;
        a = 2;
    }
}

f2(0);
f2(1);

function f3(c, d) {
    if (c) {
        if (d)
            a = 2;

        let a = 1;
        a = 3;
    }
}

f3(0);
f3(1);
f3(1, 1);

function f4() {
    var obj = {
        f(p) {
            console.log(p);
        }
    }

    obj.f(obj.f = 2);

    console.log(obj.f + 1);
}
f4();

//// 测试函数声明和变量声明同时存在的情况
function f5() {
    console.log(g);

    var g = 1;

    console.log(g);

    function g() {

    }

    console.log(g);
}
f5();

//// 测试只有函数声明，但是对函数名赋值
function f6() {
    !!function() {
        g = 1;

        console.log(g);

        {
            function g() {

            }
        }
        console.log(g);
    }();

    console.log(typeof g);
}
f6();

//// 测试有函数声明，也有变量声明，但是没有赋值的情况
function f7() {
    console.log(g);

    var g;

    console.log(g);

    function g() {
    }

    console.log(g);
}
f7();

//// 修改函数表达式的函数名
function f8() {
    var g = function k() {
        console.log(k);
        k = 1; //// 对k 的修改无效
        console.log('after k = 1', k);
    };

    g();

    console.log(typeof k);
}
f8();

//// 函数表达式的函数名，有同名变量
function f9() {
    var g = function k() {
        console.log(k); //// k 不再是函数
        var k = 1;
        console.log('after k = 1', k);
    };

    g();

    console.log(typeof k);
}
f9();

//// 同名函数的覆盖情况
function f10() {
    function g() {
        console.log('g1');
    };
    g();

    function g() {
        console.log('g2');
    };

    g();
}
f10();
