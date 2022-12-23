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
        ctx->retValue = runtime->pushObject(new JsObject());
    } else {
        ctx->retValue = args[0];
    }
}

using IJsIteratorPtr = shared_ptr<IJsIterator>;

IJsIteratorPtr getJsIteratorPtr(VMContext *ctx, const JsValue &obj, bool includeNoneEnumerable = false, bool includeProtoProps = false) {
    IJsIterator *it = nullptr;
    if (obj.type == JDT_CHAR || obj.type == JDT_STRING) {
        it = newJsStringIterator(ctx, obj, includeProtoProps, includeNoneEnumerable);
    } else if (obj.type >= JDT_OBJECT) {
        auto pobj = ctx->runtime->getObject(obj);
        it = pobj->getIteratorObject(ctx, includeProtoProps, includeNoneEnumerable);
    } else {
        return nullptr;
    }

    return IJsIteratorPtr(it);
}

SizedString objectPrototypeToSizedString(const JsValue &thiz) {
    switch (thiz.type) {
        case JDT_UNDEFINED: return MAKE_STABLE_STR("[object Undefined]");
        case JDT_NULL: return MAKE_STABLE_STR("[object Null]");
        case JDT_BOOL: return MAKE_STABLE_STR("[object Boolean]");
        case JDT_INT32: return MAKE_STABLE_STR("[object Number]");
        case JDT_SYMBOL: return MAKE_STABLE_STR("[object Symbol]");
        case JDT_CHAR:
        case JDT_STRING: return MAKE_STABLE_STR("[object String]");
        case JDT_OBJECT: return MAKE_STABLE_STR("[object Object]");
        case JDT_ARRAY: return MAKE_STABLE_STR("[object Array]");
        case JDT_REGEX: return MAKE_STABLE_STR("[object RegExp]");
        case JDT_DATE: return MAKE_STABLE_STR("[object Date]");
        case JDT_PROMISE: return MAKE_STABLE_STR("[object Promise]");
        case JDT_ARGUMENTS: return MAKE_STABLE_STR("[object Arguments]");
        case JDT_OBJ_X: return MAKE_STABLE_STR("[object Object]");
        case JDT_OBJ_BOOL: return MAKE_STABLE_STR("[object Boolean]");
        case JDT_OBJ_NUMBER: return MAKE_STABLE_STR("[object Number]");
        case JDT_OBJ_STRING: return MAKE_STABLE_STR("[object String]");
        case JDT_OBJ_SYMBOL: return MAKE_STABLE_STR("[object Symbol]");
        case JDT_OBJ_GLOBAL_THIS:
        case JDT_LIB_OBJECT: return MAKE_STABLE_STR("[object Object]");
        case JDT_ITERATOR: return MAKE_STABLE_STR("[object Iterator]");
        case JDT_NATIVE_FUNCTION:
        case JDT_BOUND_FUNCTION:
        case JDT_FUNCTION: return MAKE_STABLE_STR("[object Function]");
        default:
            assert(0);
            return MAKE_STABLE_STR("[object Object]");
            break;
    }
}

LockedSizedStringWrapper definePropertyXetterToString(VMContext *ctx, const JsValue &xetter) {
    if (xetter.type < JDT_OBJECT) {
        return ctx->runtime->toSizedString(ctx, xetter);
    } else if (xetter.type == JDT_OBJECT) {
        return MAKE_STABLE_STR("#<Object>");
    } else {
        return objectPrototypeToSizedString(xetter);
    }
}

void changeFlag(VMRuntime *runtime, const JsValue &field, JsPropertyFlag flag, JsPropertyFlags &flags) {
    if (field.isValid()) {
        if (runtime->testTrue(field)) {
            flags |= flag;
        } else {
            flags &= ~flag;
        }
    }
}

// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Object/defineProperty
void objectDefineProperty(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (args.count < 3 || args[2].type != JDT_OBJECT) {
        ctx->throwException(JE_TYPE_ERROR, "Property description must be an object");
        return;
    }

    auto obj = args[0];
    auto name = args[1];
    auto descriptor = args[2];

    if (obj.type < JDT_OBJECT) {
        ctx->throwException(JE_TYPE_ERROR, "Object.defineProperty called on non-object");
        return;
    }

    auto descriptorObj = runtime->getObject(descriptor);

    auto configurable = descriptorObj->getByName(ctx, descriptor, SS_CONFIGURABLE, jsValueEmpty);
    auto enumerable = descriptorObj->getByName(ctx, descriptor, SS_ENUMERABLE, jsValueEmpty);
    auto writable = descriptorObj->getByName(ctx, descriptor, SS_WRITABLE, jsValueEmpty);
    auto value = descriptorObj->getByName(ctx, descriptor, SS_VALUE, jsValueEmpty);
    auto get = descriptorObj->getByName(ctx, descriptor, SS_GET, jsValueEmpty);
    auto set = descriptorObj->getByName(ctx, descriptor, SS_SET, jsValueEmpty);

    if (get.type < JDT_FUNCTION && get.type > JDT_UNDEFINED) {
        auto str = definePropertyXetterToString(ctx, get);
        ctx->throwException(JE_TYPE_ERROR, "Getter must be a function: %.*s", (int)str.len, str.data);
        return;
    }

    if (set.type < JDT_FUNCTION && set.type > JDT_UNDEFINED) {
        auto str = definePropertyXetterToString(ctx, set);
        ctx->throwException(JE_TYPE_ERROR, "Setter must be a function: %.*s", (int)str.len, str.data);
        return;
    }

    if ((get.isValid() || set.isValid()) && (writable.isValid() || value.isValid())) {
        ctx->throwException(JE_TYPE_ERROR, "Invalid property descriptor. Cannot both specify accessors and a value or writable attribute, #<Object>");
    }

    auto pObj = runtime->getObject(obj);
    auto prop = pObj->getRaw(ctx, name, false);
    if (prop == nullptr || prop->isEmpty()) {
        if (pObj->isPreventedExtensions) {
            ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Cannot define property %.*s, object is not extensible", name);
            return;
        }

        // 定义新的 property
        JsValue propDescriptor;
        if (value.isValid()) {
            propDescriptor = value.asProperty(0);
        } else if (get.isValid() || set.isValid()) {
            propDescriptor = runtime->pushGetterSetter(get, set);
        }

        JsPropertyFlags flags = 0;
        if (configurable.isValid() && runtime->testTrue(configurable)) flags |= JP_CONFIGURABLE;
        if (enumerable.isValid() && runtime->testTrue(enumerable)) flags |= JP_ENUMERABLE;
        if (writable.isValid() && runtime->testTrue(writable)) flags |= JP_WRITABLE;

        propDescriptor.propFlags = flags;

        pObj->setProperty(ctx, name, propDescriptor);
    } else {
        // 修改现有的.
        JsValue newProp = *prop;

        if (get.isValid() || set.isValid()) {
            if (!prop->isConfigurable()) {
                ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Cannot redefine property: %.*s", name);
                return;
            }

            if (newProp.type != JDT_GETTER_SETTER) {
                if (!prop->isConfigurable()) {
                    ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Cannot redefine property: %.*s", name);
                    return;
                }
                newProp = runtime->pushGetterSetter(get, set);
            } else {
                auto &gs = runtime->getGetterSetter(newProp);
                if (get.isFunction()) gs.getter = get;
                if (set.isFunction()) gs.setter = set;
            }
        } else {
            if (value.isValid()) {
                newProp.setValue(value);
            }
        }

        changeFlag(runtime, configurable, JP_CONFIGURABLE, newProp.propFlags);
        changeFlag(runtime, enumerable, JP_ENUMERABLE, newProp.propFlags);
        changeFlag(runtime, writable, JP_WRITABLE, newProp.propFlags);

        if (prop->isConfigurable()) {
            pObj->setProperty(ctx, name, newProp);
        } else if (!prop->equal(newProp)) {
            ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Cannot redefine property: %.*s", name);
        }
    }

    ctx->retValue = thiz;
}

// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/Object/defineProperties
void objectDefineProperties(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type < JDT_OBJECT) {
        ctx->throwException(JE_TYPE_ERROR, "Object.defineProperties called on non-object");
        return;
    }

    if (args.count == 1 || args[1].type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto runtime = ctx->runtime;
    if (args[1].type < JDT_OBJECT) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Property description must be an object: %.*s", args[1]);
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
        ctx->throwException(JE_TYPE_ERROR, "Object prototype may only be an Object or null: undefined");
        return;
    }

    auto runtime = ctx->runtime;
    auto proto = args[0];
    if (proto.type < JDT_OBJECT && proto.type != JDT_NULL) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Object prototype may only be an Object or null: %.*s", proto);
    }

    auto obj = new JsObject(proto);
    ctx->retValue = runtime->pushObject(obj);

    if (args.count >= 2 && args[1].type > JDT_UNDEFINED) {
        objectDefineProperties(ctx, thiz, ArgumentsX(ctx->retValue, args[1]));
    }
}

