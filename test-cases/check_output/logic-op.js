// Index: 0
// &&
function f(p) {
    console.log('Round:', p);

    console.log(p && 2);
    console.log(p && null);
    console.log(p && undefined);
    console.log(p && '');
    console.log(p && 'abc');
    console.log(p && Symbol());
    console.log(p && NaN);
    console.log(p && Infinity);
}
f(2);
f(null);
f(undefined);
f('');
f('abc');
f(Symbol());
f(NaN);
f(Infinity);
/* OUTPUT
Round: 2
2
null
undefined

abc
Symbol()
NaN
Infinity
Round: null
null
null
null
null
null
null
null
null
Round: undefined
undefined
undefined
undefined
undefined
undefined
undefined
undefined
undefined
Round: 








Round: abc
2
null
undefined

abc
Symbol()
NaN
Infinity
Round: Symbol()
2
null
undefined

abc
Symbol()
NaN
Infinity
Round: NaN
NaN
NaN
NaN
NaN
NaN
NaN
NaN
NaN
Round: Infinity
2
null
undefined

abc
Symbol()
NaN
Infinity
*/


// Index: 1
// ||
function f(p) {
    console.log('Round:', p);

    console.log(p || 2);
    console.log(p || null);
    console.log(p || undefined);
    console.log(p || '');
    console.log(p || 'abc');
    console.log(p || Symbol());
    console.log(p || NaN);
    console.log(p || Infinity);
}
f(2);
f(null);
f(undefined);
f('');
f('abc');
f(Symbol());
f(NaN);
f(Infinity);
/* OUTPUT
Round: 2
2
2
2
2
2
2
2
2
Round: null
2
null
undefined

abc
Symbol()
NaN
Infinity
Round: undefined
2
null
undefined

abc
Symbol()
NaN
Infinity
Round: 
2
null
undefined

abc
Symbol()
NaN
Infinity
Round: abc
abc
abc
abc
abc
abc
abc
abc
abc
Round: Symbol()
Symbol()
Symbol()
Symbol()
Symbol()
Symbol()
Symbol()
Symbol()
Symbol()
Round: NaN
2
null
undefined

abc
Symbol()
NaN
Infinity
Round: Infinity
Infinity
Infinity
Infinity
Infinity
Infinity
Infinity
Infinity
Infinity
*/

