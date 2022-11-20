// Index: 0
// abs, ceil, floor, round, sign, sqrt
function f(r, method) {
    console.log('## Round: ', r, method);

    console.log(1, Math[method](-Infinity));
    console.log(2, Math[method](-1));
    // console.log(3, Math[method](-0));
    console.log(4, Math[method](0));
    console.log(5, Math[method](1));
    console.log(6, Math[method](Infinity));
    console.log(7, Math[method]("-1"));
    console.log(8, Math[method](-2));
    console.log(9, Math[method](null));
    console.log(10, Math[method](""));
    console.log(11, Math[method]([]));
    console.log(12, Math[method]([2]));
    console.log(13, Math[method]([1, 2]));
    console.log(14, Math[method]({}));
    console.log(15, Math[method]("string"));
    console.log(16, Math[method]());
    Math.random();
}
f(1, 'abs');
f(2, 'ceil');
f(3, 'floor');
f(4, 'round');
f(5, 'sign');
f(6, 'sqrt');
/* OUTPUT
## Round:  1 abs
1 Infinity
2 1
4 0
5 1
6 Infinity
7 1
8 2
9 0
10 0
11 0
12 2
13 NaN
14 NaN
15 NaN
16 NaN
## Round:  2 ceil
1 -Infinity
2 -1
4 0
5 1
6 Infinity
7 -1
8 -2
9 0
10 0
11 0
12 2
13 NaN
14 NaN
15 NaN
16 NaN
## Round:  3 floor
1 -Infinity
2 -1
4 0
5 1
6 Infinity
7 -1
8 -2
9 0
10 0
11 0
12 2
13 NaN
14 NaN
15 NaN
16 NaN
## Round:  4 round
1 -Infinity
2 -1
4 0
5 1
6 Infinity
7 -1
8 -2
9 0
10 0
11 0
12 2
13 NaN
14 NaN
15 NaN
16 NaN
## Round:  5 sign
1 -1
2 -1
4 0
5 1
6 1
7 -1
8 -1
9 0
10 0
11 0
12 1
13 NaN
14 NaN
15 NaN
16 NaN
## Round:  6 sqrt
1 NaN
2 NaN
4 0
5 1
6 Infinity
7 NaN
8 NaN
9 0
10 0
11 0
12 1.4142135623730951
13 NaN
14 NaN
15 NaN
16 NaN
*/


// Index: 1
// max, min, pow,
function f(r, method) {
    console.log('## Round: ', r, method);

    console.log(1, Math[method](Infinity, Infinity));
    console.log(1, Math[method](Infinity, NaN));
    console.log(1, Math[method](-Infinity, Infinity));
    console.log(1, Math[method](-Infinity, NaN));
    console.log(1, Math[method](10, -10));
    console.log(1, Math[method](11.2, 2));
    console.log(1, Math[method](11, 23.5));
}
f(0, 'max');
f(1, 'min');
f(2, 'pow');
/* OUTPUT
## Round:  0 max
1 Infinity
1 NaN
1 Infinity
1 NaN
1 10
1 11.2
1 23.5
## Round:  1 min
1 Infinity
1 NaN
1 -Infinity
1 NaN
1 -10
1 2
1 11
## Round:  2 pow
1 Infinity
1 NaN
1 Infinity
1 NaN
1 1e-10
1 125.43999999999998
1 2.969806142814286e+24
*/

