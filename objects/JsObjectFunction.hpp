//
//  JsObjectFunction.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#ifndef JsObjectFunction_hpp
#define JsObjectFunction_hpp

#include "JsObjectLazy.hpp"


class JsObjectFunction : public JsObjectLazy {
public:
    JsObjectFunction(const VecVMStackScopes &stackScopes, Function *function);
    ~JsObjectFunction();

    virtual IJsObject *clone() override;

    virtual void markReferIdx(VMRuntime *rt) override;

protected:
    virtual void onInitLazyProperty(VMContext *ctx, JsLazyProperty *prop) override;

public:
    VecVMStackScopes            stackScopes;

    // 当前函数的代码
    Function                    *function;

protected:
    JsLazyProperty              _props[5];

};


class JsObjectBoundFunction : public JsObjectLazy {
public:
    JsObjectBoundFunction(const JsValue &func, const JsValue &thiz);
    ~JsObjectBoundFunction();

    virtual IJsObject *clone() override;
    virtual void markReferIdx(VMRuntime *rt) override;

    void call(VMContext *ctx, const Arguments &args, JsValue that = jsValueUndefined);

protected:
    virtual void onInitLazyProperty(VMContext *ctx, JsLazyProperty *prop) override;

public:
    JsValue                     func, thiz;

protected:
    JsLazyProperty              _props[2];

};

#endif /* JsObjectFunction_hpp */
