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
    JsStringIterator(VMRuntime *runtime, const JsValue &str) : _keyBuf(0) {
        assert(str.type == JDT_STRING);
        _runtime = runtime;
        _str = str;
        _pos = 0;
        _len = _runtime->getStringLength(str);
    }

    virtual bool nextKey(SizedString &keyOut) override {
        if (_pos >= _len) {
            return false;
        }

        _keyBuf.set(_pos);
        keyOut = _keyBuf.str();
        _pos++;
        return true;
    }

    virtual bool nextKey(JsValue &keyOut) override {
        if (_pos >= _len) {
            return false;
        }

        _keyBuf.set(_pos);
        keyOut = _runtime->pushString(_keyBuf.str());
        _pos++;
        return true;
    }

    virtual bool nextValue(JsValue &valueOut) override {
        if (_pos >= _len) {
            return false;
        }

        auto s = _runtime->getString(_str);
        valueOut = JsValue(JDT_CHAR, s.data[_pos]);

        _pos++;
        return true;
    }

    virtual bool next(JsValue &keyOut, JsValue &valueOut) override {
        if (_pos >= _len) {
            return false;
        }

        _keyBuf.set(_pos);
        keyOut = _runtime->pushString(_keyBuf.str());

        auto s = _runtime->getString(_str);
        valueOut = JsValue(JDT_CHAR, s.data[_pos]);

        _pos++;
        return true;
    }

    virtual bool next(SizedString &keyOut, JsValue &valueOut) override {
        if (_pos >= _len) {
            return false;
        }

        _keyBuf.set(_pos);
        keyOut = _keyBuf.str();

        auto s = _runtime->getString(_str);
        valueOut = JsValue(JDT_CHAR, s.data[_pos]);

        _pos++;
        return true;
    }

protected:
    VMRuntime                       *_runtime;
    JsValue                         _str;
    uint32_t                        _len;
    uint32_t                        _pos;
    NumberToSizedString             _keyBuf;

};

/**
 * 遍历 Char
 */
class JsCharIterator : public IJsIterator {
public:
    JsCharIterator(VMRuntime *runtime, const JsValue &str) : _keyBuf(0) {
        assert(str.type == JDT_CHAR);
        _runtime = runtime;
        _str = str;
        _end = false;
    }

    virtual bool nextKey(SizedString &keyOut) override {
        if (_end) {
            return false;
        }
        _end = true;

        keyOut = _keyBuf;
        return true;
    }

    virtual bool nextKey(JsValue &keyOut) override {
        if (_end) {
            return false;
        }
        _end = true;

        keyOut = _runtime->pushString(_keyBuf.str());
        return true;
    }

    virtual bool nextValue(JsValue &valueOut) override {
        if (_end) {
            return false;
        }
        _end = true;

        valueOut = _str;
        return true;
    }

    virtual bool next(JsValue &keyOut, JsValue &valueOut) override {
        if (_end) {
            return false;
        }
        _end = true;

        keyOut = _runtime->pushString(_keyBuf.str());
        valueOut = _str;
        return true;
    }

    virtual bool next(SizedString &keyOut, JsValue &valueOut) override {
        if (_end) {
            return false;
        }
        _end = true;

        keyOut = _keyBuf.str();
        valueOut = _str;
        return true;
    }

protected:
    VMRuntime                       *_runtime;
    JsValue                         _str;
    bool                            _end;
    SizedStringWrapper              _keyBuf;

};

IJsIterator *newJsStringIterator(VMRuntime *runtime, const JsValue &str) {
    if (str.type == JDT_STRING) {
        return new JsStringIterator(runtime, str);
    } else if (str.type == JDT_CHAR) {
        return new JsCharIterator(runtime, str);
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

    auto str = ctx->runtime->getString(thiz);
    if (index < str.len) {
        return JsValue(JDT_CHAR, str.data[index]);
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
