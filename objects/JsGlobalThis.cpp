//
//  JsGlobalThis.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/20.
//

#include "JsGlobalThis.hpp"


class JsGlobalThisIterator : public IJsIterator {
public:
    JsGlobalThisIterator(VMContext *ctx, VMGlobalScope *scope, bool includeProtoProp, bool includeNoneEnumerable) : IJsIterator(includeProtoProp, includeNoneEnumerable) {
        _ctx = ctx;
        _scope = scope;
        _scopeDesc = scope->scopeDsc;
    }

    ~JsGlobalThisIterator() {
    }

    virtual bool next(SizedString *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        while (true) {
            if (_it == _scopeDesc->varDeclares.end()) {
                return false;
            }

            auto index = (*_it).second->storageIndex;
            auto prop = &_scope->vars[index];
            if (!prop->isEnumerable() && !_includeNoneEnumerable) {
                ++_it;
                continue;;
            }

            if (valueOut) {
                if (prop->isGetterSetter()) {
                    *valueOut = getPropertyValue(_ctx, jsValueGlobalThis, prop);
                } else {
                    *valueOut = *prop;
                }
            }

            if (strKeyOut) {
                *strKeyOut = (*_it).first;
            }

            if (keyOut) {
                *keyOut = _ctx->runtime->pushString((*_it).first);
            }

            ++_it;
            return true;
        }
    }

protected:
    VMContext                       *_ctx;
    VMGlobalScope                   *_scope;
    Scope                           *_scopeDesc;
    MapNameToIdentifiers::iterator  _it;

};

JsGlobalThis::JsGlobalThis(VMGlobalScope *scope) : IJsObject(jsValuePrototypeWindow, JDT_OBJ_GLOBAL_THIS), _scope(scope)
{
    _scopeDesc = scope->scopeDsc;
    _obj = nullptr;
}

JsGlobalThis::~JsGlobalThis() {
}

void JsGlobalThis::setPropertyByName(VMContext *ctx, const SizedString &name, const JsValue &descriptor) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (!declare) {
        declare = PoolNew(_scopeDesc->function->resourcePool->pool, IdentifierDeclare)(name, _scopeDesc);
        declare->storageIndex = _scopeDesc->countLocalVars++;
        declare->varStorageType = VST_GLOBAL_VAR;
        declare->isReferredByChild = true;

        _scope->vars.resize(_scopeDesc->countLocalVars, jsValueUndefined.asProperty());
    }

    auto index = declare->storageIndex;
    _scope->vars[index] = descriptor;
}

void JsGlobalThis::setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
}

void JsGlobalThis::setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) {
    if (!_obj) {
        _newObject(ctx);
    }

    _obj->setPropertyBySymbol(ctx, index, descriptor);
}

JsError JsGlobalThis::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (declare) {
        return _scope->set(ctx, declare->storageIndex, value);
    }

    // 查找 prototye 的属性
    JsValue *prop = nullptr;
    auto objProto = getPrototypeObject(ctx);
    if (objProto) {
        prop = objProto->getRawByName(ctx, name, true);
    }

    if (prop) {
        if (prop->isGetterSetter()) {
            return setPropertyValueBySetter(ctx, prop, thiz, value);
        } else if (!prop->isWritable()) {
            return JE_TYPE_PROP_READ_ONLY;
        }
    }

    auto index = _newIdentifier(name);

    _scope->vars.resize(_scopeDesc->countLocalVars, jsValueUndefined.asProperty());
    return _scope->set(ctx, index, value);
}

JsError JsGlobalThis::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    // 不支持 index 的访问
    return JE_NOT_SUPPORTED;
}

JsError JsGlobalThis::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue JsGlobalThis::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (declare) {
        return _scope->increase(ctx, declare->storageIndex, n, isPost);
    }

    // 查找 prototye 的属性
    JsValue *prop = nullptr;
    auto objProto = getPrototypeObject(ctx);
    if (objProto) {
        prop = objProto->getRawByName(ctx, name, true);
    }

    if (prop) {
        if (prop->isGetterSetter()) {
            // prototype 带 getter/setter
            return increasePropertyValue(ctx, prop, thiz, n, isPost);
        }

        // 不能修改到 proto，所以先复制到 temp，再添加
        JsValue tmp = *prop;
        auto ret = increasePropertyValue(ctx, &tmp, thiz, n, isPost);

        if (prop->isWritable()) {
            // 添加新属性
            _scope->set(ctx, _newIdentifier(name), tmp);
        }
        return ret;
    }

    auto idx = _newIdentifier(name);
    _scope->vars.resize(_scopeDesc->countLocalVars, jsValueUndefined.asProperty());
    return _scope->increase(ctx, idx, n, isPost);
}

uint32_t JsGlobalThis::_newIdentifier(const SizedString &name) {
    auto &pool = _scopeDesc->function->resourcePool->pool;
    auto nameNew = pool.duplicate(name);
    auto declare = PoolNew(_scopeDesc->function->resourcePool->pool, IdentifierDeclare)(nameNew, _scopeDesc);
    declare->storageIndex = _scopeDesc->countLocalVars++;
    declare->varStorageType = VST_GLOBAL_VAR;
    declare->isReferredByChild = true;

    assert(_scopeDesc->varDeclares.find(nameNew) == _scopeDesc->varDeclares.end());
    _scopeDesc->varDeclares[nameNew] = declare;

    return declare->storageIndex;
}

JsValue JsGlobalThis::increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    return jsValueNaN;
}

JsValue JsGlobalThis::increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) {
    if (!_obj) {
        _newObject(ctx);
    }
    return _obj->increaseBySymbol(ctx, thiz, index, n, isPost);
}

JsValue *JsGlobalThis::getRawByName(VMContext *ctx, const SizedString &name, bool includeProtoProp) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (!declare) {
        if (includeProtoProp) {
            auto objProto = getPrototypeObject(ctx);
            if (objProto) {
                return objProto->getRawByName(ctx, name, includeProtoProp);
            }
        }

        return nullptr;
    }

    auto index = declare->storageIndex;
    return &_scope->vars[index];
}

JsValue *JsGlobalThis::getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp) {
    // 不支持 index 的访问
    return nullptr;
}

JsValue *JsGlobalThis::getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp) {
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
    auto &prop = _scope->vars[index];
    if (prop.isConfigurable()) {
        prop = jsValueUndefined.asProperty();
        return true;
    }

    return false;
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

void JsGlobalThis::changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) {
    ctx->throwException(JE_TYPE_ERROR, "Cannot freeze");
}

void JsGlobalThis::preventExtensions(VMContext *ctx) {
    ctx->throwException(JE_TYPE_ERROR, "Cannot prevent extensions");
}

IJsObject *JsGlobalThis::clone() {
    assert(0 && "");
    return nullptr;
}

IJsIterator *JsGlobalThis::getIteratorObject(VMContext *ctx, bool includeProtoProp, bool includeNoneEnumerable) {
    if (_obj) {
        return _obj->getIteratorObject(ctx, includeProtoProp, includeNoneEnumerable);
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

void JsGlobalThis::_newObject(VMContext *ctx) {
    assert(_obj == nullptr);

    _obj = new JsObject(jsValuePrototypeWindow);

    if (isPreventedExtensions) {
        _obj->preventExtensions(ctx);
    }
}
