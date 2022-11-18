// Index: 0
//// for in arguments
function f1(a, b) {
    console.log('# 0')
    for (var i in arguments) {
        console.log(i, arguments[i]);
    }
    for (var i of arguments) {
        console.log(i);
    }

    // 暂时不支持 chrome 中对 length 更改之后的 for 循环
    // console.log('# 1')
    // arguments.length = 3;
    // for (var i in arguments) {
    //     console.log(i, arguments[i]);
    // }
    // for (var i of arguments) {
    //     console.log(i);
    // }

    // console.log('# 2')
    // arguments.length = 3;
    // arguments[2] = 'x';
    // for (var i in arguments) {
    //     console.log(i, arguments[i]);
    // }
    // for (var i of arguments) {
    //     console.log(i);
    // }

    console.log('# 3')
    arguments = [4, 5, 6, 7];
    for (var i in arguments) {
        console.log(i, arguments[i]);
    }
    for (var i of arguments) {
        console.log(i);
    }
}
f1(1, 2);
/* OUTPUT
# 0
0 1
1 2
1
2
# 3
0 4
1 5
2 6
3 7
4
5
6
7
*/


// Index: 1
//// for of array
function f3() {
    var a = [1, 2, 3, 5, 7];
    a['x'] = 10;
    a['y'] = 20;
    for (var i of a) {
        console.log(i);
    }
}
f3();
/* OUTPUT
1
2
3
5
7
*/


// Index: 2
//// for in with in
function f4() {
    var a = [1, 2, 3, 5, 7], i;
    a['x'] = 10;
    for (i in 1 in (console.log(21), [2]), console.log(22), console.log(23), a) {
        console.log(i);
    }
}
f4();
/* OUTPUT
21
22
23
0
1
2
3
4
x
*/


// Index: 3
//// for of array
function f5() {
    var obj = { x : 1 };
    function g() {
        console.log('g');
        return obj;
    }
    var a = [1, 2, 3, 5, 7];
    a['x'] = 10;
    a['y'] = 20;

    for (g().x of a) {
        console.log(obj.x);
    }
}
f5();
/* OUTPUT
g
1
g
2
g
3
g
5
g
7
*/


// Index: 4
//// for of array
function f6() {
    var a = [[1, 2], [3, 5]];

    for (var [i, j] of a) {
        console.log('>', i, j);
    }
}
f6();
/* OUTPUT
> 1 2
> 3 5
*/


// Index: 5
//// for of array
function f7() {
    var a = [[1, 2], [3, 5]];
    var i, j;

    for ([i, j] of a) {
        console.log('>', i, j);
    }
}
f7();
/* OUTPUT
> 1 2
> 3 5
*/


// Index: 6
//// for of array
function f71() {
    var a = [[1, 2], [3, 5], [6], []];
    var i, j;

    for ([i=3, j=4] of a) {
        console.log('>', i, j);
    }
}
f71();
/* OUTPUT
> 1 2
> 3 5
> 6 4
> 3 4
*/


// Index: 7
//// for of array
function f72() {
    var a = [[3, 5], [6], []];
    var i, j;

    function g(x) { console.log('=g: ', x); }

    for ([i=3, j=g(i)] of a) {
        console.log('>', i, j);
    }
}
f72();
/* OUTPUT
> 3 5
=g:  6
> 6 undefined
=g:  3
> 3 undefined
*/


// Index: 8
//// for ;;;
function f73() {
    var a = [0, 1, 2, 3, 4, 5];
    var i, j = 2;

    for (i = 0; j++, i in a; i++) {
        console.log('>', i, j);
    }
}
f73();
/* OUTPUT
> 0 3
> 1 4
> 2 5
> 3 6
> 4 7
> 5 8
*/


// Index: 9
//// for of array: SyntaxError: Invalid left-hand side in for-loop
function f8() {
    var a = [[1, 2], [3, 5]];
    var i, j;

    for (i, j of a) {
        console.log('>', i);
    }
}
f8();
/* OUTPUT
Uncaught SyntaxError: Invalid left-hand side in for-loop
*/


