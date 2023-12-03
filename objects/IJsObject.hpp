//
//  IJsObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#ifndef IJsObject_hpp
#define IJsObject_hpp

#include "interpreter/VirtualMachine.hpp"


inline JsValue getPropertyValue(VMContext *ctx, JsValue thiz, const JsValue *prop, JsValue defVal = jsValueUndefined) {
    if (prop->isGetterSetter()) {
        auto &gs = ctx->runtime->getGetterSetter(*prop);
        if (gs.getter.isFunction()) {
            ctx->vm->callMember(ctx, thiz, gs.getter, Arguments());
            return ctx->retValue;
        }

        return jsValueUndefined;
    } else if (prop->isEmpty()) {
        return defVal;
    } else {
        return *prop;
    }
}

inline JsValue getPropertyValue(VMContext *ctx, JsValue thiz, const JsValue &prop, JsValue defVal = jsValueUndefined) {
    return getPropertyValue(ctx, thiz, &prop, defVal);
}

inline JsError setPropertyValueBySetter(VMContext *ctx, JsValue *prop, const JsValue &thiz, const JsValue &value) {
    assert(prop->isGetterSetter());
    auto &gs = ctx->runtime->getGetterSetter(*prop);
    if (gs.setter.isFunction()) {
        // 调用 setter 函数
        ArgumentsX args(value);
        ctx->vm->callMember(ctx, thiz, gs.setter, args);
        return JE_OK;
    }

    return JE_TYPE_NO_PROP_SETTER;
}

inline JsError setPropertyValue(VMContext *ctx, JsValue *prop, const JsValue &thiz, const JsValue &value) {
    assert(prop);
    // 自身的属性，可以直接修改
    if (prop->isGetterSetter()) {
        return setPropertyValueBySetter(ctx, prop, thiz, value);
    } else if (prop->isEmpty()) {
        *prop = value.asProperty();
        return JE_OK;
    } else if (prop->isWritable()) {
        prop->setValue(value);
        return JE_OK;
    }

    return JE_TYPE_PROP_READ_ONLY;
}

/**
 * 可在 JavaScript 中使用的 Object 接口
 */
class IJsObject {
public:
    IJsObject(JsValue proto, JsDataType type);
    virtual ~IJsObject() {}

    bool getBool(VMContext *ctx, const JsValue &thiz, const StringView &name);
    bool getBool(VMContext *ctx, const JsValue &thiz, const JsValue &name);

    void setProperty(VMContext *ctx, const JsValue &name, const JsValue &descriptor);
    bool getOwnPropertyDescriptor(VMContext *ctx, const JsValue &name, JsValue &descriptorOut);

    JsValue get(VMContext *ctx, const JsValue &thiz, const JsValue &name, const JsValue &defVal = jsValueUndefined);
    JsValue *getRaw(VMContext *ctx, const JsValue &name, bool includeProtoProp = true);
     void set(VMContext *ctx, const JsValue &thiz, const JsValue &name, const JsValue &value);
    JsValue increase(VMContext *ctx, const JsValue &thiz, const JsValue &name, int n, bool isPost);
    bool remove(VMContext *ctx, const JsValue &name);

