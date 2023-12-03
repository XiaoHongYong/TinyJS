//
//  JsPrimaryObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/31.
//

#include "JsPrimaryObject.hpp"


class JsStringIterator : public IJsIterator {
public:
    JsStringIterator(VMContext *ctx, JsStringObject *objStr, const JsValue &str, bool includeProtoProp, bool includeNoneEnumerable) : IJsIterator(includeProtoProp, includeNoneEnumerable), _keyBuf(0)
    {
        assert(str.isString());
        _ctx = ctx;
        _runtime = ctx->runtime;
        _str = str;
        _pos = 0;

        if (objStr && objStr->_obj) {
            _itObjStr = objStr->_obj->getIteratorObject(_ctx, _includeProtoProp, _includeNoneEnumerable);
        } else if (_includeProtoProp) {
            _itObjStr = _ctx->runtime->objPrototypeString()->getIteratorObject(_ctx, true, _includeNoneEnumerable);
        } else {
            _itObjStr = nullptr;
        }

        if (str.type == JDT_CHAR) {
            auto len = utf32CodeToUtf8(str.value.n32, _chars);
            _strUtf16.set(StringView(_chars, len));
            _u16[0] = (utf16_t)str.value.n32;
            _strUtf16.setUtf16(_u16, 1);
        } else {
            _strUtf16 = ctx->runtime->getStringWithRandAccess(_str);
        }
        _len = _strUtf16.size();
        _isOfIterable = true;
    }

    virtual bool nextOf(JsValue &valueOut) override {
        if (_pos >= _len) {
            return false;
        }

        valueOut = makeJsValueChar(_strUtf16.chartAt(_pos));
        _pos++;

        return true;
    }

    virtual bool next(StringView *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        if (_pos >= _len) {
            if (_itObjStr) {
                return _itObjStr->next(strKeyOut, keyOut, valueOut);
            }
            return false;
        }

        _keyBuf.set(_pos);
        if (strKeyOut) {
            *strKeyOut = _keyBuf.str();
        }

        if (keyOut) {
            *keyOut = _runtime->pushString(_keyBuf.str());
        }

        if (valueOut) {
            *valueOut = makeJsValueChar(_strUtf16.chartAt(_pos));
        }

        _pos++;

        return true;
    }

    virtual void markReferIdx(VMRuntime *rt) override {
        rt->markReferIdx(_str);

        if (_itObjStr) {
            ::markReferIdx(rt, _itObjStr);
        }
    }

protected:
    VMContext                       *_ctx;
    VMRuntime                       *_runtime;
    IJsIterator                     *_itObjStr;
    JsValue                         _str;
    StringViewUtf16                _strUtf16;
    uint32_t                        _len;
    uint32_t                        _pos;
    NumberToStringView             _keyBuf;

    uint8_t                         _chars[4];
    utf16_t                         _u16[2];

};

IJsIterator *newJsStringIterator(VMContext *ctx, const JsValue &str, bool includeProtoProps, bool includeNoneEnumerable) {
    return new JsStringIterator(ctx, nullptr, str, includeProtoProps, includeNoneEnumerable);
}

JsStringObject::JsStringObject(const JsValue &value) : JsObjectLazy(nullptr, 0, jsValuePrototypeString, JDT_OBJ_STRING),_value(value)
{
    _length = -1;
    _isOfIterable = true;
}

void JsStringObject::setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) {
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

    JsObjectLazy::setPropertyByName(ctx, name, descriptor);
}

void JsStringObject::setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    _updateLength(ctx);

    if (index < _length) {
        ctx->throwException(JE_TYPE_ERROR, "Cannot redefine property: %d", index);
        return;
    }

    JsObjectLazy::setPropertyByIndex(ctx, index, descriptor);
}

JsError JsStringObject::setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) {
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

JsValue JsStringObject::increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) {
    _updateLength(ctx);

    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && index >= 0 && index < _length) {
            return increaseByIndex(ctx, thiz, (uint32_t)index, n, isPost);
        }
    }

    if (name.equal(SS_LENGTH)) {
        return makeJsValueInt32(isPost ? _length : _length + n);
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
        return makeJsValueInt32(isPost ? code : code + n);
    }

    return jsValueNaN;
}

JsValue *JsStringObject::getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp) {
    _updateLength(ctx);

    if (name.len > 0 && isDigit(name.data[0])) {
        bool successful = false;
        auto index = name.atoi(successful);
        if (successful && index >= 0 && index < _length) {
            return getRawByIndex(ctx, (uint32_t)index, includeProtoProp);
        }
    }

    if (name.equal(SS_LENGTH)) {
        static JsValue prop;
        prop = makeJsValueInt32(_length).asProperty(0);
        return &prop;
    }

    return JsObjectLazy::getRawByName(ctx, name, includeProtoProp);
}

JsValue *JsStringObject::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    _updateLength(ctx);

    if (index >= _length) {
        return JsObjectLazy::getRawByIndex(ctx, index, includeProtoProp);
    }

    auto str = ctx->runtime->getStringWithRandAccess(_value);
    auto code = str.chartAt(index);

    static JsValue prop;
    prop = makeJsValueChar(code).asProperty(JP_ENUMERABLE);

    return &prop;
}

bool JsStringObject::removeByName(VMContext *ctx, const StringView &name) {
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

IJsIterator *JsStringObject::getIteratorObject(VMContext *ctx, bool includeProtoProp, bool includeNoneEnumerable) {
    return new JsStringIterator(ctx, this, _value, includeProtoProp, includeNoneEnumerable);
}

void JsStringObject::_updateLength(VMContext *ctx) {
    if (_length == -1) {
        _length = ctx->runtime->getStringLength(_value);
    }
}
