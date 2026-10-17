// Index: 0
function f(p) {
    var a = p??1;
    console.log(a);
}
f(2);
f(null);
f(undefined);
f('');
f(NaN);
/* OUTPUT
2
1
1

NaN
*/


// Index: 1
function f(p) {
    console.log(p, p ? 'x' : 'y');
}
f(2);
f(null);
f(undefined);
f('');
f(NaN);
/* OUTPUT
2 x
null y
undefined y
 y
NaN y
*/


// Index: 2 — 三元优先级低于比较：a > b ? c : d ≡ (a > b) ? c : d
var lastExpandedWidth = 480;
var WIDTH_MINI = 250;
var WIDTH_FULL = 480;
console.log(lastExpandedWidth > WIDTH_MINI ? lastExpandedWidth : WIDTH_FULL);
console.log(200 > WIDTH_MINI ? lastExpandedWidth : WIDTH_FULL);
console.log(1 > 2 ? 'yes' : 'no');
console.log(3 > 2 ? 'yes' : 'no');
/* OUTPUT
480
480
no
yes
*/


// Index: 3 — 嵌套三元右结合：a ? b : c ? d : e ≡ a ? b : (c ? d : e)
console.log(0 ? 1 : 0 ? 2 : 3);
console.log(1 ? 0 ? 2 : 3 : 4);
console.log(null ?? 1 ? 't' : 'f');
/* OUTPUT
3
3
t
*/

