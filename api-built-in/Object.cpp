//
//  Object.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "BuiltIn.hpp"
#include "objects/JsPrimaryObject.hpp"
#include "objects/JsArray.hpp"
#include "strings/JsString.hpp"
#include "interpreter/BinaryOperation.hpp"


static void objectConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (args.count == 0) {
        ctx->retValue = runtime->pushObjectValue(new JsObject());
    } else {
        ctx->retValue = args[0];
    }
}

using IJsIteratorPtr = shared_ptr<IJsIterator>;

IJsIteratorPtr getJsIteratorPtr(VMContext *ctx, const JsValue &obj, bool includeProtoProps = false, bool onIterateOf = false) {
    IJsIterator *it = nullptr;
    if (obj.type == JDT_CHAR || obj.type == JDT_STRING) {
        it = newJsStringIterator(ctx, obj, includeProtoProps);
    } else if (obj.type >= JDT_OBJECT) {
        auto pobj = ctx->runtime->getObject(obj);
        if (onIterateOf && !pobj->isOfIterable()) {
            return nullptr;
        }
        it = pobj->getIteratorObject(ctx, includeProtoProps);
    } else {
        return nullptr;
    }

    return IJsIteratorPtr(it);
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
        case JDT_OBJ_BOOL: return SizedString("[object Boolean]");
        case JDT_OBJ_NUMBER: return SizedString("[object Number]");
        case JDT_OBJ_STRING: return SizedString("[object String]");
        case JDT_ARRAY: return SizedString("[object Array]");
        case JDT_OBJ_GLOBAL_THIS:
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

    if (get.isValid() || set.isValid()) {
        propDescriptor.isGSetter = true;
        if (get.isValid()) propDescriptor.value = get;
        if (set.isValid()) propDescriptor.setter = set;
    } else {
        propDescriptor.isGSetter = false;
        propDescriptor.value = value;
    }

    auto pObj = runtime->getObject(obj);
    pObj->defineProperty(ctx, prop, propDescriptor);

    ctx->retValue = thiz;
}

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/defineProperties
void objectDefineProperties(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type < JDT_OBJECT) {
        ctx->throwException(PE_TYPE_ERROR, "Object.defineProperties called on non-object");
        return;
    }

    if (args.count == 1 || args[1].type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto runtime = ctx->runtime;
    if (args[1].type < JDT_OBJECT) {
        ctx->throwExceptionFormatJsValue(PE_TYPE_ERROR, "Property description must be an object: %.*s", args[1]);
        return;
    }

    auto obj = args[0];
    auto propsObj = runtime->getObject(args[1]);
    unique_ptr<IJsIterator> it;
    it.reset(propsObj->getIteratorObject(ctx));

    JsValue key, descriptor;
    while (it->next(nullptr, &key, &descriptor)) {
        objectDefineProperty(ctx, thiz, ArgumentsX(obj, key, descriptor));
    }

    ctx->retValue = args[0];
}

// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Object/create
void objectCreate(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0) {
        ctx->throwException(PE_TYPE_ERROR, "Object prototype may only be an Object or null: undefined");
        return;
    }

    auto runtime = ctx->runtime;
    auto proto = args[0];
    if (proto.type < JDT_OBJECT && proto.type != JDT_NULL) {
        ctx->throwExceptionFormatJsValue(PE_TYPE_ERROR, "Object prototype may only be an Object or null: %.*s", proto);
    }

    auto obj = new JsObject(proto);
    ctx->retValue = runtime->pushObjectValue(obj);

    if (args.count >= 2 && args[1].type > JDT_UNDEFINED) {
        objectDefineProperties(ctx, thiz, ArgumentsX(ctx->retValue, args[1]));
    }
}

void objectAssign(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto runtime = ctx->runtime;
    auto targ = args[0];
    switch (targ.type) {
        case JDT_BOOL: targ = runtime->pushObjectValue(new JsBooleanObject(targ)); break;
        case JDT_INT32: targ = runtime->pushObjectValue(new JsNumberObject(targ)); break;
        case JDT_NUMBER: targ = runtime->pushObjectValue(new JsNumberObject(targ)); break;
        case JDT_SYMBOL: ctx->throwException(PE_TYPE_ERROR, "Not supported Object.assign for symbol"); return;
        case JDT_CHAR: targ = runtime->pushObjectValue(new JsStringObject(targ)); break;
        case JDT_STRING: targ = runtime->pushObjectValue(new JsStringObject(targ)); break;
        default: break;
    }

    for (uint32_t i = 1; i < args.count; i++) {
        runtime->extendObject(ctx, targ, args[i], false);
    }

    ctx->retValue = targ;
}

