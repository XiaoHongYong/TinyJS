//
//  IJsIterator.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/10/10.
//

#include "IJsIterator.hpp"
#include "JsObject.hpp"


/**
 * 遍历 IJsIterator
 */
class JsIteratorOfIterator : public IJsIterator {
public:
    JsIteratorOfIterator(VMContext *ctx, IJsIterator *it, bool includeProtoProp, bool includeNoneEnumerable) : IJsIterator(includeProtoProp, includeNoneEnumerable) {
        _ctx = ctx;
        _it = it;
        _itObj = nullptr;
    }

    ~JsIteratorOfIterator() {
        if (_itObj) {
            delete _itObj;
        }
    }

    virtual bool nextOf(JsValue &valueOut) override {
        return _it->nextOf(valueOut);
    }

    virtual bool next(StringView *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        if (_itObj == nullptr) {
            if (_it->_obj) {
                _itObj = _it->_obj->getIteratorObject(_ctx, _includeProtoProp, _includeNoneEnumerable);
            } else {
                return false;
            }
        }

        return _itObj->next(strKeyOut, keyOut, valueOut);
    }

protected:
    VMContext                       *_ctx;
    IJsIterator                     *_it;

    IJsIterator                     *_itObj;

};


void IJsIterator::setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyByName(ctx, name, descriptor);
}

void IJsIterator::setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    NumberToStringView name(index);
    setPropertyByName(ctx, name, descriptor);
}

void IJsIterator::setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyBySymbol(ctx, index, descriptor);
}

JsError IJsIterator::setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }

    return _obj->setByName(ctx, thiz, name, value);
}

JsError IJsIterator::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    NumberToStringView name(index);
    return setByName(ctx, thiz, name, value);
}

JsError IJsIterator::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue IJsIterator::increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }

    return _obj->increaseByName(ctx, thiz, name, n, isPost);
}

JsValue IJsIterator::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    NumberToStringView name(index);
    return increaseByName(ctx, thiz, name, n, isPost);
}

JsValue IJsIterator::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsValue *IJsIterator::getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawByName(ctx, name, includeProtoProp);
    }

    if (name.equal(SS___PROTO__)) {
        return &__proto__;
    }

    if (includeProtoProp) {
        // 查找 prototye 的属性
        auto obj = getPrototypeObject(ctx);
        if (obj) {
            return obj->getRawByName(ctx, name, true);
        }
    }

    return nullptr;
}

JsValue *IJsIterator::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawByIndex(ctx, index, includeProtoProp);
    }

    return nullptr;
}

JsValue *IJsIterator::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool IJsIterator::removeByName(VMContext *ctx, const StringView &name) {
    if (_obj) {
        return _obj->removeByName(ctx, name);
    }

    return true;
}

bool IJsIterator::removeByIndex(VMContext *ctx, uint32_t index) {
    NumberToStringView name(index);
    return removeByName(ctx, name);
}

bool IJsIterator::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeBySymbol(ctx, index);
    }

    return true;
}

void IJsIterator::changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) {
    if (_obj) {
        _obj->changeAllProperties(ctx, toAdd, toRemove);
    }
}

bool IJsIterator::hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) {
    return _obj && _obj->hasAnyProperty(ctx, flags);
}

void IJsIterator::preventExtensions(VMContext *ctx) {
    IJsObject::preventExtensions(ctx);

    if (_obj) {
        _obj->preventExtensions(ctx);
    }
}

IJsObject *IJsIterator::clone() {
    assert(0);
    return nullptr;
}

IJsIterator *IJsIterator::getIteratorObject(VMContext *ctx, bool includeProtoProp, bool includeNoneEnumerable) {
    return new JsIteratorOfIterator(ctx, this, includeProtoProp, includeNoneEnumerable);
}

void IJsIterator::markReferIdx(VMRuntime *rt) {
    assert(referIdx == rt->nextReferIdx());

    rt->markReferIdx(__proto__);
    if (_curValue.type > JDT_NUMBER) {
        rt->markReferIdx(_curValue);
    }

    if (_obj) {
        _obj->referIdx = rt->nextReferIdx();
        _obj->markReferIdx(rt);
    }
}

void IJsIterator::_newObject(VMContext *ctx) {
    assert(_obj == nullptr);

    _obj = new JsObject(__proto__);

    if (isPreventedExtensions) {
        _obj->preventExtensions(ctx);
    }
}
