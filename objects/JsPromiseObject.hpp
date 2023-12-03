//
//  JsPromiseObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/17.
//

#ifndef JsPromiseObject_hpp
#define JsPromiseObject_hpp

#include "JsObjectLazy.hpp"


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

public:
    JsPromiseObject(VMContext *ctx);

    virtual IJsObject *clone() override;
    virtual void markReferIdx(VMRuntime *rt) override;

    void changeStatus(Status status, const JsValue &arg);
    void onRun();

    JsValue then(const JsValue &fulfilledCallback, const JsValue &rejectedCallback, const JsValue &finallyCallback, JsPromiseObject *nextPromiseObj = nullptr);

protected:
    VMContext                   *_ctx;
    Status                      _status;
    JsValue                     _fulfillRejectArg;

    VecPromiseChain             _chainPromises;

};

#endif /* JsPromiseObject_hpp */
