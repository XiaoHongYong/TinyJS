// Index: 0
var obj1 = { toString() { return 1; }}
function g() { }

// 测试 ==
function f() {
    console.log('1', void undefined);
    console.log('2', void null);
    console.log('3', void NaN);
    console.log('4', void Infinity);
    console.log('5', void -Infinity);
    console.log('6', void 0);
    console.log('7', void 0.0);
    console.log('8', void Array);
    console.log('9', void String.prototype);
    console.log('10', void String.prototype.charAt);
    console.log('16', void true);
    console.log('17', void false);
    console.log('18', void '');
    console.log('19', void '0');
    console.log('21', void '234'.charAt(1));
    console.log('22', void '0.0');
    console.log('25', void 'false');
    console.log('26', void g);
    console.log('27', void obj1);
    console.log('29', void /a/);
    try {
        var a = Symbol();
        console.log(void a);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f();
/* OUTPUT
1 undefined
2 undefined
3 undefined
4 undefined
5 undefined
6 undefined
7 undefined
8 undefined
9 undefined
10 undefined
16 undefined
17 undefined
18 undefined
19 undefined
21 undefined
22 undefined
25 undefined
26 undefined
27 undefined
29 undefined
undefined
*/

