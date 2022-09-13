// Index: 0
function f() {
    var obj = {
        x : 'vx',
    };
    with (obj)
        console.log(x);
}
f();
/* OUTPUT
vx
*/


// Index: 1
// with 常量
function f() {
    with ('a')
        console.log(length);
}
f();
/* OUTPUT
1
*/


// Index: 2
// with undefined
function f() {
    with (undefined)
        console.log(1);
}
f();
/* OUTPUT
Uncaught TypeError: Cannot convert undefined or null to object
*/


// Index: 3
// with null
function f() {
    with (null)
        console.log(1);
}
f();
/* OUTPUT
Uncaught TypeError: Cannot convert undefined or null to object
*/


// Index: 4
// with 和 local 重名的情况
function f() {
    var obj = {
        x : 'vx',
    };
    with (obj) {
        let x = 'localx';
        console.log(x);
    }
}
f();
/* OUTPUT
localx
*/


// Index: 5
// with 和 parent 重名的情况
function f() {
    let x = 'xy';
    var obj = {
        x : 'vx',
    };
    with (obj) {
        console.log(x);
    }
}
f();
/* OUTPUT
vx
*/


// Index: 6
// with 和 parent 重名的情况
function f() {
    let x = 'xy';
    var obj = {
        x : undefined,
    };
    with (obj) {
        console.log(x);
    }
}
f();
/* OUTPUT
undefined
*/


// Index: 7
// 多层 with
function f() {
    var obj1 = {
        x : 'vx1',
    };
    with (obj1) {
        let obj2 = {
            x : 'vx2',
        };
        with (obj2) {
            console.log(x);
        }
        console.log(x);
    }
}
f();
/* OUTPUT
vx2
vx1
*/


// Index: 8
// 多层 with
function f() {
    var obj1 = {
        x : 'vx1',
        y : 'vy',
    };
    let obj2 = {
        x : 'vx2',
    };
    with (obj1) {
        with (obj2) {
            console.log(x);
            console.log(y);
        }
        console.log(x);
    }
}
f();
/* OUTPUT
vx2
vy
vx1
*/

