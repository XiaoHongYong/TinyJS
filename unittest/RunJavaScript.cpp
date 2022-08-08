//
//  RunJavaScript.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/8.
//

#include "../VirtualMachine.hpp"


#if UNIT_TEST

#include "../../Utils/unittest.h"

bool runJavascript(const string &code, string &output) {

    JsVirtualMachine vm;

    VecVMStackScopes stackScopes;
    Arguments args;

    BinaryOutputStream stream;

    auto runtime = vm.defaultRuntime();
    stackScopes.push_back(runtime->globalScope);
    runtime->mainVmCtx->curFunctionScope = runtime->globalScope;

    vm.eval(code.c_str(), code.size(), runtime->mainVmCtx, stackScopes, args);
    if (runtime->mainVmCtx->error) {
        printf("Got exception: %s\n", runtime->mainVmCtx->errorMessageString.c_str());
        return false;
    }

    return true;
}

TEST(RunJavaScript, outputCheck) {
}

#endif

