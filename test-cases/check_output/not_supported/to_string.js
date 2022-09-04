
//// 函数名和参数相同
function f1() {
    var obj = {
        toString() {
            // return 'toString_obj';
            return this
        }
    }

    var a = {};
    a[obj];

    console.log(a);
    console.log(obj.toString());
}
f1();
/* OUTPUT
[error] Uncaught TypeError: Cannot convert object to primitive value
*/
