//
//  JsObjectFunction.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#include "JsObjectFunction.hpp"


JsObjectFunction::JsObjectFunction(const VecVMStackScopes &stackScopes, Function *function) : stackScopes(stackScopes), function(function), __proto__(jsValuePrototypeFunction, false, false, false, true) {
    type = JDT_FUNCTION;

    // isGSetter, isConfigurable, isEnumerable, isWritable
    _prototype = JsProperty(jsValueNotInitialized, false, false, false, true);
    _name = JsProperty(jsValueNotInitialized, false, false, false, true);
    _length = JsProperty(JsValue(JDT_INT32, 0), false, false, false, true);
    _caller = JsProperty(jsValueNull, false, false, false, false);
    _arguments = JsProperty(jsValueNull, false, false, false, false);

    _obj = nullptr;
}

JsObjectFunction::~JsObjectFunction() {
    if (_obj) {
        delete _obj;
    }
}

void JsObjectFunction::definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) {
    if (name.equal(SS_PROTOTYPE)) {
        defineNameProperty(ctx, &_prototype, descriptor, name);
    } else if (name.equal(SS_NAME)) {
        defineNameProperty(ctx, &_name, descriptor, name);
    } else if (name.equal(SS_LENGTH)) {
        defineNameProperty(ctx, &_length, descriptor, name);
    } else if (name.equal(SS_CALLER)) {
        defineNameProperty(ctx, &_caller, descriptor, name);
    } else if (name.equal(SS_ARGUMENTS)) {
        defineNameProperty(ctx, &_arguments, descriptor, name);
    } else {
        if (!_obj) {
            _newObject();
        }
        _obj->definePropertyByName(ctx, name, descriptor);
    }
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

void JsObjectFunction::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    if (name.equal(SS_PROTOTYPE)) {
        set(ctx, &_prototype, thiz, value);
    } else if (name.equal(SS_NAME)) {
        set(ctx, &_name, thiz, value);
    } else if (name.equal(SS_LENGTH)) {
        set(ctx, &_length, thiz, value);
    } else if (name.equal(SS_CALLER)) {
        set(ctx, &_caller, thiz, value);
    } else if (name.equal(SS_ARGUMENTS)) {
        set(ctx, &_arguments, thiz, value);
    } else {
        if (!_obj) {
            _newObject();
        }

        _obj->setByName(ctx, thiz, name, value);
    }
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

JsValue JsObjectFunction::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    if (name.equal(SS_PROTOTYPE)) {
        return increase(ctx, &_prototype, thiz, n, isPost);
    } else if (name.equal(SS_NAME)) {
        return increase(ctx, &_name, thiz, n, isPost);
    } else if (name.equal(SS_LENGTH)) {
        return increase(ctx, &_length, thiz, n, isPost);
    } else if (name.equal(SS_CALLER)) {
        return increase(ctx, &_caller, thiz, n, isPost);
    } else if (name.equal(SS_ARGUMENTS)) {
        return increase(ctx, &_arguments, thiz, n, isPost);
    } else {
        if (!_obj) {
            _newObject();
        }

        return _obj->increaseByName(ctx, thiz, name, n, isPost);
    }
}

JsValue JsObjectFunction::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject();
    }
    return _obj->increaseByIndex(ctx, thiz, index, n, isPost);
}

JsValue JsObjectFunction::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject();
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsProperty *JsObjectFunction::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    funcGetterOut = nullptr;

    if (name.equal(SS_PROTOTYPE)) {
        if (_prototype.value.type == JDT_NOT_INITIALIZED) {
            // 为了节省内存分配，延迟初始化 _prototype 属性
            auto proto = new JsObject();
            _prototype.value = ctx->runtime->pushObjValue(JDT_OBJECT, proto);
            proto->setByName(ctx, _prototype.value, SS_CONSTRUCTOR, self);
        }
        return &_prototype;
    } else if (name.equal(SS_NAME)) {
        if (!_name.value.isValid()) {
            _name.value = ctx->runtime->pushString(function->name);
        }
        return &_name;
    } else if (name.equal(SS_LENGTH)) {
        return &_length;
    } else if (name.equal(SS_CALLER)) {
        return &_caller;
    } else if (name.equal(SS_ARGUMENTS)) {
        return &_arguments;
    }

    if (_obj) {
        return _obj->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
    }

    if (name.equal(SS___PROTO__)) {
        return &__proto__;
    }

    if (includeProtoProp) {
        // 查找 __proto__ 的属性
        if (__proto__.value.equal(jsValuePrototypeFunction)) {
            // 缺省的 Function.prototype
            return ctx->runtime->objPrototypeFunction->getRawByName(ctx, name, funcGetterOut, true);
        } else if (__proto__.value.type >= JDT_OBJECT) {
            auto obj = ctx->runtime->getObject(__proto__.value);
            assert(obj);
            return obj->getRawByName(ctx, name, funcGetterOut, true);
        }
    }

    return nullptr;
}

JsProperty *JsObjectFunction::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawByIndex(ctx, index, includeProtoProp);
    }

    return nullptr;
}

JsProperty *JsObjectFunction::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsObjectFunction::removeByName(VMContext *ctx, const SizedString &name) {
    if (name.equal(SS_PROTOTYPE)) {
        return false;
    } else if (name.equal(SS_CALLER)) {
        return false;
    } else if (name.equal(SS_ARGUMENTS)) {
        return false;
    }

    if (_obj) {
        return _obj->removeByName(ctx, name);
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

    if (_obj) { obj->_obj = (JsObject *)_obj->clone(); }

    return obj;
}

IJsIterator *JsObjectFunction::getIteratorObject(VMContext *ctx) {
    if (_obj) {
        return _obj->getIteratorObject(ctx);
    }

    return new EmptyJsIterator();
}

void JsObjectFunction::markReferIdx(VMRuntime *rt) {
    assert(referIdx == rt->nextReferIdx());

    for (auto scope : stackScopes) {
        rt->markReferIdx(scope);
    }

    assert(function != nullptr);
    rt->markReferIdx(function->resourcePool);

    rt->markReferIdx(_prototype);
    rt->markReferIdx(_name);
    rt->markReferIdx(_length);
    rt->markReferIdx(_caller);
    rt->markReferIdx(_arguments);
    rt->markReferIdx(__proto__);

    if (_obj) {
        _obj->referIdx = rt->nextReferIdx();
        _obj->markReferIdx(rt);
    }
}

void JsObjectFunction::_newObject() {
    assert(_obj == nullptr);

    _obj = new JsObject(__proto__.value);
}
