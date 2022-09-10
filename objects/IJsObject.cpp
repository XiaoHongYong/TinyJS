//
//  IJsObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "IJsObject.hpp"
#include <algorithm>


SizedString copyPropertyIfNeed(SizedString name) {
    if (!name.isStable()) {
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

bool IJsObject::getBool(VMContext *ctx, const JsValue &thiz, const JsValue &name) {
    auto value = get(ctx, thiz, name);
    return ctx->runtime->testTrue(value);
}

void IJsObject::defineProperty(VMContext *ctx, const JsValue &nameOrg, const JsProperty &descriptor) {
    JsValue name = nameOrg;
    if (name.type >= JDT_OBJECT) {
        name = ctx->runtime->toString(ctx, nameOrg);
    }

    switch (name.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: definePropertyByName(ctx, SS_UNDEFINED, descriptor); break;
        case JDT_NULL: definePropertyByName(ctx, SS_NULL, descriptor); break;
        case JDT_BOOL: definePropertyByName(ctx, name.value.n32 ? SS_TRUE : SS_FALSE, descriptor); break;
        case JDT_INT32: definePropertyByIndex(ctx, name.value.n32, descriptor); break;
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(name), buf);
            definePropertyByName(ctx, SizedString(buf, len), descriptor);
            break;
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = name.value.n32;
            definePropertyByName(ctx, SizedString(buf, 1), descriptor);
            break;
        }
        case JDT_STRING: {
            auto strName = ctx->runtime->getString(name);
            definePropertyByName(ctx, strName, descriptor);
            break;
        }
        case JDT_SYMBOL: definePropertyBySymbol(ctx, name.value.index, descriptor); break;
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }
}

bool IJsObject::getOwnPropertyDescriptor(VMContext *ctx, const JsValue &nameOrg, JsProperty &descriptorOut) {
    JsValue name = nameOrg;
    if (name.type >= JDT_OBJECT) {
        name = ctx->runtime->toString(ctx, nameOrg);
    }

    descriptorOut = JsProperty(jsValueUndefined);

    switch (name.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: return getOwnPropertyDescriptorByName(ctx, SS_UNDEFINED, descriptorOut);
        case JDT_NULL: return getOwnPropertyDescriptorByName(ctx, SS_NULL, descriptorOut);
        case JDT_BOOL: return getOwnPropertyDescriptorByName(ctx, name.value.n32 ? SS_TRUE : SS_FALSE, descriptorOut);
        case JDT_INT32: return getOwnPropertyDescriptorByIndex(ctx, name.value.n32, descriptorOut);
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(name), buf);
            return getOwnPropertyDescriptorByName(ctx, SizedString(buf, len), descriptorOut);
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = name.value.n32;
            return getOwnPropertyDescriptorByName(ctx, SizedString(buf, 1), descriptorOut);
        }
        case JDT_STRING: {
            auto str = ctx->runtime->getString(name);
            return getOwnPropertyDescriptorByName(ctx, str, descriptorOut);
        }
        case JDT_SYMBOL:
            return getOwnPropertyDescriptorBySymbol(ctx, name.value.index, descriptorOut);
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return false;
}

JsProperty *IJsObject::getRaw(VMContext *ctx, const JsValue &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    switch (name.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            return nullptr;
        case JDT_UNDEFINED: return getRawByName(ctx, SS_UNDEFINED, funcGetterOut, includeProtoProp); break;
        case JDT_NULL: return getRawByName(ctx, SS_NULL, funcGetterOut, includeProtoProp); break;
        case JDT_BOOL: return getRawByName(ctx, name.value.n32 ? SS_TRUE : SS_FALSE, funcGetterOut, includeProtoProp); break;
        case JDT_INT32: return getRawByIndex(ctx, name.value.n32, includeProtoProp); break;
        case JDT_NUMBER: {
            auto d = ctx->runtime->getDouble(name);
            if (d == (int32_t)d) {
                return getRawByIndex(ctx, (int32_t)d, includeProtoProp);
            } else {
                char buf[64];
                auto len = floatToString(d, buf);
                return getRawByName(ctx, SizedString(buf, len), funcGetterOut, includeProtoProp);
            }
            break;
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = name.value.n32;
            return getRawByName(ctx, SizedString(buf, 1), funcGetterOut, includeProtoProp);
            break;
        }
        case JDT_STRING: {
            auto str = ctx->runtime->getString(name);
            return getRawByName(ctx, str, funcGetterOut, includeProtoProp);
            break;
        }
        case JDT_SYMBOL: {
            return getRawBySymbol(ctx, name.value.index, includeProtoProp);
            break;
        }
        default: {
            assert(name.type >= JDT_OBJECT);
            auto nameNew = ctx->runtime->jsObjectToString(ctx, name);
            if (ctx->error != PE_OK) {
                return nullptr;
            }
            return getRaw(ctx, nameNew, funcGetterOut, includeProtoProp);
        }
    }
}

