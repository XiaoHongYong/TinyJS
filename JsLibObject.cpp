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

void JsLibObject::definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) {
    auto first = std::lower_bound(_libProps, _libPropsEnd, name, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(name)) {
        // 修改现有的属性
        _copyForModify();

        first->function = nullptr;
        defineNameProperty(ctx, &first->prop, descriptor, name);
        return;
    }

    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyByName(ctx, name, descriptor);
}

void JsLibObject::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    SizedStringWrapper name(index);
    definePropertyByName(ctx, name, descriptor);
}

void JsLibObject::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor);
}

void JsLibObject::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    if (thiz.type < JDT_OBJECT) {
        // Primitive types 不能被修改，但是其 setter 函数会被调用
        if (_obj) {
            JsNativeFunction funcGetter = nullptr;
            auto prop = _obj->getRawByName(ctx, name, funcGetter, true);
            if (prop && prop->setter.type >= JDT_FUNCTION) {
                // 调用 setter 函数
                ArgumentsX args(value);
                ctx->vm->callMember(ctx, thiz, prop->setter, args);
            }
        }
        return;
    }

    auto first = std::lower_bound(_libProps, _libPropsEnd, name, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(name)) {
        _copyForModify();

        // 修改现有的属性
        if (first->functionSet) {
            first->functionSet(ctx, thiz, ArgumentsX(value));
        } else {
            set(ctx, &first->prop, thiz, value);
        }
    } else {
        if (!_obj) {
            _newObject();
        }

        _obj->setByName(ctx, thiz, name, value);
    }
}

void JsLibObject::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    SizedStringWrapper name(index);
    setByName(ctx, thiz, name, value);
}

void JsLibObject::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject();
    }
    _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue JsLibObject::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    if (thiz.type < JDT_OBJECT) {
        // Primitive types 不能被修改，但是其 setter 函数会被调用
        if (_obj) {
            JsNativeFunction funcGetter = nullptr;
            auto prop = _obj->getRawByName(ctx, name, funcGetter, true);

            // 保存到临时对象中调用
            JsProperty tmp = *prop;
            return increase(ctx, &tmp, thiz, n, isPost);
        }
        return jsValueNaN;
    }

    auto first = std::lower_bound(_libProps, _libPropsEnd, name, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(name)) {
        _copyForModify();

        // 修改现有的属性
        if (first->functionSet) {
            // TODO: ...
            assert(0);
            // first->functionSet(ctx, thiz, ArgumentsX(value));
            return jsValueNaN;
        } else {
            return increase(ctx, &first->prop, thiz, n, isPost);
        }
    } else {
        if (!_obj) {
            _newObject();
        }

        return _obj->increaseByName(ctx, thiz, name, n, isPost);
    }
}

JsValue JsLibObject::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    SizedStringWrapper name(index);
    return increaseByName(ctx, thiz, name, n, isPost);
}

JsValue JsLibObject::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject();
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsProperty *JsLibObject::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    funcGetterOut = nullptr;

    // 使用二分查找
    auto first = std::lower_bound(_libProps, _libPropsEnd, name, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(name)) {
        if (first->prop.isGSetter) {
            funcGetterOut = first->function;
        }
        return &first->prop;
    }

    if (_obj) {
        return _obj->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
    }

    return nullptr;
}

JsProperty *JsLibObject::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        includeProtoProp = true;
        return _obj->getRawByIndex(ctx, index, includeProtoProp);
    }

    return nullptr;
}

JsProperty *JsLibObject::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        includeProtoProp = true;
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsLibObject::removeByName(VMContext *ctx, const SizedString &name) {
    auto first = std::lower_bound(_libProps, _libPropsEnd, name, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(name)) {
        if (!first->prop.isConfigurable) {
            return false;
        }

        // 删除现有的属性
        _copyForModify();

        memcpy(first, first + 1, sizeof(*first) * (_libPropsEnd - first - 1));
        _libPropsEnd--;
    }

    if (_obj) {
        return _obj->removeByName(ctx, name);
    }

    return true;
}

bool JsLibObject::removeByIndex(VMContext *ctx, uint32_t index) {
    SizedStringWrapper name(index);
    return removeByName(ctx, name);
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

IJsIterator *JsLibObject::getIteratorObject(VMContext *ctx) {
    // auto it = new JsObjectIterator(ctx, this);
    // return it;
    return nullptr;
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
    _obj = new JsObject(_isObjectPrototype ? jsValueNull : jsValueNotInitialized);
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

    prop.name = name;
    prop.function = f;
    return prop;
}
