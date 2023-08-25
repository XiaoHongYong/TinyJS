//
//  JsArguments.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/2.
//

#include "JsArguments.hpp"


/**
 * 遍历 Array
 */
class JsArgumentsIterator : public IJsIterator {
public:
    JsArgumentsIterator(VMContext *ctx, JsArguments *args, bool includeProtoProp, bool includeNoneEnumerable) : IJsIterator(includeProtoProp, includeNoneEnumerable), _keyBuf(0)
    {
        _ctx = ctx;
        _args = args;
        _pos = 0;
        _itObj = nullptr;
    }

    ~JsArgumentsIterator() {
        if (_itObj) {
            delete _itObj;
        }
    }

    inline uint32_t _len() { return _args->_args->count; }

    virtual bool nextOf(JsValue &valueOut) override {
        if (_pos >= _len()) {
            return false;
        }

        valueOut = _args->getByIndex(_ctx, _args->self, _pos);
        _pos++;

        return true;
    }

    virtual bool next(StringView *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        if (_pos >= _len()) {
            if (_includeProtoProp) {
                if (_itObj == nullptr) {
                    if (!_args->_obj) return false;
                    _itObj = _args->_obj->getIteratorObject(_ctx, true, _includeNoneEnumerable);
                }
                return _itObj->next(strKeyOut, keyOut, valueOut);
            }

            return false;
        }

        _keyBuf.set(_pos);

        if (strKeyOut) {
            *strKeyOut = _keyBuf.str();
        }

        if (keyOut) {
            *keyOut = _ctx->runtime->pushString(_keyBuf.str());
        }

        if (valueOut) {
            *valueOut = _args->getByIndex(_ctx, _args->self, _pos);
        }

        _pos++;
        return true;
    }

protected:
    VMContext                       *_ctx;
    JsArguments                     *_args;
    uint32_t                        _pos;
    NumberToStringView             _keyBuf;

    IJsIterator                     *_itObj;

};

JsArguments::JsArguments(VMScope *scope, Arguments *args) : IJsObject(jsValueEmpty, JDT_ARGUMENTS) {
    _isOfIterable = true;
    _scope = scope;
    _args = args;
    _argsDescriptors = nullptr;
    _obj = nullptr;
    _length = makeJsValueInt32(args->count).asProperty(JP_WRITABLE);
}

JsArguments::~JsArguments() {
    if (_obj) delete _obj;
    if (_argsDescriptors) delete _argsDescriptors;
}

void JsArguments::setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n < _args->count) {
            return setPropertyByIndex(ctx, (uint32_t)n, descriptor);
        }
    }

    if (name.equal(SS_LENGTH)) {
        // 设置标志调用 setByIndexCallback
        _length = descriptor;
        return;
    }

    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyByName(ctx, name, descriptor);
}

void JsArguments::setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (index >= _args->count) {
        NumberToStringView ss(index);
        setPropertyByName(ctx, ss, descriptor);
        return;
    }

    if (!_argsDescriptors) {
        _argsDescriptors = new VecJsProperties();
        for (uint32_t i = 0; i < _args->count; i++)  {
            _argsDescriptors->push_back(_args->data[i].asProperty());
        }
    }

    _argsDescriptors->at(index) = descriptor;
    if (!descriptor.isGetterSetter()) {
        _args->data[index] = descriptor;
    }
}

void JsArguments::setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyBySymbol(ctx, index, descriptor);
}

JsError JsArguments::setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n < _args->count) {
            return setByIndex(ctx, thiz, uint32_t(n), value);
        }
    }

    if (name.equal(SS_LENGTH)) {
        return setPropertyValue(ctx, &_length, thiz, value);
    }

    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setByName(ctx, thiz, name, value);
}

JsError JsArguments::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (index >= _args->count) {
        NumberToStringView name(index);
        return setByName(ctx, thiz, name, value);
    }

    if (_argsDescriptors) {
        auto prop = &_argsDescriptors->at(index);
        auto ret = setPropertyValue(ctx, prop, thiz, value);
        if (ret == JE_OK) {
            // 同步修改到 _args 中
            _args->data[index] = value;
        }
        return ret;
    } else {
        (*_args)[index] = value;
        return JE_OK;
    }
}

JsError JsArguments::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue JsArguments::increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && index < _args->count) {
            return increaseByIndex(ctx, thiz, uint32_t(index), n, isPost);
        }
    }

    if (name.equal(SS_LENGTH)) {
        return increasePropertyValue(ctx, &_length, thiz, n, isPost);
    }

    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseByName(ctx, thiz, name, n, isPost);
}

