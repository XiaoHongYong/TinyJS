// Index: 0
var obj1 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f() {
    console.log('1', typeof undefined);
    console.log('2', typeof null);
    console.log('3', typeof NaN);
    console.log('4', typeof Infinity);
    console.log('5', typeof -Infinity);
    console.log('6', typeof 0);
    console.log('7', typeof 0.0);
    console.log('8', typeof Array);
    console.log('9', typeof String.prototype);
    console.log('10', typeof String.prototype.charAt);
    console.log('16', typeof true);
    console.log('17', typeof false);
    console.log('18', typeof '');
    console.log('19', typeof '0');
    console.log('21', typeof '234'.charAt(1));
    console.log('22', typeof '0.0');
    console.log('25', typeof 'false');
    console.log('26', typeof g);
    console.log('27', typeof obj1);
    console.log('29', typeof /a/);
    try {
        var a = Symbol();
        console.log(typeof a);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
1 undefined
2 object
3 number
4 number
5 number
6 number
7 number
8 function
9 object
10 function
16 boolean
17 boolean
18 string
19 string
21 string
22 string
25 string
26 function
27 object
29 object
symbol
*/

