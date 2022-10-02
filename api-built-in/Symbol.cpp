//
//  Symbol.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/31.
//

#include "BuiltIn.hpp"


// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Symbol
static void symbolConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    LockedSizedStringWrapper name;

    if (args.count > 0) {
        name = runtime->toSizedString(ctx, args[0]);
    }

    JsSymbol symbol(name);

    ctx->retValue = runtime->pushSymbolValue(symbol);
}

static void symbolFor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

static JsLibProperty symbolFunctions[] = {
    { "for", symbolFor },
    { "name", nullptr, "Symbol" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

void symbolPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.type != JDT_SYMBOL) {
        ctx->throwException(PE_TYPE_ERROR, "Symbol.prototype.toString requires that 'this' be a Symbol");
        return;
    }

    ctx->retValue = jsValueUndefined;
}

static JsLibProperty symbolPrototypeFunctions[] = {
    { "toString", symbolPrototypeToString },
};

void registerSymbol(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, symbolPrototypeFunctions, CountOf(symbolPrototypeFunctions));
    rt->objPrototypeSymbol = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeSymbol, prototypeObj);

    SET_PROTOTYPE(symbolFunctions, jsValuePrototypeSymbol);

    rt->setGlobalObject("Symbol",
        new JsLibObject(rt, symbolFunctions, CountOf(symbolFunctions), symbolConstructor));
}
