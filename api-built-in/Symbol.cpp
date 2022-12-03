//
//  Symbol.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/31.
//

#include "BuiltIn.hpp"
#include "objects/JsPrimaryObject.hpp"


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

inline JsValue convertSymbolToJsValue(VMContext *ctx, const JsValue &thiz, const char *funcName) {
    if (thiz.type == JDT_SYMBOL) {
        return thiz;
    } else if (thiz.type == JDT_OBJ_SYMBOL) {
        auto obj = (JsSymbolObject *)ctx->runtime->getObject(thiz);
        return obj->value();
    }

    ctx->throwException(JE_TYPE_ERROR, "Symbol.prototype.%s requires that 'this' be a Symbol", funcName);
    return jsValueNotInitialized;
}

void symbolPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    ctx->retValue = convertSymbolToJsValue(ctx, thiz, "toString");
}

static JsLibProperty symbolPrototypeFunctions[] = {
    { "toString", symbolPrototypeToString },
};

void registerSymbol(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, symbolPrototypeFunctions, CountOf(symbolPrototypeFunctions));
    rt->objPrototypeSymbol = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeSymbol, prototypeObj);

    SET_PROTOTYPE(symbolFunctions, jsValuePrototypeSymbol);

    setGlobalLibObject("Symbol", rt, symbolFunctions, CountOf(symbolFunctions), symbolConstructor, jsValuePrototypeFunction);
}
