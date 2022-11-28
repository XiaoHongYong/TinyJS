//
//  JsObjectLazy.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#include "JsObjectLazy.hpp"


JsObjectLazy::JsObjectLazy(JsLazyProperty *props, uint32_t countProps, const JsValue &__proto__) : _props(props), _propsEnd(props + countProps), __proto__(__proto__) {

    _obj = nullptr;
}

JsObjectLazy::~JsObjectLazy() {
    if (_obj) {
        delete _obj;
    }
}

void JsObjectLazy::definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) {
    for (auto p = _props; p < _propsEnd; p++) {
        if (name.equal(p->name)) {
            defineNameProperty(ctx, &p->prop, descriptor, name);
            return;
        }
    }

    if (!_obj) {
        _newObject(ctx);
    }
    _obj->definePropertyByName(ctx, name, descriptor);
}

void JsObjectLazy::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->definePropertyByIndex(ctx, index, descriptor);
}

void JsObjectLazy::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor);
}

JsError JsObjectLazy::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    for (auto p = _props; p < _propsEnd; p++) {
        if (name.equal(p->name)) {
            return set(ctx, &p->prop, thiz, value);
        }
    }

    if (!_obj) {
        _newObject(ctx);
    }

    return _obj->setByName(ctx, thiz, name, value);
}

JsError JsObjectLazy::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setByIndex(ctx, thiz, index, value);
}

JsError JsObjectLazy::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue JsObjectLazy::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    for (auto p = _props; p < _propsEnd; p++) {
        if (name.equal(p->name)) {
            return increase(ctx, &p->prop, thiz, n, isPost);
        }
    }

    if (!_obj) {
        _newObject(ctx);
    }

    return _obj->increaseByName(ctx, thiz, name, n, isPost);
}

JsValue JsObjectLazy::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseByIndex(ctx, thiz, index, n, isPost);
}

JsValue JsObjectLazy::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsProperty *JsObjectLazy::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    funcGetterOut = nullptr;

    for (auto p = _props; p < _propsEnd; p++) {
        if (name.equal(p->name)) {
            if (p->isLazyInit && p->prop.value.type == JDT_NOT_INITIALIZED) {
                onInitLazyProperty(ctx, p);
            }
            return &p->prop;
        }
    }

    if (_obj) {
        return _obj->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
    }

    if (name.equal(SS___PROTO__)) {
        return &__proto__;
    }

    if (includeProtoProp) {
        // 查找 __proto__ 的属性
        if (__proto__.value.equal(jsValuePrototypeFunction)) {
            // 缺省的 Function.prototype
            return ctx->runtime->objPrototypeFunction->getRawByName(ctx, name, funcGetterOut, true);
        } else if (__proto__.value.type >= JDT_OBJECT) {
            auto obj = ctx->runtime->getObject(__proto__.value);
            assert(obj);
            return obj->getRawByName(ctx, name, funcGetterOut, true);
        }
    }

    return nullptr;
}

JsProperty *JsObjectLazy::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawByIndex(ctx, index, includeProtoProp);
    }

    return nullptr;
}

JsProperty *JsObjectLazy::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsObjectLazy::removeByName(VMContext *ctx, const SizedString &name) {
    for (auto p = _props; p < _propsEnd; p++) {
        if (name.equal(p->name)) {
            if (p->prop.isConfigurable) {
                memmove(p, p + 1, sizeof(*p) * (_propsEnd - p));
                _propsEnd--;
                return true;
            } else {
                return false;
            }
        }
    }

    if (_obj) {
        return _obj->removeByName(ctx, name);
    }

    return true;
}

bool JsObjectLazy::removeByIndex(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeByIndex(ctx, index);
    }

    return true;
}

bool JsObjectLazy::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeBySymbol(ctx, index);
    }

    return true;
}

void JsObjectLazy::changeAllProperties(VMContext *ctx, int8_t configurable, int8_t writable) {
    for (auto p = _props; p < _propsEnd; p++) {
        p->prop.changeProperty(configurable, writable);
    }

    if (_obj) {
        _obj->changeAllProperties(ctx, configurable, writable);
    }
}

bool JsObjectLazy::hasAnyProperty(VMContext *ctx, bool configurable, bool writable) {
    for (auto p = _props; p < _propsEnd; p++) {
        if (p->prop.isPropertyAny(configurable, writable)) {
            return true;
        }
    }

    return _obj && _obj->hasAnyProperty(ctx, configurable, writable);
}

void JsObjectLazy::preventExtensions(VMContext *ctx) {
    IJsObject::preventExtensions(ctx);

    if (_obj) {
        _obj->preventExtensions(ctx);
    }
}

IJsIterator *JsObjectLazy::getIteratorObject(VMContext *ctx, bool includeProtoProp, bool includeNoneEnumerable) {
    if (_obj) {
        return _obj->getIteratorObject(ctx, includeProtoProp, includeNoneEnumerable);
    }

    return new EmptyJsIterator();
}

void JsObjectLazy::markReferIdx(VMRuntime *rt) {
    assert(referIdx == rt->nextReferIdx());

    for (auto p = _props; p < _propsEnd; p++) {
        rt->markReferIdx(p->prop);
    }

    rt->markReferIdx(__proto__);

    if (_obj) {
        _obj->referIdx = rt->nextReferIdx();
        _obj->markReferIdx(rt);
    }
}

void JsObjectLazy::_newObject(VMContext *ctx) {
    assert(_obj == nullptr);

    _obj = new JsObject(__proto__.value);

    if (isPreventedExtensions) {
        _obj->preventExtensions(ctx);
    }
}
