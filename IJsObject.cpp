//
//  IJsObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#include "IJsObject.hpp"
#include <algorithm>


IJsObject::IJsObject() {
    type = JDT_OBJECT;
    referIdx = 0;
    nextFreeIdx = 0;
}

JsValue IJsObject::get(VMContext *ctx, const SizedString &prop) {
    auto it = props.find(prop);
    if (it == props.end()) {
        return JsValue();
    }

    return (*it).second;
}

void IJsObject::set(VMContext *ctx, const SizedString &prop, const JsValue &value) {
    props[prop] = value;
}

class JsLibFunctionLessCmp {
public:
    bool operator ()(const JsLibFunction &a, const JsLibFunction &b) {
        return a.name.cmp(b.name) < 0;
    }

};

JsLibObject::JsLibObject(VMContext *ctx, JsLibFunction *functions, int count) : _functions(functions), _functionsEnd(functions + count) {
    auto runtime = ctx->runtime;

    for (auto p = _functions; p != _functionsEnd; p++) {
        auto idx = runtime->pushNativeMemberFunction(p->function);
        p->value = JsValue(JDT_NATIVE_MEMBER_FUNCTION, idx);
    }

    std::sort(_functions, _functionsEnd, JsLibFunctionLessCmp());
}

JsValue JsLibObject::get(VMContext *ctx, const SizedString &prop) {
    auto it = props.find(prop);
    if (it != props.end()) {
        // 优先返回设置的值
        return (*it).second;
    }

    JsLibFunction key;
    key.name = prop;

    // 使用二分查找找到
    auto first = std::lower_bound(_functions, _functionsEnd, key, JsLibFunctionLessCmp());
    if (first != _functionsEnd && first->name.equal(prop)) {
        return first->value;
    }

    return JsValue();
}
