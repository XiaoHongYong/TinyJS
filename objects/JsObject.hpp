//
//  JsObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#ifndef JsObject_hpp
#define JsObject_hpp

#include "IJsObject.hpp"
#include "IJsIterator.hpp"


// 使用 unordered_map 可以同时使用 erase 和 iterator：不会 crash，但是不保证能够完全遍历所有的 key.
using MapNameToJsProperty = std::unordered_map<StringView, JsValue, StringViewHash, SizedStrCmpEqual>;
using MapSymbolToJsProperty = std::unordered_map<uint32_t, JsValue>;

using MapNameToJsValue = std::unordered_map<StringView, JsValue, StringViewHash, SizedStrCmpEqual>;
using MapSymbolToJsValue = std::unordered_map<uint32_t, JsValue>;

class JsObject : public IJsObject {
public:
    JsObject(const JsValue &__proto__ = jsValuePrototypeObject);
    virtual ~JsObject();

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

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

protected:
    friend class JsLibObject;
    friend class JsObjectIterator;

    // MapNameToJsProperty 中的 StringView 需要由 JsObject 自己管理内存.
    MapNameToJsProperty         _props;

    MapSymbolToJsProperty       *_symbolProps;

};

#endif /* JsObject_hpp */
