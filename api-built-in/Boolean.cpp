//
//  Boolean.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/18.
//

#include "BuiltIn.hpp"
#include "objects/JsPrimaryObject.hpp"


void booleanConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    JsValue value = jsValueFalse;
    if (args.count > 0) {
        value = makeJsValueBool(ctx->runtime->testTrue(args[0]));
    }

    if (thiz.isEmpty()) {
        // New
        auto obj = new JsBooleanObject(value);
        ctx->retValue = ctx->runtime->pushObject(obj);
    } else {
        ctx->retValue = value;
    }
}

static JsLibProperty booleanFunctions[] = {
    { "name", nullptr, "Boolean" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};


inline JsValue convertBoolToJsValue(VMContext *ctx, const JsValue &thiz, const char *funcName) {
    if (thiz.type == JDT_BOOL) {
        return thiz;
    } else if (thiz.type == JDT_OBJ_BOOL) {
        auto obj = (JsBooleanObject *)ctx->runtime->getObject(thiz);
        return obj->value();
    }

    ctx->throwException(JE_TYPE_ERROR, "Boolean.prototype.%s requires that 'this' be a Boolean", funcName);
    return jsValueEmpty;
}

void booleanPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = convertBoolToJsValue(ctx, thiz, "toString");
    if (!value.isValid()) {
        return;
    }

    ctx->retValue = getJsValueBool(value) ? jsStringValueTrue : jsStringValueFalse;
}

void booleanPrototypeValueOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    ctx->retValue = convertBoolToJsValue(ctx, thiz, "valueOf");
}

static JsLibProperty booleanPrototypeFunctions[] = {
    { "toString", booleanPrototypeToString },
    { "valueOf", booleanPrototypeValueOf },
};

void registerBoolean(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, booleanPrototypeFunctions, CountOf(booleanPrototypeFunctions));
    rt->objPrototypeBoolean = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeBool, prototypeObj);

    SET_PROTOTYPE(booleanFunctions, jsValuePrototypeBool);

    setGlobalLibObject("Boolean", rt, booleanFunctions, CountOf(booleanFunctions), booleanConstructor, jsValuePrototypeFunction);
}
