//
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
JsValue newJsError(VMContext *ctx, JsErrorType errType, const JsValue &message) {
    auto runtime = ctx->runtime;

    JsValue proto = __errorPrototype;
    switch (errType) {
        case PE_SYNTAX_ERROR: proto = __syntaxErrorPrototype; break;
        case PE_TYPE_ERROR: proto = __typeErrorPrototype; break;
        case PE_RANGE_ERROR: proto = __rangeErrorPrototype; break;
        case PE_REFERECNE_ERROR: proto = __referenceErrorPrototype; break;
        default: break;
    }

    auto errObj = new JsObject(proto);
    auto err = runtime->pushObjectValue(errObj);

    errObj->setByName(ctx, err, SS_MESSAGE, message.type != JDT_STRING ? runtime->toString(ctx, message) : message);
    errObj->setByName(ctx, err, SS_STACK, runtime->pushString(SizedString(getStack(ctx))));

    return err;
}

void errorConstructor(VMContext *ctx, JsErrorType errType, const Arguments &args) {
    JsValue message = jsValueUndefined;
    if (args.count > 0) {
        message = args[0];
    }

    ctx->retValue = newJsError(ctx, PE_OK, message);
}

// Error

static void errorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    errorConstructor(ctx, PE_OK, args);
}

static JsLibProperty errorFunctions[] = {
    { "name", nullptr, "Error" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

static void errorToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto name = ctx->vm->getMemberDot(ctx, thiz, SS_NAME);
    auto msg = ctx->vm->getMemberDot(ctx, thiz, SS_MESSAGE);

    if (msg.type == JDT_UNDEFINED) {
        ctx->retValue = name;
    } else {
        string message;
        auto s = runtime->toSizedString(ctx, name);
        message.append((cstr_t)s.data, s.len);

        message.append(": ");
        s = runtime->toSizedString(ctx, msg);
        message.append((cstr_t)s.data, s.len);

        ctx->retValue = runtime->pushString(SizedString(message));
    }
}

static JsLibProperty errorPrototypeFunctions[] = {
    { "name", nullptr, "Error" },
    { "message", nullptr, "" },
    { "toString", errorToString },
};

// SyntaxError

static void syntaxErrorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    errorConstructor(ctx, PE_REFERECNE_ERROR, args);
}

static JsLibProperty syntaxErrorFunctions[] = {
    { "name", nullptr, "SyntaxError" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

static JsLibProperty syntaxErrorPrototypeFunctions[] = {
    { "name", nullptr, "SyntaxError" },
    { "message", nullptr, "" },
    { "toString", errorToString },
};

// TypeError

static void typeErrorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    errorConstructor(ctx, PE_TYPE_ERROR, args);
}

static JsLibProperty typeErrorFunctions[] = {
    { "name", nullptr, "TypeError" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

static JsLibProperty typeErrorPrototypeFunctions[] = {
    { "name", nullptr, "TypeError" },
    { "message", nullptr, "" },
    { "toString", errorToString },
};

// ReferenceError

static void referenceErrorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    errorConstructor(ctx, PE_REFERECNE_ERROR, args);
}

static JsLibProperty referenceErrorFunctions[] = {
    { "name", nullptr, "ReferenceError" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

static JsLibProperty referenceErrorPrototypeFunctions[] = {
    { "name", nullptr, "ReferenceError" },
    { "message", nullptr, "" },
    { "toString", errorToString },
};

// RangeError

static void rangeErrorConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    errorConstructor(ctx, PE_REFERECNE_ERROR, args);
}

static JsLibProperty rangeErrorFunctions[] = {
    { "name", nullptr, "RangeError" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

static JsLibProperty rangeErrorPrototypeFunctions[] = {
    { "name", nullptr, "RangeError" },
    { "message", nullptr, "" },
    { "toString", errorToString },
};

void registerErrorAPIs(VMRuntimeCommon *rt) {
    //
    // Error
    //
    auto prototype = new JsLibObject(rt, errorPrototypeFunctions, CountOf(errorPrototypeFunctions), errorConstructor);
    __errorPrototype = rt->pushObjectValue(prototype);
    SET_PROTOTYPE(errorFunctions, __errorPrototype);
    rt->setGlobalObject("Error",
        new JsLibObject(rt, errorFunctions, CountOf(errorFunctions), errorConstructor));

    //
    // TypeError
    //
    prototype = new JsLibObject(rt, typeErrorPrototypeFunctions, CountOf(typeErrorPrototypeFunctions), typeErrorConstructor, __errorPrototype);
    __typeErrorPrototype = rt->pushObjectValue(prototype);
    SET_PROTOTYPE(typeErrorFunctions, __typeErrorPrototype);
    rt->setGlobalObject("TypeError",
        new JsLibObject(rt, typeErrorFunctions, CountOf(typeErrorFunctions), typeErrorConstructor));

    //
    // ReferenceError
    //
    prototype = new JsLibObject(rt, referenceErrorPrototypeFunctions, CountOf(referenceErrorPrototypeFunctions), referenceErrorConstructor, __errorPrototype);
    __referenceErrorPrototype = rt->pushObjectValue(prototype);
    SET_PROTOTYPE(referenceErrorFunctions, __referenceErrorPrototype);
    rt->setGlobalObject("ReferenceError",
        new JsLibObject(rt, referenceErrorFunctions, CountOf(referenceErrorFunctions), referenceErrorConstructor));

    //
    // SyntaxError
    //
    prototype = new JsLibObject(rt, syntaxErrorPrototypeFunctions, CountOf(syntaxErrorPrototypeFunctions), syntaxErrorConstructor, __errorPrototype);
    __syntaxErrorPrototype = rt->pushObjectValue(prototype);
    SET_PROTOTYPE(syntaxErrorFunctions, __syntaxErrorPrototype);
    rt->setGlobalObject("SyntaxError",
        new JsLibObject(rt, syntaxErrorFunctions, CountOf(syntaxErrorFunctions), syntaxErrorConstructor));

    //
    // RangeError
    //
    prototype = new JsLibObject(rt, rangeErrorPrototypeFunctions, CountOf(rangeErrorPrototypeFunctions), rangeErrorConstructor, __errorPrototype);
    __rangeErrorPrototype = rt->pushObjectValue(prototype);
    SET_PROTOTYPE(rangeErrorFunctions, __rangeErrorPrototype);
    rt->setGlobalObject("RangeError",
        new JsLibObject(rt, rangeErrorFunctions, CountOf(rangeErrorFunctions), rangeErrorConstructor));
}
