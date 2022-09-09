// Index: 0
var [a, b=a=1] = [3,];
console.log(a, b); // 1, 1

function f1() {
    var [a, [c, [e, f]] = [11, [12, 13]]] = [1, [2, [3, 4]]];
    console.log(a, c, e, f);
}
f1();


// 测试表达式中的 default Value 表达式 是否会被执行：不会被执行
function f2() {
    var a, b;
    function g() { console.log('g'); }

    // 不会执行 g()
    [a, b=g()] = [1, 2];

    console.log(a, b);
}
f2();
/* OUTPUT
1 1
1 2 3 4
1 2
*/


// Index: 1
// SyntaxError: Missing initializer in destructuring declaration
function $f1() {
    var [a, [c, [e, f]]];
    console.log(a, c, e, f);
}
$f1();
/* OUTPUT
Uncaught SyntaxError: Missing initializer in destructuring declaration
*/

