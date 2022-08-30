//
//  IJsObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "IJsObject.hpp"
#include <algorithm>


SizedString copyPropertyIfNeed(SizedString name) {
    if (name.unused != COMMON_STRINGS) {
        auto p = new uint8_t[name.len];
        memcpy(p, name.data, name.len);
        name.data = p;
    }

    return name;
}

bool IJsObject::getBool(VMContext *ctx, const JsValue &thiz, const SizedString &name) {
    auto value = getByName(ctx, thiz, name);
    return ctx->runtime->testTrue(value);
}

bool IJsObject::getBool(VMContext *ctx, const JsValue &thiz, const JsValue &prop) {
    auto value = get(ctx, thiz, prop);
    return ctx->runtime->testTrue(value);
}

void IJsObject::defineProperty(VMContext *ctx, const JsValue &propOrg, const JsProperty &descriptor) {
    JsValue prop = propOrg;
    if (prop.type >= JDT_OBJECT) {
        prop = ctx->runtime->toString(ctx, propOrg);
    }

    switch (prop.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: definePropertyByName(ctx, SS_UNDEFINED, descriptor); break;
        case JDT_NULL: definePropertyByName(ctx, SS_NULL, descriptor); break;
        case JDT_BOOL: definePropertyByName(ctx, prop.value.n32 ? SS_TRUE : SS_FALSE, descriptor); break;
        case JDT_INT32: definePropertyByIndex(ctx, prop.value.n32, descriptor); break;
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(prop), buf);
            definePropertyByName(ctx, SizedString(buf, len), descriptor);
            break;
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = prop.value.n32;
            definePropertyByName(ctx, SizedString(buf, 1), descriptor);
            break;
        }
        case JDT_STRING: {
            auto name = ctx->runtime->getString(prop);
            definePropertyByName(ctx, name, descriptor);
            break;
        }
        case JDT_SYMBOL: definePropertyBySymbol(ctx, prop.value.index, descriptor); break;
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }
}

bool IJsObject::getOwnPropertyDescriptor(VMContext *ctx, const JsValue &propOrg, JsProperty &descriptorOut) {
    JsValue prop = propOrg;
    if (prop.type >= JDT_OBJECT) {
        prop = ctx->runtime->toString(ctx, propOrg);
    }

    descriptorOut = JsProperty(JsUndefinedValue);

    switch (prop.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: return getOwnPropertyDescriptorByName(ctx, SS_UNDEFINED, descriptorOut);
        case JDT_NULL: return getOwnPropertyDescriptorByName(ctx, SS_NULL, descriptorOut);
        case JDT_BOOL: return getOwnPropertyDescriptorByName(ctx, prop.value.n32 ? SS_TRUE : SS_FALSE, descriptorOut);
        case JDT_INT32: return getOwnPropertyDescriptorByIndex(ctx, prop.value.n32, descriptorOut);
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(prop), buf);
            return getOwnPropertyDescriptorByName(ctx, SizedString(buf, len), descriptorOut);
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = prop.value.n32;
            return getOwnPropertyDescriptorByName(ctx, SizedString(buf, 1), descriptorOut);
        }
        case JDT_STRING: {
            auto name = ctx->runtime->getString(prop);
            return getOwnPropertyDescriptorByName(ctx, name, descriptorOut);
        }
        case JDT_SYMBOL:
            return getOwnPropertyDescriptorBySymbol(ctx, prop.value.index, descriptorOut);
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return false;
}

JsValue IJsObject::get(VMContext *ctx, const JsValue &thiz, const JsValue &propOrg, const JsValue &defVal) {
    JsValue prop = propOrg;
    if (prop.type >= JDT_OBJECT) {
        prop = ctx->runtime->toString(ctx, propOrg);
    }

    switch (prop.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: return getByName(ctx, thiz, SS_UNDEFINED, defVal);
        case JDT_NULL: return getByName(ctx, thiz, SS_NULL, defVal);
        case JDT_BOOL: return getByName(ctx, thiz, prop.value.n32 ? SS_TRUE : SS_FALSE, defVal);
        case JDT_INT32: return getByIndex(ctx, thiz, prop.value.n32, defVal);
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(prop), buf);
            return getByName(ctx, thiz, SizedString(buf, len), defVal);
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = prop.value.n32;
            return getByName(ctx, thiz, SizedString(buf, 1), defVal);
        }
        case JDT_STRING: {
            auto name = ctx->runtime->getString(prop);
            return getByName(ctx, thiz, name, defVal);
        }
        case JDT_SYMBOL:
            return getBySymbol(ctx, thiz, prop.value.index, defVal);
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return JsUndefinedValue;
}

