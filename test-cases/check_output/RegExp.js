// Index: 0
// exec
function f() {
    const myRe = /ab*/g;
    const str = "abbcdefabh";
    let myArray;
    while ((myArray = myRe.exec(str)) !== null) {
        console.log(myArray);
        console.log(myRe.lastIndex);
    }
}
f();
/* OUTPUT
[abb, groups: undefined, index: 0, input: abbcdefabh]
3
[ab, groups: undefined, index: 7, input: abbcdefabh]
9
*/


// Index: 1
// test
function f() {
    const myRe = /ab*/g;
    const str = "abbcdefabh";
    while (myRe.test(str)) {
        console.log(myRe.lastIndex);
    }
}
f();
/* OUTPUT
3
9
*/


// Index: 2
// test
function f() {
    const str = "hello world!";
    const result = /^hello/.test(str);

    console.log(result); // true
}
f();
/* OUTPUT
true
*/


// Index: 3
// test
function f() {
    const regex = /foo/g; // the "global" flag is set

    // regex.lastIndex is at 0
    console.log(regex.test("foo")); // true
    
    // regex.lastIndex is now at 3
    console.log(regex.test("foo")); // false
    
    // regex.lastIndex is at 0
    console.log(regex.test("barfoo")); // true
    
    // regex.lastIndex is at 6
    console.log(regex.test("foobar")); // false
    
    // regex.lastIndex is at 0
    console.log(regex.test("foobarfoo")); // true
    
    // regex.lastIndex is at 3
    console.log(regex.test("foobarfoo")); // true
    
    // regex.lastIndex is at 9
    console.log(regex.test("foobarfoo")); // false    
}
f();
/* OUTPUT
true
false
true
false
true
true
false
*/

