//
//  JsObjectFunction.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#include "JsObjectFunction.hpp"


JsObjectFunction::JsObjectFunction(const VecVMStackScopes &stackScopes, Function *function) : JsObjectLazy(_props, CountOf(_props), jsValuePrototypeFunction, JDT_FUNCTION), stackScopes(stackScopes), function(function)
{
    // isGSetter, isConfigurable, isEnumerable, isWritable
    JsLazyProperty props[] = {
        { SS_PROTOTYPE, jsValueUndefined.asProperty(JP_WRITABLE | JP_EMPTY), true },
        { SS_NAME, jsValueEmpty, true, },
        { SS_LENGTH, jsValueLength0Property, false, },
        { SS_CALLER, jsValueNull.asProperty(0), false, },
        { SS_ARGUMENTS, jsValueNull.asProperty(0), false, },
    };

    static_assert(sizeof(props) == sizeof(props));
    memcpy(_props, props, sizeof(props));
}

JsObjectFunction::~JsObjectFunction() {
}

void JsObjectFunction::onInitLazyProperty(VMContext *ctx, JsLazyProperty *prop) {
    auto &name = prop->name;
    assert(prop->isLazyInit);
    prop->isLazyInit = false;

    if (name.equal(SS_PROTOTYPE)) {
        // 为了节省内存分配，延迟初始化 _prototype 属性
        auto proto = new JsObject();
        prop->prop.setValue(ctx->runtime->pushObject(proto));
        proto->setByName(ctx, proto->self, SS_CONSTRUCTOR, self);
    } else if (name.equal(SS_NAME)) {
        prop->prop.setValue(ctx->runtime->pushString(function->name));
    } else {
        assert(0);
    }
}

IJsObject *JsObjectFunction::clone() {
    auto obj = new JsObjectFunction(stackScopes, function);

    obj->__proto__ = __proto__;

    if (_obj) { obj->_obj = (JsObject *)_obj->clone(); }

    return obj;
}

void JsObjectFunction::markReferIdx(VMRuntime *rt) {
    JsObjectLazy::markReferIdx(rt);

    for (auto scope : stackScopes) {
        rt->markReferIdx(scope);
    }

    assert(function != nullptr);
    rt->markReferIdx(function->resourcePool);
}


JsObjectBoundFunction::JsObjectBoundFunction(const JsValue &func, const JsValue &thiz) : JsObjectLazy(_props, CountOf(_props), jsValuePrototypeFunction, JDT_BOUND_FUNCTION), func(func), thiz(thiz)
{
    assert(func.type >= JDT_FUNCTION);

    // isGSetter, isConfigurable, isEnumerable, isWritable
    static JsLazyProperty props[] = {
        { SS_NAME, jsValueEmpty.asProperty(JP_EMPTY), true, },
        { SS_LENGTH, jsValueLength0Property, false, },
    };

    static_assert(sizeof(props) == sizeof(props));
    memcpy(_props, props, sizeof(props));
}

JsObjectBoundFunction::~JsObjectBoundFunction() {
}

void JsObjectBoundFunction::onInitLazyProperty(VMContext *ctx, JsLazyProperty *prop) {
    auto &name = prop->name;
    assert(prop->isLazyInit);
    prop->isLazyInit = false;

    if (name.equal(SS_NAME)) {
        static StringView SS_BOUND_PREFIX("bound ");

        auto value = ctx->vm->getMemberDot(ctx, func, SS_NAME);
        prop->prop.setValue(ctx->runtime->plusString(SS_BOUND_PREFIX, value));
    } else {
        assert(0);
    }
}

IJsObject *JsObjectBoundFunction::clone() {
    auto obj = new JsObjectBoundFunction(func, thiz);

    obj->__proto__ = __proto__;

    if (_obj) { obj->_obj = (JsObject *)_obj->clone(); }

    return obj;
}

void JsObjectBoundFunction::markReferIdx(VMRuntime *rt) {
    JsObjectLazy::markReferIdx(rt);

    rt->markReferIdx(func);
    rt->markReferIdx(thiz);
}

void JsObjectBoundFunction::call(VMContext *ctx, const Arguments &args, JsValue that) {
    auto runtime = ctx->runtime;

    assert(func.type >= JDT_FUNCTION);

    if (func.type == JDT_BOUND_FUNCTION) {
        // 避免调用层次太多，提前展开
        auto *objFunc = (JsObjectBoundFunction *)runtime->getObject(func);

        while (objFunc->func.type == JDT_BOUND_FUNCTION) {
            objFunc = (JsObjectBoundFunction *)runtime->getObject(objFunc->func);
        }

        if (that.type == JDT_UNDEFINED) {
            that = objFunc->thiz;
        }

        ctx->vm->callMember(ctx, that, objFunc->func, args);
    } else {
        if (that.type == JDT_UNDEFINED) {
            that = thiz;
        }

        ctx->vm->callMember(ctx, that, func, args);
    }
}