JsValue JsArguments::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (index >= _args->count) {
        NumberToStringView name(index);
        return increaseByName(ctx, thiz, name, n, isPost);
    }

    if (_argsDescriptors) {
        auto prop = &_argsDescriptors->at(index);
        auto ret = increasePropertyValue(ctx, prop, thiz, n, isPost);
        if (!prop->isGetterSetter() && prop->isWritable()) {
            // 同步修改到 _args 中
            _args->data[index] = *prop;
        }
        return ret;
    } else {
        auto org = increase(ctx, (*_args)[index], n);
        return isPost ? org : (*_args)[index];
    }
}

JsValue JsArguments::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsValue *JsArguments::getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n < _args->count) {
            return getRawByIndex(ctx, uint32_t(n), includeProtoProp);
        }
    }

    if (name.equal(SS_LENGTH)) {
        return &_length;
    }

    if (_obj) {
        return _obj->getRawByName(ctx, name, includeProtoProp);
    }

    if (includeProtoProp) {
        // 缺省的 Object.prototype
        return ctx->runtime->objPrototypeObject()->getRawByName(ctx, name, includeProtoProp);
    }

    return nullptr;
}

JsValue *JsArguments::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (index >= _args->count) {
        NumberToStringView name(index);
        return getRawByName(ctx, name, includeProtoProp);
    }

    if (_argsDescriptors) {
        return &_argsDescriptors->at(index);
    }

    // 获取值
    static JsValue prop;
    prop = _args->data[index].asProperty();
    return &prop;
}

JsValue *JsArguments::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsArguments::removeByName(VMContext *ctx, const StringView &name) {
    if (_obj) {
        return _obj->removeByName(ctx, name);
    }

    return true;
}

bool JsArguments::removeByIndex(VMContext *ctx, uint32_t index) {
    if (index >= _args->count) {
        NumberToStringView name(index);
        return removeByName(ctx, name);
    }

    if (_argsDescriptors) {
        auto prop = &_argsDescriptors->at(index);
        if (prop->isConfigurable()) {
            *prop = jsValuePropertyDefault;
        }
    } else {
        (*_args)[index] = jsValueUndefined;
    }

    if (_obj) {
        _obj->removeByIndex(ctx, index);
    }

    return true;
}

bool JsArguments::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        _obj->removeBySymbol(ctx, index);
    }

    return true;
}

void JsArguments::changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) {
    if (!_argsDescriptors) {
        _argsDescriptors = new VecJsProperties();
        for (uint32_t i = 0; i < _args->count; i++)  {
            _argsDescriptors->push_back(_args->data[i].asProperty());
        }
    }

    for (auto &item : *_argsDescriptors) {
        item.changeProperty(toAdd, toRemove);
    }

    if (_obj) {
        _obj->changeAllProperties(ctx, toAdd, toRemove);
    }
}

bool JsArguments::hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) {
    if (_argsDescriptors) {
        for (auto &v : *_argsDescriptors) {
            if (v.isPropertyAny(flags)) {
                return true;
            }
        }
    } else if (_args->count > 0) {
        return true;
    }

    return _obj && _obj->hasAnyProperty(ctx, flags);
}

void JsArguments::preventExtensions(VMContext *ctx) {
    IJsObject::preventExtensions(ctx);

    if (_obj) {
        _obj->preventExtensions(ctx);
    }
}

IJsObject *JsArguments::clone() {
    assert(0 && "NOT supported.");

    return nullptr;
}

IJsIterator *JsArguments::getIteratorObject(VMContext *ctx, bool includeProtoProp, bool includeNoneEnumerable) {
    return new JsArgumentsIterator(ctx, this, includeProtoProp, includeNoneEnumerable);
}

void JsArguments::markReferIdx(VMRuntime *rt) {
    assert(referIdx == rt->nextReferIdx());

    for (uint32_t i = 0; i < _args->count; i++) {
        auto val = _args->data[i];
        rt->markReferIdx(val);
    }

    if (_argsDescriptors) {
        for (auto &prop : *_argsDescriptors) {
            rt->markReferIdx(prop);
        }
    }

    if (_obj) {
        _obj->referIdx = rt->nextReferIdx();
        _obj->markReferIdx(rt);
    }

    rt->markReferIdx(_length);
}

void JsArguments::_newObject(VMContext *ctx) {
    assert(_obj == nullptr);
    _obj = new JsObject();

    if (isPreventedExtensions) {
        _obj->preventExtensions(ctx);
    }
}
