//
//  Array.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "BuiltIn.hpp"
#include "objects/JsArray.hpp"
#include "objects/JsArguments.hpp"


static void arrayConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (args.count == 0) {
        ctx->retValue = runtime->pushObjectValue(new JsArray());
        return;
    } else if (args.count == 1 && (args[0].type == JDT_INT32 || args[0].type == JDT_NUMBER)) {
        auto len = args[0];
        auto n = 0;
        if (len.type == JDT_NUMBER) {
            auto v = runtime->getDouble(len);
            if (v != (uint32_t)v) {
                ctx->throwException(PE_RANGE_ERROR, "Invalid array length");
                ctx->retValue = jsValueUndefined;
                return;
            }
            n = (uint32_t)v;
        }

        auto arr = new JsArray(n);
        ctx->retValue = runtime->pushObjectValue(arr);
        return;
    }

    auto arr = new JsArray();
    arr->push(ctx, args.data, args.count);
    ctx->retValue = runtime->pushObjectValue(arr);
}

static JsLibProperty arrayFunctions[] = {
    { "name", nullptr, "Array" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

void objectPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args);

void arrayPrototypeAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto index = args.getIntAt(ctx, 0, 0);

    ctx->retValue = jsValueUndefined;

    switch (thiz.type) {
        case JDT_CHAR:
            if (index == -1 || index == 0) {
                ctx->retValue = thiz;
            }
            break;
        case JDT_STRING: {
            auto &str = ctx->runtime->getStringWithRandAccess(thiz);
            if (index < 0) {
                // 负数倒着访问
                index += str.size();
            }

            if (index < str.size()) {
                ctx->retValue = JsValue(JDT_CHAR, str.chartAt(index));
            }
            break;
        }
        case JDT_ARRAY: {
            auto jsthiz = (JsArray *)runtime->getObject(thiz);
            auto length = jsthiz->length();
            if (index < 0) {
                // 负数倒着访问
                index += length;
            }

            if (index < length) {
                ctx->retValue = jsthiz->getByIndex(ctx, thiz, index);
            }
            break;
        }
        case JDT_ARGUMENTS: {
            auto jsthiz = (JsArguments *)runtime->getObject(thiz);
            auto length = jsthiz->length();
            if (index < 0) {
                // 负数倒着访问
                index += length;
            }

            if (index < length) {
                ctx->retValue = jsthiz->getByIndex(ctx, thiz, index);
            }
            break;
        }
        default: {
            if (thiz.type >= JDT_OBJECT) {
                auto jsthiz = runtime->getObject(thiz);
                ctx->retValue = jsthiz->getByIndex(ctx, thiz, index);
            }
        }
    }
}

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
    { "at", arrayPrototypeAt },
    { "toString", arrayPrototypeToString },
};

void registerArray(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, arrayPrototypeFunctions, CountOf(arrayPrototypeFunctions));
    prototypeObj->setOfIteratorTrue();

    rt->objPrototypeArray = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeArray, prototypeObj);

    SET_PROTOTYPE(arrayFunctions, jsValuePrototypeArray);

    rt->setGlobalObject("Array",
        new JsLibObject(rt, arrayFunctions, CountOf(arrayFunctions), arrayConstructor));
}
