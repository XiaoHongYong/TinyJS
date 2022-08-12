function f1() {
    var undefined = 1;
    console.log(undefined);

    console.log(undefined++);
    console.log(undefined--);
    console.log(++undefined);
    console.log(--undefined);
    console.log(undefined + =10);
}
f1();


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


function f3() {
    var a = NaN;
    console.log(a * 10);

    console.log(a * a);
    console.log(undefined * 1);
    console.log(a + 1);
}
f3();


function f4(undefined) {
    console.log(undefined);
    undefined = 4;
    console.log(undefined);
}
f4(1);


function f5() {
    function undefined() {}
    console.log(undefined);
    undefined = 2;
    console.log(undefined);
}
f5();
