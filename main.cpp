//
//  main.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#include "Parser.hpp"
#include "VirtualMachine.hpp"

void test() {
    char buf[256];
    double v = 1.0 / 0.0;
    // printf("%d, %d\n", sizeof(double), sizeof (long double));
    floatToString(v, buf);
    printf("%lf, %s, %s\n", v, buf, std::to_string(v).c_str());

    v = -1.0 / 0.0;
    floatToString(v, buf);
    printf("%lf, %s, %s\n", v, buf, std::to_string(v).c_str());

    v = 1.0 / 7;
    floatToString(v, buf);
    printf("%.30Lf, %s, %s\n", ((long double)1.0)/7, buf, std::to_string((long double)1.0/7).c_str());

    v = 1.110;
    floatToString(v, buf);
    printf("%lf, %s, %s\n", v, buf, std::to_string(v).c_str());

    v = 110;
    floatToString(v, buf);
    printf("%lf, %s, %s\n", v, buf, std::to_string(v).c_str());
}


int main(int argc, const char * argv[]) {
    // test();

    // cstr_t code = "function f() { var a = .1; console.log(a, 'hello'); console.trace(); } f();";
    // cstr_t code = "var a = { b : 1 }; a.b = 2; console.log(a.b, 'hello');";
    // cstr_t code = "function f() { console.log('f'); this.c = 1; } var b = new f(); console.log(b.b, b.c, b.d, 'hello');";
    // cstr_t code = "function f() { this.c = 'c'; } var a = { b : 'b' }; a.d = 'd'; f.prototype = a; var b = new f(); console.log(b.b, b.c, b.d, 'hello');";
    cstr_t code = "function f() { this.c = 'c'; } function g() { console.log('g', this.c); return 'gg'; } var a = { g : g }; f.prototype = a; var b = new f(); console.log(b.c, b.g(), 'hello');";

//    JSParser paser(code, strlen(code));
//
//    auto func = paser.parse(false);
//
//    BinaryOutputStream os;
//    //func->toString(os);
//
//    auto str = os.startNew();
//    printf("%s", str.data);

    JSVirtualMachine vm;
//    string errMessage;
//    auto nativeFunc = vm.compile(nullptr, func, errMessage);
//    auto ret = vm.eval(nativeFunc, vm.mainVmContext());

    VecVMStackScopes stackScopes;
    Arguments args;

    BinaryOutputStream stream;

    auto &runtime = vm.defaultRuntime();
    stackScopes.push_back(runtime.globalScope);
    vm.eval(code, strlen(code), runtime.mainVmCtx, stackScopes, args);
    if (runtime.mainVmCtx->error) {
        printf("Got exception: %s\n", runtime.mainVmCtx->errorMessage.c_str());
    }
//    vm.dump(stream);

//    auto s = stream.startNew();
//    printf("%s\n", code);
//    printf("%.*s\n", (int)s.len, s.data);

    return 0;
}
