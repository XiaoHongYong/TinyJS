//
//  JSON.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/11/20.
//

#include "BuiltIn.hpp"
#include "objects/JsObjectFunction.hpp"
#include "objects/JsArray.hpp"
#include "objects/JsArguments.hpp"
#include "rapidjson/reader.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


// https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/JSON


using namespace rapidjson;

class Stream {
public:
    Stream(uint8_t *data, uint32_t len) {
        _begin = _ptr = data;
        _end = data + len;
    }

    using Ch = uint8_t;

    //! Read the current character from stream without moving the read cursor.
    Ch Peek() const {
        return _ptr < _end ? *_ptr : '\0';
    }

    //! Read the current character from stream and moving the read cursor to next character.
    Ch Take() {
        return _ptr < _end ? *_ptr++ : '\0';
    }

    //! Get the current read cursor.
    //! \return Number of characters read from start.
    size_t Tell() {
        return _ptr - _begin;
    }

    //! Begin writing operation at the current read pointer.
    //! \return The begin writer pointer.
    Ch* PutBegin() { assert(0); return nullptr; }

    //! Write a character.
    void Put(Ch c) { assert(0); }

    //! Flush the buffer.
    void Flush() { assert(0); }

    //! End the writing operation.
    //! \param begin The begin write pointer returned by PutBegin().
    //! \return Number of characters written.
    size_t PutEnd(Ch* begin) { assert(0); return 0; }

protected:
    Ch                  *_begin, *_ptr, *_end;

};

class TsJsonHandler : public BaseReaderHandler<UTF8<>, TsJsonHandler> {
public:
    TsJsonHandler(VMContext *ctx) {
        _status = START;
        _obj = nullptr;
        _index = 0;
        _rt = ctx->runtime;
        _ctx = ctx;
    }

    enum Status {
        START,
        IN_OBJECT,
        IN_ARRAY,
    };

    struct StackItem {
        Status              status;
        IJsObject           *obj;

        StackItem(Status status, IJsObject *obj) : status(status), obj(obj) { }
    };

    inline void onValue(JsValue value) {
        if (_status == IN_OBJECT) {
            _obj->setByName(_ctx, _obj->self, _key, value);
        } else if (_status == IN_ARRAY) {
            _obj->setByIndex(_ctx, _obj->self, _index, value);
            _index++;
        } else {
            _value = value;
        }
    }

    bool Null() { onValue(jsValueNull); return true; }

    bool Bool(bool b) { onValue(JsValue(JDT_BOOL, b)); return true; }
    bool Int(int i) { onValue(JsValue(JDT_INT32, i)); return true; }
    bool Uint(unsigned u) {
        if (u > 0x7FFFFFFF) {
            onValue(_rt->pushDoubleValue(u));
        } else {
            onValue(JsValue(JDT_INT32, u));
        }
        return true;
    }
    bool Int64(int64_t i) {
        if (i == (int32_t)i) {
            onValue(JsValue(JDT_INT32, (int32_t)i));
        } else {
            onValue(_rt->pushDoubleValue(i));
        }
        return true;
    }
    bool Uint64(uint64_t u) {
        if (u == (int32_t)u) {
            onValue(JsValue(JDT_INT32, (int32_t)u));
        } else {
            onValue(_rt->pushDoubleValue(u));
        }
        return true;
    }
    bool Double(double d) { onValue(_rt->pushDoubleValue(d)); return true; }
    bool String(const char* str, SizeType length, bool copy) {
        onValue(_rt->pushString(SizedString(str, length)));
        return true;
    }
    bool StartObject() {
        auto obj = new JsObject();
        onValue(_rt->pushObjectValue(obj));

        _stack.push_back(StackItem(_status, _obj));
        _obj = obj;
        _status = IN_OBJECT;
        return true;
    }
    bool Key(const char* str, SizeType length, bool copy) {
        if (copy) {
            _keyBuf.assign(str, length);
            _key.data = (uint8_t *)_keyBuf.data();
        } else {
            _key.data = (uint8_t *)str;
        }
        _key.len = length;
        return true;
    }
    bool EndObject(SizeType memberCount) {
        auto &s = _stack.back();
        _status = s.status;
        _obj = s.obj;
        _stack.pop_back();
        return true;
    }
    bool StartArray() {
        auto obj = new JsArray();
        onValue(_rt->pushObjectValue(obj));

        _stack.push_back(StackItem(_status, _obj));
        _obj = obj;
        _status = IN_ARRAY;
        _index = 0;
        return true;
    }
    bool EndArray(SizeType elementCount) {
        auto &s = _stack.back();
        _status = s.status;
        _obj = s.obj;
        _stack.pop_back();
        return true;
    }

