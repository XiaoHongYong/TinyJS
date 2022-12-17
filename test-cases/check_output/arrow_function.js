// Index: 0
// apply
function f(p) {
    this.x = 5;
    g = ()=>console.log(this.x, arguments.length, arguments[0]);
    g();
}
new f('p1');
/* OUTPUT
5 1 p1
*/

