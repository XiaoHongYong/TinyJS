//
//  JsGlobalThis.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/20.
//

#include "JsGlobalThis.hpp"


class JsGlobalThisIterator : public IJsIterator {
public:
    JsGlobalThisIterator(VMContext *ctx, VMGlobalScope *scope) {
        _ctx = ctx;
        _scope = scope;
        _scopeDesc = scope->scopeDsc;
        _itObj = nullptr;
    }

    ~JsGlobalThisIterator() {
        if (_itObj) {
            delete _itObj;
        }
    }

    virtual bool nextKey(SizedString &keyOut) override {
//        if (_itObj) {
//            return _itObj->nextKey(keyOut);
//        }
        if (_it == _scopeDesc->varDeclares.end()) {
            return false;
        }

        keyOut = (*_it).first;
        ++_it;
        return true;
    }

    virtual bool nextKey(JsValue &keyOut) override {
        if (_it == _scopeDesc->varDeclares.end()) {
            return false;
        }

        keyOut = _ctx->runtime->pushString((*_it).first);
        ++_it;
        return true;
    }

    virtual bool nextValue(JsValue &valueOut) override {
        if (_it == _scopeDesc->varDeclares.end()) {
            return false;
        }

        auto id = (*_it).second;
        ++_it;

        valueOut = _scope->vars[id->storageIndex];
        return true;
    }

    virtual bool next(JsValue &keyOut, JsValue &valueOut) override {
        if (_it == _scopeDesc->varDeclares.end()) {
            return false;
        }

        auto id = (*_it).second;

        keyOut = _ctx->runtime->pushString((*_it).first);
        valueOut = _scope->vars[id->storageIndex];
        ++_it;
        return true;
    }

    virtual bool next(SizedString &keyOut, JsValue &valueOut) override {
        if (_it == _scopeDesc->varDeclares.end()) {
            return false;
        }

        auto id = (*_it).second;

        keyOut = (*_it).first;
        valueOut = _scope->vars[id->storageIndex];
        ++_it;
        return true;
    }

protected:
    VMContext                       *_ctx;
    VMGlobalScope                   *_scope;
    Scope                           *_scopeDesc;
    MapNameToIdentifiers::iterator  _it;

    IJsIterator                     *_itObj;

};

JsGlobalThis::JsGlobalThis(VMGlobalScope *scope) : _scope(scope) {
    type = JDT_OBJ_GLOBAL_THIS;
    _scopeDesc = scope->scopeDsc;
    _obj = nullptr;
}

JsGlobalThis::~JsGlobalThis() {
}

void JsGlobalThis::definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (!declare) {
        declare = PoolNew(_scopeDesc->function->resourcePool->pool, IdentifierDeclare)(name, _scopeDesc);
        declare->storageIndex = _scopeDesc->countLocalVars++;
        declare->varStorageType = VST_GLOBAL_VAR;
        declare->isReferredByChild = true;

        _scope->vars.resize(_scopeDesc->countLocalVars, jsValueUndefined);
    }

    auto index = declare->storageIndex;

    if (_scopeDesc->countLocalVars > _scope->varProperties.size()) {
        if (_scope->varProperties.empty()) {
            _scope->varProperties.resize(ctx->runtime->countImmutableGlobalVars,
                                         JsProperty(jsValueUndefined, false, true, true, true));
        }

        bool notDefined = index >= _scope->varProperties.size();
        _scope->varProperties.resize(_scopeDesc->countLocalVars, JsProperty(jsValueUndefined, false, true, true, true));
        if (notDefined) {
            _scope->varProperties[index] = descriptor;
            _scope->vars[index] = descriptor.value;
            return;
        }
    }

    _scope->varProperties[index].value = _scope->vars[index];
    defineNameProperty(ctx, &_scope->varProperties[index], descriptor, name);
    _scope->vars[index] = _scope->varProperties[index].value;
}

void JsGlobalThis::definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
}

