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
    virtual void preventExtensions(VMContext *ctx) override;
    virtual bool hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) override { return true; }

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

protected:
    void _newObject(VMContext *ctx);
    uint32_t _newIdentifier(const StringView &name);

protected:
    VMGlobalScope               *_scope;
    Scope                       *_scopeDesc;

    // 仅仅用于处理 Symbol 相关的属性
    JsObject                    *_obj;

};

#endif /* JsGlobalThis_hpp */
