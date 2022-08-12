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
        ctx->stack.push_back(JsUndefinedValue);
        return;
    }

    auto v = args[0];
    if (v.type == JDT_STRING) {
        auto code = runtime->getString(v);
        // ctx->vm->eval(code.data, code.len, ctx, , args);
        ctx->stack.push_back(JsUndefinedValue);
    } else if (v.type == JDT_CHAR) {
        ctx->stack.push_back(JsUndefinedValue);
    } else {
        ctx->stack.push_back(v);
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
