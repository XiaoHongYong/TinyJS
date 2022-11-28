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
using MapNameToJsProperty = std::unordered_map<SizedString, JsProperty, SizedStringHash, SizedStrCmpEqual>;
using MapSymbolToJsProperty = std::unordered_map<uint32_t, JsProperty>;

using MapNameToJsValue = std::unordered_map<SizedString, JsValue, SizedStringHash, SizedStrCmpEqual>;
using MapSymbolToJsValue = std::unordered_map<uint32_t, JsValue>;

class JsObject : public IJsObject {
public:
    JsObject(const JsValue &__proto__ = jsValuePrototypeObject);
    virtual ~JsObject();

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

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

    IJsObject *getPrototypeObject(VMContext *ctx) {
        auto &proto = __proto__.value;
        if (proto.type == JDT_NOT_INITIALIZED) {
            // 缺省的 Object.prototype
            return ctx->runtime->objPrototypeObject;
        } else if (proto.type >= JDT_OBJECT) {
            return ctx->runtime->getObject(proto);
        }

        return nullptr;
    }

protected:
    friend class JsLibObject;
    friend class JsObjectIterator;

    JsProperty                  __proto__;

    // MapNameToJsProperty 中的 SizedString 需要由 JsObject 自己管理内存.
    MapNameToJsProperty         _props;

    MapSymbolToJsProperty       *_symbolProps;

};

#endif /* JsObject_hpp */
