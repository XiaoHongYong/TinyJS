//
//  IJsObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#ifndef IJsObject_hpp
#define IJsObject_hpp

#include "VirtualMachine.hpp"


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

    virtual JsValue get(VMContext *ctx, const SizedString &prop) = 0;
    virtual void set(VMContext *ctx, const SizedString &prop, const JsValue &value) = 0;

    virtual JsValue get(VMContext *ctx, uint32_t index) = 0;
    virtual void set(VMContext *ctx, uint32_t index, const JsValue &value) = 0;

public:
    JsDataType                  type;
    int8_t                      referIdx;
    uint32_t                    nextFreeIdx;

};


class JsObject : public IJsObject {
public:
    JsObject(const JsValue &prototype = JsNullValue);
    virtual ~JsObject() {}

    virtual JsValue get(VMContext *ctx, const SizedString &prop) override;
    virtual void set(VMContext *ctx, const SizedString &prop, const JsValue &value) override;

    virtual JsValue get(VMContext *ctx, uint32_t index) override;
    virtual void set(VMContext *ctx, uint32_t index, const JsValue &value) override;

protected:
    friend class JsLibObject;

    JsValue                     prototype;

    MapNameToJsValue            props;

};

class JsArguments : public JsObject {
public:
    JsArguments();
    ~JsArguments();

    virtual JsValue get(VMContext *ctx, const SizedString &prop) override;
    virtual void set(VMContext *ctx, const SizedString &prop, const JsValue &value) override;

    virtual JsValue get(VMContext *ctx, uint32_t index) override;
    virtual void set(VMContext *ctx, uint32_t index, const JsValue &value) override;

protected:
    Arguments                   *args;
    JsObject                    *obj;

};

struct JsLibProperty {
    SizedString                 name;
    JsNativeMemberFunction      function;
    const char                  *strValue;

    JsValue                     value;
};

class JsLibObject : public IJsObject {
public:
    JsLibObject(VMRuntimeCommon *rt, JsLibProperty *libProps, int countProps);
    JsLibObject(JsLibObject *from);

    virtual JsValue get(VMContext *ctx, const SizedString &prop) override;
    virtual void set(VMContext *ctx, const SizedString &prop, const JsValue &value) override;

    virtual JsValue get(VMContext *ctx, uint32_t index) override;
    virtual void set(VMContext *ctx, uint32_t index, const JsValue &value) override;

protected:
    JsLibObject();

    JsLibProperty               *_libProps, *_libPropsEnd;
    JsObject                    *_obj;

};

#endif /* IJsObject_hpp */
