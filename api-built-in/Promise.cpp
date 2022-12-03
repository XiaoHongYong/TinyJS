//
//  Promise.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/11/28.
//

#include "BuiltIn.hpp"
#include "objects/JsPrimaryObject.hpp"
#include "objects/JsObjectFunction.hpp"


class JsPromiseObject;

JsValue jsValuePrototypePromise;
JsValue jsFuncPromiseFulfill, jsFuncPromiseRejector;


class PromiseContextTask : public VmTask {
public:
    struct PromiseTask {
        JsPromiseObject         *promise;

        PromiseTask(JsPromiseObject *promise) : promise(promise)
        {
        }
    };

    using ListPromiseTasks = list<PromiseTask>;

public:
    PromiseContextTask() { }

    void registerToRunPromise(JsPromiseObject *promise) {
        _toRunPromises.push_back(PromiseTask(promise));
    }

    virtual bool run() override;
    virtual void markReferIdx(VMRuntime *rt) override;

protected:
    ListPromiseTasks            _toRunPromises;

};

using PromiseContextTaskPtr = shared_ptr<PromiseContextTask>;

PromiseContextTaskPtr _promiseCtxTaskPtr;


class JsPromiseObject : public JsObjectLazy {
public:
    enum Status {
        PENDING,
        FULFILLED,
        REJECTED,
        FINALLY,
    };

    struct PromiseChain {
        JsValue                 funcFulfilled, funcRejected, funcFinally;
        JsPromiseObject         *nextPromise;
    };
    using VecPromiseChain = vector<PromiseChain>;

    JsPromiseObject(VMContext *ctx) : JsObjectLazy(nullptr, 0, jsValuePrototypePromise), _ctx(ctx) {
        type = JDT_PROMISE;
        _status = PENDING;
    }

    virtual IJsObject *clone() override {
        assert(0);
        return nullptr;
    }

    virtual void markReferIdx(VMRuntime *rt) override {
        rt->markReferIdx(_fulfillRejectArg);

        for (auto &item : _chainPromises) {
            if (item.funcFulfilled.isValid()) rt->markReferIdx(item.funcFulfilled);
            if (item.funcRejected.isValid()) rt->markReferIdx(item.funcRejected);
            if (item.funcFinally.isValid()) rt->markReferIdx(item.funcFinally);

            if (item.nextPromise->referIdx != rt->nextReferIdx()) {
                item.nextPromise->referIdx = rt->nextReferIdx();
                item.nextPromise->markReferIdx(rt);
            }
        }
    }

    void changeStatus(Status status, const JsValue &arg) {
        if (_status != PENDING) {
            return;
        }

        if (arg.type == JDT_PROMISE) {
            // 如果 回掉函数返回是另外一个 Promise，则需要将当前的 promise 关联返回的 promise
            auto srcPromise = (JsPromiseObject *)_ctx->runtime->getObject(arg);
            srcPromise->then(jsValueUndefined, jsValueUndefined, jsValueUndefined, this);
            return;
        }

        _promiseCtxTaskPtr->registerToRunPromise(this);

        _fulfillRejectArg = arg;
        _status = status;
    }

    void onRun() {
        assert(_status != PENDING);
        if (_chainPromises.empty()) {
            // 未处理的都需要抛出异常
            if (_status == REJECTED) {
                _ctx->throwExceptionFormatJsValue(JE_ERROR, "(in promise) %.*s", _fulfillRejectArg);
            }
            return;
        }

        auto toRuns = _chainPromises;
        _chainPromises.clear();
        JsValue callbackArg[FINALLY + 1] = { jsValueUndefined, _fulfillRejectArg, _fulfillRejectArg, jsValueUndefined };

        for (auto callback : toRuns) {
            auto func = _status == FULFILLED ? callback.funcFulfilled : callback.funcRejected;
            ArgumentsX args(callbackArg[_status]);
            if (!func.isFunction()) {
                func = callback.funcFinally;
                args.count = 0;
            }

            Status status = _status;
            JsValue arg = _fulfillRejectArg;

            if (func.isFunction()) {
                _ctx->vm->callMember(_ctx, jsValueGlobalThis, func, args);
                if (_ctx->error == JE_OK) {
                    if (!callback.funcFinally.isFunction()) {
                        // 不是 finally.
                        status = FULFILLED;
                        arg = _ctx->retValue;

                        if (arg.type == JDT_PROMISE) {
                            // 如果 回掉函数返回是另外一个 Promise，则需要将当前的 promise 关联返回的 promise
                            auto srcPromise = (JsPromiseObject *)_ctx->runtime->getObject(arg);
                            srcPromise->then(jsValueUndefined, jsValueUndefined, jsValueUndefined, callback.nextPromise);
                            continue;
                        }
                    }
                } else {
                    _ctx->error = JE_OK;
                    status = REJECTED;
                    arg = _ctx->errorMessage;
                }
            }

            // 传递到下一级
            callback.nextPromise->changeStatus(status, arg);
        }
    }

