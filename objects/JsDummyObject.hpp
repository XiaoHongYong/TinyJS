//
//  JsDummyObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#ifndef JsDummyObject_hpp
#define JsDummyObject_hpp

#include "JsDummyObject.hpp"

/**
 * 提供一个空的 IJsObject，用于占位
 */
class JsDummyObject : public IJsObject {
public:
    JsDummyObject() { }
    virtual ~JsDummyObject() { }

    virtual void definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) override { }
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override { }
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override { }

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) override { return JE_OK; }
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override { return JE_OK; }
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override { return JE_OK; }

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) override { return jsValueUndefined; }
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override { return jsValueUndefined; }
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override { return jsValueUndefined; }

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp = true) override { return nullptr; }
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override { return nullptr; }
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override { return nullptr; }

    virtual bool removeByName(VMContext *ctx, const SizedString &name) override { return false; }
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override { return false; }
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override { return false; }

    virtual void changeAllProperties(VMContext *ctx, int8_t configurable = -1, int8_t writable = -1) override {}
    virtual bool hasAnyProperty(VMContext *ctx, bool configurable, bool writable) override { return false; }

    virtual IJsObject *clone() override { return new JsDummyObject(); }
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override { return nullptr; }

    virtual void markReferIdx(VMRuntime *rt) override { }

};

#endif /* JsDummyObject_hpp */
