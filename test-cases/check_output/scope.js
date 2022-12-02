// Index: 0
//// 
function f() {
    var i = 0;
    {
        var j = 1;
        console.log('#1', i, j);
    }

    for (let k = 0; k < 3; k++) {
        console.log('#2', k, i, j);
    }
}
f();
/* OUTPUT
#1 0 1
#2 0 0 1
#2 1 0 1
#2 2 0 1
*/

