//
//  JsGlobalThis.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/20.
//

#include "JsGlobalThis.hpp"


class JsGlobalThisIterator : public IJsIterator {
public:
    JsGlobalThisIterator(VMContext *ctx, VMGlobalScope *scope, bool includeProtoProp) {
        _ctx = ctx;
        _scope = scope;
        _scopeDesc = scope->scopeDsc;
        _itObj = nullptr;
        _includeProtoProp = includeProtoProp;
    }

    ~JsGlobalThisIterator() {
        if (_itObj) {
            delete _itObj;
        }
    }

    virtual bool next(SizedString *strKeyOut = nullptr, JsValue *keyOut = nullptr, JsValue *valueOut = nullptr) override {
        while (true) {
            if (_it == _scopeDesc->varDeclares.end()) {
                return false;
            }

            auto index = (*_it).second->storageIndex;
            if (index < _scope->varProperties.size()) {
                auto prop = &_scope->varProperties[index];
                if (!prop->isEnumerable) {
                    ++_it;
                    continue;;
                }

                if (valueOut) {
                    if (prop->isGSetter) {
                        prop->value = _scope->vars[index];
                        if (prop->value.type >= JDT_FUNCTION) {
                            _ctx->vm->callMember(_ctx, jsValueGlobalThis, prop->value, Arguments());
                            *valueOut = _ctx->retValue;
                        } else {
                            *valueOut = jsValueUndefined;
                        }
                    } else {
                        *valueOut = _scope->vars[index];
                    }
                }
            } else {
                if (valueOut) {
                    *valueOut = _scope->vars[index];
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

    IJsIterator                     *_itObj;
    bool                            _includeProtoProp;

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
        _newObject(ctx);
    }

    _obj->definePropertyBySymbol(ctx, index, descriptor);
}

void JsGlobalThis::setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (declare) {
        _scope->set(ctx, declare->storageIndex, value);
        return;
    }

    // 查找 prototye 的属性
    JsProperty *prop = nullptr;
    JsNativeFunction funcGetter = nullptr;
    auto objProto = getPrototypeObject(ctx);
    if (objProto) {
        prop = objProto->getRawByName(ctx, name, funcGetter, true);
    }

    if (prop) {
        if (prop->isGSetter) {
            if (prop->setter.isValid()) {
                // prototype 带 setter 的可以直接返回用于修改调用
                set(ctx, prop, thiz, value);
            }
            return;
        } else if (!prop->isWritable) {
            return;
        }
    }

    auto index = _newIdentifier(name);

    _scope->vars.resize(_scopeDesc->countLocalVars, jsValueUndefined);
    _scope->set(ctx, index, value);
}

void JsGlobalThis::setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    // 不支持 index 的访问
}

void JsGlobalThis::setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) {
    if (!_obj) {
        _newObject(ctx);
    }
    _obj->setBySymbol(ctx, thiz, index, value);
}

JsValue JsGlobalThis::increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (declare) {
        return _scope->increase(ctx, declare->storageIndex, n, isPost);
    }

    // 查找 prototye 的属性
    JsProperty *prop = nullptr;
    JsNativeFunction funcGetter = nullptr;
    auto objProto = getPrototypeObject(ctx);
    if (objProto) {
        prop = objProto->getRawByName(ctx, name, funcGetter, true);
    }

    if (prop) {
        if (prop->isGSetter) {
            // prototype 带 getter/setter
            return increase(ctx, prop, thiz, n, isPost);
        }

        // 不能修改到 proto，所以先复制到 temp，再添加
        JsProperty tmp = *prop;
        auto ret = increase(ctx, &tmp, thiz, n, isPost);

        if (prop->isWritable) {
            // 添加新属性
            _scope->set(ctx, _newIdentifier(name), tmp.value);
        }
        return ret;
    }

    auto idx = _newIdentifier(name);
    _scope->vars.resize(_scopeDesc->countLocalVars, jsValueUndefined);
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

JsProperty *JsGlobalThis::getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp) {
    auto declare = _scopeDesc->getVarDeclarationByName(name);
    if (!declare) {
        if (includeProtoProp) {
            auto objProto = getPrototypeObject(ctx);
            if (objProto) {
                return objProto->getRawByName(ctx, name, funcGetterOut, includeProtoProp);
            }
        }

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

void JsGlobalThis::changeAllProperties(VMContext *ctx, int8_t configurable, int8_t writable) {
    ctx->throwException(PE_TYPE_ERROR, "Cannot freeze");
}

void JsGlobalThis::preventExtensions(VMContext *ctx) {
    ctx->throwException(PE_TYPE_ERROR, "Cannot prevent extensions");
}

IJsObject *JsGlobalThis::clone() {
    assert(0 && "");
    return nullptr;
}

IJsIterator *JsGlobalThis::getIteratorObject(VMContext *ctx, bool includeProtoProp) {
    if (_obj) {
        return _obj->getIteratorObject(ctx, includeProtoProp);
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
