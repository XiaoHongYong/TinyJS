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

void IJsObject::defineProperty(VMContext *ctx, const JsValue &propOrg, const JsProperty &descriptor, const JsValue &setter) {
    JsValue prop = propOrg;
    if (prop.type >= JDT_OBJECT) {
        prop = ctx->runtime->toString(ctx, propOrg);
    }

    switch (prop.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: definePropertyByName(ctx, SS_UNDEFINED, descriptor, setter); break;
        case JDT_NULL: definePropertyByName(ctx, SS_NULL, descriptor, setter); break;
        case JDT_BOOL: definePropertyByName(ctx, prop.value.n32 ? SS_TRUE : SS_FALSE, descriptor, setter); break;
        case JDT_INT32: definePropertyByIndex(ctx, prop.value.n32, descriptor, setter); break;
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(prop), buf);
            definePropertyByName(ctx, SizedString(buf, len), descriptor, setter);
            break;
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = prop.value.n32;
            definePropertyByName(ctx, SizedString(buf, 1), descriptor, setter);
            break;
        }
        case JDT_STRING: {
            auto name = ctx->runtime->getString(prop);
            definePropertyByName(ctx, name, descriptor, setter);
            break;
        }
        case JDT_SYMBOL: definePropertyBySymbol(ctx, prop.value.index, descriptor, setter); break;
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }
}

bool IJsObject::getOwnPropertyDescriptor(VMContext *ctx, const JsValue &propOrg, JsProperty &descriptorOut, JsValue &setterOut) {
    JsValue prop = propOrg;
    if (prop.type >= JDT_OBJECT) {
        prop = ctx->runtime->toString(ctx, propOrg);
    }

    descriptorOut = JsProperty(JsUndefinedValue);
    setterOut = JsUndefinedValue;

    switch (prop.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: return getOwnPropertyDescriptorByName(ctx, SS_UNDEFINED, descriptorOut, setterOut);
        case JDT_NULL: return getOwnPropertyDescriptorByName(ctx, SS_NULL, descriptorOut, setterOut);
        case JDT_BOOL: return getOwnPropertyDescriptorByName(ctx, prop.value.n32 ? SS_TRUE : SS_FALSE, descriptorOut, setterOut);
        case JDT_INT32: return getOwnPropertyDescriptorByIndex(ctx, prop.value.n32, descriptorOut, setterOut);
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(prop), buf);
            return getOwnPropertyDescriptorByName(ctx, SizedString(buf, len), descriptorOut, setterOut);
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = prop.value.n32;
            return getOwnPropertyDescriptorByName(ctx, SizedString(buf, 1), descriptorOut, setterOut);
        }
        case JDT_STRING: {
            auto name = ctx->runtime->getString(prop);
            return getOwnPropertyDescriptorByName(ctx, name, descriptorOut, setterOut);
        }
        case JDT_SYMBOL:
            return getOwnPropertyDescriptorBySymbol(ctx, prop.value.index, descriptorOut, setterOut);
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return false;
}

JsValue IJsObject::get(VMContext *ctx, const JsValue &thiz, const JsValue &propOrg) {
    JsValue prop = propOrg;
    if (prop.type >= JDT_OBJECT) {
        prop = ctx->runtime->toString(ctx, propOrg);
    }

    switch (prop.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: return getByName(ctx, thiz, SS_UNDEFINED);
        case JDT_NULL: return getByName(ctx, thiz, SS_NULL);
        case JDT_BOOL: return getByName(ctx, thiz, prop.value.n32 ? SS_TRUE : SS_FALSE);
        case JDT_INT32: return getByIndex(ctx, thiz, prop.value.n32);
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(prop), buf);
            return getByName(ctx, thiz, SizedString(buf, len));
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = prop.value.n32;
            return getByName(ctx, thiz, SizedString(buf, 1));
        }
        case JDT_STRING: {
            auto name = ctx->runtime->getString(prop);
            return getByName(ctx, thiz, name);
        }
        case JDT_SYMBOL:
            return getBySymbol(ctx, thiz, prop.value.index);
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

