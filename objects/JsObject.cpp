//
//  JsObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include <algorithm>
#include "JsObject.hpp"


StringView copyPropertyIfNeed(StringView name) {
    if (!name.isStable()) {
        auto p = new char[name.len];
        memcpy(p, name.data, name.len);
        name.data = p;
    }

    return name;
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

};


JsObject::JsObject(const JsValue &proto) : IJsObject(proto, JDT_OBJECT) {
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

void JsObject::setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) {
    auto it = _props.find(name);
    if (it == _props.end()) {
        if (isPreventedExtensions) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot define property %.*s, object is not extensible", name.len, name.data);
            return;
        }

        if (name.equal(SS___PROTO__)) {
            __proto__ = descriptor;
        } else {
            // 定义新的属性
            _props[copyPropertyIfNeed(name)] = descriptor;
        }
    } else {
        (*it).second = descriptor;
    }
}

void JsObject::setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    NumberToStringView name(index);
    return setPropertyByName(ctx, name, descriptor);
}

void JsObject::setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    auto prop = getRawBySymbol(ctx, index, false);
    if (prop) {
        *prop = descriptor;
        return;
    }

    if (isPreventedExtensions) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot define property Symbol(%d), object is not extensible", index);
        return;
    }

    // 定义新的属性
    if (!_symbolProps) {
        _symbolProps = new MapSymbolToJsProperty;
    }
    (*_symbolProps)[index] = descriptor;
}

JsError JsObject::setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) {
    auto it = _props.find(name);
    if (it == _props.end()) {
        if (name.equal(SS___PROTO__)) {
            if (!isPreventedExtensions) {
                return setPropertyValue(ctx, &__proto__, thiz, value);
            }
            return JE_TYPE_PREVENTED_EXTENSION;
        }

        // 查找 prototye 的属性
        JsValue *prop = nullptr;
        auto objProto = getPrototypeObject(ctx);
        if (objProto) {
            prop = objProto->getRawByName(ctx, name, true);
        }

        if (prop) {
            if (prop->isGetterSetter()) {
                // prototype 带 setter 的可以直接返回用于修改调用
                return setPropertyValue(ctx, prop, thiz, value);
            } else if (!prop->isWritable()) {
                return JE_TYPE_PROP_READ_ONLY;
            }
        }

        if (!isPreventedExtensions) {
            // 添加新属性
            _props[copyPropertyIfNeed(name)] = value.asProperty();
            return JE_OK;
        }
        return JE_TYPE_PREVENTED_EXTENSION;
    } else {
        return setPropertyValue(ctx, &(*it).second, thiz, value);
    }
}

JsError JsObject::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    NumberToStringView name(index);
    return setByName(ctx, thiz, name, value);
}

JsError JsObject::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    auto prop = getRawBySymbol(ctx, index, false);
    if (prop) {
        // 修改已经存在的
        return setPropertyValue(ctx, prop, thiz, value);
    } else {
        if (isPreventedExtensions) {
            return JE_TYPE_PREVENTED_EXTENSION;
        }
        // 添加新属性
        if (!_symbolProps) {
            _symbolProps = new MapSymbolToJsProperty();
        }

        (*_symbolProps)[index] = value;
        return JE_OK;
    }
}

JsValue JsObject::increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) {
    auto it = _props.find(name);
    if (it == _props.end()) {
        if (name.equal(SS___PROTO__)) {
            return jsValueNaN;
        }

        // 查找 prototye 的属性
        JsValue *prop = nullptr;
        auto objProto = getPrototypeObject(ctx);
        if (objProto) {
            prop = objProto->getRawByName(ctx, name, true);
        }

        if (prop) {
            if (prop->isGetterSetter()) {
                // prototype 带 getter/setter
                return increasePropertyValue(ctx, prop, thiz, n, isPost);
            }

            // 不能修改到 proto，所以先复制到 temp，再添加
            JsValue tmp = *prop;
            auto ret = increasePropertyValue(ctx, &tmp, thiz, n, isPost);

            if (prop->isWritable()) {
                // 添加新属性
                _props[copyPropertyIfNeed(name)] = tmp.asProperty();
            }
            return ret;
        } else {
            if (isPreventedExtensions) {
                return jsValueNaN;
            }
            // 添加新属性
            _props[copyPropertyIfNeed(name)] = jsValueNaN.asProperty();
            return jsValueNaN;
        }
    } else {
        return increasePropertyValue(ctx, &(*it).second, thiz, n, isPost);
    }
}

