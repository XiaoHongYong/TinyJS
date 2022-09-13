// Index: 0
function f1() {
    var a = [];
    Object.defineProperty(a, 'length', {
        set: function(len) { this._len = len; },
        get: function() { return this._len; }
    })
}
f1();
/* OUTPUT
Uncaught TypeError: Cannot redefine property: length
*/


// Index: 1
function f() {
    var a = [1, 2];
    a.__proto__ = { x : 'x1'};
    console.log(a.x);
    console.log(a[0]);
}
f();
/* OUTPUT
x1
1
*/

