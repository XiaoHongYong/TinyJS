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
    if (thiz.type == JDT_NUMBER || thiz.type == JDT_INT32) {
        return thiz;
    } else if (thiz.type == JDT_OBJ_NUMBER) {
        auto obj = (JsNumberObject *)ctx->runtime->getObject(thiz);
        return obj->value();
    }

    ctx->throwException(PE_TYPE_ERROR, "Number.prototype.%s requires that 'this' be a Number", funcName);
    return jsValueNotInitialized;
}

void numberPrototypeToExponential(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = convertNumberToJsValue(ctx, thiz, "toExponential");
    if (!value.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    auto d = runtime->toNumber(ctx, value);
    uint32_t size = 0;
    char buf[256];

    if (args.getAt(0).type == JDT_UNDEFINED) {
        size = floatToStringEx(d, buf, sizeof(buf), -1, F_TRIM_TAILING_ZERO | F_EXPONENTIAL_NOTATION);
    } else {
        auto fractionDigits = args.getIntAt(ctx, 0);
        if (fractionDigits < 0 || fractionDigits > 100) {
            ctx->throwException(PE_RANGE_ERROR, "toExponential() argument must be between 0 and 100");
            return;
        }

        size = floatToStringEx(d, buf, sizeof(buf), fractionDigits + 1, F_EXPONENTIAL_NOTATION);
    }

    ctx->retValue = runtime->pushString(SizedString(buf, size));
}

void numberPrototypeToFixed(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = convertNumberToJsValue(ctx, thiz, "toFixed");
    if (!value.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    auto d = runtime->toNumber(ctx, value);
    uint32_t size = 0;
    char buf[4096];

    auto precision = args.getIntAt(ctx, 0, 0);
    if (precision < 0 || precision > 100) {
        ctx->throwException(PE_RANGE_ERROR, "toFixed() digits argument must be between 0 and 100");
        return;
    }

    if (abs(d) < 1e21) {
        size = floatToStringEx(d, buf, sizeof(buf), precision + 1, F_FIXED_DIGITS);
    } else {
        size = floatToString(d, buf);
    }

    ctx->retValue = runtime->pushString(SizedString(buf, size));
}

void numberPrototypeToPrecision(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = convertNumberToJsValue(ctx, thiz, "toPrecision");
    if (!value.isValid()) {
        return;
    }

    auto runtime = ctx->runtime;
    auto d = runtime->toNumber(ctx, value);

    uint32_t size = 0;
    char buf[256];
    if (args.getAt(0).type == JDT_UNDEFINED) {
        size = floatToStringEx(d, buf, sizeof(buf), -1, F_TRIM_TAILING_ZERO);
    } else {
        auto precision = args.getIntAt(ctx, 0);
        if (precision <= 0 || precision > 100) {
            ctx->throwException(PE_RANGE_ERROR, "toPrecision() argument must be between 1 and 100");
            return;
        }

        size = floatToStringEx(d, buf, sizeof(buf), precision, F_FIXED_PRECISION);
    }

    ctx->retValue = runtime->pushString(SizedString(buf, size));
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

    char buf[2048];
    size_t size;
    if (value.type == JDT_INT32) {
        size = itoa(value.value.n32, buf, radix);
    } else {
        auto v = runtime->getDouble(value);
        size = floatToStringWithRadix(v, buf, sizeof(buf), radix);
    }

    ctx->retValue = runtime->pushString(SizedString(buf, size));
}

void numberPrototypeValueOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    ctx->retValue = convertNumberToJsValue(ctx, thiz, "valueOf");
}

static JsLibProperty numberPrototypeFunctions[] = {
    { "toExponential", numberPrototypeToExponential },
    { "toFixed", numberPrototypeToFixed },
    { "toLocaleString()", numberPrototypeToString },
    { "toPrecision", numberPrototypeToPrecision },
    { "toString", numberPrototypeToString },
    { "valueOf", numberPrototypeValueOf },
};

void registerNumber(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, numberPrototypeFunctions, CountOf(numberPrototypeFunctions));
    rt->objPrototypeNumber = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeNumber, prototypeObj);

    SET_PROTOTYPE(numberFunctions, jsValuePrototypeNumber);

    rt->setGlobalObject("Number",
        new JsLibObject(rt, numberFunctions, CountOf(numberFunctions), numberConstructor));
}
