//
//  RegExp.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "../JsRegExp.hpp"


void regExpConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    string buf;
    SizedString strRe;
    if (args.count > 0) {
        strRe = runtime->toSizedString(ctx, args[0], buf);
    }
    auto re = new JsRegExp(strRe);

    ctx->retValue = runtime->pushObjValue(JDT_REGEX, re);
}

static JsLibProperty regExpFunctions[] = {
    { "name", nullptr, "RegExp" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};


void regExpPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.type != JDT_REGEX) {
        ctx->throwException(PE_TYPE_ERROR, "RegExp.prototype.toString requires that 'this' be a RegExp");
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
    auto prototype = rt->pushObjValue(JDT_LIB_OBJECT, prototypeObj);
    assert(prototype == jsValuePrototypeRegExp);

    SET_PROTOTYPE(regExpFunctions, prototype);

    rt->setGlobalObject("RegExp",
        new JsLibObject(rt, regExpFunctions, CountOf(regExpFunctions), regExpConstructor));
}
