//
//  Promise.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/11/28.
//

#include "BuiltIn.hpp"
#include "objects/JsPrimaryObject.hpp"
#include "objects/JsObjectFunction.hpp"
#include "objects/JsPromiseObject.hpp"


class JsPromiseObject;

JsValue jsValuePrototypePromise;
JsValue jsFuncPromiseFulfill, jsFuncPromiseRejector;


void promise_fulfilledCallback(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(thiz.type == JDT_PROMISE);
    if (thiz.type != JDT_PROMISE) {
        ctx->throwException(JE_TYPE_ERROR, "this shoud be a Promise object.");
        return;
    }

    auto *promise = (JsPromiseObject *)ctx->runtime->getObject(thiz);
    promise->changeStatus(JsPromiseObject::FULFILLED, args.getAt(0));
}

void promise_rejectCallback(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(thiz.type == JDT_PROMISE);
    if (thiz.type != JDT_PROMISE) {
        ctx->throwException(JE_TYPE_ERROR, "this shoud be a Promise object.");
        return;
    }

    auto *promise = (JsPromiseObject *)ctx->runtime->getObject(thiz);
    promise->changeStatus(JsPromiseObject::REJECTED, args.getAt(0));
}

void promiseConstructor(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    if (thiz.isValid()) {
        ctx->throwException(JE_TYPE_ERROR, "Promise constructor cannot be invoked without 'new'");
        return;
    }

    auto executor = args.getAt(0);
    if (!executor.isFunction()) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Promise resolver %.*s is not a function", executor);
        return;
    }

    auto runtime = ctx->runtime;
    auto obj = new JsPromiseObject(ctx);
    auto ret = runtime->pushObject(obj);

    auto objFulfill = new JsObjectBoundFunction(jsFuncPromiseFulfill, ret);
    auto objRejector = new JsObjectBoundFunction(jsFuncPromiseRejector, ret);

    ArgumentsX execArgs(runtime->pushObject(objFulfill), runtime->pushObject(objRejector));
    ctx->vm->callMember(ctx, jsValueGlobalThis, executor, execArgs);

    if (ctx->error != JE_OK) {
        // 异常
        ctx->error = JE_OK;
        obj->changeStatus(JsPromiseObject::REJECTED, ctx->errorMessage);
    }

    ctx->retValue = ret;
}

void promise_all(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void promise_allSettled(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void promise_any(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void promise_race(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(0);
    ctx->retValue = jsValueUndefined;
}

void promise_reject(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = args.getAt(0);
    if (value.type == JDT_PROMISE) {
        ctx->retValue = value;
        return;
    }

    auto runtime = ctx->runtime;

    if (value.type == JDT_OBJECT) {
        auto obj = runtime->getObject(value);
        auto then = obj->getByName(ctx, value, SS_THEN);
        if (then.isFunction()) {
            // 有效的 thenable
            promiseConstructor(ctx, jsValueEmpty, ArgumentsX(then));
            return;
        }
    }

    auto objPromise = new JsPromiseObject(ctx);
    auto ret = runtime->pushObject(objPromise);
    objPromise->changeStatus(JsPromiseObject::REJECTED, value);
    ctx->retValue = ret;
}

void promise_resolve(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    auto value = args.getAt(0);
    if (value.type == JDT_PROMISE) {
        ctx->retValue = value;
        return;
    }

    auto runtime = ctx->runtime;

    if (value.type == JDT_OBJECT) {
        auto obj = runtime->getObject(value);
        auto then = obj->getByName(ctx, value, SS_THEN);
        if (then.isFunction()) {
            // 有效的 thenable
            promiseConstructor(ctx, jsValueEmpty, ArgumentsX(then));
            return;
        }
    }

    auto objPromise = new JsPromiseObject(ctx);
    auto ret = runtime->pushObject(objPromise);
    objPromise->changeStatus(JsPromiseObject::FULFILLED, value);
    ctx->retValue = ret;
}

static JsLibProperty promiseFunctions[] = {
    { "name", nullptr, "Promise" },
    { "length", nullptr, nullptr, jsValueLength1Property },
    { "all", promise_all },
    { "allSettled", promise_allSettled },
    { "any", promise_any },
    { "race", promise_race },
    { "reject", promise_reject },
    { "resolve", promise_resolve },
    { "prototype", nullptr, nullptr, jsValuePropertyPrototype },
};


void promise_prototype_catch(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(thiz.type == JDT_PROMISE);
    if (thiz.type != JDT_PROMISE) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Method Promise.prototype.then called on incompatible receiver %.*s", thiz);
        return;
    }

    auto *promise = (JsPromiseObject *)ctx->runtime->getObject(thiz);
    ctx->retValue = promise->then(jsValueUndefined, args.getAt(0), jsValueUndefined);
}

void promise_prototype_finally(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(thiz.type == JDT_PROMISE);
    if (thiz.type != JDT_PROMISE) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Method Promise.prototype.then called on incompatible receiver %.*s", thiz);
        return;
    }

    auto *promise = (JsPromiseObject *)ctx->runtime->getObject(thiz);
    ctx->retValue = promise->then(jsValueUndefined, jsValueUndefined,  args.getAt(0));
}

void promise_prototype_then(VMContext *ctx, const JsValue &thiz, const Arguments &args) {
    assert(thiz.type == JDT_PROMISE);
    if (thiz.type != JDT_PROMISE) {
        ctx->throwExceptionFormatJsValue(JE_TYPE_ERROR, "Method Promise.prototype.then called on incompatible receiver %.*s", thiz);
        return;
    }

    auto *promise = (JsPromiseObject *)ctx->runtime->getObject(thiz);
    ctx->retValue = promise->then(args.getAt(0), args.getAt(1), jsValueUndefined);
}

static JsLibProperty promisePrototypeFunctions[] = {
    { "catch", promise_prototype_catch },
    { "finally", promise_prototype_finally },
    { "then", promise_prototype_then },
};

void registerPromise(VMRuntimeCommon *rt) {
    auto prototypeObj = new JsLibObject(rt, promisePrototypeFunctions, CountOf(promisePrototypeFunctions));
    jsValuePrototypePromise = rt->pushObject(prototypeObj);

    SET_PROTOTYPE(promiseFunctions, jsValuePrototypePromise);

    jsFuncPromiseFulfill = rt->pushNativeFunction(promise_fulfilledCallback, SS_RESOLVE);
    jsFuncPromiseRejector = rt->pushNativeFunction(promise_rejectCallback, SS_REJECT);

    setGlobalLibObject("Promise", rt, promiseFunctions, CountOf(promiseFunctions), promiseConstructor, jsValuePrototypeFunction);
}
