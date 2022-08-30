//
//  JsObjectFunction.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#include "JsObjectFunction.hpp"


JsObjectFunction::JsObjectFunction(const VecVMStackScopes &stackScopes, Function *function) : stackScopes(stackScopes), function(function) {
    type = JDT_FUNCTION;
    __proto__ = jsValueNotInitialized;
    _prototype = jsValueNotInitialized;
    _obj__Proto__ = nullptr;
    _obj = nullptr;
}

JsObjectFunction::~JsObjectFunction() {
    if (_obj) {
        delete _obj;
    }
}

void JsObjectFunction::definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor) {
    if (prop.equal(SS_PROTOTYPE) || prop.equal(SS_CALLER) || prop.equal(SS_ARGUMENTS)) {
        ctx->throwException(PE_TYPE_ERROR, "Cannot redefine property: %.*s", (int)prop.len, prop.data);
        return;
    }

    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyByName(ctx, prop, descriptor);
}

void JsObjectFunction::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyByIndex(ctx, index, descriptor);
}

void JsObjectFunction::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor);
}

bool JsObjectFunction::getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut) {
    if (_obj && _obj->getOwnPropertyDescriptorByName(ctx, prop, descriptorOut)) {
        return true;
    }

    return false;
}

bool JsObjectFunction::getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) {
    if (_obj) {
        return _obj->getOwnPropertyDescriptorByIndex(ctx, index, descriptorOut);
    }

    return false;
}

bool JsObjectFunction::getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) {
    if (_obj) {
        return _obj->getOwnPropertyDescriptorBySymbol(ctx, index, descriptorOut);
    }

    return false;
}

JsProperty *JsObjectFunction::getRawByName(VMContext *ctx, const SizedString &prop, bool &isSelfPropOut) {
    assert(0);
    return nullptr;
}

JsProperty *JsObjectFunction::getRawByIndex(VMContext *ctx, uint32_t index, bool &isSelfPropOut) {
    assert(0);
    return nullptr;
}

JsProperty *JsObjectFunction::getRawBySymbol(VMContext *ctx, uint32_t index, bool &isSelfPropOut) {
    assert(0);
    return nullptr;
}

JsValue JsObjectFunction::getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &defVal) {
    if (prop.equal(SS_PROTOTYPE)) {
        if (_prototype.type == JDT_NOT_INITIALIZED) {
            // 为了节省内存分配，延迟初始化 _prototype 属性
            auto proto = new JsObject();
            _prototype = ctx->runtime->pushObjValue(JDT_OBJECT, proto);
            proto->setByName(ctx, _prototype, SS_CONSTRUCTOR, self);
        }
        return _prototype;
    } else if (prop.equal(SS_NAME)) {
        return ctx->runtime->pushString(function->name);
    } else if (prop.equal(SS_LENGTH)) {
        return JsValue(JDT_INT32, 0);
    } else if (prop.equal(SS_CALLER)) {
        return jsValueNull;
    } else if (prop.equal(SS_ARGUMENTS)) {
        return jsValueNull;
    }

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

    return defVal;
}

JsValue JsObjectFunction::getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal) {
    if (_obj) {
        return _obj->getByIndex(ctx, thiz, index, defVal);
    }

    return defVal;
}

JsValue JsObjectFunction::getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal) {
    if (_obj) {
        return _obj->getBySymbol(ctx, thiz, index, defVal);
    }

    return defVal;
}

void JsObjectFunction::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) {
    // function 的这些属性是不能被修改的.
    if (prop.equal(SS_PROTOTYPE)) {
        _prototype = value;
        return;
    } else if (prop.equal(SS_NAME)) {
        return;
    } else if (prop.equal(SS_LENGTH)) {
        return;
    } else if (prop.equal(SS_LENGTH)) {
        return;
    } else if (prop.equal(SS_CALLER)) {
        return;
    } else if (prop.equal(SS_ARGUMENTS)) {
        return;
    }

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
    if (prop.equal(SS_PROTOTYPE)) {
        return false;
    } else if (prop.equal(SS_CALLER)) {
        return false;
    } else if (prop.equal(SS_ARGUMENTS)) {
        return false;
    }

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

IJsIterator *JsObjectFunction::getIteratorObject(VMContext *ctx) {
    // auto it = new JsObjectIterator(ctx, this);
    // return it;
    return nullptr;
}

void JsObjectFunction::_newObject() {
    assert(_obj == nullptr);

    _obj = new JsObject();
}
