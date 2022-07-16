//
//  main.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#include "Parser.hpp"
#include "VirtualMachine.hpp"


int main(int argc, const char * argv[]) {
    cstr_t code = "function f() { var a = .1; console.log(a, 'hello'); console.trace(); } f();";

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

    stackScopes.push_back(vm.globalScope());
    vm.eval(code, strlen(code), vm.mainVmContext(), stackScopes, args);

    BinaryOutputStream stream;
    vm.dump(code, strlen(code), stream);

    auto s = stream.startNew();
    printf("%s\n", code);
    printf("%.*s\n", (int)s.len, s.data);

    return 0;
}
