//
//  Function.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#include "BuiltIn.hpp"
#include "../JsObjectFunction.hpp"


// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Function
static void functionConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;
    
    // TODO: 需要校验参数，避免这样的构造可以通过 Function('a', 'b){}, function f(a', 'a');
    string functionStr = "(function(";
    for (int i = 0; i < (int)args.count - 1; i++) {
        if (i != 0) {
            functionStr.append(",");
        }

        string buf;
        auto s = runtime->toSizedString(ctx, args[i], buf);
        functionStr.append((char *)s.data, s.len);
    }
    functionStr.append("){");
    if (args.count > 0) {
        string buf;
        auto s = runtime->toSizedString(ctx, args[args.count - 1], buf);
        functionStr.append((char *)s.data, s.len);
    }
    functionStr.append("})");

    VecVMStackScopes stackScopes;

    stackScopes.push_back(runtime->globalScope);
    ctx->curFunctionScope = runtime->globalScope;

    // eval 会将返回值存放于 retValue 中
    runtime->vm->eval(functionStr.c_str(), functionStr.size(), ctx, stackScopes, Arguments());
}

static JsLibProperty functionFunctions[] = {
    { "name", nullptr, "Function" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
};

void functionPrototypeToString(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    if (thiz.type == JDT_FUNCTION) {
        auto f = (JsObjectFunction *)runtime->getObject(thiz);
        assert(f);
        auto function = f->function;
        if (function->srcCodeValue.type <= JDT_NULL) {
            auto resourcePool = function->resourcePool;
            assert(resourcePool->strings.size() < 0xffff);
            auto idx = (uint16_t)resourcePool->strings.size();
            resourcePool->strings.push_back(function->srcCode);
            function->srcCodeValue = makeJsValueOfStringInResourcePool(resourcePool->index, idx);
        }
        ctx->retValue = function->srcCodeValue;
        return;
    } else if (thiz.type == JDT_NATIVE_FUNCTION) {
        ctx->retValue = JsStringValueFunctionNativeCode;
        return;
    } else if (thiz.type == JDT_LIB_OBJECT) {
        auto obj = (JsLibObject *)runtime->getObject(thiz);
        if (obj->getFunction()) {
            ctx->retValue = JsStringValueFunctionNativeCode;
            return;
        }
    }

    ctx->throwException(PE_TYPE_ERROR, "Function.prototype.toString requires that 'this' be a Function");
}

static JsLibProperty functionPrototypeFunctions[] = {
    { "toString", functionPrototypeToString },
};

void registerObjFunction(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, functionPrototypeFunctions, CountOf(functionPrototypeFunctions));
    rt->objPrototypeFunction = prototypeObj;
    auto prototype = rt->pushObjValue(JDT_LIB_OBJECT, prototypeObj);
    assert(prototype == jsValuePrototypeFunction);

    SET_PROTOTYPE(functionFunctions, prototype);

    rt->setGlobalObject("Function",
        new JsLibObject(rt, functionFunctions, CountOf(functionFunctions), functionConstructor));
}
