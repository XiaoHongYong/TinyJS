var obj = { toString() { return '1.0'; }}
function g() { }

// 测试 ==
function f1(r, p) {
    console.log('round: ', r, p);

    console.log(p === undefined);
    console.log(p === null);
    console.log(p === NaN);
    console.log(p === 0);
    console.log(p === 0.0);
    console.log(p === 1.0);
    console.log(p === -1);
    console.log(p === -1.0);
    console.log(p === true);
    console.log(p === false);
    console.log(p === '');
    console.log(p === '0');
    console.log(p === '1');
    console.log(p === '0.0');
    console.log(p === '1.0');
    console.log(p === 'true');
    console.log(p === 'false');
    console.log(p === g);
    console.log(p === obj);
    console.log(p === /a/);
    console.log(p === Symbol());
    console.log(p === Symbol(''));
}
f1('1', undefined);
f1('2', null);
f1('3', NaN);
f1('4', 0);
f1('5', 0.0);
f1('6', 1.0);
f1('7', -1);
f1('8', -1.0);
f1('9', true);
f1('10', false);
f1('11', '');
f1('12', '0');
f1('13', '1');
f1('14', '0.0');
f1('15', '1.0');
f1('16', 'true');
f1('17', 'false');
f1('18', g);
f1('19', obj);
f1('20', /a/);
f1('21', Symbol());
f1('22', Symbol(''));
/* OUTPUT
round:  1 undefined
true
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  2 null
false
true
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  3 NaN
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  4 0
false
false
false
true
true
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  5 0
false
false
false
true
true
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  6 1
false
false
false
false
false
true
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  7 -1
false
false
false
false
false
false
true
true
false
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  8 -1
false
false
false
false
false
false
true
true
false
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  9 true
false
false
false
false
false
false
false
false
true
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  10 false
false
false
false
false
false
false
false
false
false
true
false
false
false
false
false
false
false
false
false
false
false
false
round:  11 
false
false
false
false
false
false
false
false
false
false
true
false
false
false
false
false
false
false
false
false
false
false
round:  12 0
false
false
false
false
false
false
false
false
false
false
false
true
false
false
false
false
false
false
false
false
false
false
round:  13 1
false
false
false
false
false
false
false
false
false
false
false
false
true
false
false
false
false
false
false
false
false
false
round:  14 0.0
false
false
false
false
false
false
false
false
false
false
false
false
false
true
false
false
false
false
false
false
false
false
round:  15 1.0
false
false
false
false
false
false
false
false
false
false
false
false
false
false
true
false
false
false
false
false
false
false
round:  16 true
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
true
false
false
false
false
false
false
round:  17 false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
true
false
false
false
false
false
round:  18 function g() { }
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
true
false
false
false
false
round:  19 {toString: () { return '1.0'; }}
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
true
false
false
false
round:  20 /a/
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  21 Symbol()
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
round:  22 Symbol()
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
false
*/
