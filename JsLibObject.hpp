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
    JsProperty                  prop;

    JsNativeFunction            functionSet;

};

/**
 * JsLibObject 用于提供统一封装 JavaScript 内置对象，其包含了：
 * - 对象的构造函数
 * - 对象提供的一些方法
 * - prototye 属性的值（另外一个 JsLibObject)
 * - __proto__ 缺省为 Object.prototype, 但是 Object.prototype.__proto__ 是 null
 */
class JsLibObject : public IJsObject {
private:
    JsLibObject(const JsLibObject &);
    JsLibObject &operator=(const JsLibObject &);

public:
    JsLibObject(VMRuntimeCommon *rt, JsLibProperty *libProps, int countProps, JsNativeFunction function = nullptr);
    JsLibObject(JsLibObject *from);
    ~JsLibObject();

    virtual void definePropertyByName(VMContext *ctx, const SizedString &prop, const JsProperty &descriptor) override;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;

    virtual bool getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &prop, JsProperty &descriptorOut) override;
    virtual bool getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) override;
    virtual bool getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) override;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &prop, bool &isSelfPropOut) override;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool &isSelfPropOut) override;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool &isSelfPropOut) override;

    virtual JsValue getByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &defVal = JsUndefinedValue) override;
    virtual JsValue getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = JsUndefinedValue) override;
    virtual JsValue getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = JsUndefinedValue) override;

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) override;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &prop) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual IJsObject *clone() override;

    void setAsObjectPrototype() { _isObjectPrototype = true; }
    JsNativeFunction getFunction() const { return _function; }

protected:
    virtual void _newObject();
    void _copyForModify();

protected:
    JsLibObject();

    JsNativeFunction            _function;
    bool                        _modified;
    bool                        _isObjectPrototype;
    JsLibProperty               *_libProps, *_libPropsEnd;
    JsObject                    *_obj;

};

JsLibProperty makeJsLibPropertyGetter(const char *name, JsNativeFunction f);

void setPrototype(JsLibProperty *prop, const JsProperty &value);

#define SET_PROTOTYPE(props, prototype)     setPrototype(props + CountOf(props) - 1, prototype)

#endif /* JsLibObject_hpp */
