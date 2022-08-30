//
//  JsObjectFunction.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#ifndef JsObjectFunction_hpp
#define JsObjectFunction_hpp

#include "IJsObject.hpp"


class JsObjectFunction : public IJsObject {
public:
    JsObjectFunction(const VecVMStackScopes &stackScopes, Function *function);
    ~JsObjectFunction();

    virtual void definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor) override;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;

    virtual bool getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut) override;
    virtual bool getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) override;
    virtual bool getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) override;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &prop, bool &isSelfPropOut) override;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool &isSelfPropOut) override;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool &isSelfPropOut) override;

    virtual JsValue getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &defVal = JsUndefinedValue) override;
    virtual JsValue getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = JsUndefinedValue) override;
    virtual JsValue getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = JsUndefinedValue) override;

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) override;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &prop) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual IJsObject *clone() override;

public:
    VecVMStackScopes            stackScopes;

    // 当前函数的代码
    Function                    *function;

protected:
    virtual void _newObject();

    JsValue                     _prototype;
    JsValue                     __proto__;
    JsObject                    *_obj__Proto__;

    JsObject                    *_obj;

};

#endif /* JsObjectFunction_hpp */
