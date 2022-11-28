//
//  JsGlobalThis.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/20.
//

#ifndef JsGlobalThis_hpp
#define JsGlobalThis_hpp

#include "JsObject.hpp"


class VMGlobalScope;

/**
 * https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Global_Objects/globalThis
 * 实现
 */
class JsGlobalThis : public IJsObject {
private:
    JsGlobalThis(const JsGlobalThis &);
    JsGlobalThis &operator=(const JsGlobalThis &);

public:
    JsGlobalThis(VMGlobalScope *globalScope);
    ~JsGlobalThis();

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
    virtual void preventExtensions(VMContext *ctx) override;
    virtual bool hasAnyProperty(VMContext *ctx, bool configurable, bool writable) override { return true; }

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

protected:
    void _newObject(VMContext *ctx);
    IJsObject *getPrototypeObject(VMContext *ctx) { return ctx->runtime->objPrototypeWindow; }
    uint32_t _newIdentifier(const SizedString &name);

protected:
    VMGlobalScope               *_scope;
    Scope                       *_scopeDesc;

    // 仅仅用于处理 Symbol 相关的属性
    JsObject                    *_obj;

};

#endif /* JsGlobalThis_hpp */
