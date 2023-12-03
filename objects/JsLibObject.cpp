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

    bool operator ()(const JsLibProperty &a, const StringView &b) {
        return a.name.cmp(b) < 0;
    }

    bool operator ()(const StringView &a, const JsLibProperty &b) {
        return a.cmp(b.name) < 0;
    }

};

JsLibObject::JsLibObject(VMRuntimeCommon *rt, JsLibProperty *libProps, int countProps, cstr_t name, JsNativeFunction constructor, const JsValue &proto) : IJsObject(proto, JDT_LIB_OBJECT), _libProps(libProps), _libPropsEnd(libProps + countProps), _constructor(constructor)
{
    _name = name ? makeCommonString(name) : stringViewEmpty;
    _obj = nullptr;
    _modified = false;
    _isOfIterable = false;

    for (auto p = _libProps; p != _libPropsEnd; p++) {
        p->name.setStable();
        if (p->function) {
            p->prop = rt->pushNativeFunction(p->function, p->name).asProperty(JP_WRITABLE | JP_CONFIGURABLE);
        } else if (p->strValue) {
            p->prop = rt->pushStringValue(makeStableStr(p->strValue)).asProperty(JP_CONFIGURABLE);
        }
    }

    std::sort(_libProps, _libPropsEnd, JsLibFunctionLessCmp());
}

JsLibObject::JsLibObject(JsLibObject *from) : IJsObject(from->__proto__, from->type) {
    assert(!from->_modified);
    _name = from->_name;
    _constructor = from->_constructor;
    _obj = from->_obj;
    _libProps = from->_libProps;
    _libPropsEnd = from->_libPropsEnd;
    _modified = false;
    _isOfIterable = from->_isOfIterable;
    __proto__ = from->__proto__;
}

JsLibObject::~JsLibObject() {
    if (_modified) {
        delete [] _libProps;
    }

    if (_obj) {
        delete _obj;
    }
}

void JsLibObject::setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) {
    auto first = std::lower_bound(_libProps, _libPropsEnd, name, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(name)) {
        // 修改现有的属性
        first = _copyForModify(first);

        first->function = nullptr;
        first->prop = descriptor;
        return;
    }

    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyByName(ctx, name, descriptor);
}

void JsLibObject::setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    NumberToStringView name(index);
    setPropertyByName(ctx, name, descriptor);
}

void JsLibObject::setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyBySymbol(ctx, index, descriptor);
}

JsError JsLibObject::setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) {
    if (thiz.type < JDT_OBJECT) {
        // Primitive types 不能被修改，但是其 setter 函数会被调用
        if (_obj) {
            auto prop = _obj->getRawByName(ctx, name, true);
            if (prop && prop->isGetterSetter()) {
                // 调用 setter 函数
                setPropertyValue(ctx, prop, thiz, value);
            }
        }
        return JE_OK;
    }

    auto first = std::lower_bound(_libProps, _libPropsEnd, name, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(name)) {
        first = _copyForModify(first);

        // 修改现有的属性
        return setPropertyValue(ctx, &first->prop, thiz, value);
    } else {
        if (!_obj) {
            _newObject(ctx);
        }

        return _obj->setByName(ctx, thiz, name, value);
    }
    return JE_OK;
}

JsError JsLibObject::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    NumberToStringView name(index);
    return setByName(ctx, thiz, name, value);
}

JsError JsLibObject::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue JsLibObject::increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) {
    if (thiz.type < JDT_OBJECT) {
        // Primitive types 不能被修改，但是其 setter 函数会被调用
        if (_obj) {
            auto prop = _obj->getRawByName(ctx, name, true);

            // 保存到临时对象中调用
            JsValue tmp = *prop;
            return increasePropertyValue(ctx, &tmp, thiz, n, isPost);
        }
        return jsValueNaN;
    }

    auto first = std::lower_bound(_libProps, _libPropsEnd, name, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(name)) {
        first = _copyForModify(first);

        // 修改现有的属性
        return increasePropertyValue(ctx, &first->prop, thiz, n, isPost);
    } else {
        if (!_obj) {
            _newObject(ctx);
        }

        return _obj->increaseByName(ctx, thiz, name, n, isPost);
    }
}

JsValue JsLibObject::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    NumberToStringView name(index);
    return increaseByName(ctx, thiz, name, n, isPost);
}

