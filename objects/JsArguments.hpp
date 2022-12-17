//
//  JsArguments.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/2.
//

#ifndef JsArguments_hpp
#define JsArguments_hpp

#include "JsObject.hpp"


class JsArguments : public IJsObject {
public:
    JsArguments(VMScope *scope, Arguments *args);
    ~JsArguments();

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

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

    virtual bool getLength(VMContext *ctx, int32_t &lengthOut) override { lengthOut = _args->count; return true; }

    Arguments *getArguments() { return _args; }

    uint32_t length() const { return _args->count; }

protected:
    void _newObject(VMContext *ctx);

    friend class JsArgumentsIterator;

protected:
    VMScope                     *_scope;
    Arguments                   *_args;
    VecJsProperties             *_argsDescriptors;
    JsObject                    *_obj;
    JsValue                  _length;

};

#endif /* JsArguments_hpp */