    JsValue then(const JsValue &fulfilledCallback, const JsValue &rejectedCallback, const JsValue &finallyCallback, JsPromiseObject *nextPromiseObj = nullptr)
    {
        JsValue nextPromise;
        if (nextPromiseObj == nullptr) {
            nextPromiseObj = new JsPromiseObject(_ctx);
            nextPromise = _ctx->runtime->pushObjectValue(nextPromiseObj);
        } else {
            nextPromise = nextPromiseObj->self;
        }

        PromiseChain chain = { jsValueUndefined, jsValueUndefined, jsValueUndefined, nextPromiseObj };
        if (fulfilledCallback.isFunction()) chain.funcFulfilled = fulfilledCallback;
        if (rejectedCallback.isFunction()) chain.funcRejected = rejectedCallback;
        if (finallyCallback.isFunction()) chain.funcFinally = finallyCallback;
        _chainPromises.push_back(chain);

        return nextPromise;
    }

protected:
    VMContext                   *_ctx;
    Status                      _status;
    JsValue                     _fulfillRejectArg;

    VecPromiseChain             _chainPromises;

};

bool PromiseContextTask::run() {
    while (!_toRunPromises.empty()) {
        auto toRuns = _toRunPromises;
        _toRunPromises.clear();

        for (auto &task : toRuns) {
            task.promise->onRun();
        }
    }

    return false;
}

void PromiseContextTask::markReferIdx(VMRuntime *rt) {
    for (auto &task : _toRunPromises) {
        ::markReferIdx(rt, task.promise);
    }
}

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
    if (thiz.type != JDT_NOT_INITIALIZED) {
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
    auto ret = runtime->pushObjectValue(obj);

    auto objFulfill = new JsObjectBoundFunction(jsFuncPromiseFulfill, ret);
    auto objRejector = new JsObjectBoundFunction(jsFuncPromiseRejector, ret);

    ArgumentsX execArgs(runtime->pushObjectValue(objFulfill), runtime->pushObjectValue(objRejector));
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
            promiseConstructor(ctx, jsValueNotInitialized, ArgumentsX(then));
            return;
        }
    }

    auto objPromise = new JsPromiseObject(ctx);
    auto ret = runtime->pushObjectValue(objPromise);
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
            promiseConstructor(ctx, jsValueNotInitialized, ArgumentsX(then));
            return;
        }
    }

    auto objPromise = new JsPromiseObject(ctx);
    auto ret = runtime->pushObjectValue(objPromise);
    objPromise->changeStatus(JsPromiseObject::FULFILLED, value);
    ctx->retValue = ret;
}

static JsLibProperty promiseFunctions[] = {
    { "name", nullptr, "Promise" },
    { "length", nullptr, nullptr, JsValue(JDT_INT32, 1) },
    { "all", promise_all },
    { "allSettled", promise_allSettled },
    { "any", promise_any },
    { "race", promise_race },
    { "reject", promise_reject },
    { "resolve", promise_resolve },
    { "prototype", nullptr, nullptr, JsValue(JDT_INT32, 1) },
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
    jsValuePrototypePromise = rt->pushObjectValue(prototypeObj);

    SET_PROTOTYPE(promiseFunctions, jsValuePrototypePromise);

    jsFuncPromiseFulfill = rt->pushNativeFunction(promise_fulfilledCallback, SS_RESOLVE);
    jsFuncPromiseRejector = rt->pushNativeFunction(promise_rejectCallback, SS_REJECT);

    _promiseCtxTaskPtr = make_shared<PromiseContextTask>();
    rt->vm->registerTask(_promiseCtxTaskPtr);

    setGlobalLibObject("Promise", rt, promiseFunctions, CountOf(promiseFunctions), promiseConstructor, jsValuePrototypeFunction);
}