    JsValue value() { return _value; }

protected:
    vector<StackItem>       _stack;
    Status                  _status;
    int                     _index;

    VMRuntime               *_rt;
    VMContext               *_ctx;
    string                  _keyBuf;
    IJsObject               *_obj;
    SizedString             _key;
    JsValue                 _value;

};

void json_parse(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto str = runtime->toSizedString(ctx, args.getAt(0));

    TsJsonHandler handler(ctx);
    Reader reader;
    //StringStream ss(str.data, str.len);
    // StringStream ss("1");
    Stream ss(str.data, str.len);
    auto result = reader.Parse(ss, handler);
    if (result.Code() == kParseErrorNone) {
        ctx->retValue = handler.value();
    } else {
        int n = str.len - (int)result.Offset();
        if (n >= 20) {
            n = 20;
        }
        ctx->throwException(JE_SYNTAX_ERROR, "Not a valid JSON, at offset: %d, %.*s", result.Offset(), n, str.data + result.Offset());
    }
}

void stringify(VMContext *ctx, Writer<StringBuffer> &writer, JsValue value) {
    switch (value.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
        case JDT_SYMBOL:
            assert(0);
            break;
        case JDT_NULL:
            writer.Null();
            break;
        case JDT_INT32:
            writer.Int(value.value.n32);
            break;
        case JDT_BOOL:
            writer.Bool(value.value.n32);
            break;
        case JDT_CHAR: {
            SizedStringWrapper s(value);
            writer.String((char *)s.data, s.len, true);
            break;
        }
        case JDT_STRING: {
            auto s = ctx->runtime->toSizedString(ctx, value);
            writer.String((char *)s.data, s.len, true);
            break;
        }
        case JDT_NUMBER: {
            auto d = ctx->runtime->getDouble(value);
            if (isnan(d) || isinf(d)) {
                writer.Null();
            } else {
                writer.Double(d);
            }
            break;
        }
        case JDT_ARRAY: {
            writer.StartArray();

            auto obj = ctx->runtime->getObject(value);
            int32_t length;
            obj->getLength(ctx, length);

            for (int i = 0; i < length; i++) {
                auto v = obj->getByIndex(ctx, value, i);
                if (v.type <= JDT_NULL) {
                    writer.Null();
                } else {
                    stringify(ctx, writer, v);
                }
            }

            writer.EndArray();
            break;
        }
        default:
            writer.StartObject();

            auto obj = ctx->runtime->getObject(value);
            unique_ptr<IJsIterator> it;
            it.reset(obj->getIteratorObject(ctx));

            SizedString key;
            JsValue v;
            while (it->next(&key, nullptr, &v)) {
                if (v.type > JDT_NULL) {
                    writer.Key((char *)key.data, key.len);
                    stringify(ctx, writer, v);
                }
            }

            writer.EndObject();
            break;
    }
}

void json_stringify(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto value = args.getAt(0);
    if (value.type == JDT_UNDEFINED || value.type == JDT_SYMBOL) {
        ctx->retValue = jsValueUndefined;
        return;
    }

    StringBuffer s;
    Writer<StringBuffer> writer(s);

    stringify(ctx, writer, value);

    if (ctx->error == JE_OK) {
        ctx->retValue = runtime->pushString(SizedString(s.GetString(), s.GetSize()));
    }
}


static JsLibProperty jsonFunctions[] = {
    { "name", nullptr, "JSON" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "parse", json_parse, },
    { "stringify", json_stringify, },
};

void registerJSON(VMRuntimeCommon *rt) {
    setGlobalLibObject("JSON", rt, jsonFunctions, CountOf(jsonFunctions));
}
