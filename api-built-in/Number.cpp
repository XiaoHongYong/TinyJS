//
//  Number.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "objects/JsPrimaryObject.hpp"


void numberConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    JsValue value;
    if (args.count == 0) {
        value = JsValue(JDT_INT32, 0);
    } else {
        value = args[0];
        if (value.type != JDT_NUMBER && value.type == JDT_INT32) {
            auto runtime = ctx->runtime;
            auto n = runtime->toNumber(ctx, value);
            if (n == (int32_t)n) {
                value = JsValue(JDT_INT32, n);
            } else {
                value = runtime->pushDoubleValue(n);
            }
        }
    }

    if (thiz.type == JDT_NOT_INITIALIZED) {
        // New
        auto obj = new JsNumberObject(value);
        ctx->retValue = ctx->runtime->pushObjectValue(obj);
    } else {
        ctx->retValue = value;
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


inline JsValue convertNumberToJsValue(VMContext *ctx, const JsValue &thiz, const char *funcName) {
    if (thiz.type == JDT_NUMBER) {
        return thiz;
    } else if (thiz.type == JDT_OBJ_NUMBER) {
        auto obj = (JsNumberObject *)ctx->runtime->getObject(thiz);
        return obj->value();
    }

    ctx->throwException(PE_TYPE_ERROR, "Number.prototype.%s requires that 'this' be a Number", funcName);
    return jsValueNotInitialized;
}

void numberPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = convertNumberToJsValue(ctx, thiz, "toString");
    if (!value.isValid()) {
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
    if (value.type == JDT_INT32) {
        size = itoa(value.value.n32, buf, radix);
    } else {
        auto v = runtime->getDouble(value);
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
    rt->setPrototypeObject(jsValuePrototypeNumber, prototypeObj);

    SET_PROTOTYPE(numberFunctions, jsValuePrototypeNumber);

    rt->setGlobalObject("Number",
        new JsLibObject(rt, numberFunctions, CountOf(numberFunctions), numberConstructor));
}
