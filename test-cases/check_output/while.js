// Index: 0
function f() {
    let i = 0;
    while (i < 10) {
        console.log(i++);
    }
}
f();
/* OUTPUT
0
1
2
3
4
5
6
7
8
9
*/


// Index: 1
function f() {
    let i = 0;
    do {
        console.log(i++);
    } while (i < 0);
}
f();
/* OUTPUT
0
*/

