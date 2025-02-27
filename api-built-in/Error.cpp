﻿//
//  Error.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/31.
//

#include "BuiltIn.hpp"
#include <stdio.h>


static JsValue __errorPrototype;
static JsValue __aggregateErrorPrototype;
static JsValue __evalErrorPrototype;
static JsValue __InternalErrorPrototype;
static JsValue __rangeErrorPrototype;
static JsValue __referenceErrorPrototype;
static JsValue __syntaxErrorPrototype;
static JsValue __typeErrorPrototype;
static JsValue __uriErrorPrototype;

static string getStack(VMContext *ctx) {
    string str;
    char buf[512];

    for (auto it = ctx->stackFrames.rbegin(); it != ctx->stackFrames.rend(); ++it) {
        auto f = (*it)->function;
        auto &name = f->name;
        int len;
        if (name.len) {
            len = snprintf(buf, CountOf(buf), "%-14.*s %d:%d\n", (int)name.len, name.data, f->line, f->col);
        } else {
            len = snprintf(buf, CountOf(buf), "(anonymous)    %d:%d\n", f->line, f->col);
        }

        if (len > 0 && len < CountOf(buf)) {
            str.append(buf, len);
        }
    }

    return str;
}

// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Error
JsValue newJsError(VMContext *ctx, JsError errType, const JsValue &message) {
    auto runtime = ctx->runtime;

    JsValue proto = __errorPrototype;
    switch (errType) {
        case JE_SYNTAX_ERROR: proto = __syntaxErrorPrototype; break;
        case JE_TYPE_ERROR: proto = __typeErrorPrototype; break;
        case JE_RANGE_ERROR: proto = __rangeErrorPrototype; break;
        case JE_REFERECNE_ERROR: proto = __referenceErrorPrototype; break;
        default: break;
    }

    auto errObj = new JsObject(proto);
    auto err = runtime->pushObject(errObj);

    errObj->setByName(ctx, err, SS_MESSAGE, message.isString() ? message : runtime->toString(ctx, message));
    errObj->setByName(ctx, err, SS_STACK, runtime->pushString(StringView(getStack(ctx))));

    return err;
}

void errorConstructor(VMContext *ctx, JsError errType, const Arguments &args) {
    JsValue message = jsValueUndefined;
    if (args.count > 0) {
        message = args[0];
    }

    ctx->retValue = newJsError(ctx, JE_OK, message);
}

// Error

static void errorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    errorConstructor(ctx, JE_OK, args);
}

static JsLibProperty errorFunctions[] = {
    { "name", nullptr, "Error" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};

static void errorToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto name = ctx->vm->getMemberDot(ctx, thiz, SS_NAME);
    auto msg = ctx->vm->getMemberDot(ctx, thiz, SS_MESSAGE);

    if (msg.type == JDT_UNDEFINED) {
        ctx->retValue = name;
    } else {
        string message;
        auto s = runtime->toStringView(ctx, name);
        message.append((cstr_t)s.data, s.len);

        message.append(": ");
        s = runtime->toStringView(ctx, msg);
        message.append((cstr_t)s.data, s.len);

        ctx->retValue = runtime->pushString(StringView(message));
    }
}

static JsLibProperty errorPrototypeFunctions[] = {
    { "name", nullptr, "Error" },
    { "message", nullptr, nullptr, jsStringValueEmpty.asProperty(JP_WRITABLE | JP_CONFIGURABLE) },
    { "toString", errorToString },
};

// SyntaxError

static void syntaxErrorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    errorConstructor(ctx, JE_REFERECNE_ERROR, args);
}

static JsLibProperty syntaxErrorFunctions[] = {
    { "name", nullptr, "SyntaxError" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};

static JsLibProperty syntaxErrorPrototypeFunctions[] = {
    { "name", nullptr, "SyntaxError" },
    { "message", nullptr, nullptr, jsStringValueEmpty.asProperty(JP_WRITABLE | JP_CONFIGURABLE) },
    { "toString", errorToString },
};

// TypeError

static void typeErrorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    errorConstructor(ctx, JE_TYPE_ERROR, args);
}