void IJsObject::set(VMContext *ctx, const JsValue &thiz, const JsValue &propOrg, const JsValue &value) {
    JsValue prop = propOrg;
    if (prop.type >= JDT_OBJECT) {
        prop = ctx->runtime->toString(ctx, propOrg);
    }

    switch (prop.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            return;
        case JDT_UNDEFINED: setByName(ctx, thiz, SS_UNDEFINED, value); break;
        case JDT_NULL: setByName(ctx, thiz, SS_NULL, value); break;
        case JDT_BOOL: setByName(ctx, thiz, prop.value.n32 ? SS_TRUE : SS_FALSE, value); break;
        case JDT_INT32: setByIndex(ctx, thiz, prop.value.n32, value); break;
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(prop), buf);
            setByName(ctx, thiz, SizedString(buf, len), value);
            break;
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = prop.value.n32;
            setByName(ctx, thiz, SizedString(buf, 1), value);
            break;
        }
        case JDT_STRING: {
            auto name = ctx->runtime->getString(prop);
            setByName(ctx, thiz, name, value);
            break;
        }
        case JDT_SYMBOL:
            setBySymbol(ctx, thiz, prop.value.index, value);
            break;
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
            return;
        }
    }
}

bool IJsObject::remove(VMContext *ctx, const JsValue &propOrg) {
    JsValue prop = propOrg;
    if (prop.type >= JDT_OBJECT) {
        prop = ctx->runtime->toString(ctx, propOrg);
    }

    switch (prop.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: return removeByName(ctx, SS_UNDEFINED);
        case JDT_NULL: return removeByName(ctx, SS_NULL);
        case JDT_BOOL: return removeByName(ctx, prop.value.n32 ? SS_TRUE : SS_FALSE);
        case JDT_INT32: return removeByIndex(ctx, prop.value.n32);
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(prop), buf);
            return removeByName(ctx, SizedString(buf, len));
            break;
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = prop.value.n32;
            return removeByName(ctx, SizedString(buf, 1));
        }
        case JDT_STRING: {
            auto name = ctx->runtime->getString(prop);
            return removeByName(ctx, name);
        }
        case JDT_SYMBOL:
            return removeBySymbol(ctx, prop.value.index);
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
            break;
        }
    }

    return true;
}

JsValue IJsObject::getIterator(VMContext *ctx) {
    return JsUndefinedValue;
//    auto obj = getIteratorObject();
//    return ctx->runtime->pushObjValue(obj);
}

JsObject::JsObject(const JsValue &proto) : __proto__(proto, false, false, false, true) {
    type = JDT_OBJECT;
    _symbolProps = nullptr;
}

JsObject::~JsObject() {
    for (auto &item : _props) {
        auto &key = item.first;
        if (key.unused != COMMON_STRINGS) {
            delete [] key.data;
        }
    }
    _props.clear();

    if (_symbolProps) {
        delete _symbolProps;
    }
}

void JsObject::definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor) {
    bool isSelfProp = false;
    auto propValue = getRawByName(ctx, prop, isSelfProp);
    if (propValue) {
        if (isSelfProp) {
            // 自身的属性，可以直接修改
            mergeJsProperty(ctx, propValue, descriptor, prop);
            return;
        }
    }

    // 定义新的属性
    _props[copyPropertyIfNeed(prop)] = descriptor.defineProperty();
}

void JsObject::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    NumberToSizedString ss(index);
    return definePropertyByName(ctx, ss, descriptor);
}

