
function f1() {
    console.log(new g(1).x);

    function g(p) {
        console.log(p);
        this.x = p;
    }
}

f1();


function f2() {
    g = function(p) {
        console.log(p);
        this.x = p + 1;
    }

    console.log(new g(2).x);

    function g(p) {
        console.log(p);
        this.x = p;
    }
}

f2();


function f3() {
    var g = function(p) {
        console.log(p);
        this.x = p + 1;
    }

    console.log(new g(3).x);
}

f3();

function f4() {
    var g;
    g = function(p) {
        console.log(p);
        this.x = p + 1;
    }

    console.log(new g(4).x);
}

f4();


function f5() {
    var g = 2;
    g = function(p) {
        return function() {
            console.log(p);
            this.x = p + 1;
        }
    }

    console.log(new (g(5))().x);
}
f5();


function f6() {
    function g(p) {
        console.log(p);
        this.x = p + 1;
    }

    var obj = {
    }

    obj.g = g;
    console.log(new obj.g(6).x);
}
f6();


function f7() {
    var g = 2;
    var obj = {
        1: function x(p) {
            console.log(p);
            this.x = p + 1;
        }
    }

    g = obj[1];
    console.log(new g(7).x);
}
f7();


function f20() {
    var g = 2;
    var obj = {
        f(p) {
            console.log(p);
            this.x = p + 1;
        }
    }

    g = obj.f;
    console.log(new g(20).x); //// 抛出异常: TypeError: g is not a constructor
}
f20();


function f21() {
    var g = 2;
    var obj = {
        [1](p) {
            console.log(p);
            this.x = p + 1;
        }
    }

    g = obj[1];
    console.log(new g(21).x); //// 抛出异常: TypeError: g is not a constructor
}
f21();

