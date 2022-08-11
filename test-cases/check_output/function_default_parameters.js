function f(a, b='b', c='c') {
    console.log(a, b, c);
}

f(1, 2, 3);
f(1);
f(4, undefined);
f(5, null);


function g(a, [b, c]) {
    console.log(a, b, c);
}

g(1, [2, 3]);
g(1);


function h(a, ...b) {
    console.log(a, b);
}

h(1, [2, 3], 4);
h(1, 2, 3);
h(1);


function k(a, []) {
    console.log(a);
}

k(1, [2, 3]);
// k(1, 2);


function g1(a, [b, c]) {
    console.log(a, b, c);
    b = 4;
    console.log(a, b, c);
    console.log(arguments[1]);
}

var p2 = [2, 3];
g1(1, p2);
console.log(p2);

