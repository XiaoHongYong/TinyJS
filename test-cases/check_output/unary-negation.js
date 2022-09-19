// Index: 0
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f() {
    console.log('1', - undefined);
    console.log('2', - null);
    console.log('3', - NaN);
    console.log('4', - Infinity);
    console.log('5', - -Infinity);
    console.log('6', - 0);
    console.log('7', - 0.0);
    console.log('8', - 1.0);
    console.log('9', - -1);
    console.log('10', - -1.0);
    console.log('11', - -5);
    console.log('12', - -5.2);
    console.log('13', - 5);
    console.log('14', - 5000);
    console.log('15', - 5.2);
    console.log('16', - true);
    console.log('17', - false);
    console.log('18', - '');
    console.log('19', - '0');
    console.log('20', - '1');
    console.log('21', - '234'.charAt(1));
    console.log('22', - '0.0');
    console.log('23', - '1.0');
    console.log('24', - 'true');
    console.log('25', - 'false');
    console.log('26', - g);
    console.log('27', - obj1);
    console.log('28', - obj2);
    console.log('29', - /a/);
    try {
        var a = Symbol();
        console.log(- a);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
        // console.log(e);
    }
}
f();
/* OUTPUT-FIXED
1 NaN
2 0
3 NaN
4 -Infinity
5 Infinity
6 0
7 0
8 -1
9 1
10 1
11 5
12 5.2
13 -5
14 -5000
15 -5.2
16 -1
17 0
18 0
19 0
20 -1
21 -3
22 0
23 -1
24 NaN
25 NaN
26 NaN
27 -3
28 -1
29 NaN
TypeError: Cannot convert a Symbol value to a number
*/