void JsObject::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    bool isSelfProp = false;
    auto propValue = getRawBySymbol(ctx, index, isSelfProp);
    if (propValue) {
        if (!propValue->isConfigurable) {
            // 不能修改
            return;
        }
        if (isSelfProp) {
            // 自身的属性，可以直接修改
            mergeSymbolJsProperty(ctx, propValue, descriptor, index);
            return;
        }
    }

    // 定义新的属性
    if (!_symbolProps) {
        _symbolProps = new MapSymbolToJsProperty;
    }
    (*_symbolProps)[index] = descriptor.defineProperty();
}

bool JsObject::getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut) {
    bool ret = false;
    auto it = _props.find(prop);
    if (it == _props.end()) {
        descriptorOut = JsProperty();
    } else {
        descriptorOut = (*it).second;
        ret = true;
    }

    return ret;
}

bool JsObject::getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) {
    NumberToSizedString ss(index);
    return getOwnPropertyDescriptorByName(ctx, ss, descriptorOut);
}

bool JsObject::getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) {
    descriptorOut = JsProperty();

    bool ret = false;
    if (_symbolProps) {
        auto it = _symbolProps->find(index);
        if (it != _symbolProps->end()) {
            descriptorOut = (*it).second;
            ret = true;
        }
    }

    return ret;
}

JsProperty *JsObject::getRawByName(VMContext *ctx, const SizedString &prop, bool &isSelfPropOut) {
    isSelfPropOut = true;

    if (prop.equal(SS___PROTO__)) {
        if (__proto__.value.type == JDT_NOT_INITIALIZED) {
            // 缺省的 Object.prototype
            return &ctx->runtime->prototypeObject;
        } else {
            return &__proto__;
        }
    }

    auto it = _props.find(prop);
    if (it == _props.end()) {
        auto &proto = __proto__.value;
        JsProperty *ret = nullptr;
        if (proto.type == JDT_NOT_INITIALIZED) {
            // 缺省的 Object.prototype
            ret = ctx->runtime->objPrototypeObject->getRawByName(ctx, prop, isSelfPropOut);
        } else if (proto.type >= JDT_OBJECT) {
            auto obj = ctx->runtime->getObject(proto);
            assert(obj);
            ret = obj->getRawByName(ctx, prop, isSelfPropOut);
        }

        isSelfPropOut = false;
        return ret;
    } else {
        return &(*it).second;
    }

    return nullptr;
}

JsProperty *JsObject::getRawByIndex(VMContext *ctx, uint32_t index, bool &isSelfPropOut) {
    NumberToSizedString ss(index);
    return getRawByName(ctx, ss, isSelfPropOut);
}

JsProperty *JsObject::getRawBySymbol(VMContext *ctx, uint32_t index, bool &isSelfPropOut) {
    isSelfPropOut = true;
    if (_symbolProps) {
        auto it = _symbolProps->find(index);
        if (it == _symbolProps->end()) {
            if (__proto__.value.type >= JDT_OBJECT) {
                auto obj = ctx->runtime->getObject(__proto__.value);
                assert(obj);
                auto ret = obj->getRawBySymbol(ctx, index, isSelfPropOut);
                isSelfPropOut = false;
                return ret;
            }
        } else {
            return &(*it).second;
        }
    }

    return nullptr;
}

JsValue JsObject::getByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &defVal) {
    bool isSelfProp;
    auto propValue = getRawByName(ctx, name, isSelfProp);
    if (propValue) {
        if (propValue->isGetter) {
            ctx->vm->callMember(ctx, thiz, propValue->value, Arguments());
            return ctx->retValue;
        }

        if (propValue->value.isValid()) {
            return propValue->value;
        } else {
            // 有 key，所以返回 undefined，而非 defVal
            return JsUndefinedValue;
        }
    }

    return defVal;
}

JsValue JsObject::getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal) {
    NumberToSizedString ss(index);
    return getByName(ctx, thiz, ss, defVal);
}

