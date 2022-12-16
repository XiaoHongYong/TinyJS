// Index: 0
var obj1 = { toString() { return '3.0'; }}
var obj2 = { toString() { return 1; }}
function g() {}
function f(r, a, b, noPrint) {
    console.log('round: ', r);

    try {
        console.log(a in b);
    } catch (e) {
        console.log(e.name + ': ' + e.message);
    }
}
f('1', 'x', undefined);
f('2', 'x', null);
f('3', 'x', NaN);
f('4', 'x', Infinity);
f('5', 'x', -Infinity);
f('6', 'x', 0);
f('8', 'x', 1.0);
f('16', 'x', true);
f('17', 'x', false);
f('18', 'x', '');
f('19', 'x', '0');
f('21', 'x', '234'.charAt(1));
f('25', 'x', 'false');
f('26', 'x', g);
f('27', 'x', obj1);
f('29', 'x', /a/);
f('26', 'x', g);
f('27', 'x', obj1);
f('29', 'x', /a/);

f('26-t', 'toString', g);
f('27-t', 'toString', obj1);
f('29-t', 'toString', /a/);
f('26-t', 'toString', g);
f('27-t', 'toString', obj1);
f('29-t', 'toString', /a/);
/* OUTPUT
round:  1
TypeError: Cannot use 'in' operator to search for 'x' in undefined
round:  2
TypeError: Cannot use 'in' operator to search for 'x' in null
round:  3
TypeError: Cannot use 'in' operator to search for 'x' in NaN
round:  4
TypeError: Cannot use 'in' operator to search for 'x' in Infinity
round:  5
TypeError: Cannot use 'in' operator to search for 'x' in -Infinity
round:  6
TypeError: Cannot use 'in' operator to search for 'x' in 0
round:  8
TypeError: Cannot use 'in' operator to search for 'x' in 1
round:  16
TypeError: Cannot use 'in' operator to search for 'x' in true
round:  17
TypeError: Cannot use 'in' operator to search for 'x' in false
round:  18
TypeError: Cannot use 'in' operator to search for 'x' in 
round:  19
TypeError: Cannot use 'in' operator to search for 'x' in 0
round:  21
TypeError: Cannot use 'in' operator to search for 'x' in 3
round:  25
TypeError: Cannot use 'in' operator to search for 'x' in false
round:  26
false
round:  27
false
round:  29
false
round:  26
false
round:  27
false
round:  29
false
round:  26-t
true
round:  27-t
true
round:  29-t
true
round:  26-t
true
round:  27-t
true
round:  29-t
true
*/


// Index: 1
function f() {
    console.log(aa instanceof gg);
}
f();
/* OUTPUT
Uncaught ReferenceError: aa is not defined
*/

