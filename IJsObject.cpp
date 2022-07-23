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

    return JsUndefinedValue;
}

void JsObject::set(VMContext *ctx, const SizedString &prop, const JsValue &value) {
    props[prop] = value;
}

JsValue JsObject::get(VMContext *ctx, uint32_t index) {
    if (index < MAX_INT_TO_CONST_STR) {
        return get(ctx, intToSizedString(index));
    } else {
        char buf[32];

        SizedString indexStr;
        indexStr.len = (uint32_t)itoa(index, buf);
        indexStr.data = (uint8_t *)buf;
        indexStr.unused = 0;
        return get(ctx, indexStr);
    }
}

void JsObject::set(VMContext *ctx, uint32_t index, const JsValue &value) {
    if (index < MAX_INT_TO_CONST_STR) {
        set(ctx, intToSizedString(index), value);
    } else {
        char buf[32];

        SizedString indexStr;
        indexStr.len = (uint32_t)itoa(index, buf);
        indexStr.data = (uint8_t *)buf;
        indexStr.unused = 0;
        set(ctx, indexStr, value);
    }
}

JsArguments::JsArguments() {
    obj = nullptr;
}

JsArguments::~JsArguments() {
    if (obj) {
        delete obj;
    }
}

JsValue JsArguments::get(VMContext *ctx, const SizedString &prop) {
    bool successful = false;
    auto n = prop.atoi(successful);
    if (successful && n < args->count) {
        return args->data[n];
    }

    if (obj) {
        return obj->get(ctx, prop);
    } else {
        return JsUndefinedValue;
    }
}

void JsArguments::set(VMContext *ctx, const SizedString &prop, const JsValue &value) {
    bool successful = false;
    auto n = prop.atoi(successful);
    if (successful && n < args->count) {
        args->data[n] = value;
    }

    if (obj == nullptr) {
        obj = new JsObject();
        // 设置 length 属性
        obj->set(ctx, SS_LENGTH, JsValue(JDT_INT32, args->count));
    }

    obj->set(ctx, prop, value);
}

JsValue JsArguments::get(VMContext *ctx, uint32_t index) {
    if (index < args->count) {
        return args->data[index];
    } else {
        if (obj) {
            return obj->get(ctx, index);
        } else {
            return JsUndefinedValue;
        }
    }
}

void JsArguments::set(VMContext *ctx, uint32_t index, const JsValue &value) {
    if (index < args->count) {
        args->data[index] = value;
    } else {
        if (obj == nullptr) {
            obj = new JsObject();
            // 设置 length 属性
            obj->set(ctx, SS_LENGTH, JsValue(JDT_INT32, args->count));
        }

        obj->set(ctx, index, value);
    }
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

    return JsUndefinedValue;
}

void JsLibObject::set(VMContext *ctx, const SizedString &prop, const JsValue &value) {
    if (!_obj) {
        _obj = new JsObject();
    }

    _obj->set(ctx, prop, value);
}

JsValue JsLibObject::get(VMContext *ctx, uint32_t index) {
    if (_obj) {
        _obj->get(ctx, index);
    }

    return JsUndefinedValue;
}

void JsLibObject::set(VMContext *ctx, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _obj = new JsObject();
    }

    _obj->set(ctx, index, value);
}