// Index: 10
//// for of array: SyntaxError: Invalid left-hand side in for-loop
function f82() {
    var a = [[1, 2], [3, 5]];
    var i, j;

    for (i => x of a) {
        console.log('>', i);
    }
}
f82();
/* OUTPUT
Uncaught SyntaxError: Invalid left-hand side in for-loop
*/


// Index: 11
//// for of array: SyntaxError: for-of loop variable declaration may not have an initializer
function f10() {
    var a = [[1, 2], [3, 5]];
    var i, j;

    for (var i = 1 of a) {
        console.log('>', i);
    }
}
f10();
/* OUTPUT
Uncaught SyntaxError: for-of loop variable declaration may not have an initializer.
*/


// Index: 12
var arr1 = ['a', 'b', 1];
var arr2 = [1, 4, 'x'];
arr2.a = 'v1';
arr2.b = 'v2';

var obj = {'a': '_x', 1: 'y', f: function() {}};

function f1(r, obj, noPrint) {
    console.log('Round: ', r, noPrint ? '' : obj)

    try {
        for (var i in obj) {
            console.log(i, obj[i]);
        }
        for (var i of obj) {
            console.log(i);
        }
    } catch (e) {
        console.log(e.toString());
    }
}
f1('1', undefined);
f1('2', null);
f1('3', NaN);
f1('4', Infinity);
f1('5', 0);
f1('6', '');
f1('7', 'abc');
f1('8', 'a');
f1('9', 'abc'.charAt(2));
f1('10', true);
f1('11', '/a/');
f1('12', Symbol('x'));
f1('13', [11, 12, 13], 1);
f1('14', obj, 1);
f1('15', arr1, 1);
f1('16', arr2, 1);
f1('17', function () { return 1; }, 1);
f1('18', Array.prototype, 1);
/* OUTPUT-FIXED
Round:  1 undefined
TypeError: undefined is not iterable
Round:  2 null
TypeError: null is not iterable
Round:  3 NaN
TypeError: NaN is not iterable
Round:  4 Infinity
TypeError: Infinity is not iterable
Round:  5 0
TypeError: 0 is not iterable
Round:  6  
Round:  7 abc
0 a
1 b
2 c
a
b
c
Round:  8 a
0 a
a
Round:  9 c
0 c
c
Round:  10 true
TypeError: true is not iterable
Round:  11 /a/
0 /
1 a
2 /
/
a
/
Round:  12 Symbol(x)
TypeError: Symbol(x) is not iterable
Round:  13
0 11
1 12
2 13
11
12
13
Round:  14
f function() {}
1 y
a _x
TypeError: [object Object] is not iterable
Round:  15
0 a
1 b
2 1
a
b
1
Round:  16
0 1
1 4
2 x
b v2
a v1
1
4
x
Round:  17
TypeError: function () { return 1; } is not iterable
Round:  18
*/


// Index: 13
//// for of array
function f() {
    var a = [1, 3];
    Array.prototype.x = 'x1';
    for (var i in a) {
        console.log(i, a[i]);
    }
}
f();
/* OUTPUT
0 1
1 3
x x1
*/


// Index: 14
//// for of array
function f() {
    var a = 'str';
    String.prototype.y = 'y1';
    for (var i in a) {
        console.log(i, a[i]);
    }
}
f();
/* OUTPUT
0 s
1 t
2 r
y y1
*/


// Index: 15
//// for of array
function f() {
    var proto = [4, 5, 6];
    var a = [1, 2];
    a.__proto__ = proto;
    for (var i of a) {
        console.log(i);
    }
}
f();
/* OUTPUT
1
2
*/


// Index: 16
//// for of array
function f() {
    var a = [1, 2, 3];
    Object.defineProperty(a, '2', {
        value: 'xx',
        enumerable: false,
    });
    for (var i of a) {
        console.log(i);
    }
    console.log('===');
    for (var i in a) {
        console.log(i, a[i]);
    }
}
f();
/* OUTPUT
1
2
xx
===
0 1
1 2
*/


// Index: 17
//// for of array
function f() {
    var a = [1,,,2];
    for (var i of a) {
        console.log(i);
    }
    console.log('===');
    for (var i in a) {
        console.log(i, a[i]);
    }
}
f();
/* OUTPUT
1
undefined
undefined
2
===
0 1
3 2
*/

