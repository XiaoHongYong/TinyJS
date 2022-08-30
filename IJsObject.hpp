//
//  IJsObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#ifndef IJsObject_hpp
#define IJsObject_hpp

#include "VirtualMachine.hpp"


using DequeJsProperties = std::deque<JsProperty>;
using VecJsProperties = std::vector<JsProperty>;
using DequeJsValue = std::deque<JsValue>;

class IJsIterator {
public:
    virtual ~IJsIterator() {}

    virtual bool nextKey(SizedString &keyOut) = 0;
    virtual bool nextKey(JsValue &keyOut) = 0;
    virtual bool nextValue(JsValue &valueOut) = 0;
    virtual bool next(JsValue &keyOut, JsValue &valueOut) = 0;
    virtual bool next(SizedString &keyOut, JsValue &valueOut) = 0;

};

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

    bool getBool(VMContext *ctx, const JsValue &thiz, const SizedString &name);
    bool getBool(VMContext *ctx, const JsValue &thiz, const JsValue &prop);

    void defineProperty(VMContext *ctx, const JsValue &prop, const JsProperty &descriptor);
    bool getOwnPropertyDescriptor(VMContext *ctx, const JsValue &prop, JsProperty &descriptorOut);

    JsValue get(VMContext *ctx, const JsValue &thiz, const JsValue &prop, const JsValue &defVal = jsValueUndefined);
    void set(VMContext *ctx, const JsValue &thiz, const JsValue &prop, const JsValue &value);
    bool remove(VMContext *ctx, const JsValue &prop);

    virtual void definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor) = 0;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) = 0;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) = 0;

    virtual bool getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut) = 0;
    virtual bool getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) = 0;
    virtual bool getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) = 0;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &prop, bool &isSelfPropOut) = 0;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool &isSelfPropOut) = 0;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool &isSelfPropOut) = 0;

    virtual JsValue getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &defVal = jsValueUndefined) = 0;
    virtual JsValue getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = jsValueUndefined) = 0;
    virtual JsValue getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = jsValueUndefined) = 0;

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) = 0;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) = 0;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) = 0;

    virtual bool removeByName(VMContext *ctx, const SizedString &prop) = 0;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) = 0;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) = 0;

    virtual IJsObject *clone() = 0;

    virtual IJsIterator *getIteratorObject(VMContext *ctx) = 0;
    virtual JsValue getIterator(VMContext *ctx);

public:
    JsDataType                  type;
    int8_t                      referIdx;
    uint32_t                    nextFreeIdx;

    // self 是 "this" 放在 runtime->objValues 后对应的 JsValue.
    JsValue                     self;

};

// 使用 unordered_map 可以同时使用 erase 和 iterator：不会 crash，但是不保证能够完全遍历所有的 key.
using MapNameToJsProperty = std::unordered_map<SizedString, JsProperty, SizedStringHash, SizedStrCmpEqual>;
using MapSymbolToJsProperty = std::unordered_map<uint32_t, JsProperty>;

using MapNameToJsValue = std::unordered_map<SizedString, JsValue, SizedStringHash, SizedStrCmpEqual>;
using MapSymbolToJsValue = std::unordered_map<uint32_t, JsValue>;

class JsObject : public IJsObject {
public:
    JsObject(const JsValue &__proto__ = jsValuePrototypeObject);
    virtual ~JsObject();

    virtual void definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor) override;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;

    virtual bool getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut) override;
    virtual bool getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) override;
    virtual bool getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) override;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &prop, bool &isSelfPropOut) override;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool &isSelfPropOut) override;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool &isSelfPropOut) override;

    virtual JsValue getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &defVal = jsValueUndefined) override;
    virtual JsValue getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = jsValueUndefined) override;
    virtual JsValue getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = jsValueUndefined) override;

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) override;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &prop) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual IJsObject *clone() override;

    virtual IJsIterator *getIteratorObject(VMContext *ctx) override;

protected:
    friend class JsLibObject;
    friend class JsObjectIterator;

    JsProperty                  __proto__;

    // MapNameToJsProperty 中的 SizedString 需要由 JsObject 自己管理内存.
    MapNameToJsProperty         _props;

    MapSymbolToJsProperty       *_symbolProps;

};

void mergeJsProperty(VMContext *ctx, JsProperty *dst, const JsProperty &src, const SizedString &name);
void mergeSymbolJsProperty(VMContext *ctx, JsProperty *dst, const JsProperty &src, uint32_t index);
void mergeIndexJsProperty(VMContext *ctx, JsProperty *dst, const JsProperty &src, uint32_t index);

#endif /* IJsObject_hpp */
