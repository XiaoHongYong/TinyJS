//
//  IJsObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "IJsObject.hpp"
#include "JsObject.hpp"
#include <algorithm>


IJsObject::IJsObject(JsValue proto, JsDataType type) : __proto__(proto), type(type) {
    __proto__.setProperty(JP_WRITABLE);
    isPreventedExtensions = false;
    _isOfIterable = false;
    referIdx = 0;
    nextFreeIdx = 0;
}

bool IJsObject::getBool(VMContext *ctx, const JsValue &thiz, const StringView &name) {
    auto value = getByName(ctx, thiz, name);
    return ctx->runtime->testTrue(value);
}

bool IJsObject::getBool(VMContext *ctx, const JsValue &thiz, const JsValue &name) {
    auto value = get(ctx, thiz, name);
    return ctx->runtime->testTrue(value);
}

void IJsObject::setProperty(VMContext *ctx, const JsValue &nameOrg, const JsValue &descriptor) {
    JsValue name = nameOrg;
    if (name.type >= JDT_OBJECT) {
        name = ctx->runtime->toString(ctx, nameOrg);
    }

    switch (name.type) {
        case JDT_UNDEFINED: setPropertyByName(ctx, SS_UNDEFINED, descriptor); break;
        case JDT_NULL: setPropertyByName(ctx, SS_NULL, descriptor); break;
        case JDT_BOOL: setPropertyByName(ctx, name.value.n32 ? SS_TRUE : SS_FALSE, descriptor); break;
        case JDT_INT32: setPropertyByIndex(ctx, name.value.n32, descriptor); break;
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(name), buf);
            setPropertyByName(ctx, StringView(buf, len), descriptor);
            break;
        }
        case JDT_CHAR: {
            StringViewWrapper str(name);
            setPropertyByName(ctx, str, descriptor);
            break;
        }
        case JDT_STRING: {
            auto &strName = ctx->runtime->getUtf8String(name);
            setPropertyByName(ctx, strName, descriptor);
            break;
        }
        case JDT_SYMBOL: setPropertyBySymbol(ctx, name.value.index, descriptor); break;
        default: {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }
}

bool IJsObject::getOwnPropertyDescriptor(VMContext *ctx, const JsValue &nameOrg, JsValue &descriptorOut) {
    JsValue name = nameOrg;
    if (name.type >= JDT_OBJECT) {
        name = ctx->runtime->toString(ctx, nameOrg);
    }

    descriptorOut = jsValueEmpty;

    switch (name.type) {
        case JDT_UNDEFINED: return getOwnPropertyDescriptorByName(ctx, SS_UNDEFINED, descriptorOut);
        case JDT_NULL: return getOwnPropertyDescriptorByName(ctx, SS_NULL, descriptorOut);
        case JDT_BOOL: return getOwnPropertyDescriptorByName(ctx, name.value.n32 ? SS_TRUE : SS_FALSE, descriptorOut);
        case JDT_INT32: return getOwnPropertyDescriptorByIndex(ctx, name.value.n32, descriptorOut);
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(name), buf);
            return getOwnPropertyDescriptorByName(ctx, StringView(buf, len), descriptorOut);
        }
        case JDT_CHAR: {
            StringViewWrapper str(name);
            return getOwnPropertyDescriptorByName(ctx, str.str(), descriptorOut);
        }
        case JDT_STRING: {
            auto &str = ctx->runtime->getUtf8String(name);
            return getOwnPropertyDescriptorByName(ctx, str, descriptorOut);
        }
        case JDT_SYMBOL:
            return getOwnPropertyDescriptorBySymbol(ctx, name.value.index, descriptorOut);
        default: {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return false;
}

JsValue *IJsObject::getRaw(VMContext *ctx, const JsValue &name, bool includeProtoProp) {
    switch (name.type) {
        case JDT_UNDEFINED: return getRawByName(ctx, SS_UNDEFINED, includeProtoProp); break;
        case JDT_NULL: return getRawByName(ctx, SS_NULL, includeProtoProp); break;
        case JDT_BOOL: return getRawByName(ctx, name.value.n32 ? SS_TRUE : SS_FALSE, includeProtoProp); break;
        case JDT_INT32: return getRawByIndex(ctx, name.value.n32, includeProtoProp); break;
        case JDT_NUMBER: {
            auto d = ctx->runtime->getDouble(name);
            if (d == (int32_t)d) {
                return getRawByIndex(ctx, (int32_t)d, includeProtoProp);
            } else {
                char buf[64];
                auto len = floatToString(d, buf);
                return getRawByName(ctx, StringView(buf, len), includeProtoProp);
            }
            break;
        }
        case JDT_CHAR: {
            StringViewWrapper str(name);
            return getRawByName(ctx, str.str(), includeProtoProp);
        }
        case JDT_STRING: {
            auto &str = ctx->runtime->getUtf8String(name);
            return getRawByName(ctx, str, includeProtoProp);
        }
        case JDT_SYMBOL: {
            return getRawBySymbol(ctx, name.value.index, includeProtoProp);
        }
        default: {
            assert(name.type >= JDT_OBJECT);
            auto nameNew = ctx->runtime->jsObjectToString(ctx, name);
            if (ctx->error != JE_OK) {
                return nullptr;
            }
            return getRaw(ctx, nameNew, includeProtoProp);
        }
    }
}

