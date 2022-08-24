//
//  JsLibObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/2.
//

#include "JsLibObject.hpp"


class JsLibFunctionLessCmp {
public:
    bool operator ()(const JsLibProperty &a, const JsLibProperty &b) {
        return a.name.cmp(b.name) < 0;
    }

    bool operator ()(const JsLibProperty &a, const SizedString &b) {
        return a.name.cmp(b) < 0;
    }

    bool operator ()(const SizedString &a, const JsLibProperty &b) {
        return a.cmp(b.name) < 0;
    }

};

JsLibObject::JsLibObject(VMRuntimeCommon *rt, JsLibProperty *libProps, int countProps, JsNativeFunction function) : _libProps(libProps), _libPropsEnd(libProps + countProps), _function(function) {
    type = JDT_LIB_OBJECT;
    _obj = nullptr;
    _modified = false;

    for (auto p = _libProps; p != _libPropsEnd; p++) {
        if (p->function) {
            auto idx = rt->pushNativeFunction(p->function);
            p->value = JsValue(JDT_NATIVE_FUNCTION, idx);
        } else if (p->strValue) {
            p->value = rt->pushStringValue(p->strValue);
        }
    }

    std::sort(_libProps, _libPropsEnd, JsLibFunctionLessCmp());
}

JsLibObject::JsLibObject(JsLibObject *from) {
    type = JDT_LIB_OBJECT;

    assert(!from->_modified);
    _function = from->_function;
    _obj = from->_obj;
    _libProps = from->_libProps;
    _libPropsEnd = from->_libPropsEnd;
    _modified = false;
}

JsLibObject::~JsLibObject() {
    if (_modified) {
        delete [] _libProps;
    }

    if (_obj) {
        delete _obj;
    }
}

void JsLibObject::definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor, const JsValue &setter) {
    auto first = std::lower_bound(_libProps, _libPropsEnd, prop, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        // 修改现有的属性
        _copyForModify();

        first->function = nullptr;
        first->value = descriptor.value;
        first->isWritable = descriptor.isWritable;
        first->isGetter = descriptor.isGetter;
        first->isConfigurable = descriptor.isConfigurable;
        first->isEnumerable = descriptor.isEnumerable;

        if (setter.type > JDT_OBJECT) {
            first->valueSetter = setter;
            first->functionSet = nullptr;
        }
        return;
    }

    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyByName(ctx, prop, descriptor, setter);
}

void JsLibObject::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) {
    NumberToSizedString ss(index);
    definePropertyByName(ctx, ss, descriptor, setter);
}

void JsLibObject::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor, setter);
}

bool JsLibObject::getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut, JsValue &setterOut) {
    if (_obj && _obj->getOwnPropertyDescriptorByName(ctx, prop, descriptorOut, setterOut)) {
        return true;
    }

    // 使用二分查找找到
    auto first = std::lower_bound(_libProps, _libPropsEnd, prop, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        descriptorOut.value = first->value;
        descriptorOut.isWritable = first->isWritable;
        descriptorOut.isGetter = first->isGetter;
        descriptorOut.isConfigurable = first->isConfigurable;
        descriptorOut.isEnumerable = first->isEnumerable;
        if (first->functionSet) {
            setterOut = first->valueSetter;
        }
        return true;
    }

    return false;
}

bool JsLibObject::getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) {
    NumberToSizedString ss(index);
    return getOwnPropertyDescriptorByName(ctx, ss, descriptorOut, setterOut);
}

bool JsLibObject::getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) {
    if (_obj) {
        return _obj->getOwnPropertyDescriptorBySymbol(ctx, index, descriptorOut, setterOut);
    }

    return false;
}

JsValue JsLibObject::getSetterByName(VMContext *ctx, const SizedString &prop) {
    if (_obj) {
        return _obj->getSetterByName(ctx, prop);
    }

    return JsUndefinedValue;
}

JsValue JsLibObject::getSetterByIndex(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->getSetterByIndex(ctx, index);
    }

    return JsUndefinedValue;
}

JsValue JsLibObject::getSetterBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->getSetterBySymbol(ctx, index);
    }

    return JsUndefinedValue;
}

JsValue JsLibObject::getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop) {
    // 使用二分查找找到
    auto first = std::lower_bound(_libProps, _libPropsEnd, prop, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        if (first->isGetter) {
            if (first->function) {
                first->function(ctx, thiz, Arguments());
            } else {
                ctx->vm->callMember(ctx, thiz, first->value, Arguments());
            }
            return ctx->retValue;
        }

        return first->value;
    }

    if (_obj) {
        auto value = _obj->getByName(ctx, thiz, prop);
        if (value.type != JDT_NOT_INITIALIZED) {
            return value;
        }
    }

    return JsUndefinedValue;
}

JsValue JsLibObject::getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    NumberToSizedString ss(index);
    return getByName(ctx, thiz, ss);
}

JsValue JsLibObject::getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    if (_obj) {
        return _obj->getBySymbol(ctx, thiz, index);
    }

    return JsUndefinedValue;
}

void JsLibObject::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) {
    auto first = std::lower_bound(_libProps, _libPropsEnd, prop, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        _copyForModify();

        // 修改现有的属性
        if (first->functionSet) {
            first->functionSet(ctx, thiz, ArgumentsX(value));
        } else if (first->valueSetter.type > JDT_OBJECT) {
            ctx->vm->callMember(ctx, thiz, first->valueSetter, ArgumentsX(value));
        } else {
            first->value = value;
        }
        return;
    }

    if (!_obj) {
        _newObject();
    }

    _obj->setByName(ctx, thiz, prop, value);
}

void JsLibObject::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    NumberToSizedString ss(index);
    setByName(ctx, thiz, ss, value);
}

void JsLibObject::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject();
    }
    _obj->setBySymbol(ctx, thiz, index, value);
}

bool JsLibObject::removeByName(VMContext *ctx, const SizedString &prop) {
    auto first = std::lower_bound(_libProps, _libPropsEnd, prop, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        // 删除现有的属性
        _copyForModify();

        memcpy(first, first + 1, sizeof(*first) * (_libPropsEnd - first - 1));
        _libPropsEnd--;
    }

    if (_obj) {
        return _obj->removeByName(ctx, prop);
    }

    return true;
}

bool JsLibObject::removeByIndex(VMContext *ctx, uint32_t index) {
    NumberToSizedString ss(index);
    return removeByName(ctx, ss);
}

bool JsLibObject::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeBySymbol(ctx, index);
    }

    return true;
}

IJsObject *JsLibObject::clone() {
    auto obj = new JsLibObject(this);
    return obj;
}

void JsLibObject::_newObject() {
    assert(_obj == nullptr);

    // 如果是 Object.prototype, 避免循环调用到 __proto__ 的 get 函数中.
    _obj = new JsObject(_isObjectPrototype ? JsNullValue : JsNotInitializedValue);
}

void JsLibObject::_copyForModify() {
    if (!_modified) {
        _modified = true;

        auto count = _libPropsEnd - _libProps;
        auto tmp = new JsLibProperty[count];
        memcpy(tmp, _libProps, sizeof(JsLibProperty) * count);
        _libProps = tmp;
        _libPropsEnd = tmp + count;
    }
}

JsLibProperty makeJsLibPropertyGetter(const char *name, JsNativeFunction f) {
    JsLibProperty prop;

    memset(&prop, 0, sizeof(prop));
    prop.name = name;
    prop.function = f;
    prop.isGetter = true;
    prop.value = JsUndefinedValue;
    return prop;
}