JsValue JsLibObject::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsValue *JsLibObject::getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp) {
    // 使用二分查找
    auto first = std::lower_bound(_libProps, _libPropsEnd, name, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(name)) {
        return &first->prop;
    }

    if (_obj) {
        return _obj->getRawByName(ctx, name, includeProtoProp);
    }

    if (name.equal(SS___PROTO__)) {
        return &__proto__;
    }

    if (includeProtoProp) {
        // 查找 prototye 的属性
        auto obj = getPrototypeObject(ctx);
        if (obj) {
            return obj->getRawByName(ctx, name, true);
        }
    }

    return nullptr;
}

JsValue *JsLibObject::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawByIndex(ctx, index, includeProtoProp);
    }

    return nullptr;
}

JsValue *JsLibObject::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsLibObject::removeByName(VMContext *ctx, const StringView &name) {
    auto first = std::lower_bound(_libProps, _libPropsEnd, name, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(name)) {
        if (!first->prop.isConfigurable()) {
            return false;
        }

        // 删除现有的属性
        first = _copyForModify(first);

        memmove(first, first + 1, sizeof(*first) * (_libPropsEnd - first - 1));
        _libPropsEnd--;
    }

    if (_obj) {
        return _obj->removeByName(ctx, name);
    }

    return true;
}

bool JsLibObject::removeByIndex(VMContext *ctx, uint32_t index) {
    NumberToStringView name(index);
    return removeByName(ctx, name);
}

bool JsLibObject::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeBySymbol(ctx, index);
    }

    return true;
}

void JsLibObject::changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) {
    _copyForModify(_libProps);

    for (auto p = _libProps; p != _libPropsEnd; p++) {
        p->prop.changeProperty(toAdd, toRemove);
    }

    if (_obj) {
        _obj->changeAllProperties(ctx, toAdd, toRemove);
    }
}

bool JsLibObject::hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) {
    for (auto p = _libProps; p != _libPropsEnd; p++) {
        if (p->prop.isPropertyAny(flags)) {
            return true;
        }
    }

    return _obj && _obj->hasAnyProperty(ctx, flags);
}

void JsLibObject::preventExtensions(VMContext *ctx) {
    IJsObject::preventExtensions(ctx);

    if (_obj) {
        _obj->preventExtensions(ctx);
    }
}

IJsObject *JsLibObject::clone() {
    auto obj = new JsLibObject(this);
    return obj;
}

IJsIterator *JsLibObject::getIteratorObject(VMContext *ctx, bool includeProtoProp, bool includeNoneEnumerable) {
    if (_obj) {
        return _obj->getIteratorObject(ctx, includeProtoProp, includeNoneEnumerable);
    }

    return new EmptyJsIterator();
}

void JsLibObject::markReferIdx(VMRuntime *rt) {
    assert(referIdx == rt->nextReferIdx());

    if (_modified) {
        for (auto p = _libProps; p != _libPropsEnd; p++) {
            rt->markReferIdx(p->prop);
        }
    }

    rt->markReferIdx(__proto__);

    if (_obj) {
        _obj->referIdx = rt->nextReferIdx();
        _obj->markReferIdx(rt);
    }
}

/**
 * 约定 prototype 在最后一个位置.
 */
void setPrototype(JsLibProperty *prop, JsLibProperty *propEnd, const JsValue &value) {
    // 逆序查找，prototye 放在后面.
    for (auto p = propEnd - 1; p >= prop; p--) {
        if (p->name.equal("prototype")) {
            p->prop = value.asProperty(JP_WRITABLE);
            return;
        }
    }
    assert(0);
}

void JsLibObject::_newObject(VMContext *ctx) {
    assert(_obj == nullptr);

    _obj = new JsObject(__proto__);

    if (isPreventedExtensions) {
        _obj->preventExtensions(ctx);
    }
}

JsLibProperty *JsLibObject::_copyForModify(JsLibProperty *pos) {
    if (!_modified) {
        _modified = true;

        auto offset = pos - _libProps;
        auto count = _libPropsEnd - _libProps;
        auto tmp = new JsLibProperty[count];
        memcpy(tmp, _libProps, sizeof(JsLibProperty) * count);
        _libProps = tmp;
        _libPropsEnd = tmp + count;

        pos = _libProps + offset;
    }

    return pos;
}

JsLibProperty makeJsLibPropertyGetter(const char *name, JsNativeFunction f) {
    JsLibProperty prop;

    prop.name = name;
    prop.function = f;
    return prop;
}

JsLibObject *setGlobalLibObject(cstr_t name, VMRuntimeCommon *rt, JsLibProperty *libProps, int countProps, JsNativeFunction constructor, const JsValue &proto) {
    auto obj = new JsLibObject(rt, libProps, countProps, name, constructor, proto);
    rt->setGlobalObject(name, obj);
    return obj;
}
