// Index: 0
function f() {
    let myFirstPromise = new Promise((resolve, reject) => {
        setTimeout( function() {
            resolve("Success!");  // Yay! Everything went well!
        }, 0);
    });

    myFirstPromise.then((successMessage) => {
        // successMessage is whatever we passed in the resolve(...) function above.
        // It doesn't have to be a string, but if it is only a succeed message, it probably will be.
        console.log("Yay1! " + successMessage);
        return 'y1';
    }).then((successMessage) => {
        // successMessage is whatever we passed in the resolve(...) function above.
        // It doesn't have to be a string, but if it is only a succeed message, it probably will be.
        console.log("Yay2! " + successMessage)
        return 'y2';
    }).then((successMessage) => {
        // successMessage is whatever we passed in the resolve(...) function above.
        // It doesn't have to be a string, but if it is only a succeed message, it probably will be.
        console.log("Yay3! " + successMessage)
        return 'y3';
    });
}
f();
/* OUTPUT
Yay1! Success!
Yay2! y1
Yay3! y2
*/


// Index: 1
// 调用2次 resolve, 实际上只有第一次会调用
function f() {
    let myFirstPromise = new Promise((resolve, reject) => {
        console.log('x1');
        resolve('s1');
        resolve('s2');
        throw 'e1';
    });

    myFirstPromise.then(function (successMessage) {
        console.log("Yay1! ", successMessage);
    });
}
f();
/* OUTPUT
x1
Yay1!  s1
*/


// Index: 2
// 检查返回的 promise 对象
function f() {
    let p = new Promise((resolve, reject) => {
        setTimeout(function() {
            console.log('x1');
            //resolve('s2');
            reject('r1');
            console.log('x2');
        });
    });
    let p1 = p.then((r)=>(console.log('resolved1:', r), 'r2'),
        function (r) {
            console.log('rejected:', r); throw 'j2';
        });
    let p2 = p1.catch(reason => console.log('catch: ', reason));
    let p3 = p2.finally(() => console.log('finally.'));
    console.log('p1 === p', p === p1);
    console.log('p1 === p2', p1 === p2);
    console.log('p2 === p3', p2 === p3);
    console.log('p1 === p3', p1 === p3);
    console.log('p === p3', p1 === p3);

    // setTimeout(()=>console.log('timeout0'));
    console.log('End');
}
f();
/* OUTPUT
p1 === p false
p1 === p2 false
p2 === p3 false
p1 === p3 false
p === p3 false
End
x1
x2
rejected: r1
catch:  j2
finally.
*/


// Index: 3
// 检查返回的 promise 对象
function f() {
    let p = new Promise((resolve, reject) => {
        setTimeout(function() {
            console.log('x1');
            //resolve('s2');
            reject('r1');
            console.log('x2');
        });
    });
    let p1 = p.then((r)=>(console.log('resolved1:', r), 'r2'),
        function (r) {
            console.log('rejected:', r);
        });
    let p2 = p1.catch(reason => console.log('catch: ', reason));
    let p3 = p2.finally(() => console.log('finally.'));
    console.log('p1 === p', p === p1);
    console.log('p1 === p2', p1 === p2);
    console.log('p2 === p3', p2 === p3);
    console.log('p1 === p3', p1 === p3);
    console.log('p === p3', p1 === p3);

    // setTimeout(()=>console.log('timeout0'));
    console.log('End');
}
f();
/* OUTPUT
p1 === p false
p1 === p2 false
p2 === p3 false
p1 === p3 false
p === p3 false
End
x1
x2
rejected: r1
finally.
*/


// Index: 4
// 多次 then, catch, finally，在前面的会先调用
function f() {
    let p = new Promise((resolve, reject) => {
        console.log('x1');
        reject('r1');
        resolve('s2');
        console.log('x2');
        throw 'e1';
    });

    p.finally(() => console.log('finally1.'));
    p.then((r)=>console.log('resolved1:', r), (r)=>console.log('rejected1:', r))
    p.then((r)=>console.log('resolved2:', r), (r)=>console.log('rejected2:', r))
    p.catch(reason => console.log('catch1: ', reason));
    p.finally(() => console.log('finally2.'));
    p.catch(reason => console.log('catch2: ', reason));
}
f();
/* OUTPUT
x1
x2
finally1.
rejected1: r1
rejected2: r1
catch1:  r1
finally2.
catch2:  r1
*/


// Index: 5
// 延迟的多次调用
function f() {
    let p = new Promise((resolve, reject) => {
        setTimeout(function() {
            console.log('x1');
            resolve('s2');
            reject('r1');
            console.log('x2');
        }, 0);
    });

    p.then((r)=>console.log('resolved:', r), (r)=>console.log('rejected:', r))
    p.finally(() => console.log('finally1.'));
    p.then((r)=>console.log('resolved:', r), (r)=>console.log('rejected:', r))
    p.catch(reason => console.log('catch1: ', reason));
    p.finally(() => console.log('finally2.'));
    p.catch(reason => console.log('catch2: ', reason));
}
f();
/* OUTPUT
x1
x2
resolved: s2
finally1.
resolved: s2
finally2.
*/


