//
//  JsLibObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/2.
//

#ifndef JsLibObject_hpp
#define JsLibObject_hpp

#include "IJsObject.hpp"


struct JsLibProperty {
    SizedString                 name;
    JsNativeFunction            function;
    const char                  *strValue;
    JsValue                     value;

    JsNativeFunction            functionSet;
    JsValue                     valueSetter;

    bool                        isGetter;
    bool                        isConfigurable;
    bool                        isEnumerable;
    bool                        isWritable;

};

class JsLibObject : public IJsObject {
public:
    JsLibObject(VMRuntimeCommon *rt, JsLibProperty *libProps, int countProps, JsNativeFunction function = nullptr);
    JsLibObject(JsLibObject *from);
    ~JsLibObject();

    virtual void definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor, const JsValue &setter) override;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) override;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) override;

    virtual bool getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut, JsValue &setterOut) override;
    virtual bool getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) override;
    virtual bool getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) override;

    virtual JsValue getSetterByName(VMContext *ctx, const SizedString &prop) override;
    virtual JsValue getSetterByIndex(VMContext *ctx, uint32_t index) override;
    virtual JsValue getSetterBySymbol(VMContext *ctx, uint32_t index) override;

    virtual JsValue getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop) override;
    virtual JsValue getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index) override;
    virtual JsValue getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index) override;

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) override;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &prop) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    JsNativeFunction getFunction() const { return _function; }

protected:
    void _newObject();
    void _copyForModify();

protected:
    JsLibObject();

    JsNativeFunction            _function;
    bool                        _modified;
    JsLibProperty               *_libProps, *_libPropsEnd;
    JsObject                    *_obj;

};

JsLibProperty makeJsLibPropertyGetter(const char *name, JsNativeFunction f);

#endif /* JsLibObject_hpp */
