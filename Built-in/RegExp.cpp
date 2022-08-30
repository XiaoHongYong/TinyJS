//
//  RegExp.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"


void regExpConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    ctx->retValue = jsValueUndefined;
}

static JsLibProperty regExpFunctions[] = {
    { "name", nullptr, "RegExp" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};


void regExpPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.type != JDT_REGEX) {
        ctx->throwException(PE_TYPE_ERROR, "RegExp.prototype.toString requires that 'this' be a RegExp");
        return;
    }

    ctx->retValue = jsValueUndefined;
}

static JsLibProperty regExpPrototypeFunctions[] = {
    { "toString", regExpPrototypeToString },
};

void registerRegExp(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, regExpPrototypeFunctions, CountOf(regExpPrototypeFunctions));
    rt->objPrototypeRegex = prototypeObj;
    auto prototype = rt->pushObjValue(JDT_LIB_OBJECT, prototypeObj);
    assert(prototype == jsValuePrototypeRegExp);

    SET_PROTOTYPE(regExpFunctions, prototype);

    rt->setGlobalObject("RegExp",
        new JsLibObject(rt, regExpFunctions, CountOf(regExpFunctions), regExpConstructor));
}