bool IJsObject::remove(VMContext *ctx, const JsValue &propOrg, const JsValue &value) {
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

JsObject::JsObject(const JsValue &__proto__) : __proto__(__proto__) {
    type = JDT_OBJECT;
    _setters = nullptr;
    _symbolProps = nullptr;
    _symbolSetters = nullptr;
}

JsObject::~JsObject() {
    for (auto &item : _props) {
        auto &key = item.first;
        if (key.unused != COMMON_STRINGS) {
            delete [] key.data;
        }
    }
    _props.clear();

    if (_setters) {
        for (auto &item : *_setters) {
            auto &key = item.first;
            if (key.unused != COMMON_STRINGS) {
                delete [] key.data;
            }
        }
        _setters->clear();
    }

    if (_symbolProps) {
        delete _symbolProps;
    }

    if (_symbolSetters) {
        delete _symbolSetters;
    }
}

void JsObject::definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor, const JsValue &setter) {
    auto it = _props.find(prop);
    if (it == _props.end()) {
        _props[copyPropertyIfNeed(prop)] = descriptor;
    } else {
        (*it).second = descriptor;
    }

    if (setter.type != JDT_UNDEFINED) {
        if (setter.type <= JDT_REGEX) {
            string buf;
            auto s = ctx->runtime->toSizedString(ctx, setter, buf);
            ctx->throwException(PE_TYPE_ERROR, "Setter must be a function: %.*s", (int)s.len, s.data);
            return;
        }

        if (_setters == nullptr) {
            _setters = new MapNameToJsValue();
        }
        auto it = _setters->find(prop);
        if (it == _setters->end()) {
            (*_setters)[copyPropertyIfNeed(prop)] = setter;
        } else {
            (*it).second = setter;
        }
    }
}

void JsObject::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) {
    NumberToSizedString ss(index);
    return definePropertyByName(ctx, ss, descriptor, setter);
}

void JsObject::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) {
    if (!_symbolProps) {
        _symbolProps = new MapSymbolToJsProperty;
    }

    auto it = _symbolProps->find(index);
    if (it == _symbolProps->end()) {
        (*_symbolProps)[index] = descriptor;
    } else {
        (*it).second = descriptor;
    }

    if (setter.type > JDT_OBJECT) {
        // 有 setter
        if (!_symbolSetters) {
            _symbolSetters = new MapSymbolToJsValue;
        }

        auto it = _symbolSetters->find(index);
        if (it == _symbolSetters->end()) {
            (*_symbolSetters)[index] = setter;
        } else {
            (*it).second = setter;
        }
    }
}

bool JsObject::getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut, JsValue &setterOut) {
    bool ret = false;
    auto it = _props.find(prop);
    if (it == _props.end()) {
        descriptorOut = JsProperty(JsUndefinedValue);
    } else {
        descriptorOut = (*it).second;
        ret = true;
    }

    if (_setters) {
        auto it = _setters->find(prop);
        if (it == _setters->end()) {
            setterOut = JsUndefinedValue;
        } else {
            setterOut = (*it).second;
            ret = true;
        }
    }

    return ret;
}

bool JsObject::getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) {
    NumberToSizedString ss(index);
    return getOwnPropertyDescriptorByName(ctx, ss, descriptorOut, setterOut);
}

bool JsObject::getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) {
    descriptorOut = JsProperty(JsUndefinedValue);
    setterOut = JsUndefinedValue;

    bool ret = false;
    if (_symbolProps) {
        auto it = _symbolProps->find(index);
        if (it != _symbolProps->end()) {
            descriptorOut = (*it).second;
            ret = true;
        }
    }

    if (_symbolSetters) {
        auto it = _symbolSetters->find(index);
        if (it != _symbolSetters->end()) {
            setterOut = (*it).second;
            ret = true;
        }
    }

    return ret;
}

JsValue JsObject::getSetterByName(VMContext *ctx, const SizedString &prop) {
    if (_setters) {
        auto it = _setters->find(prop);
        if (it != _setters->end()) {
            return (*it).second;
        }
    }

    if (__proto__.type >= JDT_OBJECT) {
        auto obj = ctx->runtime->getObject(__proto__);
        assert(obj);
        return obj->getSetterByName(ctx, prop);
    }

    return JsUndefinedValue;
}

JsValue JsObject::getSetterByIndex(VMContext *ctx, uint32_t index) {
    NumberToSizedString ss(index);
    return getSetterByName(ctx, ss);
}

JsValue JsObject::getSetterBySymbol(VMContext *ctx, uint32_t index) {
    if (_symbolSetters) {
        auto it = _symbolSetters->find(index);
        if (it != _symbolSetters->end()) {
            return (*it).second;
        }
    }

    if (__proto__.type >= JDT_OBJECT) {
        auto obj = ctx->runtime->getObject(__proto__);
        assert(obj);
        obj->getSetterBySymbol(ctx, index);
    }

    return JsUndefinedValue;
}

