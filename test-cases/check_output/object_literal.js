
function f1() {
    var obj = {
        async f() {
        },
        get 1() {
            return 0;
        },
        get "1"() {
            return 0;
        },
        get g() {
            return 0;
        },
        set g(a,) {
        },
        *h(a) {
        },
        f() {},
        a: function(){

        },
        ...f(),
        1: 1,
        "abc": 1,
    };
}
f1();


function f2() {
    var obj = {
        1: 1,
        "abc": 1,
    };

    obj[-1] = 2;

    for (var i in obj) {
        console.log(i, typeof i);
    }
}
f2();


function f3() {
    var obj = [1, 2];


    console.log(obj['+1']);
    obj[-1] = 2;
    console.log('len: ', obj.length);

    obj[500000000] = 3;
    console.log('len: ', obj.length);

    // obj.length = 50000000;
    console.log(obj.length, obj[1], obj[500000000]);


    obj[5000000000] = 3;
    console.log('len: ', obj.length);

    console.log(obj.length, obj[1], obj[5000000000]);

    for (var i in obj) {
        // obj[i + 1] = i;
        delete obj[1];
        console.log(i, typeof i);
    }
}
f3();
