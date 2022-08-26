//
//  Boolean.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"


void booleanConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    bool value = false;
    if (args.count > 0) {
        value = ctx->runtime->testTrue(args[0]);
    }

    ctx->retValue = JsValue(JDT_BOOL, value);
}

static JsLibProperty booleanFunctions[] = {
    { "name", nullptr, "Boolean" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};


void booleanPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.type != JDT_BOOL) {
        ctx->throwException(PE_TYPE_ERROR, "Boolean.prototype.toString requires that 'this' be a Boolean");
        return;
    }

    ctx->retValue = thiz.value.n32 ? JsStringValueTrue : JsStringValueFalse;
}

static JsLibProperty booleanPrototypeFunctions[] = {
    { "toString", booleanPrototypeToString },
};

void registerBoolean(VMRuntimeCommon *rt) {
    auto prototype = new JsLibObject(rt, booleanPrototypeFunctions, CountOf(booleanPrototypeFunctions));
    rt->objPrototypeBoolean = prototype;
    rt->prototypeBoolean = rt->pushObjValue(JDT_LIB_OBJECT, prototype);

    auto idxPrototype = CountOf(booleanFunctions) - 1;
    assert(booleanFunctions[idxPrototype].name.equal("prototype"));
    booleanFunctions[idxPrototype].value = rt->prototypeBoolean;

    rt->setGlobalObject("Boolean",
        new JsLibObject(rt, booleanFunctions, CountOf(booleanFunctions), booleanConstructor));
}
