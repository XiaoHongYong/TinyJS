// Index: 0
var NaN = 1;
console.log(NaN);
var Infinity = 2;
console.log(Infinity);
var undefined = 2;
console.log(undefined);
/* OUTPUT
NaN
Infinity
undefined
*/


// Index: 1
function NaN() {
}
console.log(NaN);
/* OUTPUT
Uncaught SyntaxError: Identifier 'NaN' has already been declared
*/


// Index: 2
function f1() {
    var undefined = 1;
    console.log(undefined);

    console.log(undefined++);
    console.log(undefined--);
    console.log(++undefined);
    console.log(--undefined);
    console.log(undefined += 10);
}
f1();
/* OUTPUT
1
1
2
2
1
11
*/


// Index: 3
function f2() {
    var NaN = 1;
    console.log(NaN);

    console.log(NaN++);
    console.log(NaN--);
    console.log(NaN == NaN);
}
f2();
var NaN = 1;
console.log(NaN);
/* OUTPUT
1
1
2
true
NaN
*/


// Index: 4
function f3() {
    var a = NaN;
    console.log(a * 10);

    console.log(a * a);
    console.log(undefined * 1);
    console.log(a + 1);
}
f3();
/* OUTPUT
NaN
NaN
NaN
NaN
*/


// Index: 5
function f4(undefined) {
    console.log(undefined);
    undefined = 4;
    console.log(undefined);
}
f4(1);
/* OUTPUT
1
4
*/


// Index: 6
function f5() {
    function undefined() {}
    console.log(undefined);
    undefined = 2;
    console.log(undefined);
}
f5();
    1 + Symbol()
    console.log(error);
/* OUTPUT
function undefined() {}
2
Uncaught TypeError: Cannot convert a Symbol value to a number
*/

