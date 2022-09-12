// Index: 0
function f(p) {
    var a = p??1;
    console.log(a);
}
f(2);
f(null);
f(undefined);
f('');
f(NaN);
/* OUTPUT
2
1
1

NaN
*/


// Index: 1
function f(p) {
    console.log(p, p ? 'x' : 'y');
}
f(2);
f(null);
f(undefined);
f('');
f(NaN);
/* OUTPUT
2 x
null y
undefined y
 y
NaN y
*/

