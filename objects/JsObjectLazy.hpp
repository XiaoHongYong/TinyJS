//
//  JsObjectLazy.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/11.
//

#ifndef JsObjectLazy_hpp
#define JsObjectLazy_hpp

#include "JsObject.hpp"


struct JsLazyProperty {
    SizedString             name;
    JsValue              prop;
    bool                    isLazyInit;
};

class JsObjectLazy : public IJsObject {
public:
    JsObjectLazy(JsLazyProperty *props, uint32_t countProps, const JsValue &__proto__, JsDataType type);
    ~JsObjectLazy();

    virtual void setPropertyByName(VMContext *ctx, const SizedString &name, const JsValue &descriptor) override;
    virtual void setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) override;
    virtual void setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) override;

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) override;
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsValue *getRawByName(VMContext *ctx, const SizedString &name, bool includeProtoProp = true) override;
    virtual JsValue *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;
    virtual JsValue *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &name) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual void changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) override;
    virtual bool hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) override;
    virtual void preventExtensions(VMContext *ctx) override;

    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

protected:
    void _newObject(VMContext *ctx);

    void setProperties(JsLazyProperty *props, uint32_t countProps) {
        _props = props;
        _propsEnd = props + countProps;
    }

    virtual void onInitLazyProperty(VMContext *ctx, JsLazyProperty *prop) { assert(0); }

    JsLazyProperty              *_props, *_propsEnd;
    uint32_t                    _countProps;

    JsObject                    *_obj;

};

#endif /* JsObjectLazy_hpp */
