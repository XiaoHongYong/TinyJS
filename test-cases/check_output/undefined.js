// Index: 0
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


// Index: 1
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


// Index: 2
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


// Index: 3
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


// Index: 4
function f5() {
    function undefined() {}
    console.log(undefined);
    undefined = 2;
    console.log(undefined);
}
f5();
/* OUTPUT
function undefined() {}
2
*/

