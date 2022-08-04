//
//  Symbol.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/31.
//

#include "BuiltIn.hpp"


// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Symbol
static void symbol(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
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
};

void registerSymbol(VMRuntimeCommon *rt) {
    rt->setGlobalObject("Symbol",
        new JsLibObject(rt, symbolFunctions, CountOf(symbolFunctions), symbol));

}