void JsGlobalThis::definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) {
    if (!_obj) {
        _newObject();
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor);
}

void JsGlobalThis::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (!declare) {
        declare = PoolNew(_scopeDesc->function->resourcePool->pool, IdentifierDeclare)(name, _scopeDesc);
        declare->storageIndex = _scopeDesc->countLocalVars++;
        declare->varStorageType = VST_GLOBAL_VAR;
        declare->isReferredByChild = true;

        _scope->vars.resize(_scopeDesc->countLocalVars, jsValueUndefined);
    }

    _scope->set(ctx, declare->storageIndex, value);
}

void JsGlobalThis::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    // 不支持 index 的访问
}

void JsGlobalThis::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject();
    }
    _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue JsGlobalThis::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (!declare) {
        auto &pool = _scopeDesc->function->resourcePool->pool;
        auto nameNew = pool.duplicate(name);
        declare = PoolNew(_scopeDesc->function->resourcePool->pool, IdentifierDeclare)(nameNew, _scopeDesc);
        declare->storageIndex = _scopeDesc->countLocalVars++;
        declare->varStorageType = VST_GLOBAL_VAR;
        declare->isReferredByChild = true;
        _scopeDesc->varDeclares[nameNew] = declare;

        _scope->vars.resize(_scopeDesc->countLocalVars, jsValueUndefined);
    }

    return _scope->increase(ctx, declare->storageIndex, n, isPost);
}

JsValue JsGlobalThis::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    return jsValueNaN;
}

JsValue JsGlobalThis::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject();
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsProperty *JsGlobalThis::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (!declare) {
        return nullptr;
    }

    auto index = declare->storageIndex;
    if (index >= _scope->varProperties.size()) {
        static JsProperty prop;
        // 暂时存在这里
        prop = JsProperty(_scope->vars[index], false, true, true, true);
        if (!prop.value.isValid()) {
            prop.value = jsValueUndefined;
        }
        return &prop;
    }

    _scope->varProperties[index].value = _scope->vars[index];
    return &_scope->varProperties[index];
}

JsProperty *JsGlobalThis::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    // 不支持 index 的访问
    return nullptr;
}

JsProperty *JsGlobalThis::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    if (_obj) {
        return _obj->getRawBySymbol(ctx, index, includeProtoProp);
    }

    return nullptr;
}

bool JsGlobalThis::removeByName(VMContext *ctx, const SizedString &name) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (!declare) {
        return true;
    }

    auto index = declare->storageIndex;
    if (index < ctx->runtime->countImmutableGlobalVars) {
        return false;
    } else if (index < _scope->varProperties.size()) {
        auto &prop = _scope->varProperties[index];
        if (!prop.isConfigurable) {
            return false;
        }

        prop = JsProperty(jsValueUndefined);
        _scope->vars[index] = jsValueUndefined;
    } else {
        _scope->vars[index] = jsValueUndefined;
    }

    return true;
}

bool JsGlobalThis::removeByIndex(VMContext *ctx, uint32_t index) {
    // 不支持 index 的访问
    return true;
}

bool JsGlobalThis::removeBySymbol(VMContext *ctx, uint32_t index) {
    if (_obj) {
        return _obj->removeBySymbol(ctx, index);
    }

    return true;
}

IJsObject *JsGlobalThis::clone() {
    assert(0 && "");
    return nullptr;
}

IJsIterator *JsGlobalThis::getIteratorObject(VMContext *ctx) {
    if (_obj) {
        return _obj->getIteratorObject(ctx);
    }

    return new EmptyJsIterator();
}

void JsGlobalThis::markReferIdx(VMRuntime *rt) {
    assert(referIdx == rt->nextReferIdx());

    if (_obj) {
        _obj->referIdx = rt->nextReferIdx();
        _obj->markReferIdx(rt);
    }
}

void JsGlobalThis::_newObject() {
    assert(_obj == nullptr);

    _obj = new JsObject();
}
