// Index: 0
function f1() {
    try {
        try {
            let a = xxyy;
        } catch (e) {
            console.log('in catch1', e.name + ': ' + e.message);
            let b = xxy;;
        }
    } catch (e) {
        console.log('in catch2', e.name + ': ' + e.message);
    }
}
f1();
/* OUTPUT
in catch1 ReferenceError: xxyy is not defined
in catch2 ReferenceError: xxy is not defined
*/


// Index: 1
function f2() {
    try {
        try {
            let a = xxyy;
        } finally  {
            console.log('in finally1');
            // let b = xxy;;
        }
    } catch (e) {
        console.log('in catch2');
    } finally {
        console.log('in finally2');
    }
}
f2();
/* OUTPUT
in finally1
in catch2
in finally2
*/


// Index: 2
function f3() {
    try {
        try {
            let a = xxyy;
        } catch (e) {
            console.log('in catch1');
            throw "a";
        } finally  {
            console.log('in finally1');
            // let b = xxy;;
        }
    } catch (e) {
        console.log('in catch2');
    } finally {
        console.log('in finally2');
    }
}
f3();
/* OUTPUT
in catch1
in finally1
in catch2
in finally2
*/


// Index: 3
//// 在 finally 抛出的异常会导致在 try 中的 return 不生效
function f4() {
    try {
        try {
            let a = xxyy;
            return 1;
        } catch (e) {
            console.log('in catch1');
        } finally  {
            console.log('in finally1');
            // let b = xxy;;
            throw "a";
        }
    } catch (e) {
        console.log('in catch2');
    } finally {
        console.log('in finally2');
        // return 2;
    }

    console.log('f4 end');
    //return 'end';
}
console.log(f4());
/* OUTPUT
in catch1
in finally1
in catch2
in finally2
f4 end
undefined
*/


// Index: 4
//// finally 处理后，会继续外层的异常处理
function f41() {
    try {
        try {
            let a = xxyy;
            throw "try1";
        } finally  {
            console.log('in finally1');
            //throw "a";
        }
    } catch (e) {
        console.log('in catch2', e.name + ': ' + e.message);
    } finally {
        console.log('in finally2');
    }

    console.log('f4 end');
}
f41();
/* OUTPUT
in finally1
in catch2 ReferenceError: xxyy is not defined
in finally2
f4 end
*/


// Index: 5
//// finally 中的 return 会覆盖 throw 的异常，不会进入 catch2.
function f42() {
    try {
        try {
            let a = xxyy;
            throw "try1";
        } finally  {
            console.log('in finally1');
            return 1;
        }
    } catch (e) {
        console.log('in catch2', e.name + ': ' + e.message);
    } finally {
        console.log('in finally2');
    }

    console.log('f4 end');
}
f42();
/* OUTPUT
in finally1
in finally2
*/


// Index: 6
function f5() {
    try {
        throw Error(['abc', 'def'])
    } catch (e) {
        console.log(typeof e.message);
        console.log('in catch', e.name + ': ' + e.message);
    }

    try {
        throw 'abc';
    } catch (e) {
        console.log(typeof e);
    }

    try {
        throw NaN;
    } catch (e) {
        console.log(typeof e);
    }
}
f5();
/* OUTPUT
string
in catch Error: abc,def
string
number
*/


// Index: 7
function f6() {
    try {
    } catch (e) {
    }
    console.log('f6');
}
f6();
/* OUTPUT
f6
*/


// Index: 8
function f7() {
    try {
    } catch {
    }
    console.log('f7');
}
f7();
/* OUTPUT
f7
*/


// Index: 9
function f8() {
    try {
    } finally {
    }
    console.log('f8');
}
f8();
/* OUTPUT
f8
*/


// Index: 10
function f9() {
    if (1) // 如果 是 “0”， 在 chrome 下 会抛出异常 g 未定义，我们为了简化，仍然会把 g 添加到上层的 scope 中
        function g() { console.log(2); }
    else
        console.log(1);

    g();
    console.log('f9');
}
f9();
/* OUTPUT
2
f9
*/


// Index: 11
function f10() {
    try {
        throw [1, 2];
    } catch ([a, b=3, c=4]) {
        console.log('in catch1:', a, b, c);
    }
}
f10();
/* OUTPUT
in catch1: 1 2 4
*/