// Index: 6
// 在 promise catch 之后，会走 then 的 resolved.
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        throw 'e1';
    })
    .catch(reason => (console.log('catch: ', reason), reason + 'catch'))
    .then(function(r) {
        console.log('Resolved: ', r);
        return 'then resolved: ' + r;
    }, function(r) {
        console.log('Rejected: ', r);
        return 'then rejected: ' + r;
    })
    .finally(() => console.log('finally.'))
    .then(function(r) {
        console.log('Resolved-f: ', r);
        return 'then resolved-f: ' + r;
    }, function(r) {
        console.log('Rejected-f: ', r);
        return 'then rejected-f: ' + r;
    });
}
f();
/* OUTPUT
x1
catch:  e1
Resolved:  e1catch
finally.
Resolved-f:  then resolved: e1catch
*/


// Index: 7
// 在 promise finally 中 的异常：有后续的 then
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        resolve('s1');
    })
    .finally(function(r) {
        console.log('finally: ', r);
        throw 'e2';
    })
    .then(function(r) {
        console.log('Resolved-f: ', r);
        return 'then resolved-f: ' + r;
    }, function(r) {
        console.log('Rejected-f: ', r);
        return 'then rejected-f: ' + r;
    });
}
f();
/* OUTPUT
x1
finally:  undefined
Rejected-f:  e2
*/


// Index: 8
// 在 promise reject e1, finally 不会清除异常
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        reject('e1');
    })
    .finally(function(r) {
        console.log('fianlly-f: ', r);
    });
}
f();
/* OUTPUT
x1
fianlly-f:  undefined
*/


// Index: 9
// 在 promise reject e1, finally 不会清除异常
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        reject('e1');
    })
    .finally(function(r) {
        console.log('fianlly-f: ', r);
    })
    .then(undefined, function(r) {
        console.log('rejected-f: ', r);
    });
}
f();
/* OUTPUT
x1
fianlly-f:  undefined
rejected-f:  e1
*/


// Index: 10
// 在 promise throw e1, finally 不会清除异常
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        throw 'e1';
    })
    .finally(function(r) {
        console.log('fianlly-f: ', r);
    });
}
f();
/* OUTPUT
x1
fianlly-f:  undefined
*/


// Index: 11
// 在 promise finally 中 的异常：无有后续的 then
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        reject('e1');
    })
    .finally(function(r) {
        console.log('fianlly-f: ', r);
        throw 'e2';
    });
}
f();
/* OUTPUT
x1
fianlly-f:  undefined
*/


// Index: 12
// 在 promise then 中 的异常：无后续的 then
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        throw 'e1';
    });
}
f();
/* OUTPUT
x1
*/


// Index: 13
// 在 promise then 中 的异常：有后续的 then
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        throw 'e1';
    })
    .then(undefined, function(r) {
        console.log('Rejected-f: ', r);
    });
}
f();
/* OUTPUT
x1
Rejected-f:  e1
*/


// Index: 14
// 在 promise 中的异常被拦截
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        // resolve('s2');
        reject('r1');
        console.log('x2');
        throw 'e1';
    }).catch(reason => console.log('catch: ', reason))
    .finally(() => console.log('finally.'));
}
f();
/* OUTPUT
x1
x2
catch:  r1
finally.
*/


// Index: 15
// arguments 的长度
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        // resolve('s2');
        reject('r1', 'r2');
        console.log('x2');
        throw 'e1';
    }).then(undefined, function() { console.log(arguments.length);})
    .catch(function() { console.log('catch: ', arguments.length);})
    .finally(function() { console.log('finally:', arguments.length);});
}
f();
/* OUTPUT
x1
x2
1
finally: 0
*/


// Index: 16
// 在各处抛异常被
function f() {
    new Promise((resolve, reject) => {
        console.log('x1');
        // resolve('s2');
        reject('r1', 'r2');
        console.log('x2');
        throw 'e1';
    }).then(undefined, function() { console.log('reject1'); throw 'e1';})
    .catch(function() { console.log('catch: '); throw 'ecatch';});

    console.log('end');
}
f();
/* OUTPUT
x1
x2
end
reject1
catch:
*/


// Index: 17
// 多次 finally，只要有一个 then/catch 就会处理异常.
function f() {
    var p = new Promise((resolve, reject) => {
        console.log('x1');
        throw 'e1';
    });
    var p1 = p.finally(function() { console.log('finally1:'); });
    var p2 = p.finally(function() { console.log('finally2:'); });

    console.log('end');
}
f();
/* OUTPUT
x1
end
finally1:
finally2:
*/


