//
//  String.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"


void ucs2ToUtf8(uint16_t code, string &out) {
    if (code < 0x80) {
        out.push_back((uint8_t)code);
    } else if (code < 0x0800) {
        out.push_back((uint8_t)((code >> 6) | 0xC0));
        out.push_back((uint8_t)((code & 0x3F) | 0x80));
    } else {
        out.push_back((uint8_t)((code >> 12) | 0xE0));
        out.push_back((uint8_t)((code >> 6 & 0x3F) | 0x80));
        out.push_back((uint8_t)((code & 0x3F) | 0x80));
    }
}

static void stringConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (args.count > 0) {
        ctx->retValue = runtime->toString(ctx, args[0]);
    } else {
        ctx->retValue = JsStringValueEmpty;
    }
}

void stringFromCharCode(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    string str;
    for (uint32_t i = 0; i < args.count; i++) {
        auto &item = args.data[i];
        if (item.type == JDT_INT32) {
            // 按照数据类型都完全正确的情况处理
            if (item.value.n32 <= 0xFFFF) {
                ucs2ToUtf8(item.value.n32, str);
            }
            continue;
        }

        auto f = runtime->toNumber(ctx, item);
        ucs2ToUtf8((uint16_t)f, str);
    }

    auto poolStr = runtime->allocString((uint32_t)str.size());
    memcpy(poolStr.value.data, str.c_str(), str.size());
    ctx->retValue = runtime->pushString(JsString(poolStr));
}

void stringFromCodePoint(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    ctx->retValue = jsValueUndefined;
}

static JsLibProperty stringFunctions[] = {
    { "fromCharCode", stringFromCharCode },
    { "fromCodePoint", stringFromCodePoint },
    { "name", nullptr, "String" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};


void stringPrototypeAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    if (thiz.type != JDT_STRING && thiz.type != JDT_CHAR) {
        ctx->throwException(PE_TYPE_ERROR, "String.prototype.at requires that 'this' be a String");
        return;
    }

    SizedString str;
    uint8_t buf[32];
    if (thiz.type == JDT_STRING) {
        str = runtime->getString(thiz);
    } else {
        buf[0] = thiz.value.n32;
        str = SizedString(buf, 1);
    }

    int32_t index = 0;
    if (args.count > 0) {
        index = (int32_t)runtime->toNumber(ctx, args.data[0]);
    }

    if (index < 0) {
        index = str.len + index;
        if (index < 0) {
            ctx->retValue = jsValueUndefined;
            return;
        }
    }

    if (index < str.len) {
        ctx->retValue = JsValue(JDT_CHAR, str.data[index]);
    } else {
        ctx->retValue = jsValueUndefined;
    }
}

void stringPrototypeCharAt(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    if (thiz.type != JDT_STRING && thiz.type != JDT_CHAR) {
        ctx->throwException(PE_TYPE_ERROR, "String.prototype.at requires that 'this' be a String");
        return;
    }

    SizedString str;
    uint8_t buf[32];
    if (thiz.type == JDT_STRING) {
        str = runtime->getString(thiz);
    } else {
        buf[0] = thiz.value.n32;
        str = SizedString(buf, 1);
    }

    int32_t index = 0;
    if (args.count > 0) {
        index = (int32_t)runtime->toNumber(ctx, args.data[0]);
    }

    if (index < 0 || index >= str.len) {
        ctx->retValue = JsStringValueEmpty;
        return;
    }

    ctx->retValue = JsValue(JDT_CHAR, str.data[index]);
}

void stringPrototypeLength(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    if (thiz.type != JDT_STRING && thiz.type != JDT_CHAR) {
        ctx->throwException(PE_TYPE_ERROR, "String.prototype.length requires that 'this' be a String");
        return;
    }

    ctx->retValue = JsValue(JDT_INT32, runtime->getStringLength(thiz));
}

static JsLibProperty stringPrototypeFunctions[] = {
    { "at", stringPrototypeAt },
    { "charAt", stringPrototypeCharAt },
    makeJsLibPropertyGetter("length", stringPrototypeLength),
};

void registerString(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, stringPrototypeFunctions, CountOf(stringPrototypeFunctions));
    rt->objPrototypeString = prototypeObj;
    auto prototype = rt->pushObjValue(JDT_LIB_OBJECT, prototypeObj);
    assert(prototype == jsValuePrototypeString);

    SET_PROTOTYPE(stringFunctions, prototype);

    rt->setGlobalObject("String",
        new JsLibObject(rt, stringFunctions, CountOf(stringFunctions), stringConstructor));
}
