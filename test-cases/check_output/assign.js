
function f1() {
    var obj = {x: 1};
    function a() {
        console.log('a');
        return obj;
    }

    console.log(a().x += 1);
}
f1();
/* OUTPUT
a
2
*/

function f1() {
    
}
f1();
/* OUTPUT
a
2
*/