void objectAssign(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto runtime = ctx->runtime;
    auto targ = args[0];
    switch (targ.type) {
        case JDT_BOOL: targ = runtime->pushObject(new JsBooleanObject(targ)); break;
        case JDT_INT32: targ = runtime->pushObject(new JsNumberObject(targ)); break;
        case JDT_NUMBER: targ = runtime->pushObject(new JsNumberObject(targ)); break;
        case JDT_SYMBOL: ctx->throwException(JE_TYPE_ERROR, "Not supported Object.assign for symbol"); return;
        case JDT_CHAR: targ = runtime->pushObject(new JsStringObject(targ)); break;
        case JDT_STRING: targ = runtime->pushObject(new JsStringObject(targ)); break;
        default: break;
    }

    for (uint32_t i = 1; i < args.count; i++) {
        runtime->extendObject(ctx, targ, args[i], false);
    }

    ctx->retValue = targ;
}

void objectEntries(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto runtime = ctx->runtime;
    auto obj = args[0];

    auto arr = new JsArray();
    auto ret = runtime->pushObject(arr);

    auto it = getJsIteratorPtr(ctx, obj);
    if (it) {
        JsValue key, value;
        while (it->next(nullptr, &key, &value)) {
            auto item = new JsArray();
            item->push(ctx, key);
            item->push(ctx, value);

            arr->push(ctx, runtime->pushObject(item));
        }
    }

    ctx->retValue = ret;
}

void stringPrototypeCharAt(VMContext *ctx, const JsValue &thiz, const Arguments &args);

bool getStringOwnPropertyDescriptor(VMContext *ctx, const JsValue &thiz, JsValue name, JsValue &descriptorOut) {
    assert(thiz.type == JDT_CHAR || thiz.type == JDT_STRING);

    auto runtime = ctx->runtime;

    auto len = runtime->getStringLength(thiz);

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
            ArgumentsX args(makeJsValueInt32(index));
            stringPrototypeCharAt(ctx, thiz, args);
            descriptorOut = ctx->retValue.asProperty(JP_ENUMERABLE);
            return true;
        }
        return false;
    } else if (name.type == JDT_SYMBOL) {
        return false;
    } else {
        bool ret = false;

        auto str = runtime->toSizedString(ctx, name);
        if (str.equal(SS_LENGTH)) {
            descriptorOut = makeJsValueInt32(len).asProperty(JP_ENUMERABLE);
            ret = true;
        } else {
            bool successful;
            auto n = str.atoi(successful);
            if (successful) {
                name = makeJsValueInt32((uint32_t)n);
                ret = true;
            }
        }

        return ret;
    }
}

// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Object/getOwnPropertyDescriptor
void objectGetOwnPropertyDescriptor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    ctx->retValue = jsValueUndefined;

    if (args.count < 1 || args[0].type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    } else if (args.count < 2) {
        return;
    }

    auto &obj = args[0];
    auto &name = args[1];

    JsValue descriptor;

    switch (obj.type) {
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
    auto descValue = runtime->pushObject(desc);

    desc->setByName(ctx, descValue, SS_CONFIGURABLE, makeJsValueBool(descriptor.isConfigurable()));
    desc->setByName(ctx, descValue, SS_ENUMERABLE, makeJsValueBool(descriptor.isEnumerable()));

    if (descriptor.isGetterSetter()) {
        auto &gs = runtime->getGetterSetter(descriptor);
        desc->setByName(ctx, descValue, SS_GET, gs.getter);
        desc->setByName(ctx, descValue, SS_SET, gs.setter);
    } else {
        desc->setByName(ctx, descValue, SS_WRITABLE, makeJsValueBool(descriptor.isWritable()));
        desc->setByName(ctx, descValue, SS_VALUE, descriptor.isValid() ? descriptor : jsValueUndefined);
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
    pobj->changeAllProperties(ctx, 0, JP_CONFIGURABLE | JP_WRITABLE);
    pobj->preventExtensions(ctx);
}

void objectFromEntries(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "undefined is not iterable");
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
            ctx->throwException(JE_TYPE_ERROR, "%.*s is not iterable (cannot read property Symbol(Symbol.iterator))",
                                name.len, name.data);
            return;
        }
        it.reset(obj->getIteratorObject(ctx, false));
    } else {
        auto name = runtime->toTypeName(entries);
        auto s = runtime->toSizedString(ctx, entries);
        ctx->throwException(JE_TYPE_ERROR, "%.*s %.*s is not iterable (cannot read property Symbol(Symbol.iterator))",
                            name.len, name.data, s.len, s.data);
        return;
    }

    auto pobj = new JsObject();
    auto obj = runtime->pushObject(pobj);

    JsValue item;
    while (it->nextOf(item)) {
        if (item.type < JDT_OBJECT) {
            ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Iterator value %.*s is not an entry object", item);
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
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto obj = args[0];
    auto descsObj = new JsObject();
    auto descs = ctx->runtime->pushObject(descsObj);

    auto it = getJsIteratorPtr(ctx, obj, true);
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
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto obj = args[0];
    auto namesObj = new JsArray();
    auto names = ctx->runtime->pushObject(namesObj);

    auto it = getJsIteratorPtr(ctx, obj, true);
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
        case JDT_UNDEFINED:
            ctx->throwException(JE_TYPE_ERROR, "Cannot read properties of undefined (reading '__proto__')");
            break;
        case JDT_NULL:
            ctx->throwException(JE_TYPE_ERROR, "Cannot read properties of null (reading '__proto__')");
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
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
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
            auto s = runtime->toSizedString(ctx, name);
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
        JsValue descriptor;

        if (pobj->getOwnPropertyDescriptor(ctx, name, descriptor)) {
            ret = jsValueTrue;
        }
    }

    ctx->retValue = ret;
}

void objectHasOwn(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
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
                ctx->retValue = makeJsValueBool(a == b);
            }
        }
    } else {
        ctx->retValue = makeJsValueBool(relationalStrictEqual(ctx->runtime, left, right));
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

    ctx->retValue = makeJsValueBool(extensible);
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

    bool isFrozen = !pobj->hasAnyProperty(ctx, JP_CONFIGURABLE | JP_WRITABLE);

    ctx->retValue = makeJsValueBool(isFrozen);
}

void objectIsSealed(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
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

    bool isSealed = !pobj->hasAnyProperty(ctx, JP_CONFIGURABLE);

    ctx->retValue = makeJsValueBool(isSealed);
}

void objectKeys(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto obj = args.getAt(0);
    if (obj.type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto arr = new JsArray();
    auto ret = ctx->runtime->pushObject(arr);

    auto it = getJsIteratorPtr(ctx, obj);
    if (it) {
        JsValue key;
        while (it->next(nullptr, &key, nullptr)) {
            arr->push(ctx, key);
        }
    }

    ctx->retValue = ret;
}

void objectPreventExtensions(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (args.count == 0 || args[0].type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
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

void objectSeal(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto obj = args.getAt(0);
    ctx->retValue = obj;
    if (obj.type < JDT_OBJECT) {
        return;
    }

    auto pobj = ctx->runtime->getObject(obj);
    pobj->preventExtensions(ctx);
    pobj->changeAllProperties(ctx, 0, JP_CONFIGURABLE);
}

void objectSetPrototypeOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto obj = args.getAt(0);
    auto prototype = args.getAt(1);

    if (obj.type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Object.setPrototypeOf called on null or undefined");
        return;
    }

    if (prototype.type != JDT_NULL && prototype.type < JDT_OBJECT) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Object prototype may only be an Object or null: %.*s", prototype);
        return;
    }

    ctx->retValue = obj;
    if (obj.type < JDT_OBJECT) {
        return;
    }

    auto pobj = ctx->runtime->getObject(obj);
    pobj->setByName(ctx, obj, SS___PROTO__, prototype);
}

void objectValues(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto obj = args.getAt(0);
    auto runtime = ctx->runtime;

    if (obj.type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    auto arr = new JsArray();
    auto ret = runtime->pushObject(arr);

    auto it = getJsIteratorPtr(ctx, obj);
    if (it) {
        JsValue value;
        while (it->next(nullptr, nullptr, &value)) {
            arr->push(ctx, value);
        }
    }

    ctx->retValue = ret;
}

static JsLibProperty objectFunctions[] = {
    { "name", nullptr, "Object" },
    { "length", nullptr, nullptr, jsValueLength0Property },
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
    { "isSealed", objectIsSealed },
    { "keys", objectKeys },
    { "preventExtensions", objectPreventExtensions },
    { "seal", objectSeal },
    { "setPrototypeOf", objectSetPrototypeOf },
    { "values", objectValues },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};

void objectPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.type == JDT_LIB_OBJECT) {
        auto obj = (JsLibObject *)ctx->runtime->getObject(thiz);
        auto name = obj->getName();
        if (name.len > 0) {
            auto str = stringPrintf("[object %.*s]", name.len, name.data);
            ctx->retValue = ctx->runtime->pushString(str);
            return;
        }
    }

    auto str = objectPrototypeToSizedString(thiz);
    ctx->retValue = ctx->runtime->pushString(str);
}

