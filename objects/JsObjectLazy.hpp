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
    JsProperty              prop;
    bool                    isLazyInit;
};

class JsObjectLazy : public IJsObject {
public:
    JsObjectLazy(JsLazyProperty *props, uint32_t countProps, const JsValue &__proto__ = jsValuePrototypeObject);
    ~JsObjectLazy();

    virtual void definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) override;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) override;
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp = true) override;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &name) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual void changeAllProperties(VMContext *ctx, int8_t configurable = -1, int8_t writable = -1) override;
    virtual bool hasAnyProperty(VMContext *ctx, bool configurable, bool writable) override;
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

    JsProperty                  __proto__;

    JsObject                    *_obj;

};

#endif /* JsObjectLazy_hpp */
