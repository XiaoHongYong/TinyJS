

// 测试修改 String.prototype
function f2() {
    var obj = { toString() { return '1.0'; }}

    console.log(null == undefined);
    console.log(null == null);
    console.log(1 == undefined);
    console.log(1 == 1);
    console.log(1 == 2);
    console.log(1 == '1');
    console.log(1 == '1.1');
    console.log(1 == 'true');
    console.log(1 == 'false');
    console.log(1 == true);
    console.log(1 == false);
    console.log(1 == obj);
    console.log(0 == false);

    console.log('#2')
}
f2();
/* OUTPUT
get
undefined
set 3
function (a) {
    console.log('y', a);
}
*/
