//
//  PromiseTasks.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/9.
//

#include "PromiseTasks.hpp"
#include "VirtualMachine.hpp"
#include "objects/JsPromiseObject.hpp"


bool PromiseTasks::run() {
    while (!_toRunPromises.empty()) {
        auto toRuns = _toRunPromises;
        _toRunPromises.clear();

        for (auto &task : toRuns) {
            task.promise->onRun();
        }
    }

    return false;
}

void PromiseTasks::markReferIdx(VMRuntime *rt) {
    for (auto &task : _toRunPromises) {
        ::markReferIdx(rt, task.promise);
    }
}
