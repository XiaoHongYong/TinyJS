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
            p->prop.value = JsValue(JDT_NATIVE_FUNCTION, idx);
        } else if (p->strValue) {
            p->prop.value = rt->pushStringValue(p->strValue);
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

void JsLibObject::definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor) {
    auto first = std::lower_bound(_libProps, _libPropsEnd, prop, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        // 修改现有的属性
        _copyForModify();

        first->function = nullptr;
        mergeJsProperty(ctx, &first->prop, descriptor, prop);
        return;
    }

    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyByName(ctx, prop, descriptor);
}

void JsLibObject::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    NumberToSizedString ss(index);
    definePropertyByName(ctx, ss, descriptor);
}

void JsLibObject::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor);
}

bool JsLibObject::getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut) {
    if (_obj && _obj->getOwnPropertyDescriptorByName(ctx, prop, descriptorOut)) {
        return true;
    }

    // 使用二分查找找到
    auto first = std::lower_bound(_libProps, _libPropsEnd, prop, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        descriptorOut = first->prop;
        return true;
    }

    return false;
}

bool JsLibObject::getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) {
    NumberToSizedString ss(index);
    return getOwnPropertyDescriptorByName(ctx, ss, descriptorOut);
}

bool JsLibObject::getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) {
    if (_obj) {
        return _obj->getOwnPropertyDescriptorBySymbol(ctx, index, descriptorOut);
    }

    return false;
}

JsProperty *JsLibObject::getRawByName(VMContext *ctx, const SizedString &prop, bool &isSelfPropOut) {
    // 使用二分查找
    auto first = std::lower_bound(_libProps, _libPropsEnd, prop, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        return &first->prop;
    }

    if (_obj) {
        return _obj->getRawByName(ctx, prop, isSelfPropOut);
    }

    return nullptr;
}

JsProperty *JsLibObject::getRawByIndex(VMContext *ctx, uint32_t index, bool &isSelfPropOut) {
    if (_obj) {
        isSelfPropOut = true;
        return _obj->getRawByIndex(ctx, index, isSelfPropOut);
    }

    return nullptr;
}

JsProperty *JsLibObject::getRawBySymbol(VMContext *ctx, uint32_t index, bool &isSelfPropOut) {
    if (_obj) {
        isSelfPropOut = true;
        return _obj->getRawBySymbol(ctx, index, isSelfPropOut);
    }

    return nullptr;
}

JsValue JsLibObject::getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &defVal) {
    // 使用二分查找找到
    auto first = std::lower_bound(_libProps, _libPropsEnd, prop, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        auto prop = first->prop;
        if (prop.isGetter) {
            if (first->function) {
                first->function(ctx, thiz, Arguments());
            } else {
                ctx->vm->callMember(ctx, thiz, prop.value, Arguments());
            }
            return ctx->retValue;
        }

        return prop.value;
    }

    if (_obj) {
        auto value = _obj->getByName(ctx, thiz, prop, defVal);
        if (value.type != JDT_NOT_INITIALIZED) {
            return value;
        }
    }

    return defVal;
}

JsValue JsLibObject::getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal) {
    NumberToSizedString ss(index);
    return getByName(ctx, thiz, ss, defVal);
}

JsValue JsLibObject::getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal) {
    if (_obj) {
        return _obj->getBySymbol(ctx, thiz, index, defVal);
    }

    return defVal;
}

void JsLibObject::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) {
    if (thiz.type < JDT_OBJECT) {
        // Primitive types 不能被修改，但是其 setter 函数会被调用
        if (_obj) {
            bool isSelfProp;
            auto propValue = _obj->getRawByName(ctx, prop, isSelfProp);
            if (propValue && propValue->setter.type >= JDT_FUNCTION) {
                // 调用 setter 函数
                ArgumentsX args(value);
                ctx->vm->callMember(ctx, thiz, propValue->setter, args);
                return;
            }
        }

        return;
    }

    auto first = std::lower_bound(_libProps, _libPropsEnd, prop, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        _copyForModify();

        // 修改现有的属性
        if (first->functionSet) {
            first->functionSet(ctx, thiz, ArgumentsX(value));
        } else if (first->prop.setter.type >= JDT_FUNCTION) {
            ctx->vm->callMember(ctx, thiz, first->prop.setter, ArgumentsX(value));
        } else if (first->prop.isWritable) {
            first->prop.value = value;
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
        if (!first->prop.isConfigurable) {
            return false;
        }

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

/**
 * 约定 prototype 在最后一个位置.
 */
void setPrototype(JsLibProperty *prop, const JsProperty &value) {
    assert(prop->name.equal("prototype"));
    prop->prop = value;
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
    return prop;
}
