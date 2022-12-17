//
//  JsObjectLazy.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#include "JsObjectLazy.hpp"


JsObjectLazy::JsObjectLazy(JsLazyProperty *props, uint32_t countProps, const JsValue &__proto__, JsDataType type) : IJsObject(__proto__, type), _props(props), _propsEnd(props + countProps)
{
    _obj = nullptr;
}

JsObjectLazy::~JsObjectLazy() {
    if (_obj) {
        delete _obj;
    }
}

void JsObjectLazy::setPropertyByName(VMContext *ctx, const SizedString &name, const JsValue &descriptor) {
    for (auto p = _props; p < _propsEnd; p++) {
        if (name.equal(p->name)) {
            p->prop = descriptor;
            return;
        }
    }

    if (!_obj) {
        _newObject(ctx);
    }
    _obj->setPropertyByName(ctx, name, descriptor);
}

void JsObjectLazy::setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyByIndex(ctx, index, descriptor);
}

void JsObjectLazy::setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyBySymbol(ctx, index, descriptor);
}

JsError JsObjectLazy::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    for (auto p = _props; p < _propsEnd; p++) {
        if (name.equal(p->name)) {
            return setPropertyValue(ctx, &p->prop, thiz, value);
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
            return increasePropertyValue(ctx, &p->prop, thiz, n, isPost);
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

JsValue *JsObjectLazy::getRawByName(VMContext *ctx, const SizedString &name, bool includeProtoProp) {
    for (auto p = _props; p < _propsEnd; p++) {
        if (name.equal(p->name)) {
            if (p->isLazyInit && p->prop.isEmpty()) {
                onInitLazyProperty(ctx, p);
            }
            return &p->prop;
        }
    }

    if (_obj) {
        return _obj->getRawByName(ctx, name, includeProtoProp);
    }

    if (name.equal(SS___PROTO__)) {
        return &__proto__;
    }

    if (includeProtoProp) {
        // 查找 __proto__ 的属性
        auto obj = getPrototypeObject(ctx);
        if (obj) {
            return obj->getRawByName(ctx, name, true);
        }
    }

    return nullptr;
}

JsValue *JsObjectLazy::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawByIndex(ctx, index, includeProtoProp);
    }

    return nullptr;
}

JsValue *JsObjectLazy::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsObjectLazy::removeByName(VMContext *ctx, const SizedString &name) {
    for (auto p = _props; p < _propsEnd; p++) {
        if (name.equal(p->name)) {
            if (p->prop.isConfigurable()) {
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

void JsObjectLazy::changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) {
    for (auto p = _props; p < _propsEnd; p++) {
        p->prop.changeProperty(toAdd, toRemove);
    }

    if (_obj) {
        _obj->changeAllProperties(ctx, toAdd, toRemove);
    }
}

bool JsObjectLazy::hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) {
    for (auto p = _props; p < _propsEnd; p++) {
        if (p->prop.isPropertyAny(flags)) {
            return true;
        }
    }

    return _obj && _obj->hasAnyProperty(ctx, flags);
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

    _obj = new JsObject(__proto__);

    if (isPreventedExtensions) {
        _obj->preventExtensions(ctx);
    }
}