void objectEntries(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto runtime = ctx->runtime;
    auto obj = args[0];

    auto arr = new JsArray();
    ctx->retValue = runtime->pushObjectValue(arr);

    auto it = getJsIteratorPtr(ctx, obj);
    if (it) {
        JsValue key, value;
        while (it->next(nullptr, &key, &value)) {
            auto item = new JsArray();
            item->push(ctx, key);
            item->push(ctx, value);

            arr->push(ctx, runtime->pushObjectValue(item));
        }
    }
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
void objectGetOwnPropertyDescriptor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
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
    auto descValue = runtime->pushObjectValue(desc);

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

void objectFreeze(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0) {
        ctx->retValue = jsValueUndefined;
        return;
    }

    auto obj = args[0];
    ctx->retValue = obj;
    if (obj.type < JDT_OBJECT) {
        return;
    }

    auto pobj = ctx->runtime->getObject(obj);
    pobj->changeAllProperties(ctx, false, false);
    pobj->preventExtensions(ctx);
}

void objectFromEntries(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "undefined is not iterable");
        return;
    }

    auto entries = args[0];
    auto runtime = ctx->runtime;

    IJsIteratorPtr it;
    if (entries.type == JDT_CHAR || entries.type == JDT_STRING) {
        it.reset(newJsStringIterator(ctx, entries, false));
    } else if (entries.type >= JDT_OBJECT) {
        auto obj = runtime->getObject(entries);
        if (!obj->isOfIterable()) {
            auto name = runtime->toTypeName(entries);
            ctx->throwException(PE_TYPE_ERROR, "%.*s is not iterable (cannot read property Symbol(Symbol.iterator))",
                                name.len, name.data);
            return;
        }
        it.reset(obj->getIteratorObject(ctx, false));
    } else {
        string buf;
        auto name = runtime->toTypeName(entries);
        auto s = runtime->toSizedString(ctx, entries, buf);
        ctx->throwException(PE_TYPE_ERROR, "%.*s %.*s is not iterable (cannot read property Symbol(Symbol.iterator))",
                            name.len, name.data, s.len, s.data);
        return;
    }

    auto pobj = new JsObject();
    auto obj = runtime->pushObjectValue(pobj);

    JsValue item;
    while (it->nextOf(item)) {
        if (item.type < JDT_OBJECT) {
            ctx->throwExceptionFormatJsValue(PE_TYPE_ERROR, "Iterator value %.*s is not an entry object", item);
            return;
        } else {
            auto entry = runtime->getObject(item);
            pobj->set(ctx, obj, entry->getByIndex(ctx, item, 0), entry->getByIndex(ctx, item, 1));
        }
    }

    ctx->retValue = obj;
}

void objectGetOwnPropertyDescriptors(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto obj = args[0];
    auto descsObj = new JsObject();
    auto descs = ctx->runtime->pushObjectValue(descsObj);

    auto it = getJsIteratorPtr(ctx, obj);
    if (it) {
        JsValue name, value;
        while (it->next(nullptr, &name, &value)) {
            objectGetOwnPropertyDescriptor(ctx, obj, ArgumentsX(obj, name));
            descsObj->set(ctx, descs, name, ctx->retValue);
        }
    }

    ctx->retValue = descs;
}

void objectGetOwnPropertyNames(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto obj = args[0];
    auto namesObj = new JsArray();
    auto names = ctx->runtime->pushObjectValue(namesObj);

    auto it = getJsIteratorPtr(ctx, obj);
    if (it) {
        JsValue name, value;
        while (it->next(nullptr, &name, nullptr)) {
            namesObj->push(ctx, name);
        }
    }

    ctx->retValue = names;
}

JsValue get__proto__(VMContext *ctx, const JsValue &value) {
    auto runtime = ctx->runtime;
    switch (value.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of undefined (reading '__proto__')");
            break;
        case JDT_NULL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of null (reading '__proto__')");
            break;
        case JDT_BOOL:
            return jsValuePrototypeBool;
        case JDT_INT32:
        case JDT_NUMBER:
            return jsValuePrototypeNumber;
        case JDT_CHAR:
        case JDT_STRING:
            return jsValuePrototypeString;
        case JDT_SYMBOL:
            return jsValuePrototypeSymbol;
        case JDT_NATIVE_FUNCTION:
            return jsValuePrototypeFunction;
        default: {
            auto obj = runtime->getObject(value);
            return obj->getByName(ctx, value, SS___PROTO__, jsValuePrototypeObject);
        }
    }

    return jsValueNull;
}

void objectGetPrototypeOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto obj = args[0];
    auto proto = get__proto__(ctx, obj);

    ctx->retValue = proto;
}

