
var a = 3, b = 4;

function f1() {
    var a = 1;
    eval('var b = 2; function g() { console.log("g"); }');
    console.log(a, b);
    g();
}
f1();
console.log(a, b); // 1, 1


//// 测试 eval 中函数的作用域
function f2() {
    eval('function g() { console.log("g"); }');
    eval('{ function f() { console.log("f"); } }');

    g();
    f();
}
f2();
