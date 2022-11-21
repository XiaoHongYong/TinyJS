// Index: 0
// parse
function f(r, s) {

    try {
        console.log(r, JSON.parse(s));
    } catch (e) {
        console.log(r, e.name);
    }
}
f(0, null);
f(1, true);
f(2, 1);
f(3, 0);
f(4, '{}');
f(5, '[]');
f(6, '[');
f(7, 'abc');
f(8, '"x"');
f(9, '{"a": 1, "b": [2, 3,3, true, false, null], "c": {}}');
/* OUTPUT
0 null
1 true
2 1
3 0
4 {}
5 []
6 SyntaxError
7 SyntaxError
8 x
9 {a: 1, b: Array(6), c: Object}
*/


// Index: 1
// stringify
function f(r, o) {

    try {
        console.log(r, JSON.stringify(o));
    } catch (e) {
        console.log(r, e.name);
    }
}
f(0, null);
f(1, true);
f(2, 1);
f(2.1, NaN);
f(2.2, Infinity);
f(3, 0);
f(4, {});
f(5, []);
f(8, "x");
f(9, {"b": [2, 3,3, true, false, null], "1": {}});
/* OUTPUT
0 null
1 true
2 1
2.1 null
2.2 null
3 0
4 {}
5 []
8 "x"
9 {"1":{},"b":[2,3,3,true,false,null]}
*/


// Index: 2
// stringify
function f(r, s) {
    var a = [];
    a._0 = 'x';
    a._2 = 'b';
    Object.defineProperty(a, 0, {get: function() { console.log('get 0', this._0); return this._0; }, set: function(p) { this._0 = p; console.log('set 0: ', p); }})
    Object.defineProperty(a, 2, {get: function() { console.log('get 2', this._2); return this._2; }, set: function(p) { this._2 = p; console.log('set 2: ', p); }})
    Object.defineProperty(a, 1, {
        enumerable: false,
        value: 1,    
    })

    console.log(JSON.stringify(a));
}
f();
/* OUTPUT
get 0 x
get 2 b
["x",1,"b"]
*/


// Index: 3
// stringify
function f(r, s) {
    var a = {};
    Object.defineProperty(a, 'a', {enumerable: true, get: function() { console.log('get a'); return 'va'; }})
    Object.defineProperty(a, 'b', {get: function() { console.log('get b'); return 'vb'; }})
    Object.defineProperty(a, 'c', {
        enumerable: false,
        value: 1,    
    })

    console.log(JSON.stringify(a));
}
f();
/* OUTPUT
get a
{"a":"va"}
*/

