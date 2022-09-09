// Index: 0
// 测试等号两边的执行先后顺序
function f1() {
    var obj = { y: '2' };
    function f() { console.log('f'); return obj; }
    function g() { console.log('g'); return obj; }

    f().x = g().y;
    console.log(obj.x, obj.y);
}
f1();
/* OUTPUT
f
g
2 2
*/

