//
//  JsString.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/8.
//

#include "JsString.hpp"


/**
 * 遍历 String
 */
class JsStringIterator : public IJsIterator {
public:
    JsStringIterator(VMContext *ctx, const JsValue &str, bool includeProtoProp) : _keyBuf(0) {
        assert(str.type == JDT_STRING);
        _ctx = ctx;
        _str = str;
        _pos = 0;
        _itObj = nullptr;
        _len = _ctx->runtime->getStringLength(str);
        _includeProtoProp = includeProtoProp;
    }

    virtual bool nextOf(JsValue &valueOut) override {
        if (_pos >= _len) {
            return false;
        }

        auto &s = _ctx->runtime->getStringWithRandAccess(_str);
        valueOut = JsValue(JDT_CHAR, s.chartAt(_pos));
        _pos++;

        return true;
    }

    virtual bool next(SizedString *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        if (_pos >= _len) {
            if (_includeProtoProp) {
                if (_itObj == nullptr) {
                    _itObj = _ctx->runtime->objPrototypeString->getIteratorObject(_ctx, true);
                }
                return _itObj->next(strKeyOut, keyOut, valueOut);
            }
            return false;
        }

        _keyBuf.set(_pos);
        if (strKeyOut) {
            *strKeyOut = _keyBuf.str();
        }

        if (keyOut) {
            *keyOut = _ctx->runtime->pushString(_keyBuf.str());
        }

        if (valueOut) {
            auto &s = _ctx->runtime->getStringWithRandAccess(_str);
            *valueOut = JsValue(JDT_CHAR, s.chartAt(_pos));
        }

        _pos++;

        return true;
    }

protected:
    VMContext                       *_ctx;
    JsValue                         _str;
    uint32_t                        _len;
    uint32_t                        _pos;
    NumberToSizedString             _keyBuf;

    IJsIterator                     *_itObj;
    bool                            _includeProtoProp;

};

/**
 * 遍历 Char
 */
class JsCharIterator : public IJsIterator {
public:
    JsCharIterator(VMContext *ctx, const JsValue &str, bool includeProtoProp) : _keyBuf(0) {
        assert(str.type == JDT_CHAR);
        _ctx = ctx;
        _str = str;
        _end = false;
        _itObj = nullptr;
        _includeProtoProp = includeProtoProp;
    }

    virtual bool nextOf(JsValue &valueOut) override {
        if (_end) {
            return false;
        }

        valueOut = _str;
        _end = true;

        return true;
    }

    virtual bool next(SizedString *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        if (_end) {
            if (_includeProtoProp) {
                if (_itObj == nullptr) {
                    _itObj = _ctx->runtime->objPrototypeString->getIteratorObject(_ctx, true);
                }
                return _itObj->next(strKeyOut, keyOut, valueOut);
            }
            return false;
        }

        if (strKeyOut) {
            *strKeyOut = _keyBuf.str();
        }

        if (keyOut) {
            *keyOut = _ctx->runtime->pushString(_keyBuf.str());
        }

        if (valueOut) {
            *valueOut = _str;
        }

        _end = true;
        return true;
    }

protected:
    VMContext                       *_ctx;
    JsValue                         _str;
    bool                            _end;
    SizedStringWrapper              _keyBuf;

    IJsIterator                     *_itObj;
    bool                            _includeProtoProp;

};

IJsIterator *newJsStringIterator(VMContext *ctx, const JsValue &str, bool includeProtoProps) {
    if (str.type == JDT_STRING) {
        return new JsStringIterator(ctx, str, includeProtoProps);
    } else if (str.type == JDT_CHAR) {
        return new JsCharIterator(ctx, str, includeProtoProps);
    } else {
        assert(0);
    }
}

JsValue getStringCharAtIndex(VMContext *ctx, const JsValue &thiz, uint32_t index) {
    if (thiz.type == JDT_CHAR) {
        if (index == 0) {
            return thiz;
        }
        return jsValueUndefined;
    }

    auto &str = ctx->runtime->getStringWithRandAccess(thiz);
    if (index < str.size()) {
        return JsValue(JDT_CHAR, str.chartAt(index));
    }

    return jsValueUndefined;
}

JsValue getStringMemberIndex(VMContext *ctx, const JsValue &thiz, const JsValue &name) {
    VMRuntime *runtime = ctx->runtime;

    if (name.type == JDT_INT32) {
        return getStringCharAtIndex(ctx, thiz, name.value.n32);
    }

    auto value = runtime->objPrototypeString->get(ctx, thiz, name, jsValueNotInitialized);
    if (value.isValid()) {
        return value;
    }

    string buf;
    auto strName = runtime->toSizedString(ctx, name, buf);

    bool successful = false;
    auto index = strName.atoi(successful);
    if (successful) {
        return getStringCharAtIndex(ctx, thiz, (uint32_t)index);
    }

    return jsValueUndefined;
}
