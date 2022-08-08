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
static void error(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto message = JsUndefinedValue;
    if (args.count > 0) {
        message = runtime->toString(ctx, args[0]);
    }

    auto errObj = new JsObject(__errorPrototype);
    auto err = JsValue(JDT_LIB_OBJECT, runtime->pushObjValue(errObj));

    errObj->setByName(ctx, err, SS_MESSAGE, message);
    errObj->setByName(ctx, err, SS_STACK, runtime->pushString(SizedString(getStack(ctx))));

    ctx->stack.push_back(err);
}

static void errorToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto msg = ctx->vm->getMemberDot(ctx, thiz, SS_MESSAGE);
    if (msg.type == JDT_UNDEFINED) {
        ctx->stack.push_back(ctx->runtime->pushString(SS_ERROR));
    } else {
        ctx->stack.push_back(ctx->runtime->addString(__errorToStringPrefix, msg));
    }
}

static JsLibProperty errorProtoFunctions[] = {
    { "toString", errorToString },
};

void registerErrorAPIs(VMRuntimeCommon *rt) {
    __errorToStringPrefix = rt->pushStringValue(SS_ERR_STRING_PREFIX);

    auto errorPrototypeObj = new JsLibObject(rt, errorProtoFunctions, CountOf(errorProtoFunctions));
    __errorPrototype = JsValue(JDT_LIB_OBJECT, rt->pushObjValue(errorPrototypeObj));
    rt->setGlobalValue("Error", JsValue(JDT_NATIVE_FUNCTION, rt->pushNativeFunction(error)));
}
