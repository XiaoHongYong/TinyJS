
var a = 3, b = 4;

function f1() {
    var a = 5, b = 6;

    {
        let c = 9, d = 10;
        eval('console.log(c, d); c = 11, d = 12');
        console.log(c, d);
    }

    console.log(a, b);

    {
        let e = 1, f = 2;
        eval('console.log(e, f); e = 7, f = 8');
        console.log(e, f);
    }

    console.log(a, b);
}

f1();
console.log(a, b); // 1, 1


function f2() {
    {
        let c = 3, d = 4;
        eval('let c = 1, d = 2');
        console.log(c, d);
    }
}

f2();


function f3() {
    var a = 1, b = 2;
    {
        let a = 3, b = 4;
        eval('var a = 5, b = 6'); // 异常: Identifier 'a' has already been declared
        console.log(a, b);
    }
    console.log(a, b);
}

f3();


function f4() {
    var a = 1, b = 2;

    function g() {
        eval('a = 5, b = 6');
        console.log(a, b);
    }

    g();
    console.log(a, b);
}

f4();


function f5() {
    var a = 1, b = 2;
    var g;

    {
        let a, b;
        eval(`
        g = function () {
            eval('a = 5, b = 6');
            console.log(a, b);
        }
        `)
    }

    g();
    console.log(a, b);
}

f5();



function f6() {
    var a = 1, b = 2;
    var g;

    {
        let a, b;
        
        g = function () {
            eval('a = 5, b = 6');
            console.log(a, b);
        }
    }

    g();
    console.log(a, b);
}

f6();


function f7() {
    var a = 1, b = 2;
    var g, h;

    for (var i = 0; i < 2; i++) {
        let a = 3, b = 4;
        
        if (i == 0) 
            g = function () {
                eval('a = 5, b = 6');
                console.log(a, b);
            }
        else
            h = function () {
                console.log(a, b);
            }
    }

    g();
    h();
    console.log(a, b);
}

f7();


function g1() {
    // var g, h;

    eval('{ function g() { console.log("g"); } }');
    eval('function h() { console.log("h"); }');

    g();
    h();
    console.log(a, b);
}

g1();



function f8() {
    var a = 1, b = 2;

    {
        let a = 3, b = 4;
        
        function g() {
            console.log(a++, b++);
        }
    }

    g();
    g();
}

f8();


function f9() {
    var a = 1, b = 2;

    if (0)
    {
        let a = 3, b = 4;
        
        function g() {
            console.log(a, b);
        }
    }

    g(); // Uncaught TypeError: g is not a function
}

f9();


function f10() {
    var a = 1, b = 2;

    g(); // Uncaught TypeError: g is not a function

    {
        let a = 3, b = 4;
        
        function g() {
            console.log(a, b);
        }
    }
}

f10();


function f11() {
    var a = 1, b = 2;

    {
        let a = 3, b = 4;

        function g() {
            console.log(a, b);
        }
    }

    g();

    {
        let a = 5, b = 6;

        function g() {
            console.log(a, b);
        }
    }

    g();
}

f11();
