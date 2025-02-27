﻿//
//  main.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#include "parser/Parser.hpp"
#include "interpreter/VirtualMachine.hpp"
#include "interpreter/VMRuntime.hpp"
#include "utils/unittest.h"


void test() {
    char buf[256];
    double v = 1.0 / 0.01;
    // printf("%d, %d\n", sizeof(double), sizeof (long double));
    floatToString(v, buf);
    printf("%lf, %s, %s\n", v, buf, std::to_string(v).c_str());

    v = -1.0 / 0.01;
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

    runUnittest(argc, argv);
    
    // cstr_t code = "function f() { var a = .1; console.log(a, 'hello'); console.trace(); } f();";
    // cstr_t code = "var a = { b : 1 }; a.b = 2; console.log(a.b, 'hello');";
    // cstr_t code = "function f() { console.log('f'); this.c = 1; } var b = new f(); console.log(b.b, b.c, b.d, 'hello');";
    // cstr_t code = "function f() { this.c = 'c'; } var a = { b : 'b' }; a.d = 'd'; f.prototype = a; var b = new f(); console.log(b.b, b.c, b.d, 'hello');";
    // cstr_t code = "function f() { this.c = 'c'; } function g() { console.log('g', this.c); return 'gg'; } var a = { g : g }; f.prototype = a; var b = new f(); console.log(b.c, b.g(), 'hello');";

    // cstr_t code = "var a = { b : 1 }; a.b = 2; console.log(a.b + 'hello' + a);";
    // cstr_t code = "var a = { b : 1 }; a.b = 'x: '; console.log(a.b + 'hello' + ', x');";
    // cstr_t code = "var a = String.fromCharCode(63, 64, 65, 66, 67.0, '67', '68.1', 'a', '69'); console.log(a.length, a, String.name, String.length, a.at(1), a.charAt(20), a.charAt(2));";
    // cstr_t code = "function f(a) { console.log('hello:', a); } var g = f; g('x');";
    // cstr_t code = "function f() { try { try { throw 'e1'; } catch (e) { console.log('exception1:', e); throw 'e2'; } finally { console.log('f1'); } } catch (e) { console.log('exception2:', e); } finally { console.log('f2'); } } f();";
    cstr_t code = "function f() { try { try { return 'e1'; } catch (e) { console.log('exception1:', e); throw 'e1'; } finally { console.log('f1'); } } finally { console.log('f2'); } } console.log(f());";

    JsVirtualMachine vm;

    VecVMStackScopes stackScopes;
    Arguments args;

    BinaryOutputStream stream;

    auto runtime = vm.defaultRuntime();
    auto ctx = runtime->mainCtx();

    vm.run(code, strlen(code), runtime);
    if (ctx->error) {
        auto err = runtime->toStringView(ctx, ctx->errorMessage);
        printf("Got exception: %.*s\n", int(err.len), err.data);
    }

    for (int i = 0; i < 3; i++) {
        if (runtime->onRunTasks()) {
            i = 0;
        } else {
            Sleep(1);
        }
    }

//    code = "g('y');";
//    vm.eval(code, strlen(code), runtime->mainCtx(), stackScopes, args);
//    if (runtime->mainCtx()->error) {
//        auto err = runtime->toStringView(ctx, ctx->errorMessage);
//        printf("Got exception: %.*s\n", int(err.len), err.data);
//    }

    for (int i = 1; i < argc; i++) {
        string code;
        if (readFile(argv[i], code)) {
            printf("Eval file: %s\n", argv[i]);
            vm.eval(code.c_str(), code.size(), ctx, stackScopes, args);
        } else {
            printf("Can NOT read file: %s\n", argv[i]);
        }
    }

    return 0;
}
