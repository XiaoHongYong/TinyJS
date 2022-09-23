//
//  JsGlobalThis.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/20.
//

#ifndef JsGlobalThis_hpp
#define JsGlobalThis_hpp

#include "IJsObject.hpp"


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

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) override;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp = true) override;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &name) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true) override;

    virtual void markReferIdx(VMRuntime *rt) override;

protected:
    void _newObject();

protected:
    VMGlobalScope               *_scope;
    Scope                       *_scopeDesc;

    JsObject                    *_obj;

};

#endif /* JsGlobalThis_hpp */
