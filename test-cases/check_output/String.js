// Index: 0
//// fromCharCode 参数为字符串
function f() {
    console.log(String.fromCharCode(65, '66'));
    console.log(String.fromCharCode(65, '66', 67, 67.8, 68));

    var obj = { toString() { return 69; }};
    console.log(String.fromCharCode(obj));
}
f();
/* OUTPUT
AB
ABCCD
E
*/


// Index: 1
//// charAt
function f() {
    var s = 'abcde';
    console.log(s.charAt(0));
    console.log(s.charCodeAt(0));
    console.log(s.at(0));

    console.log(s.charAt(-1));
    console.log(s.charCodeAt(-1));
    console.log(s.at(-1));
}
f();
/* OUTPUT
a
97
a

NaN
e
*/

