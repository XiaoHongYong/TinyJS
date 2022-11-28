//
//  IJsIterator.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/10/10.
//

#ifndef IJsIterator_hpp
#define IJsIterator_hpp

#include "IJsObject.hpp"


class IJsIterator : public IJsObject {
public:
    IJsIterator(bool includeProtoProp, bool includeNoneEnumerable) {
        _includeProtoProp = includeProtoProp;
        _includeNoneEnumerable = includeNoneEnumerable;

        __proto__ = JsProperty(jsValueNotInitialized, false, false, false, true);
        _obj = nullptr;
        type = JDT_ITERATOR;
        referIdx = 0;
        nextFreeIdx = 0;
    }
    virtual ~IJsIterator() {}

    virtual bool nextOf(JsValue &valueOut) { return false; }
    virtual bool next(SizedString *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) { return false; }

    inline bool nextKey(SizedString &keyOut) { return next(&keyOut); }
    inline bool nextKey(JsValue &keyOut) { return next(nullptr, &keyOut); }
    inline bool nextValue(JsValue &valueOut) { return next(nullptr, nullptr, &valueOut); }

    inline void nextOf() { if (!_done) { _done = !nextOf(_curValue); } }
    inline JsValue curValue() { return _curValue; }

    //
    // IJsObject 的接口实现
    //
    virtual void definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) override;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) override;
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp = true) override;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &name) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual void changeAllProperties(VMContext *ctx, int8_t configurable = -1, int8_t writable = -1) override;
    virtual bool hasAnyProperty(VMContext *ctx, bool configurable, bool writable) override;
    virtual void preventExtensions(VMContext *ctx) override;

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

protected:
    virtual void _newObject(VMContext *ctx);

protected:
    friend class JsIteratorOfIterator;

    JsProperty                  __proto__;
    JsObject                    *_obj;

    JsValue                     _curValue;

    bool                        _done;
    bool                        _includeProtoProp;
    bool                        _includeNoneEnumerable;

};

class EmptyJsIterator : public IJsIterator {
public:
    EmptyJsIterator() : IJsIterator(false, false) {
        _isOfIterable = true;
    }

};

#endif /* IJsIterator_hpp */
