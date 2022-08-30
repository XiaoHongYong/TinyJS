//
//  Number.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"


void numberConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0) {
        ctx->retValue = JsValue(JDT_INT32, 0);
    } else {
        auto v = args[0];
        if (v.type == JDT_NUMBER || v.type == JDT_INT32) {
            ctx->retValue = v;
        } else {
            auto runtime = ctx->runtime;
            auto n = runtime->toNumber(ctx, v);
            if (n == (int32_t)n) {
                ctx->retValue = JsValue(JDT_INT32, n);
            } else {
                ctx->retValue = runtime->pushDoubleValue(n);
            }
        }
    }
}


void numberIsNaN(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    ctx->retValue = jsValueUndefined;
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
    } else {
        auto v = runtime->getDouble(thiz);
        size = floatToString(v, buf, radix);
    }

    ctx->retValue = runtime->pushString(SizedString(buf, size));
}

static JsLibProperty numberPrototypeFunctions[] = {
    { "toString", numberPrototypeToString },
};

void registerNumber(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, numberPrototypeFunctions, CountOf(numberPrototypeFunctions));
    rt->objPrototypeNumber = prototypeObj;
    auto prototype = rt->pushObjValue(JDT_LIB_OBJECT, prototypeObj);
    assert(prototype == jsValuePrototypeNumber);

    SET_PROTOTYPE(numberFunctions, prototype);

    rt->setGlobalObject("Number",
        new JsLibObject(rt, numberFunctions, CountOf(numberFunctions), numberConstructor));
}
