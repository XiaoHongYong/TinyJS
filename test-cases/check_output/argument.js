// Index: 0
/* NOT_SUPPORTED，这种情况在正常情况几乎不会存在，故不解决

//// 函数名和参数相同
function f1(a) {
    console.log(a);

    {
        //// 函数 a 不会修改到参数 a
        function a() {
            console.log('a');
        }
        console.log(a);
    }

    //// 仍然是 1
    console.log(a);
}
f1(1);
/* NOT_SUPPORTED OUTPUT
1
function a() {
            console.log('a');
        }
1
*/


//// 函数名和参数相同
function f2(a) {
    //// 参数 a 被修改为函数
    console.log(a);

    function a() {
        console.log('a');
    }

    console.log(arguments[0]);
}
f2(1);
/* OUTPUT
function a() {
        console.log('a');
    }
function a() {
        console.log('a');
    }
*/


// Index: 1
//// 变量名和参数相同
function f3(a) {
    console.log(a);

    var a = 2;

    console.log(a);
    console.log(arguments[0]);
}
f3(1);
/* OUTPUT
1
2
2
*/


// Index: 2
//// 重新对 arguments 赋值，不不会影响 a
function f4(a) {
    console.log(arguments[0]);
    arguments = [3, 4];
    console.log(arguments.length);
    console.log(arguments[0], a);
}
f4(1);
/* OUTPUT
1
2
3 1
*/


// Index: 3
//// arguments 在 arrow function 中，使用的 arguments 是外面的
function f5(a) {
    var f = x => arguments[1];

    console.log(f(a + 1));
}
f5(1, 20);
/* OUTPUT
20
*/


// Index: 4
//// 修改 argument 的 length 属性：并不会影响 arguments 内的参数个数
function f6(a, b) {
    console.log(a, b, arguments.length);
    arguments.length = 3;
    console.log(a, b, arguments[2], arguments.length);

    arguments.length = 1;
    console.log(a, b, arguments[1], arguments.length);
    b = 2;
    console.log(a, b, arguments[1], arguments.length);

    arguments[2] = '3'; arguments[3] = '4'; arguments['xx'] = 'xx';
    console.log(arguments[2], arguments['3'], arguments['xx']);

    arguments.length = {};
    console.log(arguments.length);

    arguments.length = undefined;
    console.log('set as undefined', arguments.length);

    arguments.length = 'xy';
    console.log(arguments.length);
}
f6(1, 20);
/* OUTPUT
1 20 2
1 20 undefined 3
1 20 20 1
1 2 2 1
3 4 xx
{}
set as undefined undefined
xy
*/


// Index: 5
//// arguments 出现在 eval 中
function f7(a, b) {
    eval('console.log(arguments.length, arguments[0], arguments[1])');
    eval('arguments[0] = 10; console.log(arguments[0], arguments[1])');
}
f7(1, 2);
/* OUTPUT
2 1 2
10 2
*/


// Index: 6
//// 参数个数不足的情况，修改 b，并不能修改到 arguments[1]
function f8(a, b) {
    console.log(a, b);
    b = 2;
    console.log(a, b);
    console.log(arguments.length, arguments[0], arguments[1]);
}
f8(1);
/* OUTPUT
1 undefined
1 2
1 1 undefined
*/


// Index: 7
//// 同名的参数，第二个参数会覆盖第一个参数
function f9(a, a) {
    console.log(a);
    console.log(arguments.length, arguments[0], arguments[1]);
    a = 3;
    console.log(arguments.length, arguments[0], arguments[1]);
}
f9(1, 2);
/* OUTPUT
2
2 1 2
2 1 3
*/

