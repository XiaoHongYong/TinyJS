//
//  JsPrimaryObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/9/12.
//

#ifndef JsPrimaryObject_hpp
#define JsPrimaryObject_hpp


#include "IJsObject.hpp"
#include <regex>


class JsBooleanObject : public JsObject {
public:
    JsBooleanObject(const JsValue &value) : JsObject(jsValuePrototypeBool), _value(value) {
        type = JDT_OBJ_BOOL;
    }

    virtual void markReferIdx(VMRuntime *rt) override {
        rt->markReferIdx(_value);

        JsObject::markReferIdx(rt);
    }

    JsValue value() { return _value; }

protected:
    JsValue                     _value;

};


class JsNumberObject : public JsObject {
public:
    JsNumberObject(const JsValue &value) : JsObject(jsValuePrototypeNumber), _value(value) {
        type = JDT_OBJ_NUMBER;
    }

    JsValue value() { return _value; }

    virtual void markReferIdx(VMRuntime *rt) override {
        rt->markReferIdx(_value);

        JsObject::markReferIdx(rt);
    }

protected:
    JsValue                     _value;

};


class JsStringObject : public JsObject {
public:
    JsStringObject(const JsValue &value) : JsObject(jsValuePrototypeString), _value(value) {
        type = JDT_OBJ_STRING;
    }

    JsValue value() { return _value; }

    virtual void markReferIdx(VMRuntime *rt) override {
        rt->markReferIdx(_value);

        JsObject::markReferIdx(rt);
    }

protected:
    JsValue                     _value;

};

#endif /* JsPrimaryObject_hpp */
