//
//  RegExp.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "objects/JsRegExp.hpp"


void regExpConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    auto strVal = args.getAt(0, jsValueUndefined);
    if (strVal.type == JDT_UNDEFINED) {
        strVal = jsStringValueEmpty;
    }

    auto flagsVal = args.getAt(1, jsValueUndefined);
    if (flagsVal.type >= JDT_OBJECT) {
        flagsVal = runtime->jsObjectToString(ctx, flagsVal);
        if (ctx->error != JE_OK) {
            return;
        }
    }

    if (flagsVal.type == JDT_UNDEFINED) {
        flagsVal = jsStringValueEmpty;
    } else if (!flagsVal.isString()) {
        ctx->throwExceptionFormatJsValue(JE_SYNTAX_ERROR, "Invalid flags supplied to RegExp constructor '%.*s'", flagsVal);
        return;
    }

    LockedSizedStringWrapper strRe = ctx->runtime->toSizedStringStrictly(ctx, strVal);
    LockedSizedStringWrapper strFlags = ctx->runtime->toSizedStringStrictly(ctx, flagsVal);
    string all = stringPrintf("/%.*s/%.*s", strRe.len, strRe.data, strFlags.len, strFlags.data);

    uint32_t flags;
    if (!parseRegexpFlags(strFlags, flags)) {
        ctx->throwExceptionFormatJsValue(JE_SYNTAX_ERROR, "Invalid flags supplied to RegExp constructor '%.*s'", flagsVal);
        return;
    }
    std::regex re((cstr_t)strRe.data, strRe.len, (std::regex::flag_type)flags);
    auto reObj = new JsRegExp(SizedString(all), re, flags);

    ctx->retValue = runtime->pushObjectValue(reObj);
}

static JsLibProperty regExpFunctions[] = {
    { "name", nullptr, "RegExp" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};


void regExpPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.type != JDT_REGEX) {
        ctx->throwException(JE_TYPE_ERROR, "RegExp.prototype.toString requires that 'this' be a RegExp");
        return;
    }

    auto re = (JsRegExp *)ctx->runtime->getObject(thiz);
    auto &str = re->toString();

    ctx->retValue = ctx->runtime->pushString(SizedString(str));
}

static JsLibProperty regExpPrototypeFunctions[] = {
    { "toString", regExpPrototypeToString },
};

void registerRegExp(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, regExpPrototypeFunctions, CountOf(regExpPrototypeFunctions));
    rt->objPrototypeRegex = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeRegExp, prototypeObj);

    SET_PROTOTYPE(regExpFunctions, jsValuePrototypeRegExp);

    rt->setGlobalObject("RegExp",
        new JsLibObject(rt, regExpFunctions, CountOf(regExpFunctions), regExpConstructor));
}
