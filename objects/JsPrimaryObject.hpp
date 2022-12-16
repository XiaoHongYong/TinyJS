//
//  JsPrimaryObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/12.
//

#ifndef JsPrimaryObject_hpp
#define JsPrimaryObject_hpp


#include "JsObjectLazy.hpp"
#include <regex>


template<int protoIndex_, JsDataType type_>
class JsPrimaryObject_ : public JsObjectLazy {
public:
    JsPrimaryObject_(const JsValue &value) : JsObjectLazy(nullptr, 0, JsValue(JDT_LIB_OBJECT, protoIndex_)), _value(value) {
        type = type_;
    }

    virtual void markReferIdx(VMRuntime *rt) override {
        rt->markReferIdx(_value);

        JsObjectLazy::markReferIdx(rt);
    }

    JsValue value() { return _value; }

    virtual IJsObject *clone() override {
        return new JsPrimaryObject_<protoIndex_, type_>(_value);
    }

protected:
    JsValue                     _value;

};

class JsStringObject : public JsObjectLazy {
public:
    JsStringObject(const JsValue &value);

    virtual void definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) override;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) override;
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp = true) override;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &name) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;

    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override {
        rt->markReferIdx(_value);

        JsObjectLazy::markReferIdx(rt);
    }

    JsValue value() { return _value; }

    virtual IJsObject *clone() override {
        return new JsStringObject(_value);
    }

protected:
    void _updateLength(VMContext *ctx);

    friend class JsStringIterator;

    uint32_t                    _length;
    JsValue                     _value;

};

using JsBooleanObject = JsPrimaryObject_<JS_OBJ_PROTOTYPE_IDX_BOOL, JDT_OBJ_BOOL>;
using JsNumberObject = JsPrimaryObject_<JS_OBJ_PROTOTYPE_IDX_NUMBER, JDT_OBJ_NUMBER>;
using JsSymbolObject = JsPrimaryObject_<JS_OBJ_PROTOTYPE_IDX_SYMBOL, JDT_OBJ_SYMBOL>;

#endif /* JsPrimaryObject_hpp */
