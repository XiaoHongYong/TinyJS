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
        ctx->retValue = runtime->pushObjValue(JDT_LIB_OBJECT, new JsObject());
    } else {
        ctx->retValue = args[0];
    }
}


SizedString objectPrototypeToSizedString(const JsValue &thiz) {
    switch (thiz.type) {
        case JDT_UNDEFINED: return SizedString("[object Undefined]");
        case JDT_NULL: return SizedString("[object Null]");
        case JDT_BOOL: return SizedString("[object Boolean]");
        case JDT_INT32: return SizedString("[object Number]");
        case JDT_SYMBOL: return SizedString("[object Symbol]");
        case JDT_CHAR:
        case JDT_STRING: return SizedString("[object String]");
        case JDT_OBJECT: return SizedString("[object Object]");
        case JDT_REGEX: return SizedString("[object RegExp]");
        case JDT_ARRAY: return SizedString("[object Array]");
        case JDT_LIB_OBJECT: return SizedString("[object Object]");
        case JDT_NATIVE_FUNCTION:
        case JDT_FUNCTION: return SizedString("[object Function]");
        default:
            assert(0);
            return SizedString("[object Object]");
            break;
    }
}

SizedString definePropertyXetterToString(VMContext *ctx, const JsValue &xetter, string &buf) {
    if (xetter.type < JDT_OBJECT) {
        return ctx->runtime->toSizedString(ctx, xetter, buf);
    } else if (xetter.type == JDT_OBJECT) {
        return SizedString("#<Object>");
    } else {
        return objectPrototypeToSizedString(xetter);
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

    auto configurable = descriptorObj->getByName(ctx, descriptor, SS_CONFIGURABLE, jsValueNotInitialized);
    auto enumerable = descriptorObj->getByName(ctx, descriptor, SS_ENUMERABLE, jsValueNotInitialized);
    auto writable = descriptorObj->getByName(ctx, descriptor, SS_WRITABLE, jsValueNotInitialized);
    auto value = descriptorObj->getByName(ctx, descriptor, SS_VALUE, jsValueNotInitialized);
    auto get = descriptorObj->getByName(ctx, descriptor, SS_GET, jsValueNotInitialized);
    auto set = descriptorObj->getByName(ctx, descriptor, SS_SET, jsValueNotInitialized);

    if (get.type < JDT_FUNCTION && get.type > JDT_UNDEFINED) {
        string buf;
        SizedString str = definePropertyXetterToString(ctx, get, buf);
        ctx->throwException(PE_TYPE_ERROR, "Getter must be a function: %.*s", (int)str.len, str.data);
        return;
    }

    if (set.type < JDT_FUNCTION && set.type > JDT_UNDEFINED) {
        string buf;
        SizedString str = definePropertyXetterToString(ctx, set, buf);
        ctx->throwException(PE_TYPE_ERROR, "Setter must be a function: %.*s", (int)str.len, str.data);
        return;
    }

    if ((get.isValid() || set.isValid()) && (writable.isValid() || value.isValid())) {
        ctx->throwException(PE_TYPE_ERROR, "Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>");
    }

    JsProperty propDescriptor(jsValueNotInitialized, -1, -1, -1, -1);
    propDescriptor.setter = jsValueNotInitialized;

    if (configurable.isValid()) propDescriptor.isConfigurable = runtime->testTrue(configurable);
    if (enumerable.isValid()) propDescriptor.isEnumerable = runtime->testTrue(enumerable);
    if (writable.isValid()) propDescriptor.isWritable = runtime->testTrue(writable);

    if (get.isValid()) {
        propDescriptor.isGSetter = true;
        propDescriptor.value = get;
    } else {
        propDescriptor.value = value;
    }

    if (set.isValid()) {
        propDescriptor.isGSetter = true;
        propDescriptor.setter = set;
    }

    auto pObj = runtime->getObject(obj);
    pObj->defineProperty(ctx, prop, propDescriptor);

    ctx->retValue = thiz;
}

void stringPrototypeCharAt(VMContext *ctx, const JsValue &thiz, const Arguments &args);

bool getStringOwnPropertyDescriptor(VMContext *ctx, const JsValue &thiz, JsValue name, JsProperty &descriptorOut) {
    assert(thiz.type == JDT_CHAR || thiz.type == JDT_STRING);

    auto runtime = ctx->runtime;

    auto len = runtime->getStringLength(thiz);

    while (true) {
        if (name.type == JDT_NUMBER || name.type == JDT_INT32) {
            int32_t index;
            if (name.type == JDT_NUMBER) {
                auto v = runtime->getDouble(name);
                index = (int32_t)v;
                if (v != index) {
                    return false;
                }
            } else {
                index = name.value.n32;
            }

            if (index >= 0 && index < len) {
                ArgumentsX args(JsValue(JDT_INT32, index));
                stringPrototypeCharAt(ctx, thiz, args);
                descriptorOut.value = ctx->retValue;
                descriptorOut.isConfigurable = false;
                descriptorOut.isEnumerable = true;
                descriptorOut.isWritable = false;
                descriptorOut.isGSetter = false;
                return true;
            }
            return false;
        } else {
            bool ret = false;
            string buf;

            auto str = runtime->toSizedString(ctx, name, buf);
            if (str.equal(SS_LENGTH)) {
                descriptorOut.value = JsValue(JDT_INT32, len);
                descriptorOut.isConfigurable = false;
                descriptorOut.isEnumerable = true;
                descriptorOut.isWritable = false;
                descriptorOut.isGSetter = false;
                ret = true;
            } else {
                bool successful;
                auto n = str.atoi(successful);
                if (successful) {
                    name = JsValue(JDT_INT32, (uint32_t)n);
                    ret = true;
                }
            }

            return ret;
        }
    }
}

// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Object/getOwnPropertyDescriptor
void getOwnPropertyDescriptor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    ctx->retValue = jsValueUndefined;

    if (args.count < 1 || args[0].type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    } else if (args.count < 2) {
        return;
    }

    auto &obj = args[0];
    auto &name = args[1];

    if (obj.type < JDT_OBJECT) {
        return;
    }

    JsProperty descriptor;

    switch (obj.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
        case JDT_NULL:
            assert(0);
            return;
        case JDT_BOOL:
        case JDT_INT32:
        case JDT_NUMBER:
        case JDT_SYMBOL:
            return;
        case JDT_CHAR:
        case JDT_STRING:
            if (!getStringOwnPropertyDescriptor(ctx, obj, name, descriptor)) {
                return;
            }
            break;
        default: {
            auto pobj = runtime->getObject(obj);
            if (!pobj->getOwnPropertyDescriptor(ctx, name, descriptor)) {
                return;
            }
            break;
        }
    }

    // 将 descriptor 转换为 object
    auto desc = new JsObject();
    auto descValue = runtime->pushObjValue(JDT_OBJECT, desc);

    desc->setByName(ctx, descValue, SS_CONFIGURABLE, JsValue(JDT_BOOL, descriptor.isConfigurable));
    desc->setByName(ctx, descValue, SS_ENUMERABLE, JsValue(JDT_BOOL, descriptor.isEnumerable));

    if (descriptor.isGSetter) {
        desc->setByName(ctx, descValue, SS_GET, descriptor.value);
        desc->setByName(ctx, descValue, SS_SET, descriptor.setter);
    } else {
        desc->setByName(ctx, descValue, SS_WRITABLE, JsValue(JDT_BOOL, descriptor.isWritable));
        desc->setByName(ctx, descValue, SS_VALUE, descriptor.value.isValid() ? descriptor.value : jsValueUndefined);
    }

    ctx->retValue = descValue;
}

static JsLibProperty objectFunctions[] = {
    { "name", nullptr, "Object" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "defineProperty", objectDefineProperty },
    { "getOwnPropertyDescriptor", getOwnPropertyDescriptor },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

void objectPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto str = objectPrototypeToSizedString(thiz);
    ctx->retValue = ctx->runtime->pushString(str);
}

static JsLibProperty objectPrototypeFunctions[] = {
    { "toString", objectPrototypeToString },
};

void registerObject(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, objectPrototypeFunctions, CountOf(objectPrototypeFunctions));
    prototypeObj->setAsObjectPrototype();
    rt->objPrototypeObject = prototypeObj;
    auto prototype = rt->pushObjValue(JDT_LIB_OBJECT, prototypeObj);
    assert(prototype == jsValuePrototypeObject);

    SET_PROTOTYPE(objectFunctions, prototype);

    rt->setGlobalObject("Object",
        new JsLibObject(rt, objectFunctions, CountOf(objectFunctions), objectConstructor));
}
