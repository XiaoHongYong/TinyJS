//
//  JsObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include <algorithm>
#include "JsObject.hpp"


SizedString copyPropertyIfNeed(SizedString name) {
    if (!name.isStable()) {
        auto p = new uint8_t[name.len];
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
        if (isPreventedExtensions) {
            return;
        }

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

    if (isPreventedExtensions) {
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
            if (!isPreventedExtensions) {
                set(ctx, &__proto__, thiz, value);
            }
            return;
        }

        // 查找 prototye 的属性
        JsProperty *prop = nullptr;
        JsNativeFunction funcGetter = nullptr;
        auto objProto = getPrototypeObject(ctx);
        if (objProto) {
            prop = objProto->getRawByName(ctx, name, funcGetter, true);
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

        if (!isPreventedExtensions) {
            // 添加新属性
            _props[copyPropertyIfNeed(name)] = value;
        }
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
        if (isPreventedExtensions) {
            return;
        }
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
        JsProperty *prop = nullptr;
        JsNativeFunction funcGetter = nullptr;
        auto objProto = getPrototypeObject(ctx);
        if (objProto) {
            prop = objProto->getRawByName(ctx, name, funcGetter, true);
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
            if (isPreventedExtensions) {
                return jsValueNaN;
            }
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

JsProperty *JsObject::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    auto it = _props.find(name);
    if (it == _props.end()) {
        if (name.equal(SS___PROTO__)) {
            return &__proto__;
        }

        if (includeProtoProp) {
            auto objProto = getPrototypeObject(ctx);
            if (objProto) {
                return objProto->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
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

bool JsObject::removeByName(VMContext *ctx, const SizedString &name) {
    auto it = _props.find(name);
    if (it != _props.end()) {
        auto &prop = (*it).second;
        if (prop.isConfigurable) {
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

void JsObject::changeAllProperties(VMContext *ctx, int8_t configurable, int8_t writable) {
    for (auto &item : _props) {
        item.second.changeProperty(configurable, writable);
    }

    if (_symbolProps) {
        for (auto &item : *_symbolProps) {
            item.second.changeProperty(configurable, writable);
        }
    }
}

bool JsObject::hasAnyProperty(VMContext *ctx, bool configurable, bool writable) {
    for (auto &item : _props) {
        if (item.second.isPropertyAny(configurable, writable)) {
            return true;
        }
    }

    if (_symbolProps) {
        for (auto &item : *_symbolProps) {
            if (item.second.isPropertyAny(configurable, writable)) {
                return true;
            }
        }
    }

    return false;
}

IJsObject *JsObject::clone() {
    auto obj = new JsObject(__proto__.value);

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

IJsIterator *JsObject::getIteratorObject(VMContext *ctx, bool includeProtoProp) {
    auto it = new JsObjectIterator(ctx, this, includeProtoProp);
    return it;
}

void JsObject::markReferIdx(VMRuntime *rt) {
    assert(referIdx == rt->nextReferIdx());

    for (auto &item : _props) {
        rt->markReferIdx(item.second);
    }

    if (__proto__.value.type > JDT_NUMBER) {
        rt->markReferIdx(__proto__.value);
    }

    if (_symbolProps) {
        for (auto &item : *_symbolProps) {
            rt->markSymbolUsed(item.first);
            rt->markReferIdx(item.second);
        }
    }
}
