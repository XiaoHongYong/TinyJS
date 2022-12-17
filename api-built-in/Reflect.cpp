//
//  Reflect.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/11/27.
//

#include "BuiltIn.hpp"
#include "objects/JsArray.hpp"

void reflect_apply(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_construct(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_defineProperty(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_deleteProperty(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_get(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_getOwnPropertyDescriptor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_getPrototypeOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_has(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_isExtensible(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_ownKeys(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto objVal = args.getAt(0);
    if (objVal.type < JDT_OBJECT) {
        ctx->throwException(JE_TYPE_ERROR, "Reflect.ownKeys called on non-object");
        return;
    }

    auto obj = ctx->runtime->getObject(objVal);
    shared_ptr<IJsIterator> it(obj->getIteratorObject(ctx, false, true));
    JsValue key;
    auto arrObj = new JsArray();
    auto arr = ctx->runtime->pushObject(arrObj);

    while (it->next(nullptr, &key, nullptr)) {
        arrObj->push(ctx, key);
    }

    ctx->retValue = arr;
}

void reflect_preventExtensions(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_set(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void reflect_setPrototypeOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

static JsLibProperty reflectFunctions[] = {
    { "name", nullptr, "Reflect" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "apply", reflect_apply },
    { "construct", reflect_construct },
    { "defineProperty", reflect_defineProperty },
    { "deleteProperty", reflect_deleteProperty },
    { "get", reflect_get },
    { "getOwnPropertyDescriptor", reflect_getOwnPropertyDescriptor },
    { "getPrototypeOf", reflect_getPrototypeOf },
    { "has", reflect_has },
    { "isExtensible", reflect_isExtensible },
    { "ownKeys", reflect_ownKeys },
    { "preventExtensions", reflect_preventExtensions },
    { "set", reflect_set },
    { "setPrototypeOf", reflect_setPrototypeOf },
};

void registerReflect(VMRuntimeCommon *rt) {
    setGlobalLibObject("Reflect", rt, reflectFunctions, CountOf(reflectFunctions));
}
