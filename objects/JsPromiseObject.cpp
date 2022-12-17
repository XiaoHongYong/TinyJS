//
//  JsPromiseObject.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/17.
//

#include "JsPromiseObject.hpp"


extern JsValue jsValuePrototypePromise;

JsPromiseObject::JsPromiseObject(VMContext *ctx) : JsObjectLazy(nullptr, 0, jsValuePrototypePromise, JDT_PROMISE), _ctx(ctx)
{
    _status = PENDING;
}


IJsObject *JsPromiseObject::clone() {
    assert(0);
    return nullptr;
}

void JsPromiseObject::markReferIdx(VMRuntime *rt) {
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

void JsPromiseObject::changeStatus(Status status, const JsValue &arg) {
    if (_status != PENDING) {
        return;
    }

    if (arg.type == JDT_PROMISE) {
        // 如果 回掉函数返回是另外一个 Promise，则需要将当前的 promise 关联返回的 promise
        auto srcPromise = (JsPromiseObject *)_ctx->runtime->getObject(arg);
        srcPromise->then(jsValueUndefined, jsValueUndefined, jsValueUndefined, this);
        return;
    }

    _ctx->vm->registerToRunPromise(this);

    _fulfillRejectArg = arg;
    _status = status;
}

void JsPromiseObject::onRun() {
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

JsValue JsPromiseObject::then(const JsValue &fulfilledCallback, const JsValue &rejectedCallback, const JsValue &finallyCallback, JsPromiseObject *nextPromiseObj)
{
    JsValue nextPromise;
    if (nextPromiseObj == nullptr) {
        nextPromiseObj = new JsPromiseObject(_ctx);
        nextPromise = _ctx->runtime->pushObject(nextPromiseObj);
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
