//
//  VMScope.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/17.
//

#include "VMScope.hpp"
#include "VirtualMachine.hpp"
#include "objects/IJsObject.hpp"
#include "BinaryOperation.hpp"
#include "UnaryOperation.hpp"


VMScope::VMScope(Scope *scopeDsc) : scopeDsc(scopeDsc) {
    referIdx = 0;
    nextFreeIdx = 0;

    if (scopeDsc) {
        vars.resize(scopeDsc->countLocalVars, jsValueUndefined.asProperty());
    }
}

void VMScope::free() {
    scopeDsc = nullptr;

    vars.clear();
    vars.shrink_to_fit();

    args.free();
    withValue = jsValueUndefined;
}

VMGlobalScope::VMGlobalScope() : VMScope(nullptr) {
    _rootFunc = PoolNew(_resourcePool.pool, Function)(&_resourcePool, nullptr, 0);
    scopeDsc = _rootFunc->scope;
}

JsValue VMGlobalScope::get(VMContext *ctx, uint32_t index) const {
    assert(index < vars.size());

    return getPropertyValue(ctx, jsValueGlobalThis, vars[index], jsValueEmpty);
}

JsError VMGlobalScope::set(VMContext *ctx, uint32_t index, const JsValue &value) {
    assert(index < vars.size());
    return setPropertyValue(ctx, &vars[index], jsValueGlobalThis, value);
}

JsValue VMGlobalScope::increase(VMContext *ctx, uint32_t index, int inc, bool isPost) {
    assert(index < vars.size());
    return IJsObject::increasePropertyValue(ctx, &vars[index], jsValueGlobalThis, inc, isPost);
}

bool VMGlobalScope::remove(VMContext *ctx, uint32_t index) {
    assert(index < vars.size());
    auto &prop = vars[index];
    if (!prop.isConfigurable()) {
        return false;
    }

    prop = jsValuePropertyDefault;
    return true;
}

void VMGlobalScope::set(const SizedString &name, const JsValue &value) {
    auto id = PoolNew(_resourcePool.pool, IdentifierDeclare)(name, scopeDsc);
    id->storageIndex = scopeDsc->countLocalVars++;
    id->varStorageType = VST_GLOBAL_VAR;
    id->isReferredByChild = true;

    assert(scopeDsc->varDeclares.find(name) == scopeDsc->varDeclares.end());
    scopeDsc->varDeclares[name] = id;

    assert(id->storageIndex == vars.size());
    vars.push_back(value);
}
