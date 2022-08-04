//
//  Object.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "BuiltIn.hpp"


// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Object/defineProperty
void objectDefineProperty(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (args.count < 3 || args[2].type != JDT_OBJECT) {
        ctx->throwException(PE_TYPE_ERROR, "Property description must be an object");
        return;
    }

    auto obj = args[0];
    auto prop = args[1];
    auto descriptor = args[2];

    if (obj.type < JDT_OBJECT) {
        ctx->throwException(PE_TYPE_ERROR, "Object.defineProperty called on non-object");
        return;
    }

    auto descriptorObj = runtime->getObject(descriptor);
    JsProperty propDescriptor;
    propDescriptor.isConfigurable = descriptorObj->getBool(ctx, thiz, "configurable");
    propDescriptor.isEnumerable = descriptorObj->getBool(ctx, thiz, "enumerable");
    propDescriptor.isWritable = descriptorObj->getBool(ctx, thiz, "writable");

    auto getter = descriptorObj->get(ctx, thiz, "get");
    if (getter.type > JDT_OBJECT) {
        propDescriptor.isGetter = true;
        propDescriptor.value = getter;
    }

    auto pObj = runtime->getObject(obj);
    pObj->defineProperty(ctx, prop, propDescriptor, descriptorObj->get(ctx, thiz, "set"));
}

static JsLibProperty objectFunctions[] = {
    { "defineProperty", objectDefineProperty },
};

void registerObject(VMRuntimeCommon *rt) {
    rt->setGlobalObject("Object",
        new JsLibObject(rt, objectFunctions, CountOf(objectFunctions)));

}