JsValue IJsObject::get(VMContext *ctx, const JsValue &thiz, const JsValue &nameOrg, const JsValue &defVal) {
    JsNativeFunction funcGetter = nullptr;
    JsProperty *prop = getRaw(ctx, nameOrg, funcGetter, true);
    if (prop) {
        if (prop->isGSetter) {
            if (funcGetter) {
                funcGetter(ctx, thiz, Arguments());
            } else if (prop->value.type >= JDT_FUNCTION) {
                ctx->vm->callMember(ctx, thiz, prop->value, Arguments());
            } else {
                // getter 未设置，返回 undefined.
                return jsValueUndefined;
            }
            return ctx->retValue;
        }
        return prop->value;
    }
    return defVal;
}

void IJsObject::set(VMContext *ctx, const JsValue &thiz, const JsValue &nameOrg, const JsValue &value) {
    JsValue name = nameOrg;
    if (name.type >= JDT_OBJECT) {
        name = ctx->runtime->toString(ctx, nameOrg);
    }
//
//    if (thiz.type < JDT_OBJECT) {
//        // Primitive types 不能被修改，但是其 setter 函数会被调用
//        string buf;
//        auto strName = ctx->runtime->toSizedString(ctx, name, buf);
//        JsNativeFunction funcGetter = nullptr;
//        auto prop = getRawByName(ctx, strName, funcGetter, true);
//        if (prop && prop->setter.type >= JDT_FUNCTION) {
//            // 调用 setter 函数
//            ArgumentsX args(value);
//            ctx->vm->callMember(ctx, thiz, prop->setter, args);
//        }
//        return;
//    }

    switch (name.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            return;
        case JDT_UNDEFINED: setByName(ctx, thiz, SS_UNDEFINED, value); break;
        case JDT_NULL: setByName(ctx, thiz, SS_NULL, value); break;
        case JDT_BOOL: setByName(ctx, thiz, name.value.n32 ? SS_TRUE : SS_FALSE, value); break;
        case JDT_INT32: setByIndex(ctx, thiz, name.value.n32, value); break;
        case JDT_NUMBER: {
            auto d = ctx->runtime->getDouble(name);
            if (d == (int32_t)d) {
                setByIndex(ctx, thiz, (int32_t)d, value);
            } else {
                char buf[64];
                auto len = floatToString(d, buf);
                setByName(ctx, thiz, SizedString(buf, len), value);
            }
            break;
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = name.value.n32;
            setByName(ctx, thiz, SizedString(buf, 1), value);
            break;
        }
        case JDT_STRING: {
            auto strName = ctx->runtime->getString(name);
            setByName(ctx, thiz, strName, value);
            break;
        }
        case JDT_SYMBOL:
            setBySymbol(ctx, thiz, name.value.index, value);
            break;
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
            return;
        }
    }
}

