//
//  Error.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/31.
//

#include "BuiltIn.hpp"


static JsValue __errorPrototype;
static JsValue __errorToStringPrefix;
static SizedString SS_ERR_STRING_PREFIX = makeCommonString("Error: ");


// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/eval
static void _eval(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (args.count == 0) {
        ctx->stack.push_back(jsValueUndefined);
        return;
    }

    auto v = args[0];
    if (v.type == JDT_STRING) {
        VecVMStackScopes stackScopesGlobal;

        auto code = runtime->getString(v);
        auto stackScopes = ctx->stackScopesForNativeFunctionCall;
        if (stackScopes == nullptr) {
            stackScopesGlobal.push_back(runtime->globalScope);
            stackScopes = &stackScopesGlobal;
        }

        ctx->vm->eval((cstr_t)code.data, code.len, ctx, *stackScopes, args);
    } else if (v.type == JDT_CHAR) {
        ctx->retValue = jsValueUndefined;
    } else {
        ctx->retValue = v;
    }
}

static JsLibProperty errorFunctions[] = {
    { "name", nullptr, "eval" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

void registerEval(VMRuntimeCommon *rt) {
    rt->setGlobalObject("eval",
        new JsLibObject(rt, errorFunctions, CountOf(errorFunctions), _eval));
}
