//
//  JsDummyObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#ifndef JsDummyObject_hpp
#define JsDummyObject_hpp

#include "IJsObject.hpp"

/**
 * 提供一个空的 IJsObject，用于占位
 */
class JsDummyObject : public IJsObject {
public:
    JsDummyObject() : IJsObject(jsValueEmpty, JDT_OBJECT) { }
    virtual ~JsDummyObject() { }

    virtual void setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) override { }
    virtual void setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) override { }
    virtual void setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) override { }

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) override { return JE_OK; }
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override { return JE_OK; }
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override { return JE_OK; }

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) override { return jsValueUndefined; }
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override { return jsValueUndefined; }
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override { return jsValueUndefined; }

    virtual JsValue *getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp = true) override { return nullptr; }
    virtual JsValue *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override { return nullptr; }
    virtual JsValue *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override { return nullptr; }

    virtual bool removeByName(VMContext *ctx, const StringView &name) override { return false; }
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override { return false; }
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override { return false; }

    virtual void changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) override {}
    virtual bool hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) override { return false; }

    virtual IJsObject *clone() override { return new JsDummyObject(); }
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override { return nullptr; }

    virtual void markReferIdx(VMRuntime *rt) override { }

};

#endif /* JsDummyObject_hpp */