// Index: 18
// 多次 finally，只要有一个 then/catch 就会处理异常.
function f() {
    var p = new Promise((resolve, reject) => {
        console.log('x1');
        throw 'e1';
    });
    var p1 = p.finally(function() { console.log('finally1:'); });
    var p2 = p.finally(function() { console.log('finally2:'); });

    p1.then(function() { console.log('resolve');}, function() { console.log('rejected');})

    console.log('end');
}
f();
/* OUTPUT
x1
end
finally1:
finally2:
rejected
*/


// Index: 19
// 多次 finally，只要有一个 then/catch 就会处理异常.
function f() {
    var p = new Promise((resolve, reject) => {
        console.log('x1');
        throw 'e1';
    });
    var p1 = p.finally(function() { console.log('finally1:'); throw 'e2'; });
    var p2 = p.finally(function() { console.log('finally2:'); });

    p1.then(function() { console.log('resolve');}, function() { console.log('rejected');})
    p2.then(function() { console.log('resolve');}, function() { console.log('rejected');})

    console.log('end');
}
f();
/* OUTPUT
x1
end
finally1:
finally2:
rejected
rejected
*/


// Index: 20
// resolve: value is Promise
function f() {
    var p = new Promise(function() {});

    var p1 = Promise.resolve(p);

    console.log(p === p1);
}
f();
/* OUTPUT
true
*/


// Index: 21
// thenable
function f() {
    var p1 = Promise.resolve({
        then: undefined
    });
    console.log(p1 instanceof Promise) // true，这是一个 Promise 对象
    p1.finally(()=> console.log('finally.'));

    console.log('end');
}
f();
/* OUTPUT
true
end
finally.
*/


// Index: 22
// thenable
function f() {
    var p1 = Promise.resolve({
        then: function(onFulfill, onReject) { console.log('thenable1'); onFulfill("fulfilled!"); }
    });
    // 我们的实现会提前执行 Promise.resolve thenable 的函数，所以 console.log 会在后面.
    // console.log(p1 instanceof Promise) // true，这是一个 Promise 对象

    p1.then(function(v) {
        setTimeout(function() {
            console.log('then1', v); // 输出"fulfilled!"
        });
        return 'then1 ret.';
    });

    p1.then(function(v) {
        console.log('then2', v); // 输出"fulfilled!"
        return 'then2 ret.';
    });

    p1.then(function(v) {
        console.log('then3', v); // 输出"fulfilled!"
        return 'then3 ret.';
    });

    // console.log('end');
}
f();
/* OUTPUT
thenable1
then2 fulfilled!
then3 fulfilled!
then1 fulfilled!
*/


// Index: 23
// then 中返回 Promise
function f() {
    var p1 = Promise.resolve({
        then: function(onFulfill, onReject) { console.log('thenable1'); onFulfill("fulfilled!"); }
    });

    p1.then(function(v) {
        return new Promise(function(resolve) {
            setTimeout(function() {
                console.log('then1', v); // 输出"fulfilled!"
                resolve('then1 ret.');
            });
        });
    }).then(function(v) {
        console.log('then2', v); // 输出"fulfilled!"
        return 'then2 ret.';
    }).then(function(v) {
        console.log('then3', v); // 输出"fulfilled!"
        return 'then3 ret.';
    });
}
f();
/* OUTPUT
thenable1
then1 fulfilled!
then2 then1 ret.
then3 then2 ret.
*/


// Index: 24
// 有空的 then，传递的是之前的 resolve 值.
function f() {
    var p1 = new Promise(function(resolve) {
        setTimeout(function() {
            resolve('p1');
        }, 20);
    })
    .then()
    .then(function(v) {
        console.log('then2', v); // 输出"fulfilled!"
        return 'then2 ret.';
    }).then(function(v) {
        console.log('then2#', v); // 输出"fulfilled!"
        return 'then2# ret.';
    });

    console.log('end');
}
f();
/* OUTPUT
end
then2 p1
then2# then2 ret.
*/


// Index: 25
// promise 构造中返回 promise
function f() {
    new Promise(function(resolve) {
        console.log('p1');
        setTimeout(function() {
            console.log('p2');
            var p = new Promise(function(resolve) {
                console.log('p3');
                setTimeout(function () {
                    console.log('p4');
                    resolve('s1');
                }, 20);
            });

            resolve(p);
        }, 20);
    })
    .then((x)=>console.log('then1: ', x));

    console.log('end');
}
f();
/* OUTPUT
p1
end
p2
p3
p4
then1:  s1
*/


// Index: 26
// 递归的 promise
function f() {
    var p1 = new Promise(function(resolve) {
        setTimeout(function() {
            resolve(p1);
        }, 20);
    });

    console.log('end');
}
f();
/* OUTPUT-FIXED
end
*/