JsValue JsObject::getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop) {
    if (prop.equal(SS___PROTO__)) {
        if (__proto__.type == JDT_NOT_INITIALIZED) {
            // 缺省的 Object.prototype
            return ctx->runtime->prototypeObject;
        } else {
            return __proto__;
        }
    }

    auto it = _props.find(prop);
    if (it == _props.end()) {
        if (__proto__.type == JDT_NOT_INITIALIZED) {
            // 缺省的 Object.prototype
            return ctx->runtime->objPrototypeObject->getByName(ctx, thiz, prop);
        } else if (__proto__.type >= JDT_OBJECT) {
            auto obj = ctx->runtime->getObject(__proto__);
            assert(obj);
            return obj->getByName(ctx, thiz, prop);
        }

        return JsUndefinedValue;
    } else {
        auto &propValue = (*it).second;
        if (propValue.isGetter) {
            ctx->vm->callMember(ctx, thiz, propValue.value, Arguments());
            return ctx->retValue;
        }

        return propValue.value;
    }
}

JsValue JsObject::getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    NumberToSizedString ss(index);
    return getByName(ctx, thiz, ss);
}

JsValue JsObject::getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    if (_symbolProps) {
        auto it = _symbolProps->find(index);
        if (it == _symbolProps->end()) {
            if (__proto__.type >= JDT_OBJECT) {
                auto obj = ctx->runtime->getObject(__proto__);
                assert(obj);
                return obj->getBySymbol(ctx, thiz, index);
            }
        } else {
            auto &propValue = (*it).second;
            if (propValue.isGetter) {
                ctx->vm->callMember(ctx, thiz, propValue.value, Arguments());
                return ctx->retValue;
            }

            return propValue.value;
        }
    }

    return JsUndefinedValue;
}

void JsObject::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) {
    if (prop.equal(SS___PROTO__)) {
        __proto__ = value;
        return;
    }

    if (_setters) {
        // 检查本地的 Setter
        auto it = _setters->find(prop);
        if (it != _setters->end()) {
            auto &setter = (*it).second;
            if (setter.type > JDT_OBJECT) {
                // 调用 setter 函数
                ArgumentsX args(value);
                ctx->vm->callMember(ctx, thiz, setter, args);
                return;
            }
        }
    }

    auto it = _props.find(prop);
    if (it == _props.end()) {
        if (__proto__.type >= JDT_OBJECT) {
            // 查找 prototype 链
            auto obj = ctx->runtime->getObject(__proto__);
            assert(obj);
            auto setter = obj->getSetterByName(ctx, prop);
            if (setter.type > JDT_OBJECT) {
                // 调用 setter 函数
                ArgumentsX args(value);
                ctx->vm->callMember(ctx, thiz, setter, args);
                return;
            }
        }

        _props[copyPropertyIfNeed(prop)] = value;
    } else {
        auto &propValue = (*it).second;
        if (propValue.isWritable) {
            propValue.value = value;
        }
    }
}

void JsObject::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    NumberToSizedString ss(index);
    setByName(ctx, thiz, ss, value);
}

void JsObject::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (_symbolSetters) {
        auto it = _symbolSetters->find(index);
        if (it != _symbolSetters->end()) {
            // 有 setter 函数
            ArgumentsX args(value);
            ctx->vm->callMember(ctx, thiz, (*it).second, args);
            return;
        }
    }

    if (!_symbolProps) {
        _symbolProps = new MapSymbolToJsProperty();
    }

    auto it = _symbolProps->find(index);
    if (it == _symbolProps->end()) {
        if (__proto__.type >= JDT_OBJECT) {
            // 查找 prototype 链
            auto obj = ctx->runtime->getObject(__proto__);
            assert(obj);
            auto setter = obj->getSetterBySymbol(ctx, index);
            if (setter.type > JDT_OBJECT) {
                // 调用 setter 函数
                ArgumentsX args(value);
                ctx->vm->callMember(ctx, thiz, setter, args);
                return;
            }
        }

        (*_symbolProps)[index] = JsProperty(value);
    } else {
        auto &propValue = (*it).second;
        if (propValue.isWritable) {
            propValue.value = value;
        }
    }
}

bool JsObject::removeByName(VMContext *ctx, const SizedString &prop) {
    auto it = _props.find(prop);
    if (it != _props.end()) {
        auto &key = (*it).first;
        if (key.unused != COMMON_STRINGS) {
            delete [] key.data;
        }
        _props.erase(it);
    }

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
    auto obj = new JsObject(__proto__);

    obj->_props = _props;
    if (_setters) { obj->_setters = new MapNameToJsValue; *obj->_setters = *_setters; }
    if (_symbolProps) { obj->_symbolProps = new MapSymbolToJsProperty; *obj->_symbolProps = *_symbolProps; }
    if (_symbolSetters) { obj->_symbolSetters = new MapSymbolToJsValue; *obj->_symbolSetters = *_symbolSetters; }

    return obj;
}