JsValue IJsObject::increase(VMContext *ctx, const JsValue &thiz, const JsValue &nameOrg, int n, bool isPost) {
    JsValue name = nameOrg;
    if (name.type >= JDT_OBJECT) {
        name = ctx->runtime->toString(ctx, nameOrg);
    }

    switch (name.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            return jsValueNaN;
        case JDT_UNDEFINED: return increaseByName(ctx, thiz, SS_UNDEFINED, n, isPost);
        case JDT_NULL: return increaseByName(ctx, thiz, SS_NULL, n, isPost);
        case JDT_BOOL: return increaseByName(ctx, thiz, name.value.n32 ? SS_TRUE : SS_FALSE, n, isPost);
        case JDT_INT32: return increaseByIndex(ctx, thiz, name.value.n32, n, isPost);
        case JDT_NUMBER: {
            auto d = ctx->runtime->getDouble(name);
            if (d == (int32_t)d) {
                return increaseByIndex(ctx, thiz, (int32_t)d, n, isPost);
            } else {
                char buf[64];
                auto len = floatToString(d, buf);
                return increaseByName(ctx, thiz, SizedString(buf, len), n, isPost);
            }
            break;
        }
        case JDT_CHAR: {
            char buf[32];
            buf[0] = name.value.n32;
            return increaseByName(ctx, thiz, SizedString(buf, 1), n, isPost);
        }
        case JDT_STRING: {
            auto strName = ctx->runtime->getString(name);
            return increaseByName(ctx, thiz, strName, n, isPost);
        }
        case JDT_SYMBOL:
            return increaseBySymbol(ctx, thiz, name.value.index, n, isPost);
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
            return jsValueNaN;
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

/**
 * 为了遍历 Object 的所有属性
 */
class JsObjectIterator : public IJsIterator {
public:
    JsObjectIterator(VMContext *ctx, JsObject *obj) {
        assert(obj->type == JDT_OBJECT);
        _ctx = ctx;
        _obj = obj;
        _it = obj->_props.begin();
    }

    virtual bool nextKey(SizedString &keyOut) override {
        if (_it == _obj->_props.end()) {
            return false;
        }
        keyOut = (*_it).first;

        _it++;
        return true;
    }

    virtual bool nextKey(JsValue &keyOut) override {
        if (_it == _obj->_props.end()) {
            return false;
        }
        keyOut = _ctx->runtime->pushString((*_it).first);

        _it++;
        return true;
    }

    virtual bool nextValue(JsValue &valueOut) override {
        if (_it == _obj->_props.end()) {
            return false;
        }

        auto &prop = (*_it).second;
        valueOut = getPropertyValue(_ctx, prop, _obj->self);

        _it++;
        return true;
    }

    virtual bool next(JsValue &keyOut, JsValue &valueOut) override {
        if (_it == _obj->_props.end()) {
            return false;
        }

        keyOut = _ctx->runtime->pushString((*_it).first);

        auto &prop = (*_it).second;
        valueOut = getPropertyValue(_ctx, prop, _obj->self);

        _it++;
        return true;
    }

    virtual bool next(SizedString &keyOut, JsValue &valueOut) override {
        if (_it == _obj->_props.end()) {
            return false;
        }

        keyOut = (*_it).first;

        auto &prop = (*_it).second;
        valueOut = getPropertyValue(_ctx, prop, _obj->self);

        _it++;
        return true;
    }

protected:
    VMContext                       *_ctx;
    JsObject                        *_obj;
    MapNameToJsProperty::iterator   _it;

};


JsObject::JsObject(const JsValue &proto) : __proto__(proto, false, false, false, true) {
    type = JDT_OBJECT;
    _symbolProps = nullptr;
}

JsObject::~JsObject() {
    for (auto &item : _props) {
        auto &key = item.first;
        if (!key.isStable()) {
            delete [] key.data;
        }
    }
    _props.clear();

    if (_symbolProps) {
        delete _symbolProps;
    }
}

void JsObject::definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) {
    JsProperty *prop;

    auto it = _props.find(name);
    if (it == _props.end()) {
        if (name.equal(SS___PROTO__)) {
            prop = &__proto__;
        } else {
            // 定义新的属性
            _props[copyPropertyIfNeed(name)] = descriptor.defineProperty();
            return;
        }
    } else {
        prop = &(*it).second;
    }

    defineNameProperty(ctx, prop, descriptor, name);
}

void JsObject::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    NumberToSizedString name(index);
    return definePropertyByName(ctx, name, descriptor);
}

void JsObject::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    auto prop = getRawBySymbol(ctx, index, false);
    if (prop) {
        defineSymbolProperty(ctx, prop, descriptor, index);
        return;
    }

    // 定义新的属性
    if (!_symbolProps) {
        _symbolProps = new MapSymbolToJsProperty;
    }
    (*_symbolProps)[index] = descriptor.defineProperty();
}

void JsObject::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    auto it = _props.find(name);
    if (it == _props.end()) {
        if (name.equal(SS___PROTO__)) {
            set(ctx, &__proto__, thiz, value);
            return;
        }

        // 查找 prototye 的属性
        auto &proto = __proto__.value;
        JsNativeFunction funcGetter = nullptr;
        JsProperty *prop = nullptr;
        if (proto.type == JDT_NOT_INITIALIZED) {
            // 缺省的 Object.prototype
            prop = ctx->runtime->objPrototypeObject->getRawByName(ctx, name, funcGetter, true);
        } else if (proto.type >= JDT_OBJECT) {
            auto obj = ctx->runtime->getObject(proto);
            assert(obj);
            prop = obj->getRawByName(ctx, name, funcGetter, true);
        }

        if (prop) {
            if (prop->isGSetter) {
                if (prop->setter.isValid()) {
                    // prototype 带 setter 的可以直接返回用于修改调用
                    set(ctx, prop, thiz, value);
                }
                return;
            } else if (!prop->isWritable) {
                return;
            }
        }

        // 添加新属性
        _props[copyPropertyIfNeed(name)] = value;
    } else {
        set(ctx, &(*it).second, thiz, value);
    }
}

void JsObject::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    NumberToSizedString name(index);
    setByName(ctx, thiz, name, value);
}