    virtual void setPropertyByName(VMContext *ctx, const StringView &name, const JsValue &descriptor) = 0;
    virtual void setPropertyByIndex(VMContext *ctx, uint32_t index, const JsValue &descriptor) = 0;
    virtual void setPropertyBySymbol(VMContext *ctx, uint32_t index, const JsValue &descriptor) = 0;

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &value) = 0;
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) = 0;
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) = 0;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const StringView &name, int n, bool isPost) = 0;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) = 0;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) = 0;

    virtual JsValue *getRawByName(VMContext *ctx, const StringView &name, bool includeProtoProp = true) = 0;
    virtual JsValue *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) = 0;
    virtual JsValue *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) = 0;

    virtual bool removeByName(VMContext *ctx, const StringView &name) = 0;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) = 0;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) = 0;

    virtual void changeAllProperties(VMContext *ctx, JsPropertyFlags toAdd, JsPropertyFlags toRemove) = 0;
    virtual bool hasAnyProperty(VMContext *ctx, JsPropertyFlags flags) = 0;
    virtual void preventExtensions(VMContext *ctx) { isPreventedExtensions = true; }
    virtual bool isExtensible() { return !isPreventedExtensions; }

    virtual IJsObject *clone() = 0;

    virtual bool isOfIterable() { return _isOfIterable; }
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true, bool includeNoneEnumerable = false) = 0;

    virtual void markReferIdx(VMRuntime *rt) = 0;

    virtual bool getLength(VMContext *ctx, int32_t &lengthOut);

    void addGetterSetterByName(VMContext *ctx, const StringView &name, const JsValue &getter, const JsValue &setter);

    IJsObject *getPrototypeObject(VMContext *ctx) {
        if (__proto__.isEmpty()) {
            // 缺省的 Object.prototype
            return ctx->runtime->objPrototypeObject();
        } else if (__proto__.type >= JDT_OBJECT) {
            return ctx->runtime->getObject(__proto__);
        }

        return nullptr;
    }

    bool getOwnPropertyDescriptorByName(VMContext *ctx, const StringView &name, JsValue &descriptorOut) {
        auto prop = getRawByName(ctx, name, false);
        if (prop) {
            descriptorOut = *prop;
            return true;
        }
        return false;
    }

    bool getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsValue &descriptorOut) {
        auto prop = getRawByIndex(ctx, index, false);
        if (prop) {
            descriptorOut = *prop;
            return true;
        }
        return false;
    }

    bool getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsValue &descriptorOut) {
        auto prop = getRawBySymbol(ctx, index, false);
        if (prop) {
            descriptorOut = *prop;
            return true;
        }
        return false;
    }

    JsValue getByName(VMContext *ctx, const JsValue &thiz, const StringView &name, const JsValue &defVal = jsValueUndefined) {
        auto prop = getRawByName(ctx, name, true);
        if (prop) {
            return getPropertyValue(ctx, thiz, prop, defVal);
        }
        return defVal;
    }

    JsValue getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = jsValueUndefined) {
        auto prop = getRawByIndex(ctx, index, true);
        if (prop) {
            return getPropertyValue(ctx, thiz, prop, defVal);
        }
        return defVal;
    }

    JsValue getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = jsValueUndefined) {
        auto prop = getRawBySymbol(ctx, index, true);
        if (prop) {
            return getPropertyValue(ctx, thiz, prop, defVal);
        }
        return defVal;
    }

public:
    static inline JsValue increase(VMContext *ctx, JsValue &v, int x) {
        if (v.type == JDT_INT32) {
            int64_t n = v.value.n32 + (int64_t)x;
            if (n == (int32_t)n) {
                return makeJsValueInt32((int32_t)n);
            }
            return ctx->runtime->pushDouble(n);
        }

        double d = NAN;
        if (!ctx->runtime->toNumber(ctx, v, d)) {
            d = NAN;
        }
        if (isnan(d)) {
            v = jsValueNaN;
            return jsValueNaN;
        }

        if ((int32_t)d == d) {
            v = makeJsValueInt32((int32_t)d);
        } else {
            v = JsValue(JDT_NUMBER, d);
        }

        d += x;
        int32_t n = (int32_t)d;
        if (n == d) {
            return makeJsValueInt32(n);
        }
        return JsValue(JDT_NUMBER, d);
    }

    static JsValue increasePropertyValue(VMContext *ctx, JsValue *prop, const JsValue &thiz, int n, bool isPost) {
        JsValue newValue;
        if (prop->isGetterSetter()) {
            auto &gs = ctx->runtime->getGetterSetter(*prop);
            if (gs.getter.isFunction()) {
                ctx->vm->callMember(ctx, thiz, gs.getter, Arguments());

                auto org = ctx->retValue;
                newValue = increase(ctx, org, n);
                if (gs.setter.isFunction()) {
                    // 有 setter，修改.
                    ctx->vm->callMember(ctx, thiz, gs.setter, ArgumentsX(newValue));
                }

                return isPost ? org : newValue;
            } else {
                return jsValueNaN;
            }
        } else {
            auto org = *prop;
            newValue = increase(ctx, org, n);
            if (prop->isWritable()) {
                prop->setValue(newValue);
            }

            return isPost ? org : newValue;
        }
    }

public:
    JsDataType                  type;
    bool                        isPreventedExtensions;
    bool                        _isOfIterable;

    int8_t                      referIdx;
    uint32_t                    nextFreeIdx;

    JsValue                     __proto__;

    // self 是 "this" 放在 runtime->_objValues 后对应的 JsValue.
    JsValue                     self;

};

inline void markReferIdx(VMRuntime *rt, IJsObject *obj) {
    if (obj->referIdx != rt->nextReferIdx()) {
        obj->referIdx = rt->nextReferIdx();
        obj->markReferIdx(rt);
    }
}

#endif /* IJsObject_hpp */
