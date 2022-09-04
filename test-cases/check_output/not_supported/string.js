
//// fromCharCode 参数为字符串
function f1() {
    console.log(String.fromCharCode(65, '66'));
    console.log(String.fromCharCode(65, null, '66', 'b', 67, 67.8, true, 68));

    var obj = { toString() { return 69; }};
    console.log(String.fromCharCode(obj));
}
f1();
