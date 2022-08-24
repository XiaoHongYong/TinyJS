//
//  Object.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "BuiltIn.hpp"


static void objectConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (args.count == 0) {
        auto idx = runtime->pushObjValue(new JsObject());
        ctx->retValue = JsValue(JDT_OBJECT, idx);
    } else {
        ctx->retValue = args[0];
    }
}

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

    ctx->retValue = JsUndefinedValue;
}

static JsLibProperty objectFunctions[] = {
    { "defineProperty", objectDefineProperty },
};

void objectPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    JsValue value;

    switch (thiz.type) {
        case JDT_UNDEFINED:
            value = runtime->pushString("[object Undefined]");
            break;
        case JDT_NULL:
            value = runtime->pushString("[object Null]");
            break;
        case JDT_BOOL:
            value = runtime->pushString("[object Boolean]");
            break;
        case JDT_INT32:
            value = runtime->pushString("[object Number]");
            break;
        case JDT_SYMBOL:
            value = runtime->pushString("[object Symbol]");
            break;
        case JDT_CHAR:
        case JDT_STRING:
            value = runtime->pushString("[object String]");
            break;
        case JDT_OBJECT:
            value = runtime->pushString("[object Object]");
            break;
        case JDT_REGEX:
            value = runtime->pushString("[object RegExp]");
            break;
        case JDT_ARRAY:
            value = runtime->pushString("[object Array]");
            break;
        case JDT_LIB_OBJECT:
            value = runtime->pushString("[object Object]");
            break;
        case JDT_NATIVE_FUNCTION:
        case JDT_FUNCTION:
            value = runtime->pushString("[object Function]");
            break;
        default:
            assert(0);
            value = runtime->pushString("[object Object]");
            break;
    }

    ctx->retValue = value;
}

static JsLibProperty objectPrototypeFunctions[] = {
    { "toString", objectPrototypeToString },
};

void registerObject(VMRuntimeCommon *rt) {
    auto prototype = new JsLibObject(rt, objectPrototypeFunctions, CountOf(objectPrototypeFunctions));
    prototype->setAsObjectPrototype();
    rt->objPrototypeObject = prototype;
    rt->prototypeObject = JsValue(JDT_OBJECT, rt->pushObjValue(prototype));

    rt->setGlobalObject("Object",
        new JsLibObject(rt, objectFunctions, CountOf(objectFunctions), objectConstructor));
}
