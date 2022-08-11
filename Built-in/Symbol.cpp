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
    char *name = nullptr;

    if (args.count > 0) {
        string buf;
        auto ss = runtime->toSizedString(ctx, args[0], buf);
        name = new char[ss.len + 1];
        memcpy(name, ss.data, ss.len);
        name[ss.len - 1] = '\0';
    }

    JsSymbol symbol(name);

    ctx->stack.push_back(runtime->pushSymbolValue(symbol));
}

static void symbolFor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
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

    ctx->stack.push_back(JsUndefinedValue);
}

static JsLibProperty symbolPrototypeFunctions[] = {
    { "toString", symbolPrototypeToString },
};

void registerSymbol(VMRuntimeCommon *rt) {
    auto prototype = new JsLibObject(rt, symbolPrototypeFunctions, CountOf(symbolPrototypeFunctions));
    rt->objPrototypeSymbol = prototype;
    rt->prototypeSymbol = JsValue(JDT_OBJECT, rt->pushObjValue(prototype));

    auto idxPrototype = CountOf(symbolFunctions) - 1;
    assert(symbolFunctions[idxPrototype].name.equal("prototype"));
    symbolFunctions[idxPrototype].value = rt->prototypeSymbol;

    rt->setGlobalObject("Symbol",
        new JsLibObject(rt, symbolFunctions, CountOf(symbolFunctions), symbolConstructor));
}
