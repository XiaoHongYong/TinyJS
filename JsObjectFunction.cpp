//
//  JsObjectFunction.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#include "JsObjectFunction.hpp"


JsObjectFunction::JsObjectFunction(const VecVMStackScopes &stackScopes, Function *function) : stackScopes(stackScopes), function(function) {
    type = JDT_FUNCTION;
    __proto__ = JsNotInitializedValue;
    _obj__Proto__ = nullptr;
    _obj = nullptr;
}

JsObjectFunction::~JsObjectFunction() {
    if (_obj) {
        delete _obj;
    }
}

void JsObjectFunction::definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor, const JsValue &setter) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyByName(ctx, prop, descriptor, setter);
}

void JsObjectFunction::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyByIndex(ctx, index, descriptor, setter);
}

void JsObjectFunction::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor, setter);
}

bool JsObjectFunction::getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut, JsValue &setterOut) {
    if (_obj && _obj->getOwnPropertyDescriptorByName(ctx, prop, descriptorOut, setterOut)) {
        return true;
    }

    return false;
}

bool JsObjectFunction::getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) {
    if (_obj) {
        return _obj->getOwnPropertyDescriptorByIndex(ctx, index, descriptorOut, setterOut);
    }

    return false;
}

bool JsObjectFunction::getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) {
    if (_obj) {
        return _obj->getOwnPropertyDescriptorBySymbol(ctx, index, descriptorOut, setterOut);
    }

    return false;
}

JsValue JsObjectFunction::getSetterByName(VMContext *ctx, const SizedString &prop) {
    assert(0);
    return JsUndefinedValue;
}

JsValue JsObjectFunction::getSetterByIndex(VMContext *ctx, uint32_t index) {
    assert(0);
    return JsUndefinedValue;
}

JsValue JsObjectFunction::getSetterBySymbol(VMContext *ctx, uint32_t index) {
    assert(0);
    return JsUndefinedValue;
}

JsValue JsObjectFunction::getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop) {
    if (_obj) {
        return _obj->getByName(ctx, thiz, prop);
    }

    if (__proto__.type == JDT_NOT_INITIALIZED) {
        // 缺省的 Function.prototype
        return ctx->runtime->objPrototypeFunction->getByName(ctx, thiz, prop);
    } else if (__proto__.type >= JDT_OBJECT) {
        auto obj = ctx->runtime->getObject(__proto__);
        assert(obj);
        return obj->getByName(ctx, thiz, prop);
    }

    return JsUndefinedValue;
}

JsValue JsObjectFunction::getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    if (_obj) {
        return _obj->getByIndex(ctx, thiz, index);
    }

    return JsUndefinedValue;
}

JsValue JsObjectFunction::getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    if (_obj) {
        return _obj->getBySymbol(ctx, thiz, index);
    }

    return JsUndefinedValue;
}

void JsObjectFunction::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) {
    if (!_obj) {
        _newObject();
    }

    _obj->setByName(ctx, thiz, prop, value);
}

void JsObjectFunction::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject();
    }
    _obj->setByIndex(ctx, thiz, index, value);
}

void JsObjectFunction::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject();
    }
    _obj->setBySymbol(ctx, thiz, index, value);
}

bool JsObjectFunction::removeByName(VMContext *ctx, const SizedString &prop) {
    if (_obj) {
        return _obj->removeByName(ctx, prop);
    }

    return true;
}

bool JsObjectFunction::removeByIndex(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeByIndex(ctx, index);
    }

    return true;
}

bool JsObjectFunction::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeBySymbol(ctx, index);
    }

    return true;
}

IJsObject *JsObjectFunction::clone() {
    auto obj = new JsObjectFunction(stackScopes, function);

    obj->__proto__ = __proto__;
    obj->_obj__Proto__ = _obj__Proto__;

    if (_obj) { obj->_obj = (JsObject *)_obj->clone(); }

    return obj;
}

void JsObjectFunction::_newObject() {
    assert(_obj == nullptr);

    _obj = new JsObject();
}
