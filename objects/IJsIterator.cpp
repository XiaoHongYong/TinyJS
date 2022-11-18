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
    JsIteratorOfIterator(VMContext *ctx, IJsIterator *it, bool includeProtoProp) {
        _ctx = ctx;
        _it = it;
        _itObj = nullptr;
        _includeProtoProp = includeProtoProp;
    }

    ~JsIteratorOfIterator() {
        if (_itObj) {
            delete _itObj;
        }
    }

    virtual bool nextOf(JsValue &valueOut) override {
        return _it->nextOf(valueOut);
    }

    virtual bool next(SizedString *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        if (_itObj == nullptr) {
            if (_it->_obj) {
                _itObj = _it->_obj->getIteratorObject(_ctx, _includeProtoProp);
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
    bool                            _includeProtoProp;

};


void IJsIterator::definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->definePropertyByName(ctx, name, descriptor);
}

void IJsIterator::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    NumberToSizedString name(index);
    definePropertyByName(ctx, name, descriptor);
}

void IJsIterator::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor);
}

JsError IJsIterator::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }

    return _obj->setByName(ctx, thiz, name, value);
}

JsError IJsIterator::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    NumberToSizedString name(index);
    return setByName(ctx, thiz, name, value);
}

JsError IJsIterator::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue IJsIterator::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }

    return _obj->increaseByName(ctx, thiz, name, n, isPost);
}

JsValue IJsIterator::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    NumberToSizedString name(index);
    return increaseByName(ctx, thiz, name, n, isPost);
}

JsValue IJsIterator::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsProperty *IJsIterator::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
    }

    if (name.equal(SS___PROTO__)) {
        return &__proto__;
    }

    if (includeProtoProp) {
        // 查找 prototye 的属性
        auto &proto = __proto__.value;
        JsNativeFunction funcGetter = nullptr;
        if (proto.type == JDT_NOT_INITIALIZED) {
            // 缺省的 Object.prototype
            return ctx->runtime->objPrototypeObject->getRawByName(ctx, name, funcGetter, true);
        } else if (proto.type >= JDT_OBJECT) {
            auto obj = ctx->runtime->getObject(proto);
            assert(obj);
            return obj->getRawByName(ctx, name, funcGetter, true);
        }
    }

    return nullptr;
}

JsProperty *IJsIterator::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawByIndex(ctx, index, includeProtoProp);
    }

    return nullptr;
}

JsProperty *IJsIterator::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool IJsIterator::removeByName(VMContext *ctx, const SizedString &name) {
    if (_obj) {
        return _obj->removeByName(ctx, name);
    }

    return true;
}

bool IJsIterator::removeByIndex(VMContext *ctx, uint32_t index) {
    NumberToSizedString name(index);
    return removeByName(ctx, name);
}

bool IJsIterator::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeBySymbol(ctx, index);
    }

    return true;
}

void IJsIterator::changeAllProperties(VMContext *ctx, int8_t configurable, int8_t writable) {
    if (_obj) {
        _obj->changeAllProperties(ctx, configurable, writable);
    }
}

bool IJsIterator::hasAnyProperty(VMContext *ctx, bool configurable, bool writable) {
    return _obj && _obj->hasAnyProperty(ctx, configurable, writable);
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

IJsIterator *IJsIterator::getIteratorObject(VMContext *ctx, bool includeProtoProp) {
    return new JsIteratorOfIterator(ctx, this, includeProtoProp);
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

    _obj = new JsObject(__proto__.value);

    if (isPreventedExtensions) {
        _obj->preventExtensions(ctx);
    }
}
