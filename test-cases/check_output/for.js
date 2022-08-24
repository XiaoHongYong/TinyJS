//// for in array
function f2() {
    var a = [1, 2, 3, 5, 7];
    a['x'] = 10;
    a['y'] = 20;
    for (var i in a) {
        console.log(i);
    }
}
f2();
/* OUTPUT
0 1 2 3 4 x y
*/


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
1 2 3 5 7 10 20
*/


//// for in with in
function f4() {
    var a = [1, 2, 3, 5, 7];
    a['x'] = 10;
    a['y'] = 20;
    for (var i in 1 in (console.log(21), [2]), console.log(22), console.log(23), a) {
        console.log(i);
    }
}
f4();
/* OUTPUT
21 22 23
0 1 2 3 4 x y
*/


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
> 1 2
> 3 5
> 6 4
> 3 4
*/

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
0 3
1 4
2 5
3 6
4 7
5 8
*/


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
*/


//// for of array: SyntaxError: Invalid left-hand side in for-loop
function f81() {
    var a = [[1, 2], [3, 5]];
    var i, j;

    for (i = 1 of a) {
        console.log('>', i);
    }
}
f81();
/* OUTPUT
*/


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
*/


//// for of array: SyntaxError: Invalid left-hand side in for-in loop: Must have a single binding
function f9() {
    var a = [[1, 2], [3, 5]];
    var i, j;

    for (var i, j of a) {
        console.log('>', i);
    }
}
f9();
/* OUTPUT
*/


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
*/
