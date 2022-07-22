//
//  IJsObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "IJsObject.hpp"
#include <algorithm>


static const SizedString __PROTO__ = makeSizedString("__proto__");

JsObject::JsObject(const JsValue &prototype) : prototype(prototype) {
    type = JDT_OBJECT;
}

JsValue JsObject::get(VMContext *ctx, const SizedString &prop) {
    auto it = props.find(prop);
    if (it != props.end()) {
        return (*it).second;
    }

    if (prototype.type > JDT_NULL) {
        
    }

    return JsValue();
}

void JsObject::set(VMContext *ctx, const SizedString &prop, const JsValue &value) {
    props[prop] = value;
}

class JsLibFunctionLessCmp {
public:
    bool operator ()(const JsLibProperty &a, const JsLibProperty &b) {
        return a.name.cmp(b.name) < 0;
    }

};

JsLibObject::JsLibObject(VMRuntimeCommon *rt, JsLibProperty *libProps, int countProps) : _libProps(libProps), _libPropsEnd(libProps + countProps) {
    type = JDT_LIB_OBJECT;
    _obj = nullptr;

    for (auto p = _libProps; p != _libPropsEnd; p++) {
        if (p->function) {
            auto idx = rt->pushNativeMemberFunction(p->function);
            p->value = JsValue(JDT_NATIVE_MEMBER_FUNCTION, idx);
        } else if (p->strValue) {
            auto idx = rt->pushStringValue(p->strValue);
            p->value = JsValue(JDT_STRING, idx);
        }
    }

    std::sort(_libProps, _libPropsEnd, JsLibFunctionLessCmp());
}

JsLibObject::JsLibObject(JsLibObject *from) {
    type = JDT_LIB_OBJECT;

    _obj = from->_obj;
    _libProps = from->_libProps;
    _libPropsEnd = from->_libPropsEnd;
}

JsValue JsLibObject::get(VMContext *ctx, const SizedString &prop) {
    if (_obj) {
        auto it = _obj->props.find(prop);
        if (it != _obj->props.end()) {
            // 优先返回设置的值
            return (*it).second;
        }
    }

    JsLibProperty key;
    key.name = prop;

    // 使用二分查找找到
    auto first = std::lower_bound(_libProps, _libPropsEnd, key, JsLibFunctionLessCmp());
    if (first != _libPropsEnd && first->name.equal(prop)) {
        return first->value;
    }

    return JsValue();
}

void JsLibObject::set(VMContext *ctx, const SizedString &prop, const JsValue &value) {
    if (!_obj) {
        _obj = new JsObject();
    }

    _obj->set(ctx, prop, value);
}