void hasOwnProperty(VMContext *ctx, const JsValue &obj, const JsValue &name) {
    assert(obj.type > JDT_NULL);

    auto runtime = ctx->runtime;
    JsValue ret = jsValueFalse;

    if (obj.type < JDT_OBJECT) {
        if (obj.type == JDT_CHAR || obj.type == JDT_STRING) {
            string buf;
            auto s = runtime->toSizedString(ctx, name, buf);
            if (s.equal(SS_LENGTH)) {
                ret = jsValueTrue;
            } else {
                bool successful;
                auto n = s.atoi(successful);
                if (successful) {
                    auto len = runtime->getStringLength(obj);
                    if (n >= 0 && n < len) {
                        ret = jsValueTrue;
                    }
                }
            }
        }
    } else {
        auto pobj = runtime->getObject(obj);
        JsProperty descriptor;

        if (pobj->getOwnPropertyDescriptor(ctx, name, descriptor)) {
            ret = jsValueTrue;
        }
    }

    ctx->retValue = ret;
}

void objectHasOwn(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto obj = args[0];
    JsValue name = args.getAt(1, jsStringValueUndefined);

    hasOwnProperty(ctx, obj, name);
}

void objectIs(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    JsValue left = args.getAt(0);
    JsValue right = args.getAt(1);

    if (left.type == JDT_NUMBER && right.type == JDT_NUMBER) {
        if (left.value.index == right.value.index) {
            ctx->retValue = jsValueTrue;
        } else {
            auto a = ctx->runtime->getDouble(left);
            auto b = ctx->runtime->getDouble(right);
            if (isnan(a) && isnan(b)) {
                ctx->retValue = jsValueTrue;
            } else {
                ctx->retValue = JsValue(JDT_BOOL, a == b);
            }
        }
    } else {
        ctx->retValue = JsValue(JDT_BOOL, relationalStrictEqual(ctx->runtime, left, right));
    }
}

void objectIsExtensible(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto obj = args.getAt(0);
    bool extensible;
    if (obj.type < JDT_OBJECT) {
        extensible = false;
    } else {
        auto pobj = ctx->runtime->getObject(obj);
        extensible = pobj->isExtensible();
    }

    ctx->retValue = JsValue(JDT_BOOL, extensible);
}

void objectIsFrozen(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto obj = args.getAt(0);
    if (obj.type < JDT_OBJECT) {
        ctx->retValue = jsValueTrue;
        return;
    }

    auto pobj = ctx->runtime->getObject(obj);
    auto extensible = pobj->isExtensible();
    if (extensible) {
        ctx->retValue = jsValueFalse;
        return;
    }

    bool isFrozen = !pobj->hasAnyProperty(ctx, true, true);

    ctx->retValue = JsValue(JDT_BOOL, isFrozen);
}

void objectPreventExtensions(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto obj = args[0];
    ctx->retValue = obj;
    if (obj.type < JDT_OBJECT) {
        return;
    }

    auto pobj = ctx->runtime->getObject(obj);
    pobj->preventExtensions(ctx);
}

static JsLibProperty objectFunctions[] = {
    { "name", nullptr, "Object" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "assign", objectAssign },
    { "create", objectCreate },
    { "defineProperties", objectDefineProperties },
    { "defineProperty", objectDefineProperty },
    { "entries", objectEntries },
    { "freeze", objectFreeze },
    { "fromEntries", objectFromEntries },
    { "getOwnPropertyDescriptor", objectGetOwnPropertyDescriptor },
    { "getOwnPropertyDescriptors", objectGetOwnPropertyDescriptors },
    { "getOwnPropertyNames", objectGetOwnPropertyNames },
    { "getPrototypeOf", objectGetPrototypeOf },
    { "hasOwn", objectHasOwn },
    { "is", objectIs },
    { "isExtensible", objectIsExtensible },
    { "isFrozen", objectIsFrozen },
    { "preventExtensions", objectPreventExtensions },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

void objectPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto str = objectPrototypeToSizedString(thiz);
    ctx->retValue = ctx->runtime->pushString(str);
}

void objectPrototypeHasOwnProperty(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.type <= JDT_NULL) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    JsValue name = args.count >= 1 ? args[0] : jsStringValueUndefined;

    hasOwnProperty(ctx, thiz, name);
}

static JsLibProperty objectPrototypeFunctions[] = {
    { "toString", objectPrototypeToString },
    { "hasOwnProperty", objectPrototypeHasOwnProperty },
};

void registerObject(VMRuntimeCommon *rt) {
    // Object.prototype 是 null
    auto prototypeObj = new JsLibObject(rt, objectPrototypeFunctions, CountOf(objectPrototypeFunctions), nullptr, jsValueNull);
    rt->objPrototypeObject = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeObject, prototypeObj);

    SET_PROTOTYPE(objectFunctions, jsValuePrototypeObject);

    rt->setGlobalObject("Object",
        new JsLibObject(rt, objectFunctions, CountOf(objectFunctions), objectConstructor));
}
