//
//  Function.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#include "BuiltIn.hpp"
#include "objects/JsObjectFunction.hpp"
#include "objects/JsArray.hpp"
#include "objects/JsArguments.hpp"


// https://developer.mozilla.org/zh-CN/docs/Web/JavaScript/Reference/Global_Objects/Function
static void functionConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    // TODO: 需要校验参数，避免这样的构造可以通过 Function('a', 'b){}, function f(a', 'a');
    string functionStr = "(function(";
    for (int i = 0; i < (int)args.count - 1; i++) {
        if (i != 0) {
            functionStr.append(",");
        }

        auto s = runtime->toStringViewStrictly(ctx, args[i]);
        functionStr.append((char *)s.data, s.len);
    }
    functionStr.append("){");
    if (args.count > 0) {
        auto s = runtime->toStringViewStrictly(ctx, args[args.count - 1]);
        functionStr.append((char *)s.data, s.len);
    }
    functionStr.append("})");

    VecVMStackScopes stackScopes;

    stackScopes.push_back(runtime->globalScope());
    ctx->curFunctionScope = runtime->globalScope();

    // eval 会将返回值存放于 retValue 中
    ctx->vm->eval(functionStr.c_str(), functionStr.size(), ctx, stackScopes, Arguments());
}

static JsLibProperty functionFunctions[] = {
    { "name", nullptr, "Function" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};

void functionPrototypeApply(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    auto that = args.getAt(0);
    auto argArray = args.getAt(1);

    if (argArray.type == JDT_ARRAY) {
        auto arrObj = (JsArray *)runtime->getObject(argArray);
        VecJsValues arr;
        arrObj->dump(ctx, argArray, arr);

        Arguments subArgs(arr.data(), (uint32_t)arr.size());
        ctx->vm->callMember(ctx, that, thiz, subArgs);
    } else if (argArray.type == JDT_ARGUMENTS) {
        auto argObj = (JsArguments *)runtime->getObject(argArray);
        ctx->vm->callMember(ctx, that, thiz, *argObj->getArguments());
    } else {
        ctx->throwException(JE_TYPE_ERROR, "Arguments array must be an array-like object.");
    }
}

void functionPrototypeBind(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto runtime = ctx->runtime;

    // thiz 是 function, args[0] 是 this
    if (thiz.type >= JDT_FUNCTION) {
        if (thiz.type == JDT_LIB_OBJECT) {
            auto obj = (JsLibObject *)runtime->getObject(thiz);
            if (!obj->getFunction()) {
                ctx->throwException(JE_TYPE_ERROR, "Bind must be called on a function");
                return;
            }
        }
    } else {
        ctx->throwException(JE_TYPE_ERROR, "Bind must be called on a function");
        return;
    }

    auto that = args.getAt(0);

    auto obj = new JsObjectBoundFunction(thiz, that);
    ctx->retValue = runtime->pushObject(obj);
}

void functionPrototypeCall(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto that = args.getAt(0);

    Arguments subArgs(args.data + 1, args.count > 1 ? args.count - 1 : 0);
    ctx->vm->callMember(ctx, that, thiz, subArgs);
}


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
        auto &name = runtime->getNativeFunctionName(thiz.value.index);
        auto str = stringPrintf("function %.*s() { [native code] }", name.len, name.data);
        ctx->retValue = runtime->pushString(str);
        return;
    } else if (thiz.type == JDT_LIB_OBJECT) {
        auto obj = (JsLibObject *)runtime->getObject(thiz);
        if (obj->getFunction()) {
            auto name = obj->getName();
            auto str = stringPrintf("function %.*s() { [native code] }", name.len, name.data);
            ctx->retValue = runtime->pushString(str);
            return;
        }
    }

    ctx->throwException(JE_TYPE_ERROR, "Function.prototype.toString requires that 'this' be a Function");
}

static JsLibProperty functionPrototypeFunctions[] = {
    { "name", nullptr, "" },
    { "length", nullptr, nullptr, jsValueLength0Property },
    { "apply", functionPrototypeApply },
    { "bind", functionPrototypeBind },
    { "call", functionPrototypeCall },
    { "toString", functionPrototypeToString },
};

void registerObjFunction(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, functionPrototypeFunctions, CountOf(functionPrototypeFunctions));
    rt->objPrototypeFunction = prototypeObj;
    rt->setPrototypeObject(jsValuePrototypeFunction, prototypeObj);

    SET_PROTOTYPE(functionFunctions, jsValuePrototypeFunction);

    setGlobalLibObject("Function", rt, functionFunctions, CountOf(functionFunctions), functionConstructor, jsValuePrototypeFunction);
}
