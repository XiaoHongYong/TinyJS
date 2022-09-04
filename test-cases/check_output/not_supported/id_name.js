function f(a) {
    console.log(a);
    var a = 1;
    console.log(a);
    console.log(arguments[0]);
}

f(2);
/* OUTPUT
2
1
1
*/
