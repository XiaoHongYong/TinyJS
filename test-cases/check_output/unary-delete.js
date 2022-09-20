// Index: 0
var obj1 = { toString() { return 1; }, x : 2}
function g() { }

// 测试 ==
function f() {
    console.log('1', delete undefined);
    console.log('2', delete null);
    console.log('3', delete NaN);
    console.log('4', delete Infinity);
    console.log('5', delete -Infinity);
    console.log('6', delete 0);
    console.log('7', delete 0.0);
    console.log('8', delete Array);
    console.log('9', delete String.prototype);
    console.log('16', delete true);
    console.log('17', delete false);
    console.log('18', delete '');
    console.log('19', delete '0');
    console.log('21', delete '234'.charAt(1));
    console.log('22', delete '0.0');
    console.log('25', delete 'false');
    console.log('27', delete obj1.toString, obj1.toString);
    console.log('28', delete obj1['x'], obj1.x);
    console.log('29', delete /a/);

    try {
        console.log('30', delete String.prototype.charAt);
        console.log('31', '234'.charAt(1));
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }

    // console.log('26', delete g);
    // aaa = 'aaa#';
    // console.log('32', aaa, delete aaa, aaa);
}
f();
/* OUTPUT-FIXED
1 false
2 true
3 false
4 false
5 true
6 true
7 true
8 true
9 false
16 true
17 true
18 true
19 true
21 true
22 true
25 true
27 true function toString() { [native code] }
28 true undefined
29 true
30 true
TypeError: undefined is not a function
*/

