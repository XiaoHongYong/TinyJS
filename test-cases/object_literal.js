
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
