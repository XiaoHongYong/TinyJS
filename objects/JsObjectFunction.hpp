//
//  JsObjectFunction.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#ifndef JsObjectFunction_hpp
#define JsObjectFunction_hpp

#include "JsObject.hpp"


class JsObjectFunction : public IJsObject {
public:
    JsObjectFunction(const VecVMStackScopes &stackScopes, Function *function);
    ~JsObjectFunction();

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

    virtual void changeAllProperties(VMContext *ctx, int8_t configurable = -1, int8_t writable = -1) override;
    virtual bool hasAnyProperty(VMContext *ctx, bool configurable, bool writable) override;
    virtual void preventExtensions(VMContext *ctx) override;

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true) override;

    virtual void markReferIdx(VMRuntime *rt) override;

public:
    VecVMStackScopes            stackScopes;

    // 当前函数的代码
    Function                    *function;

protected:
    virtual void _newObject(VMContext *ctx);

    JsProperty                  _prototype;
    JsProperty                  _name;
    JsProperty                  _length;
    JsProperty                  _caller;
    JsProperty                  _arguments;

    JsProperty                  __proto__;

    JsObject                    *_obj;

};

#endif /* JsObjectFunction_hpp */
