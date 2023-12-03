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
    IJsIterator(bool includeProtoProp, bool includeNoneEnumerable) : IJsObject(jsValueEmpty, JDT_ITERATOR) {
        _includeProtoProp = includeProtoProp;
        _includeNoneEnumerable = includeNoneEnumerable;

        _obj = nullptr;
        referIdx = 0;
        nextFreeIdx = 0;
    }
    virtual ~IJsIterator() {}

    virtual bool nextOf(JsValue &valueOut) { return false; }
    virtual bool next(StringView *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) { return false; }

    inline bool nextKey(StringView &keyOut) { return next(&keyOut); }
    inline bool nextKey(JsValue &keyOut) { return next(nullptr, &keyOut); }
    inline bool nextValue(JsValue &valueOut) { return next(nullptr, nullptr, &valueOut); }

    inline void nextOf() { if (!_done) { _done = !nextOf(_curValue); } }
    inline JsValue curValue() { return _curValue; }

    //
    // IJsObject 的接口实现
    //
    virtual void setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) override;
    virtual void setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) override;
    virtual void setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) override;

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) override;
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsValue *getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp = true) override;
    virtual JsValue *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;
    virtual JsValue *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const StringView &name) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual void changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) override;
    virtual bool hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) override;
    virtual void preventExtensions(VMContext *ctx) override;

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

protected:
    virtual void _newObject(VMContext *ctx);

protected:
    friend class JsIteratorOfIterator;

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
