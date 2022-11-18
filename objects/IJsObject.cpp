//
//  IJsObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "IJsObject.hpp"
#include "JsObject.hpp"
#include <algorithm>


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
            SizedStringWrapper str(name);
            definePropertyByName(ctx, str, descriptor);
            break;
        }
        case JDT_STRING: {
            auto &strName = ctx->runtime->getUtf8String(name);
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
            SizedStringWrapper str(name);
            return getOwnPropertyDescriptorByName(ctx, str.str(), descriptorOut);
        }
        case JDT_STRING: {
            auto &str = ctx->runtime->getUtf8String(name);
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
            SizedStringWrapper str(name);
            return getRawByName(ctx, str.str(), funcGetterOut, includeProtoProp);
        }
        case JDT_STRING: {
            auto &str = ctx->runtime->getUtf8String(name);
            return getRawByName(ctx, str, funcGetterOut, includeProtoProp);
        }
        case JDT_SYMBOL: {
            return getRawBySymbol(ctx, name.value.index, includeProtoProp);
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
        } else if (prop->value.type == JDT_NOT_INITIALIZED) {
            return defVal;
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
//        auto strName = ctx->runtime->toSizedString(ctx, name);
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
            SizedStringWrapper s(name);
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
            SizedStringWrapper str(name);
            return increaseByName(ctx, thiz, str.str(), n, isPost);
        }
        case JDT_STRING: {
            auto &strName = ctx->runtime->getUtf8String(name);
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
            SizedStringWrapper str(prop);
            return removeByName(ctx, str.str());
        }
        case JDT_STRING: {
            auto &name = ctx->runtime->getUtf8String(prop);
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
 * 为了遍历 Object 的所有属性
 */
class JsObjectIterator : public IJsIterator {
public:
    JsObjectIterator(VMContext *ctx, JsObject *obj, bool includeProtoProp) {
        _ctx = ctx;
        _obj = obj;
        _it = obj->_props.begin();
        _itProto = nullptr;
        _includeProtoProp = includeProtoProp;
    }

    virtual bool next(SizedString *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
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
            if (prop.isEnumerable) {
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
            *valueOut = getPropertyValue(_ctx, (*_it).second, _obj->self);
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