JsValue IJsObject::get(VMContext *ctx, const JsValue &thiz, const JsValue &nameOrg, const JsValue &defVal) {
    auto *prop = getRaw(ctx, nameOrg, true);
    if (prop) {
        return getPropertyValue(ctx, thiz, prop, defVal);
    }
    return defVal;
}

void IJsObject::set(VMContext *ctx, const JsValue &thiz, const JsValue &nameOrg, const JsValue &value) {
    JsValue name = nameOrg;
    if (name.type >= JDT_OBJECT) {
        name = ctx->runtime->toString(ctx, nameOrg);
    }

    switch (name.type) {
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
                setByName(ctx, thiz, StringView(buf, len), value);
            }
            break;
        }
        case JDT_CHAR: {
            StringViewWrapper s(name);
            setByName(ctx, thiz, s.str(), value);
            break;
        }
        case JDT_STRING: {
            auto &strName = ctx->runtime->getUtf8String(name);
            setByName(ctx, thiz, strName, value);
            break;
        }
        case JDT_SYMBOL:
            setBySymbol(ctx, thiz, name.value.index, value);
            break;
        default: {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert object to primitive value");
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
                return increaseByName(ctx, thiz, StringView(buf, len), n, isPost);
            }
            break;
        }
        case JDT_CHAR: {
            StringViewWrapper str(name);
            return increaseByName(ctx, thiz, str.str(), n, isPost);
        }
        case JDT_STRING: {
            auto &strName = ctx->runtime->getUtf8String(name);
            return increaseByName(ctx, thiz, strName, n, isPost);
        }
        case JDT_SYMBOL:
            return increaseBySymbol(ctx, thiz, name.value.index, n, isPost);
        default: {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert object to primitive value");
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
        case JDT_UNDEFINED: return removeByName(ctx, SS_UNDEFINED);
        case JDT_NULL: return removeByName(ctx, SS_NULL);
        case JDT_BOOL: return removeByName(ctx, prop.value.n32 ? SS_TRUE : SS_FALSE);
        case JDT_INT32: return removeByIndex(ctx, prop.value.n32);
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(ctx->runtime->getDouble(prop), buf);
            return removeByName(ctx, StringView(buf, len));
            break;
        }
        case JDT_CHAR: {
            StringViewWrapper str(prop);
            return removeByName(ctx, str.str());
        }
        case JDT_STRING: {
            auto &name = ctx->runtime->getUtf8String(prop);
            return removeByName(ctx, name);
        }
        case JDT_SYMBOL:
            return removeBySymbol(ctx, prop.value.index);
        default: {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert object to primitive value");
            break;
        }
    }

    return true;
}

bool IJsObject::getLength(VMContext *ctx, int32_t &lengthOut) {
    auto n = ctx->runtime->toNumber(ctx, this->getByName(ctx, self, SS_LENGTH));
    if (!isnan(n)) {
        // Array like.
        lengthOut = (int32_t)n;
        return true;
    }

    return false;
}

/**
 * 通过 { get x() {}, set x() {} } 时调用.
 */
void IJsObject::addGetterSetterByName(VMContext *ctx, const StringView &name, const JsValue &getter, const JsValue &setter) {
    auto prop = getRawByName(ctx, name);
    if (prop == nullptr) {
        auto gs = ctx->runtime->pushGetterSetter(getter, setter);
        setPropertyByName(ctx, name, gs.asProperty(JP_DEFAULT));
    } else {
        auto &gs = ctx->runtime->getGetterSetter(*prop);
        if (getter.isFunction()) gs.getter = getter;
        if (setter.isFunction()) gs.setter = setter;
    }
}

/**
 * 为了遍历 Object 的所有属性
 */
class JsObjectIterator : public IJsIterator {
public:
    JsObjectIterator(VMContext *ctx, JsObject *obj, bool includeProtoProp, bool includeNoneEnumerable) : IJsIterator(includeProtoProp, includeNoneEnumerable)
    {
        _ctx = ctx;
        _obj = obj;
        _it = obj->_props.begin();
        _itProto = nullptr;
        _includeProtoProp = includeProtoProp;
    }

    virtual bool next(StringView *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        if (_itProto) {
            return _itProto->next(strKeyOut, keyOut, valueOut);
        }

        while (true) {
            if (_it == _obj->_props.end()) {
                if (_includeProtoProp) {
                    if (_itProto == nullptr) {
                        auto objProto = _obj->getPrototypeObject(_ctx);
                        if (objProto) {
                            _itProto = objProto->getIteratorObject(_ctx, true);
                        } else {
                            return false;
                        }
                    }
                    return _itProto->next(strKeyOut, keyOut, valueOut);
                }
                return false;
            }

            auto &prop = (*_it).second;
            if (prop.isEnumerable() || _includeNoneEnumerable) {
                break;
            }
            _it++;
        }

        if (strKeyOut) {
            *strKeyOut = (*_it).first;
        }

        if (keyOut) {
            *keyOut = _ctx->runtime->pushString((*_it).first);
        }

        if (valueOut) {
            *valueOut = getPropertyValue(_ctx, _obj->self, (*_it).second);
        }

        _it++;

        return true;
    }

protected:
    VMContext                       *_ctx;
    JsObject                        *_obj;
    MapNameToJsProperty::iterator   _it;
    IJsIterator                     *_itProto;
    bool                            _includeProtoProp;

};
