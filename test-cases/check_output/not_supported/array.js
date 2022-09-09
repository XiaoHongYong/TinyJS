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

