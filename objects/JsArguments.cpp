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
    JsArgumentsIterator(VMContext *ctx, JsArguments *args) : _keyBuf(0) {
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

    virtual bool nextKey(SizedString &keyOut) override {
        if (_pos >= _len()) {
            if (_itObj == nullptr) {
                if (!_args->_obj) return false;
                _itObj = _args->_obj->getIteratorObject(_ctx);
            }
            return _itObj->nextKey(keyOut);
        }

        _keyBuf.set(_pos);
        keyOut = _keyBuf.str();
        _pos++;
        return true;
    }

    virtual bool nextKey(JsValue &keyOut) override {
        if (_pos >= _len()) {
            if (_itObj == nullptr) {
                if (!_args->_obj) return false;
                _itObj = _args->_obj->getIteratorObject(_ctx);
            }
            return _itObj->nextKey(keyOut);
        }

        _keyBuf.set(_pos);
        keyOut = _ctx->runtime->pushString(_keyBuf.str());
        _pos++;
        return true;
    }

    virtual bool nextValue(JsValue &valueOut) override {
        if (_pos >= _len()) {
            return false;
        }

        valueOut = _args->_args->data[_pos];

        _pos++;
        return true;
    }

    virtual bool next(JsValue &keyOut, JsValue &valueOut) override {
        if (_pos >= _len()) {
            if (_itObj == nullptr) {
                if (!_args->_obj) return false;
                _itObj = _args->_obj->getIteratorObject(_ctx);
            }
            return _itObj->next(keyOut, valueOut);
        }

        _keyBuf.set(_pos);
        keyOut = _ctx->runtime->pushString(_keyBuf.str());
        valueOut = _args->_args->data[_pos];

        _pos++;
        return true;
    }

    virtual bool next(SizedString &keyOut, JsValue &valueOut) override {
        if (_pos >= _len()) {
            if (_itObj == nullptr) {
                if (!_args->_obj) return false;
                _itObj = _args->_obj->getIteratorObject(_ctx);
            }
            return _itObj->next(keyOut, valueOut);
        }

        _keyBuf.set(_pos);
        keyOut = _keyBuf.str();
        valueOut = _args->_args->data[_pos];

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
        _newObject();
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
        _newObject();
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor);
}

void JsArguments::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto n = name.atoi(successful);
        if (successful && n < _args->count) {
            setByIndex(ctx, thiz, uint32_t(n), value);
            return;
        }
    }

    if (name.equal(SS_LENGTH)) {
        set(ctx, &_length, thiz, value);
        return;
    }

    if (!_obj) {
        _newObject();
    }
    _obj->setByName(ctx, thiz, name, value);
}

void JsArguments::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (index >= _args->count) {
        NumberToSizedString name(index);
        return setByName(ctx, thiz, name, value);
    }

    if (_argsDescriptors) {
        auto prop = &_argsDescriptors->at(index);
        set(ctx, prop, thiz, value);
        if (!prop->isGSetter && prop->isWritable) {
            // 同步修改到 _args 中
            _args->data[index] = value;
        }
    } else {
        (*_args)[index] = value;
    }
}

void JsArguments::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject();
    }
    _obj->setBySymbol(ctx, thiz, index, value);
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
        _newObject();
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
        _newObject();
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

IJsObject *JsArguments::clone() {
    assert(0 && "NOT supported.");

    return nullptr;
}

IJsIterator *JsArguments::getIteratorObject(VMContext *ctx) {
    return new JsArgumentsIterator(ctx, this);
}

void JsArguments::_newObject() {
    assert(_obj == nullptr);
    _obj = new JsObject();
}
