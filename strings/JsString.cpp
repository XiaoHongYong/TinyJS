//
//  JsString.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/8.
//

#include "JsString.hpp"


JsValue getStringCharAtIndex(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    if (thiz.type == JDT_CHAR) {
        if (index == 0) {
            return thiz;
        }
        return jsValueUndefined;
    }

    auto &str = ctx->runtime->getStringWithRandAccess(thiz);
    if (index < str.size()) {
        return makeJsValueChar(str.chartAt(index));
    }

    return jsValueUndefined;
}

JsValue getStringMemberIndex(VMContext *ctx, const JsValue &thiz, const JsValue &name) {
    VMRuntime *runtime = ctx->runtime;

    if (name.type == JDT_INT32) {
        return getStringCharAtIndex(ctx, thiz, name.value.n32);
    }

    auto value = runtime->objPrototypeString()->get(ctx, thiz, name, jsValueEmpty);
    if (value.isValid()) {
        return value;
    }

    auto strName = runtime->toStringView(ctx, name);
    if (strName.equal(SS_LENGTH)) {
        if (thiz.type == JDT_CHAR) {
            return makeJsValueInt32(1);
        } else {
            return makeJsValueInt32(runtime->getStringLength(thiz));
        }
    }


    bool successful = false;
    auto index = strName.atoi(successful);
    if (successful) {
        return getStringCharAtIndex(ctx, thiz, (uint32_t)index);
    }

    return jsValueUndefined;
}
