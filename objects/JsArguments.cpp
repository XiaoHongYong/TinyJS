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

    virtual bool next(SizedString *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
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
    NumberToSizedString             _keyBuf;

    IJsIterator                     *_itObj;

};

JsArguments::JsArguments(VMScope *scope, Arguments *args) {
    type = JDT_ARGUMENTS;

    _isOfIterable = true;
    _scope = scope;
    _args = args;
    _argsDescriptors = nullptr;
    _obj = nullptr;
    _length = JsValue(JDT_INT32, args->count);
}

JsArguments::~JsArguments() {
    if (_obj) delete _obj;
    if (_argsDescriptors) delete _argsDescriptors;
}

void JsArguments::definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n < _args->count) {
            return definePropertyByIndex(ctx, (uint32_t)n, descriptor);
        }
    }

    if (name.equal(SS_LENGTH)) {
        // 设置标志调用 setByIndexCallback
        defineNameProperty(ctx, &_length, descriptor, name);
        return;
    }

    if (!_obj) {
        _newObject(ctx);
    }

    _obj->definePropertyByName(ctx, name, descriptor);
}

void JsArguments::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (index >= _args->count) {
        NumberToSizedString ss(index);
        definePropertyByName(ctx, ss, descriptor);
        return;
    }

    if (!_argsDescriptors) {
        _argsDescriptors = new VecJsProperties();
        for (uint32_t i = 0; i < _args->count; i++)  {
            _argsDescriptors->push_back(_args->data[i]);
        }
    }

    auto &prop = _argsDescriptors->at(index);
    defineIndexProperty(ctx, &prop, descriptor, index);

    if (!prop.isGSetter) {
        _args->data[index] = prop.value;
    }
}

void JsArguments::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor);
}

JsError JsArguments::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n < _args->count) {
            return setByIndex(ctx, thiz, uint32_t(n), value);
        }
    }

    if (name.equal(SS_LENGTH)) {
        return set(ctx, &_length, thiz, value);
    }

    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setByName(ctx, thiz, name, value);
}

JsError JsArguments::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (index >= _args->count) {
        NumberToSizedString name(index);
        return setByName(ctx, thiz, name, value);
    }

    if (_argsDescriptors) {
        auto prop = &_argsDescriptors->at(index);
        auto ret = set(ctx, prop, thiz, value);
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

JsValue JsArguments::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && index < _args->count) {
            return increaseByIndex(ctx, thiz, uint32_t(index), n, isPost);
        }
    }

    if (name.equal(SS_LENGTH)) {
        return increase(ctx, &_length, thiz, n, isPost);
    }

    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseByName(ctx, thiz, name, n, isPost);
}

JsValue JsArguments::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (index >= _args->count) {
        NumberToSizedString name(index);
        return increaseByName(ctx, thiz, name, n, isPost);
    }

    if (_argsDescriptors) {
        auto prop = &_argsDescriptors->at(index);
        auto ret = increase(ctx, prop, thiz, n, isPost);
        if (!prop->isGSetter && prop->isWritable) {
            // 同步修改到 _args 中
            _args->data[index] = prop->value;
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

JsProperty *JsArguments::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    funcGetterOut = nullptr;

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
        return _obj->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
    }

    if (includeProtoProp) {
        // 缺省的 Object.prototype
        return ctx->runtime->objPrototypeObject->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
    }

    return nullptr;
}

JsProperty *JsArguments::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (index >= _args->count) {
        NumberToSizedString name(index);
        JsNativeFunction funcGetter;
        return getRawByName(ctx, name, funcGetter, includeProtoProp);
    }

    if (_argsDescriptors) {
        return &_argsDescriptors->at(index);
    }

    // 获取值
    static JsProperty prop;
    prop.value = _args->data[index];
    return &prop;
}

JsProperty *JsArguments::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsArguments::removeByName(VMContext *ctx, const SizedString &name) {
    if (_obj) {
        return _obj->removeByName(ctx, name);
    }

    return true;
}

bool JsArguments::removeByIndex(VMContext *ctx, uint32_t index) {
    if (index >= _args->count) {
        NumberToSizedString name(index);
        return removeByName(ctx, name);
    }

    if (_argsDescriptors) {
        auto prop = &_argsDescriptors->at(index);
        if (prop->isConfigurable) {
            prop->value = jsValueUndefined;
            prop->isGSetter = false;
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

void JsArguments::changeAllProperties(VMContext *ctx, int8_t configurable, int8_t writable) {
    if (!_argsDescriptors) {
        _argsDescriptors = new VecJsProperties();
        for (uint32_t i = 0; i < _args->count; i++)  {
            _argsDescriptors->push_back(_args->data[i]);
        }
    }

    for (auto &item : *_argsDescriptors) {
        item.changeProperty(configurable, writable);
    }

    if (_obj) {
        _obj->changeAllProperties(ctx, configurable, writable);
    }
}

bool JsArguments::hasAnyProperty(VMContext *ctx, bool configurable, bool writable) {
    if (!_argsDescriptors) {
        return true;
    }

    return _obj && _obj->hasAnyProperty(ctx, configurable, writable);
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
