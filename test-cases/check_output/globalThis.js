// Index: 0
var a = 1;

// 测试 ==
function f() {
    console.log('1', globalThis.a);
    console.log('2', globalThis['a']);
    console.log('3', globalThis.NaN);
    console.log('4', globalThis.Infinity);
    console.log('5', globalThis[0] = 10, globalThis[0]);
    console.log('6', globalThis.x);
    console.log('7', delete globalThis.NaN);
    Object.defineProperty(globalThis, 'y', {
        get: function() {
            console.log('#in get y');
            return 'y-1';
        },
        set: function(p) {
            console.log('#in set y');
        },
        configurable: false,
    });
    console.log('8-1', globalThis.y, globalThis.y = 2);
    console.log('8-2', y);
    console.log('8-3', y = 5);
    console.log('8-4', globalThis.y++);
    console.log('8-5', y++);
    Object.defineProperty(globalThis, 'z', {
        value: 'vz-1',
        configurable: true,
        writable: false,
    });
    console.log('9-1', z = 2, z);
    console.log('10', globalThis.x1++, globalThis.x1);

    try {
        console.log(x);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
1 1
2 1
3 NaN
4 Infinity
5 10 undefined
6 undefined
7 false
#in get y
#in set y
8-1 y-1 2
#in get y
8-2 y-1
#in set y
8-3 5
#in get y
#in set y
8-4 NaN
#in get y
#in set y
8-5 NaN
9-1 2 vz-1
10 NaN NaN
ReferenceError: x is not defined
*/

