//
//  JsLibObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/2.
//

#ifndef JsLibObject_hpp
#define JsLibObject_hpp

#include "JsObject.hpp"


struct JsLibProperty {
    StringView                 name;
    JsNativeFunction            function;
    const char                  *strValue;
    JsValue                     prop;

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
    JsLibObject(VMRuntimeCommon *rt, JsLibProperty *libProps, int countProps, cstr_t name = nullptr, JsNativeFunction function = nullptr, const JsValue &proto = jsValuePrototypeObject);
    JsLibObject(JsLibObject *from);
    ~JsLibObject();

    virtual void setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) override;
    virtual void setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) override;
    virtual void setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) override;

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) override;
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsValue *getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp = true) override;
    virtual JsValue *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;
    virtual JsValue *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const StringView &name) override;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) override;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) override;

    virtual void changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) override;
    virtual bool hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) override;
    virtual void preventExtensions(VMContext *ctx) override;

    virtual IJsObject *clone() override;
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) override;

    virtual void markReferIdx(VMRuntime *rt) override;

    const StringView &getName() const { return _name; }
    JsNativeFunction getFunction() const { return _constructor; }
    void setOfIteratorTrue() { _isOfIterable = true; }

    bool isModified() const { return _modified || _obj; }

protected:
    virtual void _newObject(VMContext *ctx);
    JsLibProperty *_copyForModify(JsLibProperty *pos);

protected:
    JsLibObject();

    StringView                 _name;
    JsNativeFunction            _constructor;
    bool                        _modified;
    JsLibProperty               *_libProps, *_libPropsEnd;
    JsObject                    *_obj;

};

JsLibProperty makeJsLibPropertyGetter(const char *name, JsNativeFunction f);

void setPrototype(JsLibProperty *prop, JsLibProperty *propEnd, const JsValue &value);

#define SET_PROTOTYPE(props, prototype)     setPrototype(props, props + CountOf(props), prototype)

JsLibObject *setGlobalLibObject(cstr_t name, VMRuntimeCommon *rt, JsLibProperty *libProps, int countProps, JsNativeFunction function = nullptr, const JsValue &proto = jsValuePrototypeObject);

#endif /* JsLibObject_hpp */