static JsLibProperty typeErrorFunctions[] = {
    { "name", nullptr, "TypeError" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};

static JsLibProperty typeErrorPrototypeFunctions[] = {
    { "name", nullptr, "TypeError" },
    { "message", nullptr, nullptr, jsStringValueEmpty.asProperty(JP_WRITABLE | JP_CONFIGURABLE) },
    { "toString", errorToString },
};

// ReferenceError

static void referenceErrorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    errorConstructor(ctx, JE_REFERECNE_ERROR, args);
}

static JsLibProperty referenceErrorFunctions[] = {
    { "name", nullptr, "ReferenceError" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};

static JsLibProperty referenceErrorPrototypeFunctions[] = {
    { "name", nullptr, "ReferenceError", },
    { "message", nullptr, nullptr, jsStringValueEmpty.asProperty(JP_WRITABLE | JP_CONFIGURABLE) },
    { "toString", errorToString },
};

// RangeError

static void rangeErrorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    errorConstructor(ctx, JE_REFERECNE_ERROR, args);
}

static JsLibProperty rangeErrorFunctions[] = {
    { "name", nullptr, "RangeError" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};

static JsLibProperty rangeErrorPrototypeFunctions[] = {
    { "name", nullptr, "RangeError" },
    { "message", nullptr, nullptr, jsStringValueEmpty.asProperty(JP_WRITABLE | JP_CONFIGURABLE) },
    { "toString", errorToString },
};

void registerErrorAPIs(VMRuntimeCommon *rt) {
    //
    // Error
    //
    auto prototype = new JsLibObject(rt, errorPrototypeFunctions, CountOf(errorPrototypeFunctions), nullptr, nullptr);
    __errorPrototype = rt->pushObject(prototype);
    SET_PROTOTYPE(errorFunctions, __errorPrototype);
    setGlobalLibObject("Error", rt, errorFunctions, CountOf(errorFunctions), errorConstructor, jsValuePrototypeFunction);

    //
    // TypeError
    //
    prototype = new JsLibObject(rt, typeErrorPrototypeFunctions, CountOf(typeErrorPrototypeFunctions), nullptr, nullptr, __errorPrototype);
    __typeErrorPrototype = rt->pushObject(prototype);
    SET_PROTOTYPE(typeErrorFunctions, __typeErrorPrototype);
    setGlobalLibObject("TypeError", rt, typeErrorFunctions, CountOf(typeErrorFunctions), typeErrorConstructor, jsValuePrototypeFunction);

    //
    // ReferenceError
    //
    prototype = new JsLibObject(rt, referenceErrorPrototypeFunctions, CountOf(referenceErrorPrototypeFunctions), nullptr, nullptr, __errorPrototype);
    __referenceErrorPrototype = rt->pushObject(prototype);
    SET_PROTOTYPE(referenceErrorFunctions, __referenceErrorPrototype);
    setGlobalLibObject("ReferenceError", rt, referenceErrorFunctions, CountOf(referenceErrorFunctions), referenceErrorConstructor, jsValuePrototypeFunction);

    //
    // SyntaxError
    //
    prototype = new JsLibObject(rt, syntaxErrorPrototypeFunctions, CountOf(syntaxErrorPrototypeFunctions), nullptr, nullptr, __errorPrototype);
    __syntaxErrorPrototype = rt->pushObject(prototype);
    SET_PROTOTYPE(syntaxErrorFunctions, __syntaxErrorPrototype);
    setGlobalLibObject("SyntaxError", rt, syntaxErrorFunctions, CountOf(syntaxErrorFunctions), syntaxErrorConstructor, jsValuePrototypeFunction);

    //
    // RangeError
    //
    prototype = new JsLibObject(rt, rangeErrorPrototypeFunctions, CountOf(rangeErrorPrototypeFunctions), nullptr, nullptr, __errorPrototype);
    __rangeErrorPrototype = rt->pushObject(prototype);
    SET_PROTOTYPE(rangeErrorFunctions, __rangeErrorPrototype);
    setGlobalLibObject("RangeError", rt, rangeErrorFunctions, CountOf(rangeErrorFunctions), rangeErrorConstructor, jsValuePrototypeFunction);
}
