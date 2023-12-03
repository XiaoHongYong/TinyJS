//
//  RegExp.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "objects/JsRegExp.hpp"
#include "objects/JsArray.hpp"


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

    LockedStringViewWrapper strRe = ctx->runtime->toStringViewStrictly(ctx, strVal);
    LockedStringViewWrapper strFlags = ctx->runtime->toStringViewStrictly(ctx, flagsVal);
    string all = stringPrintf("/%.*s/%.*s", strRe.len, strRe.data, strFlags.len, strFlags.data);

    uint32_t flags;
    if (!parseRegexpFlags(strFlags, flags)) {
        ctx->throwExceptionFormatJsValue(JE_SYNTAX_ERROR, "Invalid flags supplied to RegExp constructor '%.*s'", flagsVal);
        return;
    }
    std::regex re((cstr_t)strRe.data, strRe.len, (std::regex::flag_type)flags);
    auto reObj = new JsRegExp(StringView(all), re, flags);

    ctx->retValue = runtime->pushObject(reObj);
}

static JsLibProperty regExpFunctions[] = {
    { "name", nullptr, "RegExp" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};


void regexp_exec(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    if (thiz.type != JDT_REGEX) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Method RegExp.prototype.exec called on incompatible receiver %.*s", thiz);
        return;
    }

    auto strVal = runtime->toString(ctx, args.getAt(0));
    auto strU16 = runtime->getString(strVal);
    auto regexp = (JsRegExp *)runtime->getObject(thiz);
    auto &re = regexp->getRegexp();
    auto flags = regexp->flags();
    int lastIndex = 0;
    StringView str = strU16.utf8Str();

    if (flags & RF_GLOBAL_SEARCH) {
        lastIndex = regexp->getByName(ctx, thiz, SS_LASTINDEX).value.n32;
        if (lastIndex < strU16.size()) {
            auto p = utf8ToUtf16Seek(str.data, str.len, lastIndex);
            str.len -= p - str.data;
            str.data = p;
        } else {
            str.len = 0;
        }
    }

    std::cmatch matches;
    if (std::regex_search((cstr_t)str.data, (cstr_t)str.data + str.len, matches, re)) {
        auto arr = new JsArray();
        auto ret = runtime->pushObject(arr);

        for (auto &m : matches) {
            arr->push(ctx, runtime->pushString(str.substr(m.first, m.second)));
        }

        auto index = utf8ToUtf16Length(str.data, (uint32_t)(matches[0].first - (cstr_t)str.data));
        lastIndex += index;
        arr->setByName(ctx, ret, SS_INDEX, makeJsValueInt32(lastIndex));
        arr->setByName(ctx, ret, SS_GROUPS, jsValueUndefined);
        arr->setByName(ctx, ret, SS_INPUT, strVal);
        regexp->setLastIndex(lastIndex + utf8ToUtf16Length((uint8_t *)matches[0].first,
                                                           (uint32_t)(matches[0].second - matches[0].first)));

        ctx->retValue = ret;
    } else {
        regexp->setLastIndex(0);

        ctx->retValue = jsValueNull;
    }
}

void regexp_test(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    if (thiz.type != JDT_REGEX) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Method RegExp.prototype.test called on incompatible receiver %.*s", thiz);
        return;
    }

    auto strVal = runtime->toString(ctx, args.getAt(0));
    auto strU16 = runtime->getString(strVal);
    auto regexp = (JsRegExp *)runtime->getObject(thiz);
    auto &re = regexp->getRegexp();
    auto flags = regexp->flags();
    int lastIndex = 0;
    StringView str = strU16.utf8Str();

    if (flags & RF_GLOBAL_SEARCH) {
        lastIndex = regexp->getByName(ctx, thiz, SS_LASTINDEX).value.n32;
        if (lastIndex < strU16.size()) {
            auto p = utf8ToUtf16Seek(str.data, str.len, lastIndex);
            str.len -= p - str.data;
            str.data = p;
        } else {
            str.len = 0;
        }
    }

    std::cmatch matches;
    if (std::regex_search((cstr_t)str.data, (cstr_t)str.data + str.len, matches, re)) {
        regexp->setLastIndex(lastIndex + utf8ToUtf16Length(str.data,
                                                           (uint32_t)(matches[0].second - (char *)str.data)));

        ctx->retValue = jsValueTrue;
    } else {
        regexp->setLastIndex(0);

        ctx->retValue = jsValueFalse;
    }
}

void regExpPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.type != JDT_REGEX) {
        ctx->throwException(JE_TYPE_ERROR, "RegExp.prototype.toString requires that 'this' be a RegExp");
        return;
    }

    auto re = (JsRegExp *)ctx->runtime->getObject(thiz);
    auto &str = re->toString();

    ctx->retValue = ctx->runtime->pushString(StringView(str));
}

static JsLibProperty regExpPrototypeFunctions[] = {
    { "exec", regexp_exec },
    { "test", regexp_test },
    { "toString", regExpPrototypeToString },
};

void registerRegExp(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, regExpPrototypeFunctions, CountOf(regExpPrototypeFunctions));
    rt->objPrototypeRegex = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeRegExp, prototypeObj);

    SET_PROTOTYPE(regExpFunctions, jsValuePrototypeRegExp);

    setGlobalLibObject("RegExp", rt, regExpFunctions, CountOf(regExpFunctions), regExpConstructor, jsValuePrototypeFunction);
}
