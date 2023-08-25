//
//  JsObjectX.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/22.
//

#include "JsObjectX.hpp"

JsObjectX::JsObjectX(uint32_t extraType, const JsValue &__proto__) : IJsObject(__proto__, JDT_OBJ_X), _extraType(extraType) {
    _obj = nullptr;
}

JsObjectX::~JsObjectX() {
    if (_obj) {
        delete _obj;
    }
}

void JsObjectX::setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) {
    if (onSetValue(ctx, name, descriptor)) {
        return;
    }

    if (!_obj) {
        _newObject(ctx);
    }
    _obj->setPropertyByName(ctx, name, descriptor);
}

void JsObjectX::setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyByIndex(ctx, index, descriptor);
}

void JsObjectX::setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyBySymbol(ctx, index, descriptor);
}

JsError JsObjectX::setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) {
    if (onSetValue(ctx, name, value)) {
        return JE_OK;
    }

    if (!_obj) {
        _newObject(ctx);
    }

    return _obj->setByName(ctx, thiz, name, value);
}

JsError JsObjectX::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setByIndex(ctx, thiz, index, value);
}

JsError JsObjectX::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue JsObjectX::increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) {
    auto value = onGetValue(ctx, name);
    if (value.isValid()) {
        auto newValue = increase(ctx, value, n);
        onSetValue(ctx, name, newValue);
        return isPost ? value : newValue;
    }

    if (!_obj) {
        _newObject(ctx);
    }

    return _obj->increaseByName(ctx, thiz, name, n, isPost);
}

JsValue JsObjectX::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseByIndex(ctx, thiz, index, n, isPost);
}

JsValue JsObjectX::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsValue *JsObjectX::getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp) {
    _tmpRawHolder = onGetValue(ctx, name);
    if (_tmpRawHolder.isValid()) {
        return &_tmpRawHolder;
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

JsValue *JsObjectX::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawByIndex(ctx, index, includeProtoProp);
    }

    return nullptr;
}

JsValue *JsObjectX::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsObjectX::removeByName(VMContext *ctx, const StringView &name) {
    if (onSetValue(ctx, name, jsValueEmpty)) {
        return true;
    }

    if (_obj) {
        return _obj->removeByName(ctx, name);
    }

    return true;
}

bool JsObjectX::removeByIndex(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeByIndex(ctx, index);
    }

    return true;
}

bool JsObjectX::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeBySymbol(ctx, index);
    }

    return true;
}

void JsObjectX::changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) {
    if (_obj) {
        _obj->changeAllProperties(ctx, toAdd, toRemove);
    }
}

bool JsObjectX::hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) {
    return _obj && _obj->hasAnyProperty(ctx, flags);
}

void JsObjectX::preventExtensions(VMContext *ctx) {
    IJsObject::preventExtensions(ctx);

    if (_obj) {
        _obj->preventExtensions(ctx);
    }
}

IJsIterator *JsObjectX::getIteratorObject(VMContext *ctx, bool includeProtoProp, bool includeNoneEnumerable) {
    if (_obj) {
        return _obj->getIteratorObject(ctx, includeProtoProp, includeNoneEnumerable);
    }

    return new EmptyJsIterator();
}

void JsObjectX::markReferIdx(VMRuntime *rt) {
    assert(referIdx == rt->nextReferIdx());

    rt->markReferIdx(__proto__);

    if (_obj) {
        _obj->referIdx = rt->nextReferIdx();
        _obj->markReferIdx(rt);
    }
}

void JsObjectX::_newObject(VMContext *ctx) {
    assert(_obj == nullptr);

    _obj = new JsObject(__proto__);

    if (isPreventedExtensions) {
        _obj->preventExtensions(ctx);
    }
}