JsValue JsObject::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    NumberToStringView name(index);
    return increaseByName(ctx, thiz, name, n, isPost);
}

JsValue JsObject::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    auto prop = getRawBySymbol(ctx, index, false);
    if (prop) {
        // 修改已经存在的
        return increasePropertyValue(ctx, prop, thiz, n, isPost);
    } else {
        if (isPreventedExtensions) {
            return jsValueNaN;
        }

        // 添加新属性
        if (!_symbolProps) {
            _symbolProps = new MapSymbolToJsProperty();
        }

        (*_symbolProps)[index] = jsValueNaN;
        return jsValueNaN;
    }
}

JsValue *JsObject::getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp) {
    auto it = _props.find(name);
    if (it == _props.end()) {
        if (name.equal(SS___PROTO__)) {
            return &__proto__;
        }

        if (includeProtoProp) {
            auto objProto = getPrototypeObject(ctx);
            if (objProto) {
                return objProto->getRawByName(ctx, name, includeProtoProp);
            }
        }
    } else {
        return &(*it).second;
    }

    return nullptr;
}

JsValue *JsObject::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    NumberToStringView name(index);
    return getRawByName(ctx, name, includeProtoProp);
}

JsValue *JsObject::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (!_symbolProps) {
        return nullptr;
    }

    auto it = _symbolProps->find(index);
    if (it == _symbolProps->end()) {
        if (includeProtoProp) {
            auto objProto = getPrototypeObject(ctx);
            if (objProto) {
                return objProto->getRawBySymbol(ctx, index, includeProtoProp);
            }
        }
    } else {
        return &(*it).second;
    }

    return nullptr;
}

bool JsObject::removeByName(VMContext *ctx, const StringView &name) {
    auto it = _props.find(name);
    if (it != _props.end()) {
        auto &prop = (*it).second;
        if (prop.isConfigurable()) {
            // 删除自己的属性
            auto key = (*it).first;
            if (!key.isStable()) {
                // 释放 Key 的内存
                delete [] key.data;
            }

            _props.erase(it);
            return true;
        } else {
            return false;
        }
    }

    // 不存在的属性，也返回 true
    return true;
}

bool JsObject::removeByIndex(VMContext *ctx, uint32_t index) {
    NumberToStringView name(index);
    return removeByName(ctx, name);
}

bool JsObject::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_symbolProps) {
        auto it = _symbolProps->find(index);
        if (it != _symbolProps->end()) {
            auto &prop = (*it).second;
            if (prop.isConfigurable()) {
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

void JsObject::changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) {

    for (auto &item : _props) {
        item.second.changeProperty(toAdd, toRemove);
    }

    if (_symbolProps) {
        for (auto &item : *_symbolProps) {
            item.second.changeProperty(toAdd, toRemove);
        }
    }
}

bool JsObject::hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) {
    for (auto &item : _props) {
        if (item.second.isPropertyAny(flags)) {
            return true;
        }
    }

    if (_symbolProps) {
        for (auto &item : *_symbolProps) {
            if (item.second.isPropertyAny(flags)) {
                return true;
            }
        }
    }

    return false;
}

IJsObject *JsObject::clone() {
    auto obj = new JsObject(__proto__);

    for (auto &item : _props) {
        auto &key = item.first;
        if (!key.isStable()) {
            delete [] key.data;
        }
    }

    obj->_props = _props;
    if (_symbolProps) { obj->_symbolProps = new MapSymbolToJsProperty; *obj->_symbolProps = *_symbolProps; }

    return obj;
}

IJsIterator *JsObject::getIteratorObject(VMContext *ctx, bool includeProtoProp, bool includeNoneEnumerable) {
    auto it = new JsObjectIterator(ctx, this, includeProtoProp, includeNoneEnumerable);
    return it;
}

void JsObject::markReferIdx(VMRuntime *rt) {
    assert(referIdx == rt->nextReferIdx());

    for (auto &item : _props) {
        rt->markReferIdx(item.second);
    }

    if (__proto__.isValid()) {
        rt->markReferIdx(__proto__);
    }

    if (_symbolProps) {
        for (auto &item : *_symbolProps) {
            rt->markSymbolUsed(item.first);
            rt->markReferIdx(item.second);
        }
    }
}
