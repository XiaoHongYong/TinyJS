
function f1() {
    try {
        try {
            let a = xxyy;
        } catch (error) {
            console.log('in catch1');
            let b = xxy;;
        }
    } catch (error) {
        console.log('in catch2');
    }
}
f1();


function f2() {
    try {
        try {
            let a = xxyy;
        } finally  {
            console.log('in finally1');
            // let b = xxy;;
        }
    } catch (error) {
        console.log('in catch2');
    } finally {
        console.log('in finally2');
    }
}
f2();


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
    } catch (error) {
        console.log('in catch2');
    } finally {
        console.log('in finally2');
    }
}
f3();


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
    } catch (error) {
        console.log('in catch2');
    } finally {
        console.log('in finally2');
        // return 2;
    }

    console.log('f4 end');
    //return 'end';
}
console.log(f4());


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
    } catch (error) {
        console.log('in catch2', error);
    } finally {
        console.log('in finally2');
    }

    console.log('f4 end');
}
f41();


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
    } catch (error) {
        console.log('in catch2', error);
    } finally {
        console.log('in finally2');
    }

    console.log('f4 end');
}
f42();


function f5() {
    try {
        throw Error(['abc', 'def'])
    } catch (error) {
        console.log('in catch', error.toString());
    }
}
f5();
