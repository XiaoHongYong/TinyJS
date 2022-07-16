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
    IJsObject();
    virtual ~IJsObject() {}

    virtual JsValue get(VMContext *ctx, const SizedString &prop);
    virtual void set(VMContext *ctx, const SizedString &prop, const JsValue &value);

public:
    JsDataType                  type;
    int8_t                      referIdx;
    uint32_t                    nextFreeIdx;

    MapNameToJsValue            props;

};

struct JsLibFunction {
    SizedString                 name;
    JsNativeMemberFunction      function;
    JsValue                     value;
};

class JsLibObject : public IJsObject {
public:
    JsLibObject(VMContext *ctx, JsLibFunction *functions, int count);

    virtual JsValue get(VMContext *ctx, const SizedString &prop);

protected:
    JsLibFunction               *_functions, *_functionsEnd;

};

#endif /* IJsObject_hpp */