JsValue JsObject::getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal) {
    bool isSelfProp;
    auto propValue = getRawBySymbol(ctx, index, isSelfProp);
    if (propValue) {
        if (propValue->isGetter) {
            ctx->vm->callMember(ctx, thiz, propValue->value, Arguments());
            return ctx->retValue;
        }
        return propValue->value;
    }

    return defVal;
}

void JsObject::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) {
    bool isSelfProp;
    auto propValue = getRawByName(ctx, prop, isSelfProp);
    if (propValue) {
        // 修改已经存在的
        if (propValue->setter.isValid()) {
            assert(propValue->setter.type > JDT_OBJECT);
            // 调用 setter 函数
            ArgumentsX args(value);
            ctx->vm->callMember(ctx, thiz, propValue->setter, args);
        } else if (propValue->isWritable) {
            if (isSelfProp) {
                // 可直接修改自己的属性
                propValue->value = value;
            } else {
                // 添加新属性
                _props[copyPropertyIfNeed(prop)] = value;
            }
        }
    } else {
        // 添加新属性
        _props[copyPropertyIfNeed(prop)] = value;
    }
}

void JsObject::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    NumberToSizedString ss(index);
    setByName(ctx, thiz, ss, value);
}

void JsObject::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    bool isSelfProp;
    auto propValue = getRawBySymbol(ctx, index, isSelfProp);
    if (propValue) {
        // 修改已经存在的
        if (propValue->setter.isValid()) {
            assert(propValue->setter.type > JDT_OBJECT);
            // 调用 setter 函数
            ArgumentsX args(value);
            ctx->vm->callMember(ctx, thiz, propValue->setter, args);
        } else if (propValue->isWritable) {
            if (isSelfProp) {
                // 可直接修改自己的属性
                propValue->value = value;
            } else {
                // 添加新属性
                if (!_symbolProps) {
                    _symbolProps = new MapSymbolToJsProperty();
                }

                (*_symbolProps)[index] = value;
            }
        }
    } else {
        // 添加新属性
        if (!_symbolProps) {
            _symbolProps = new MapSymbolToJsProperty();
        }

        (*_symbolProps)[index] = value;
    }
}

bool JsObject::removeByName(VMContext *ctx, const SizedString &prop) {
    bool isSelfProp;
    auto propValue = getRawByName(ctx, prop, isSelfProp);
    if (propValue) {
        if (isSelfProp) {
            if (propValue->isConfigurable) {
                // 删除自己的属性
                _props.erase(prop);
                return true;
            }
            return false;
        }

        // 无法删除 __proto__ 的属性，但是返回 true
        return true;
    }

    // 不存在的属性，也返回 true
    return true;
}

bool JsObject::removeByIndex(VMContext *ctx, uint32_t index) {
    NumberToSizedString ss(index);
    return removeByName(ctx, ss);
}

bool JsObject::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_symbolProps) {
        auto it = _symbolProps->find(index);
        if (it != _symbolProps->end()) {
            _symbolProps->erase(it);
        }
    }

    return true;
}

IJsObject *JsObject::clone() {
    auto obj = new JsObject(__proto__.value);

    obj->_props = _props;
    if (_symbolProps) { obj->_symbolProps = new MapSymbolToJsProperty; *obj->_symbolProps = *_symbolProps; }

    return obj;
}

void mergeJsProperty(VMContext *ctx, JsProperty *dst, const JsProperty &src, const SizedString &name) {
    if (!dst->merge(src)) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot redefine property: %.*s", (int)name.len, name.data);
    }
}

void mergeSymbolJsProperty(VMContext *ctx, JsProperty *dst, const JsProperty &src, uint32_t index) {
    if (!dst->merge(src)) {
        string buf;
        auto name = ctx->runtime->toSizedString(ctx, JsValue(JDT_SYMBOL, index), buf);
        ctx->throwException(PE_TYPE_ERROR, "Cannot redefine property: %.*s", (int)name.len, name.data);
    }
}

void mergeIndexJsProperty(VMContext *ctx, JsProperty *dst, const JsProperty &src, uint32_t index) {
    if (!dst->merge(src)) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot redefine property: %d", index);
    }
}
