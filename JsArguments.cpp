//
//  JsArguments.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/2.
//

#include "JsArguments.hpp"


JsArguments::JsArguments(VMScope *scope, Arguments *args) {
    _scope = scope;
    _args = args;
    _argDescriptors = nullptr;
    _setters = nullptr;
    _obj = nullptr;
    _length = JsValue(JDT_INT32, args->count);
}

JsArguments::~JsArguments() {
    if (_obj) delete _obj;
    if (_argDescriptors) delete _argDescriptors;
    if (_setters) delete _setters;
}

void JsArguments::definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor, const JsValue &setter) {
    if (prop.len > 0 && isDigit(prop.data[0])) {
        bool successful = false;
        auto n = prop.atoi(successful);
        if (successful && n < _args->count) {
            return definePropertyByIndex(ctx, (uint32_t)n, descriptor, setter);
        }
    }

    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyByName(ctx, prop, descriptor, setter);
}

void JsArguments::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) {
    if (index >= _args->count) {
        NumberToSizedString ss(index);
        definePropertyByName(ctx, ss, descriptor, setter);
        return;
    }

    if (!_argDescriptors) {
        _argDescriptors = new VecJsProperties;
        _argDescriptors->resize(_args->count);
    }
    _argDescriptors->at(index) = descriptor;

    if (setter.type > JDT_OBJECT) {
        if (!_setters) {
            _setters = new VecJsValues;
            _setters->resize(_args->count);
        }

        _setters->at(index) = setter;
    }
}

void JsArguments::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor, setter);
}

bool JsArguments::getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut, JsValue &setterOut) {
    if (prop.len > 0 && isDigit(prop.data[0])) {
        bool successful = false;
        auto n = prop.atoi(successful);
        if (successful && n < _args->count) {
            return getOwnPropertyDescriptorByIndex(ctx, (uint32_t)n, descriptorOut, setterOut);
        }
    }

    if (_obj) {
        return _obj->getOwnPropertyDescriptorByName(ctx, prop, descriptorOut, setterOut);
    }

    return false;
}

bool JsArguments::getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) {
    descriptorOut = JsProperty(JsUndefinedValue);
    setterOut = JsUndefinedValue;

    if (index >= _args->count) {
        NumberToSizedString ss(index);
        return getOwnPropertyDescriptorByName(ctx, ss, descriptorOut, setterOut);
    }

    if (!_argDescriptors) {
        return false;
    }
    descriptorOut = _argDescriptors->at(index);

    if (_setters) {
        setterOut = _setters->at(index);
    }

    return true;
}

bool JsArguments::getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) {
    if (_obj) {
        return _obj->getOwnPropertyDescriptorBySymbol(ctx, index, descriptorOut, setterOut);
    }

    return false;
}

JsValue JsArguments::getSetterByName(VMContext *ctx, const SizedString &prop) {
    assert(0);
    return JsUndefinedValue;
}

JsValue JsArguments::getSetterByIndex(VMContext *ctx, uint32_t index) {
    assert(0);
    return JsUndefinedValue;
}

JsValue JsArguments::getSetterBySymbol(VMContext *ctx, uint32_t index) {
    assert(0);
    return JsUndefinedValue;
}

JsValue JsArguments::getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop) {
    if (prop.len > 0 && isDigit(prop.data[0])) {
        bool successful = false;
        auto n = prop.atoi(successful);
        if (successful && n < _args->count) {
            return getByIndex(ctx, thiz, (uint32_t)n);
        }
    }

    if (prop.equal(SS_LENGTH)) {
        return _length;
    }

    if (_obj) {
        return _obj->getByName(ctx, thiz, prop);
    }

    return JsUndefinedValue;
}

JsValue JsArguments::getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    if (index >= _args->count) {
        NumberToSizedString ss(index);
        return getByName(ctx, thiz, ss);
    }

    if (_argDescriptors) {
        auto &propValue = _argDescriptors->at(index);
        if (propValue.isGetter) {
            ctx->vm->callMember(ctx, thiz, propValue.value, Arguments());
            auto ret = ctx->stack.back();
            ctx->stack.pop_back();
            return ret;
        }
    }

    return (*_args)[index];
}

JsValue JsArguments::getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    if (_obj) {
        return _obj->getBySymbol(ctx, thiz, index);
    }

    return JsUndefinedValue;
}

void JsArguments::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) {
    if (prop.len > 0 && isDigit(prop.data[0])) {
        bool successful = false;
        auto n = prop.atoi(successful);
        if (successful && n < _args->count) {
            setByIndex(ctx, thiz, uint32_t(n), value);
            return;
        }
    }

    if (prop.equal(SS_LENGTH)) {
        _length = value;
        return;
    }

    if (!_obj) {
        _newObject();
    }
    _obj->setByName(ctx, thiz, prop, value);
}

void JsArguments::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (index >= _args->count) {
        NumberToSizedString ss(index);
        return setByName(ctx, thiz, ss, value);
    }

    if (_setters) {
        // 调用 setter 函数
        auto setter = _setters->at(index);
        if (setter.type > JDT_OBJECT) {
            ArgumentsX args(value);
            ctx->vm->callMember(ctx, thiz, setter, args);
            return;
        }
    }

    (*_args)[index] = value;
}

void JsArguments::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject();
    }
    _obj->setBySymbol(ctx, thiz, index, value);
}

bool JsArguments::removeByName(VMContext *ctx, const SizedString &prop) {
    if (_obj) {
        return _obj->removeByName(ctx, prop);
    }

    return true;
}

bool JsArguments::removeByIndex(VMContext *ctx, uint32_t index) {
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

void JsArguments::_newObject() {
    assert(_obj == nullptr);
    _obj = new JsObject();
}
