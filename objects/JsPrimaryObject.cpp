//
//  JsPrimaryObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/31.
//

#include "JsPrimaryObject.hpp"


JsStringObject::JsStringObject(const JsValue &value) : JsObjectLazy(nullptr, 0, jsValuePrototypeString),_value(value) {
    type = JDT_OBJ_STRING;
    _length = -1;
}

void JsStringObject::definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) {
    _updateLength(ctx);

    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && index >= 0 && index < _length) {
            ctx->throwException(JE_TYPE_ERROR, "Cannot redefine property: %d", index);
        }
    }

    if (name.equal(SS_LENGTH)) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot redefine property: length");
        return;
    }

    JsObjectLazy::definePropertyByName(ctx, name, descriptor);
}

void JsStringObject::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    _updateLength(ctx);

    if (index < _length) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot redefine property: %d", index);
        return;
    }

    JsObjectLazy::definePropertyByIndex(ctx, index, descriptor);
}

JsError JsStringObject::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    _updateLength(ctx);

    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && index >= 0 && index < _length) {
            return JE_TYPE_PROP_READ_ONLY;
        }
    }

    if (name.equal(SS_LENGTH)) {
        return JE_TYPE_LENGTH_NOT_WRITABLE;
    }

    return JsObjectLazy::setByName(ctx, thiz, name, value);
}

JsError JsStringObject::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    _updateLength(ctx);

    if (index < _length) {
        return JE_TYPE_PROP_READ_ONLY;
    }

    return JsObjectLazy::setByIndex(ctx, thiz, index, value);
}

JsValue JsStringObject::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    _updateLength(ctx);

    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && index >= 0 && index < _length) {
            return increaseByIndex(ctx, thiz, (uint32_t)index, n, isPost);
        }
    }

    if (name.equal(SS_LENGTH)) {
        return JsValue(JDT_INT32, isPost ? _length : _length + n);
    }

    return JsObjectLazy::increaseByName(ctx, thiz, name, n, isPost);
}

JsValue JsStringObject::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    _updateLength(ctx);

    if (index >= _length) {
        return JsObjectLazy::increaseByIndex(ctx, thiz, index, n, isPost);
    }

    auto str = ctx->runtime->getStringWithRandAccess(_value);
    auto code = str.chartAt(index);
    if (isDigit(code)) {
        code -= '0';
        return JsValue(JDT_INT32, isPost ? code : code + n);
    }

    return jsValueNaN;
}

JsProperty *JsStringObject::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    _updateLength(ctx);

    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && index >= 0 && index < _length) {
            funcGetterOut = nullptr;
            return getRawByIndex(ctx, (uint32_t)index, includeProtoProp);
        }
    }

    if (name.equal(SS_LENGTH)) {
        static JsProperty prop(jsValueUndefined, false, false, true, false);
        prop.value = JsValue(JDT_INT32, _length);
        return &prop;
    }

    return JsObjectLazy::getRawByName(ctx, name, funcGetterOut, includeProtoProp);
}

JsProperty *JsStringObject::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    _updateLength(ctx);

    if (index >= _length) {
        return JsObjectLazy::getRawByIndex(ctx, index, includeProtoProp);
    }

    auto str = ctx->runtime->getStringWithRandAccess(_value);
    auto code = str.chartAt(index);

    static JsProperty prop(jsValueUndefined, false, false, true, false);
    prop.value = JsValue(JDT_CHAR, code);

    return &prop;
}

bool JsStringObject::removeByName(VMContext *ctx, const SizedString &name) {
    _updateLength(ctx);

    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && index >= 0 && index < _length) {
            return false;
        }
    }

    return JsObjectLazy::removeByName(ctx, name);
}

bool JsStringObject::removeByIndex(VMContext *ctx, uint32_t index) {
    _updateLength(ctx);

    if (index < _length) {
        return false;
    }

    return JsObjectLazy::removeByIndex(ctx, index);
}

void JsStringObject::_updateLength(VMContext *ctx) {
    if (_length == -1) {
        _length = ctx->runtime->getStringLength(_value);
    }
}
