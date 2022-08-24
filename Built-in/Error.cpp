//
//  Error.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/31.
//

#include "BuiltIn.hpp"
#include <stdio.h>


static JsValue __errorPrototype;
static JsValue __errorToStringPrefix;
static SizedString SS_ERR_STRING_PREFIX = makeCommonString("Error: ");

static string getStack(VMContext *ctx) {
    string str;
    char buf[512];

    for (auto it = ctx->stackFrames.rbegin(); it != ctx->stackFrames.rend(); ++it) {
        auto f = (*it)->function;
        auto &name = f->name;
        int len;
        if (name.len) {
            len = snprintf(buf, CountOf(buf), "%-14.*s %d:%d\n", (int)name.len, name.data, f->line, f->col);
        } else {
            len = snprintf(buf, CountOf(buf), "(anonymous)    %d:%d\n", f->line, f->col);
        }

        if (len > 0 && len < CountOf(buf)) {
            str.append(buf, len);
        }
    }

    return str;
}

// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Error
static void errorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto message = JsUndefinedValue;
    if (args.count > 0) {
        string buf;
        auto str = runtime->toSizedString(ctx, args[0], buf);
        string str2 = "Error: ";
        str2.append((cstr_t)str.data, str.len);
        message = runtime->pushString(str2);
    }

    auto errObj = new JsObject(__errorPrototype);
    auto err = JsValue(JDT_LIB_OBJECT, runtime->pushObjValue(errObj));

    errObj->setByName(ctx, err, SS_MESSAGE, message);
    errObj->setByName(ctx, err, SS_STACK, runtime->pushString(SizedString(getStack(ctx))));

    ctx->retValue = err;
}

static JsLibProperty errorFunctions[] = {
    { "name", nullptr, "Error" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

static void errorToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto msg = ctx->vm->getMemberDot(ctx, thiz, SS_MESSAGE);
    if (msg.type == JDT_UNDEFINED) {
        ctx->retValue = ctx->runtime->pushString(SS_ERROR);
    } else {
        ctx->retValue = ctx->runtime->addString(__errorToStringPrefix, msg);
    }
}

static JsLibProperty errorPrototypeFunctions[] = {
    { "toString", errorToString },
};

void registerErrorAPIs(VMRuntimeCommon *rt) {
    auto prototype = new JsLibObject(rt, errorPrototypeFunctions, CountOf(errorPrototypeFunctions));
    __errorPrototype = JsValue(JDT_OBJECT, rt->pushObjValue(prototype));

    auto idxPrototype = CountOf(errorFunctions) - 1;
    assert(errorFunctions[idxPrototype].name.equal("prototype"));
    errorFunctions[idxPrototype].value = __errorPrototype;

    rt->setGlobalObject("Error",
        new JsLibObject(rt, errorFunctions, CountOf(errorFunctions), errorConstructor));
}
