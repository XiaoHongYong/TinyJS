// Index: 0
async function timeout(ms) {
    await new Promise((resolve) => {
        setTimeout(resolve, ms);
    });
}

async function asyncPrint(value, ms) {
    await timeout(ms);
    console.log(value);
}

await asyncPrint('hello world', 10);
console.log('Ended');
/* OUTPUT
Uncaught SyntaxError: await is only valid in async functions and the top level bodies of modules
*/


// Index: 1
var s = `
async function f1() {
    async function timeout(ms) {
        await new Promise((resolve) => {
            setTimeout(resolve, ms);
        });
    }

    async function asyncPrint(value, ms) {
        await timeout(ms);
        console.log(value);
    }

    await asyncPrint('hello world', 10);
    return 'Ended';
}
await f1();
`

await eval(s);
console.log('Evaled');
/* OUTPUT
Uncaught SyntaxError: await is only valid in async functions and the top level bodies of modules
*/


// Index: 2
var obj = {
    async t(ms) {
        await new Promise((resolve) => { setTimeout(resolve, ms); });
        console.log('t2 ended');
    }
};

await obj.t(10);
console.log('Ended');
/* OUTPUT
Uncaught SyntaxError: await is only valid in async functions and the top level bodies of modules
*/


// Index: 3
var obj = {
    async t(ms) {
        await new Promise((resolve) => { setTimeout(resolve, ms); });
        console.log('t2 ended');
    }
};

obj.t(10);
console.log('Ended');
/* OUTPUT
Ended
*/


// Index: 4
var obj = {
    async t() {
        console.log('t2 ended');
    }
};

await obj.t();
console.log('Ended');
/* OUTPUT
Uncaught SyntaxError: await is only valid in async functions and the top level bodies of modules
*/


// Index: 5
var obj = {
    async t(ms) {
        await new Promise((resolve) => { });
        console.log('t2 ended');
    }
};

obj.t(10);
console.log('Ended');
/* OUTPUT
Ended
*/