void objectPrototypeHasOwnProperty(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.type <= JDT_NULL) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot convert undefined or null to object");
        return;
    }

    JsValue name = args.getAt(0, jsStringValueUndefined);

    hasOwnProperty(ctx, thiz, name);
}

void objectPrototypeIsPrototypeOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    JsValue obj = args.getAt(0);
    auto runtime = ctx->runtime;
    bool ret = false;

    while (obj.type >= JDT_OBJECT) {
        auto pobj = runtime->getObject(obj);
        obj = pobj->getByName(ctx, obj, SS___PROTO__);
        if (obj == thiz) {
            ret = true;
            break;
        }
    }

    ctx->retValue = makeJsValueBool(ret);
}

void objectPrototypeValueOf(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto ret = thiz;
    switch (thiz.type) {
        case JDT_OBJ_BOOL: ret = ((JsBooleanObject *)runtime->getObject(thiz))->value(); break;
        case JDT_OBJ_NUMBER: ret = ((JsNumberObject *)runtime->getObject(thiz))->value(); break;
        case JDT_OBJ_STRING: ret = ((JsStringObject *)runtime->getObject(thiz))->value(); break;
        case JDT_OBJ_SYMBOL: ret = ((JsSymbolObject *)runtime->getObject(thiz))->value(); break;
        default: break;
    }

    ctx->retValue = ret;
}

void objectPrototypePropertyIsEnumerable(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    JsValue name = args.getAt(0, jsStringValueUndefined);
    auto runtime = ctx->runtime;
    bool ret = false;

    if (thiz.type < JDT_OBJECT) {
        if (thiz.type == JDT_CHAR || thiz.type == JDT_STRING) {
            auto s = runtime->toSizedString(ctx, name);
            bool successful;
            auto index = s.atoi(successful);
            if (successful && index >= 0 && index < runtime->getStringLength(thiz)) {
                ret = true;
            }
        }
    } else {
        auto pobj = runtime->getObject(thiz);
        auto prop = pobj->getRaw(ctx, name, false);
        if (prop && prop->isEnumerable()) {
            ret = true;
        }
    }

    ctx->retValue = makeJsValueBool(ret);
}

static JsLibProperty objectPrototypeFunctions[] = {
    { "toLocaleString", objectPrototypeToString },
    { "toString", objectPrototypeToString },
    { "hasOwnProperty", objectPrototypeHasOwnProperty },
    { "isPrototypeOf", objectPrototypeIsPrototypeOf },
    { "valueOf", objectPrototypeValueOf },
    { "propertyIsEnumerable", objectPrototypePropertyIsEnumerable },
};

void registerObject(VMRuntimeCommon *rt) {
    // Object.prototype 是 null
    auto prototypeObj = new JsLibObject(rt, objectPrototypeFunctions, CountOf(objectPrototypeFunctions), nullptr, nullptr, jsValueNull);
    rt->objPrototypeObject = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeObject, prototypeObj);

    SET_PROTOTYPE(objectFunctions, jsValuePrototypeObject);

    setGlobalLibObject("Object", rt, objectFunctions, CountOf(objectFunctions), objectConstructor, jsValuePrototypeFunction);
}
