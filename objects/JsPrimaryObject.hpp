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

using JsBooleanObject = JsPrimaryObject_<JS_OBJ_PROTOTYPE_IDX_BOOL, JDT_OBJ_BOOL>;
using JsNumberObject = JsPrimaryObject_<JS_OBJ_PROTOTYPE_IDX_NUMBER, JDT_OBJ_NUMBER>;
using JsStringObject = JsPrimaryObject_<JS_OBJ_PROTOTYPE_IDX_STRING, JDT_OBJ_STRING>;

#endif /* JsPrimaryObject_hpp */
