
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


//// 函数名和参数相同
function f2(a) {
    //// 参数 a 被修改为函数
    console.log(a);

    function a() {
        console.log('a');
    }
}
f2(1);


//// 变量名和参数相同
function f3(a) {
    console.log(a);

    var a = 2;

    console.log(a);
    console.log(arguments[0]);
}
f3(1);


//// 重新对 arguments 赋值
function f4(a) {
    console.log(arguments[0]);
    arguments = [3, 4];
    console.log(arguments.length);
    console.log(arguments[0], a);
}
f4(1);


//// arguments 在 arrow function 中
function f5(a) {
    var f = x => arguments[1];

    console.log(f(a + 1));
}
f5(1, 20);


//// arguments 在 arrow function 中
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
