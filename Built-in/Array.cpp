//
//  Array.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "BuiltIn.hpp"
#include "../JsArray.hpp"


static void arrayConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (args.count == 0) {
        auto idx = runtime->pushObjValue(new JsArray());
        ctx->retValue = JsValue(JDT_ARRAY, idx);
        return;
    } else if (args.count == 1 && (args[0].type == JDT_INT32 || args[0].type == JDT_NUMBER)) {
        auto len = args[0];
        auto n = 0;
        if (len.type == JDT_NUMBER) {
            auto v = runtime->getDouble(len);
            if (v != (uint32_t)v) {
                ctx->throwException(PE_RANGE_ERROR, "Invalid array length");
                ctx->retValue = JsUndefinedValue;
                return;
            }
            n = (uint32_t)v;
        }

        auto arr = new JsArray(n);
        ctx->retValue = runtime->pushObjValue(JDT_ARRAY, arr);
        return;
    }

    auto arr = new JsArray();
    arr->push(ctx, args.data, args.count);
    ctx->retValue = runtime->pushObjValue(JDT_ARRAY, arr);
}

static JsLibProperty arrayFunctions[] = {
    { "name", nullptr, "Array" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

void objectPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args);

void arrayPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (thiz.type != JDT_ARRAY) {
        objectPrototypeToString(ctx, thiz, args);
        return;
    }

    JsArray *arr = (JsArray *)runtime->getObject(thiz);

    BinaryOutputStream stream;
    arr->toString(ctx, thiz, stream);
    ctx->retValue = runtime->pushString(stream.toSizedString());
}

static JsLibProperty arrayPrototypeFunctions[] = {
    { "toString", arrayPrototypeToString },
};

void registerArray(VMRuntimeCommon *rt) {
    auto prototype = new JsLibObject(rt, arrayPrototypeFunctions, CountOf(arrayPrototypeFunctions));

    rt->objPrototypeArray = prototype;
    rt->prototypeArray = JsValue(JDT_OBJECT, rt->pushObjValue(prototype));
    
    auto idxPrototype = CountOf(arrayFunctions) - 1;
    assert(arrayFunctions[idxPrototype].name.equal("prototype"));
    arrayFunctions[idxPrototype].value = prototype;

    rt->setGlobalObject("Array",
        new JsLibObject(rt, arrayFunctions, CountOf(arrayFunctions), arrayConstructor));
}
