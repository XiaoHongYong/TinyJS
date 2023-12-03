//
//  PromiseTasks.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/9.
//

#ifndef PromiseTasks_hpp
#define PromiseTasks_hpp

#include "VirtualMachineTypes.hpp"


class JsPromiseObject;

class PromiseTasks {
public:
    struct PromiseTask {
        JsPromiseObject         *promise;

        PromiseTask(JsPromiseObject *promise) : promise(promise) { }
    };

    using ListPromiseTasks = list<PromiseTask>;

public:
    PromiseTasks() { }

    void registerToRunPromise(JsPromiseObject *promise) {
        _toRunPromises.push_back(PromiseTask(promise));
    }

    bool run();
    void markReferIdx(VMRuntime *rt);

protected:
    ListPromiseTasks            _toRunPromises;

};

#endif /* PromiseTasks_hpp */
