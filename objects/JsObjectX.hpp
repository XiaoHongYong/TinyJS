//
//  JsObjectX.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/22.
//

#ifndef JsObjectX_hpp
#define JsObjectX_hpp

#include "JsObject.hpp"


class JsObjectX : public IJsObject {
public:
    JsObjectX(uint32_t extraType, const JsValue &__proto__ = jsValuePrototypeObject);
    ~JsObjectX();

    uint32_t extraType() const { return _extraType; }

    virtual void setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) override;
    virtual void setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) override;
    virtual void setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) override;

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) override;
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsValue *getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp = true) override;
    virtual JsValue *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;
    virtual JsValue *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const StringView &name) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual void changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) override;
    virtual bool hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) override;
    virtual void preventExtensions(VMContext *ctx) override;

    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

    virtual IJsObject *clone() override { assert(0); return nullptr; }

protected:
    void _newObject(VMContext *ctx);

    virtual bool onSetValue(VMContext *ctx, const StringView &name, const JsValue &value) = 0;
    virtual JsValue onGetValue(VMContext *ctx, const StringView &name) = 0;
    virtual void onEnumAllProperties(VMContext *ctx, VecStringViews &names, VecJsValues &values) = 0;

    uint32_t                    _extraType;
    JsObject                    *_obj;
    JsValue                     _tmpRawHolder;

};

#endif /* JsObjectX_hpp */