void JsObject::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    auto prop = getRawBySymbol(ctx, index, false);
    if (prop) {
        // 修改已经存在的
        set(ctx, prop, thiz, value);
    } else {
        // 添加新属性
        if (!_symbolProps) {
            _symbolProps = new MapSymbolToJsProperty();
        }

        (*_symbolProps)[index] = value;
    }
}

JsValue JsObject::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    auto it = _props.find(name);
    if (it == _props.end()) {
        if (name.equal(SS___PROTO__)) {
            return jsValueNaN;
        }

        // 查找 prototye 的属性
        auto &proto = __proto__.value;
        JsNativeFunction funcGetter = nullptr;
        JsProperty *prop = nullptr;
        if (proto.type == JDT_NOT_INITIALIZED) {
            // 缺省的 Object.prototype
            prop = ctx->runtime->objPrototypeObject->getRawByName(ctx, name, funcGetter, true);
        } else if (proto.type >= JDT_OBJECT) {
            auto obj = ctx->runtime->getObject(proto);
            assert(obj);
            prop = obj->getRawByName(ctx, name, funcGetter, true);
        }

        if (prop) {
            if (prop->isGSetter) {
                // prototype 带 getter/setter
                return increase(ctx, prop, thiz, n, isPost);
            }

            // 不能修改到 proto，所以先复制到 temp，再添加
            JsProperty tmp = *prop;
            auto ret = increase(ctx, &tmp, thiz, n, isPost);

            if (prop->isWritable) {
                // 添加新属性
                _props[copyPropertyIfNeed(name)] = tmp.value;
            }
            return ret;
        } else {
            // 添加新属性
            _props[copyPropertyIfNeed(name)] = jsValueNaN;
            return jsValueNaN;
        }
    } else {
        return increase(ctx, &(*it).second, thiz, n, isPost);
    }
}

JsValue JsObject::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    NumberToSizedString name(index);
    return increaseByName(ctx, thiz, name, n, isPost);
}

JsValue JsObject::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    auto prop = getRawBySymbol(ctx, index, false);
    if (prop) {
        // 修改已经存在的
        return increase(ctx, prop, thiz, n, isPost);
    } else {
        // 添加新属性
        if (!_symbolProps) {
            _symbolProps = new MapSymbolToJsProperty();
        }

        (*_symbolProps)[index] = jsValueNaN;
        return jsValueNaN;
    }
}

JsProperty *JsObject::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    if (name.equal(SS___PROTO__)) {
        return &__proto__;
    }

    auto it = _props.find(name);
    if (it == _props.end()) {
        if (includeProtoProp) {
            auto &proto = __proto__.value;
            if (proto.type == JDT_NOT_INITIALIZED) {
                // 缺省的 Object.prototype
                return ctx->runtime->objPrototypeObject->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
            } else if (proto.type >= JDT_OBJECT) {
                auto obj = ctx->runtime->getObject(proto);
                assert(obj);
                return  obj->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
            }
        }
    } else {
        return &(*it).second;
    }

    return nullptr;
}

JsProperty *JsObject::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    NumberToSizedString name(index);
    JsNativeFunction funcGetter;
    return getRawByName(ctx, name, funcGetter, includeProtoProp);
}

JsProperty *JsObject::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    includeProtoProp = true;
    if (_symbolProps) {
        auto it = _symbolProps->find(index);
        if (it == _symbolProps->end()) {
            if (__proto__.value.type >= JDT_OBJECT) {
                auto obj = ctx->runtime->getObject(__proto__.value);
                assert(obj);
                auto ret = obj->getRawBySymbol(ctx, index, includeProtoProp);
                includeProtoProp = false;
                return ret;
            }
        } else {
            return &(*it).second;
        }
    }

    return nullptr;
}

bool JsObject::removeByName(VMContext *ctx, const SizedString &name) {
    auto it = _props.find(name);
    if (it != _props.end()) {
        auto &prop = (*it).second;
        if (prop.isConfigurable) {
            // 删除自己的属性
            _props.erase(name);
            return true;
        } else {
            return false;
        }
    }

    // 不存在的属性，也返回 true
    return true;
}

bool JsObject::removeByIndex(VMContext *ctx, uint32_t index) {
    NumberToSizedString name(index);
    return removeByName(ctx, name);
}

bool JsObject::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_symbolProps) {
        auto it = _symbolProps->find(index);
        if (it != _symbolProps->end()) {
            auto &prop = (*it).second;
            if (prop.isConfigurable) {
                // 删除自己的属性
                _symbolProps->erase(it);
                return true;
            } else {
                return false;
            }
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

IJsIterator *JsObject::getIteratorObject(VMContext *ctx) {
    auto it = new JsObjectIterator(ctx, this);
    return it;
}
