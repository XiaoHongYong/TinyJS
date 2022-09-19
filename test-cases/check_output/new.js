// Index: 0
function f1() {
    console.log(new g(1).x);

    function g(p) {
        console.log(p);
        this.x = p;
    }
}
f1();
/* OUTPUT
1
1
*/


// Index: 1
function f2() {
    g = function(p) {
        console.log('#1', p);
        this.x = p + 1;
    }

    console.log(new g(2).x);

    function g(p) {
        console.log('#2', p);
        this.x = p;
    }
}
f2();
/* OUTPUT
#1 2
3
*/


// Index: 2
function f3() {
    var g = function(p) {
        console.log(p);
        this.x = p + 1;
    }

    console.log(new g(3).x);
}
f3();
/* OUTPUT
3
4
*/


// Index: 3
function f4() {
    var g;
    g = function(p) {
        console.log(p);
        this.x = p + 1;
    }

    console.log(new g(4).x);
}
f4();
/* OUTPUT
4
5
*/


// Index: 4
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
/* OUTPUT
5
6
*/


// Index: 5
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
/* OUTPUT
6
7
*/


// Index: 6
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
/* OUTPUT
7
8
*/


// Index: 7
function f() {
    var g = 2;
    var obj = {
        f(p) {
            console.log(p);
            this.x = p + 1;
        }
    }

    g = obj.f;
    console.log(new g(20).x); // 抛出异常: TypeError: g is not a constructor
}
f();
/* OUTPUT-FIXED
Uncaught TypeError: ? is not a constructor
*/


// Index: 8
function f() {
    var g = 2;
    var obj = {
        [1](p) {
            console.log(p);
            this.x = p + 1;
        }
    }

    g = obj[1];
    console.log(new g(21).x); // 抛出异常: TypeError: g is not a constructor
}
f();
/* OUTPUT-FIXED
Uncaught TypeError: ? is not a constructor
*/

