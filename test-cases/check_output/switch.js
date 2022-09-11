// Index: 0
function f(p) {
    switch (p) {
        case 1, 2, 3: console.log('>p3'); break;
        case 1: console.log('>p1'); break;
        default: console.log('>default'); break;
    }
    return '@f';
}
console.log(f(1));
/* OUTPUT
>p1
@f
*/


// Index: 1
// case 的条件是 ===
function f(p) {
    console.log('Round: ', p, typeof p);

    switch (p) {
        case 1: console.log('>a1, number 1'); break;
        case '1': console.log('>a2, string 1'); break;
        case 2: console.log('>a3, number 2'); break;
        case true: console.log('>a4, true'); break;
        case false: console.log('>a5, false'); break;
        case 0: console.log('>a6, number 0'); break;
        case undefined: console.log('>a7, undefined'); break;
        case null: console.log('>a8, null'); break;
        case NaN: console.log('>a9, NaN'); break;
        default: console.log('>default'); // 不 break
        case Infinity: console.log('>a10, Infinity'); break;
    }
    return '@f';
}
console.log(f(1));
console.log(f('1'));
console.log(f('2'));
console.log(f(true));
console.log(f(false));
console.log(f(0));
console.log(f(undefined));
console.log(f(null));
console.log(f(NaN));
console.log(f(Infinity));
console.log(f(112));
/* OUTPUT
Round:  1 number
>a1, number 1
@f
Round:  1 string
>a2, string 1
@f
Round:  2 string
>default
>a10, Infinity
@f
Round:  true boolean
>a4, true
@f
Round:  false boolean
>a5, false
@f
Round:  0 number
>a6, number 0
@f
Round:  undefined undefined
>a7, undefined
@f
Round:  null object
>a8, null
@f
Round:  NaN number
>default
>a10, Infinity
@f
Round:  Infinity number
>a10, Infinity
@f
Round:  112 number
>default
>a10, Infinity
@f
*/


// Index: 2
// case 的比较条件是非常量的，需要及时获取
function f(p) {
    console.log('round: ', p);

    function g() {
        console.log('g');
        return '1';
    }
    function h() {
        console.log('h');
        return '1';
    }
    function i() {
        console.log('i');
        return 1;
    }

    switch (p) {
        case g(): console.log('>a1'); break;
        case h(): console.log('>a2'); break;
        case i(): console.log('>a3'); break;
        case '1': console.log('>a4'); break;
        case 1: console.log('>a5'); break;
        default: console.log('>default'); break;
    }
    return '@f';
}
console.log(f(1));
console.log(f('1'));
/* OUTPUT
round:  1
g
h
i
>a3
@f
round:  1
g
>a1
@f
*/


// Index: 3
// case 条件需要运行，没有 break 的情况.
var obj = {};
function g() {
    console.log('g');
    return '1';
}

function f(p) {
    console.log('round: ', p);

    function h() {
        console.log('h');
        return '1';
    }
    function i() {
        console.log('i');
        return 1;
    }

    switch (p) {
        case g: console.log('>a1');
        case i(): console.log('>a3'); break;
        case h(): console.log('>a2');
        case '1': console.log('>a4'); break;
        case obj: console.log('>a5');
        case 1: console.log('>a6'); break;
        default: console.log('>default'); break;
    }
    return '@f';
}
console.log(f(1));
console.log(f(3));
console.log(f(g));
console.log(f(obj));
/* OUTPUT
round:  1
i
>a3
@f
round:  3
i
h
>default
@f
round:  function g() {
    console.log('g');
    return '1';
}
>a1
>a3
@f
round:  {  }
i
h
>a5
>a6
@f
*/


// Index: 4
// 测试无效的 switch，带 continue
function f(p) {
    switch (p) {
        case 2: console.log('>a1'); continue;
        case 1: console.log('>a5'); break;
        default: console.log('>default'); break;
    }
}
console.log(f(1));
/* OUTPUT
Uncaught SyntaxError: Illegal continue statement: no surrounding iteration statement
*/


// Index: 5
// 测试 no break 的情况
function f(p) {
    console.log('round: ', p);
    switch (p) {
        case 2: console.log('>a1');
        case 1: console.log('>a5'); break;
        default: console.log('>default'); break;
    }
    return '@f';
}
console.log(f(1));
console.log(f(2));
/* OUTPUT
round:  1
>a5
@f
round:  2
>a1
>a5
@f
*/


// Index: 6
// 测试 default 在前面的情况
function f(p) {
    console.log('round: ', p);
    switch (p) {
        default: console.log('>default');
        case 2: console.log('>a1'); break;
        case 1: console.log('>a5');
    }
    return '@f';
}
console.log(f(1));
console.log(f(3));
/* OUTPUT
round:  1
>a5
@f
round:  3
>default
>a1
@f
*/


// Index: 7
// 空的 switch
function f(p) {
    console.log('round: ', p);
    switch (p) {
    }
    return '@f';
}
console.log(f(1));
/* OUTPUT
round:  1
@f
*/


// Index: 8
// 只有 default
function f(p) {
    console.log('round: ', p);
    switch (p) {
        default: console.log('default');
    }

    return '@f';
}
console.log(f(1));
/* OUTPUT
round:  1
default
@f
*/


// Index: 9
// 多个 default
function f(p) {
    console.log('round: ', p);
    switch (p) {
        default: console.log('default1');
        case 1: break;
        default: console.log('default2');
    }
    return '@f';
}
console.log(f(1));
/* OUTPUT
Uncaught SyntaxError: More than one default clause in switch statement
*/


// Index: 10
// 只有 case
function f(p) {
    console.log('round: ', p);
    switch (p) {
        case 1: console.log('c1');
    }
    return '@f';
}
console.log(f(1));
console.log(f(2));
/* OUTPUT
round:  1
c1
@f
round:  2
@f
*/

