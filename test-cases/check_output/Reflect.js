// Index: 0
// abs, ceil, floor, round, sign, sqrt
function f() {
    try {
        console.log(1, Reflect.ownKeys(undefined));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        console.log(2, Reflect.ownKeys(1));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    try {
        console.log(4, Reflect.ownKeys('abc'));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
    console.log(8, Reflect.ownKeys([1,2,3]));
    var a = [4, 5]; a.x = 1;
    console.log(9, Reflect.ownKeys(a));
    console.log(10, Reflect.ownKeys({}));
    console.log(11, Reflect.ownKeys({a: 1}));

    var a = [];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})

    // Reflect.ownKeys(a);
    // Object.keys(a);
    // console.log(6, Reflect.ownKeys(/x/));
}
f();
/* OUTPUT
TypeError: Reflect.ownKeys called on non-object
TypeError: Reflect.ownKeys called on non-object
TypeError: Reflect.ownKeys called on non-object
8 [0, 1, 2, length]
9 [0, 1, length, x]
10 []
11 [a]
*/

