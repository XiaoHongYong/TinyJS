//
//  IJsObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#ifndef IJsObject_hpp
#define IJsObject_hpp

#include "VirtualMachine.hpp"


struct JsProperty {
    JsValue                     value;
    bool                        isGetter;
    bool                        isConfigurable;
    bool                        isEnumerable;
    bool                        isWritable;

    JsProperty(const JsValue &value) : value(value) {
        isGetter = false;
        isConfigurable = true;
        isEnumerable = true;
        isWritable = true;
    }

    JsProperty() : JsProperty(JsUndefinedValue) { }
};

using DequeJsProperties = std::deque<JsProperty>;
using VecJsProperties = std::vector<JsProperty>;
using DequeJsValue = std::deque<JsValue>;

/**
 * 可在 JavaScript 中使用的 Object 接口
 */
class IJsObject {
public:
    IJsObject() {
        type = JDT_NOT_INITIALIZED;
        referIdx = 0;
        nextFreeIdx = 0;
    }
    virtual ~IJsObject() {}

    bool getBool(VMContext *ctx, const JsValue &thiz, const JsValue &prop);

    void defineProperty(VMContext *ctx, const JsValue &prop, const JsProperty &descriptor, const JsValue &setter = JsUndefinedValue);
    bool getOwnPropertyDescriptor(VMContext *ctx, const JsValue &prop, JsProperty &descriptorOut, JsValue &setterOut);

    JsValue get(VMContext *ctx, const JsValue &thiz, const JsValue &prop);
    void set(VMContext *ctx, const JsValue &thiz, const JsValue &prop, const JsValue &value);
    bool remove(VMContext *ctx, const JsValue &prop, const JsValue &value);

    virtual void definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor, const JsValue &setter) = 0;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) = 0;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor, const JsValue &setter) = 0;

    virtual bool getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut, JsValue &setterOut) = 0;
    virtual bool getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) = 0;
    virtual bool getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut, JsValue &setterOut) = 0;

    virtual JsValue getSetterByName(VMContext *ctx, const SizedString &prop) = 0;
    virtual JsValue getSetterByIndex(VMContext *ctx, uint32_t index) = 0;
    virtual JsValue getSetterBySymbol(VMContext *ctx, uint32_t index) = 0;

    virtual JsValue getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop) = 0;
    virtual JsValue getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index) = 0;
    virtual JsValue getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index) = 0;

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) = 0;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) = 0;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) = 0;

    virtual bool removeByName(VMContext *ctx, const SizedString &prop) = 0;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) = 0;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) = 0;

    virtual IJsObject *clone() = 0;
    
public:
    JsDataType                  type;
    int8_t                      referIdx;
    uint32_t                    nextFreeIdx;

};

using MapNameToJsProperty = std::unordered_map<SizedString, JsProperty, SizedStringHash, SizedStrCmpEqual>;
using MapSymbolToJsProperty = std::unordered_map<uint32_t, JsProperty>;

using MapNameToJsValue = std::unordered_map<SizedString, JsValue, SizedStringHash, SizedStrCmpEqual>;
using MapSymbolToJsValue = std::unordered_map<uint32_t, JsValue>;

class JsObject : public IJsObject {
public:
    JsObject(const JsValue &__proto__ = JsNotInitializedValue);
    virtual ~JsObject();

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

    virtual IJsObject *clone() override;

protected:
    friend class JsLibObject;

    JsValue                     __proto__;

    // MapNameToJsProperty 中的 SizedString 需要由 JsObject 自己管理内存.
    MapNameToJsProperty         _props;
    MapNameToJsValue            *_setters;

    MapSymbolToJsProperty       *_symbolProps;
    MapSymbolToJsValue          *_symbolSetters;

};

#endif /* IJsObject_hpp */
