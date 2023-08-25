// Index: 0
function f() {
    var sum = 0;
    let i = 0;
    while (i < 1000) {
        // console.log(i++);
        sum += i;
        i++;
    }
    console.log(sum);
}
f();
/* OUTPUT
499500
*/

