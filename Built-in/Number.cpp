//
//  Number.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"


void numberConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0) {
        ctx->stack.push_back(JsValue(JDT_INT32, 0));
    } else {
        auto v = args[0];
        if (v.type == JDT_NUMBER || v.type == JDT_INT32) {
            ctx->stack.push_back(v);
        } else {
            auto runtime = ctx->runtime;
            auto n = runtime->toNumber(ctx, v);
            if (n == (int32_t)n) {
                ctx->stack.push_back(JsValue(JDT_INT32, n));
            } else {
                ctx->stack.push_back(runtime->pushDoubleValue(n));
            }
        }
    }
}


void numberIsNaN(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
}

static JsLibProperty numberFunctions[] = {
    { "isNaN", numberIsNaN },
    { "name", nullptr, "Number" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};


void numberPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.type != JDT_INT32 && thiz.type != JDT_NUMBER) {
        ctx->throwException(PE_TYPE_ERROR, "Number.prototype.toString requires that 'this' be a Number");
        return;
    }

    auto runtime = ctx->runtime;
    
    int radix = 10;
    if (args.count > 0) {
        radix = (int)runtime->toNumber(ctx, args[0]);
        if (radix < 2 || radix > 36) {
            ctx->throwException(PE_RANGE_ERROR, "toString() radix argument must be between 2 and 36");
            return;
        }
    }

    char buf[64];
    size_t size;
    if (thiz.type == JDT_INT32) {
        size = itoa(thiz.value.n32, buf, radix);
        ctx->stack.push_back(runtime->pushString(SizedString(buf, size)));
    } else {
        auto v = runtime->getDouble(thiz);
        size = floatToString(v, buf, radix);
    }

    ctx->stack.push_back(runtime->pushString(SizedString(buf, size)));
}

static JsLibProperty numberPrototypeFunctions[] = {
    { "toString", numberPrototypeToString },
};

void registerNumber(VMRuntimeCommon *rt) {
    auto prototype = new JsLibObject(rt, numberPrototypeFunctions, CountOf(numberPrototypeFunctions));
    rt->objPrototypeNumber = prototype;
    rt->prototypeNumber = JsValue(JDT_OBJECT, rt->pushObjValue(prototype));

    auto idxPrototype = CountOf(numberFunctions) - 1;
    assert(numberFunctions[idxPrototype].name.equal("prototype"));
    numberFunctions[idxPrototype].value = rt->prototypeNumber;

    rt->setGlobalObject("Number",
        new JsLibObject(rt, numberFunctions, CountOf(numberFunctions), numberConstructor));
}
