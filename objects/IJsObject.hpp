//
//  IJsObject.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/11.
//

#ifndef IJsObject_hpp
#define IJsObject_hpp

#include "VirtualMachine.hpp"


inline JsValue getPropertyValue(VMContext *ctx, const JsProperty *prop, JsValue thiz, JsValue defVal = jsValueUndefined) {
    if (prop->isGSetter && prop->value.type >= JDT_FUNCTION) {
        ctx->vm->callMember(ctx, thiz, prop->value, Arguments());
        return ctx->retValue;
    } else if (prop->value.type == JDT_NOT_INITIALIZED) {
        return defVal;
    } else {
        return prop->value;
    }
}

inline JsValue getPropertyValue(VMContext *ctx, const JsProperty &prop, JsValue thiz, JsValue defVal = jsValueUndefined) {
    if (prop.isGSetter && prop.value.type >= JDT_FUNCTION) {
        ctx->vm->callMember(ctx, thiz, prop.value, Arguments());
        return ctx->retValue;
    } else if (prop.value.type == JDT_NOT_INITIALIZED) {
        return defVal;
    } else {
        return prop.value;
    }
}

/**
 * 可在 JavaScript 中使用的 Object 接口
 */
class IJsObject {
public:
    IJsObject() {
        type = JDT_NOT_INITIALIZED;
        isPreventedExtensions = false;
        _isOfIterable = false;
        referIdx = 0;
        nextFreeIdx = 0;
    }
    virtual ~IJsObject() {}

    bool getBool(VMContext *ctx, const JsValue &thiz, const SizedString &name);
    bool getBool(VMContext *ctx, const JsValue &thiz, const JsValue &name);

    void defineProperty(VMContext *ctx, const JsValue &name, const JsProperty &descriptor);
    bool getOwnPropertyDescriptor(VMContext *ctx, const JsValue &name, JsProperty &descriptorOut);

    JsValue get(VMContext *ctx, const JsValue &thiz, const JsValue &name, const JsValue &defVal = jsValueUndefined);
    JsProperty *getRaw(VMContext *ctx, const JsValue &name, JsNativeFunction &funcGetterOut, bool includeProtoProp = true);
     void set(VMContext *ctx, const JsValue &thiz, const JsValue &name, const JsValue &value);
    JsValue increase(VMContext *ctx, const JsValue &thiz, const JsValue &name, int n, bool isPost);
    bool remove(VMContext *ctx, const JsValue &name);

    virtual void definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) = 0;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) = 0;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) = 0;

    virtual JsError setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) = 0;
    virtual JsError setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) = 0;
    virtual JsError setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) = 0;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) = 0;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) = 0;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) = 0;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp = true) = 0;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) = 0;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) = 0;

    virtual bool removeByName(VMContext *ctx, const SizedString &name) = 0;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) = 0;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) = 0;

    virtual void changeAllProperties(VMContext *ctx, int8_t configurable = -1, int8_t writable = -1) = 0;
    virtual bool hasAnyProperty(VMContext *ctx, bool configurable, bool writable) = 0;
    virtual void preventExtensions(VMContext *ctx) { isPreventedExtensions = true; }
    virtual bool isExtensible() { return !isPreventedExtensions; }

    virtual IJsObject *clone() = 0;

    virtual bool isOfIterable() { return _isOfIterable; }
    virtual IJsIterator *getIteratorObject(VMContext *ctx, bool includeProtoProp = true) = 0;

    virtual void markReferIdx(VMRuntime *rt) = 0;

    virtual bool getLength(VMContext *ctx, int32_t &lengthOut);

    bool getOwnPropertyDescriptorByName(VMContext *ctx, const SizedString &name, JsProperty &descriptorOut) {
        JsNativeFunction funcGetter = nullptr;
        auto prop = getRawByName(ctx, name, funcGetter, false);
        if (prop) {
            descriptorOut = *prop;
            return true;
        }
        return false;
    }

    bool getOwnPropertyDescriptorByIndex(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) {
        auto prop = getRawByIndex(ctx, index, false);
        if (prop) {
            descriptorOut = *prop;
            return true;
        }
        return false;
    }

    bool getOwnPropertyDescriptorBySymbol(VMContext *ctx, uint32_t index, JsProperty &descriptorOut) {
        auto prop = getRawBySymbol(ctx, index, false);
        if (prop) {
            descriptorOut = *prop;
            return true;
        }
        return false;
    }

    JsValue getByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &defVal = jsValueUndefined) {
        JsNativeFunction funcGetter = nullptr;
        auto prop = getRawByName(ctx, name, funcGetter, true);
        if (prop) {
            if (prop->isGSetter) {
                if (funcGetter) {
                    funcGetter(ctx, thiz, Arguments());
                } else if (prop->value.type >= JDT_FUNCTION) {
                    ctx->vm->callMember(ctx, thiz, prop->value, Arguments());
                } else {
                    return jsValueUndefined;
                }
                return ctx->retValue;
            }
            return prop->value;
        }
        return defVal;
    }

    JsValue getByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = jsValueUndefined) {
        auto prop = getRawByIndex(ctx, index, true);
        if (prop) {
            return getPropertyValue(ctx, prop, thiz, defVal);
        }
        return defVal;
    }

    JsValue getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = jsValueUndefined) {
        auto prop = getRawBySymbol(ctx, index, true);
        if (prop) {
            return getPropertyValue(ctx, prop, thiz, defVal);
        }
        return defVal;
    }

protected:
    inline void defineNameProperty(VMContext *ctx, JsProperty *prop, const JsProperty &descriptor, const SizedString &name) {
        assert(prop);
        // 自身的属性，可以直接修改
        if (!prop->merge(descriptor)) {
            ctx->throwException(PE_TYPE_ERROR, "Cannot redefine property: %.*s", (int)name.len, name.data);
        }
    }

    inline void defineIndexProperty(VMContext *ctx, JsProperty *prop, const JsProperty &descriptor, uint32_t index) {
        assert(prop);
        // 自身的属性，可以直接修改
        if (!prop->merge(descriptor)) {
            ctx->throwException(PE_TYPE_ERROR, "Cannot redefine property: %d", index);
        }
    }

    inline void defineSymbolProperty(VMContext *ctx, JsProperty *prop, const JsProperty &descriptor, uint32_t index) {
        assert(prop);
        // 自身的属性，可以直接修改
        if (!prop->merge(descriptor)) {
            auto name = ctx->runtime->toSizedString(ctx, JsValue(JDT_SYMBOL, index));
            ctx->throwException(PE_TYPE_ERROR, "Cannot redefine property: %.*s", (int)name.len, name.data);
        }
    }

    inline JsError set(VMContext *ctx, JsProperty *prop, const JsValue &thiz, const JsValue &value) {
        assert(prop);
        // 自身的属性，可以直接修改
        if (prop->isGSetter) {
            if (prop->setter.type > JDT_OBJECT) {
                // 调用 setter 函数
                ArgumentsX args(value);
                ctx->vm->callMember(ctx, thiz, prop->setter, args);
                return JE_OK;
            }
            return JE_TYPE_NO_PROP_SETTER;
        } else if (prop->isWritable) {
            prop->value = value;
            return JE_OK;
        }

        return JE_TYPE_PROP_READ_ONLY;
    }

public:
    static inline JsValue increase(VMContext *ctx, JsValue &v, int x) {
        if (v.type == JDT_INT32) {
            int64_t n = v.value.n32 + (int64_t)x;
            if (n == (int32_t)n) {
                return JsValue(JDT_INT32, (int32_t)n);
            }
            return ctx->runtime->pushDoubleValue(n);
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
            v = JsValue(JDT_INT32, (int32_t)d);
        } else {
            v = JsValue(JDT_NUMBER, d);
        }

        d += x;
        int32_t n = (int32_t)d;
        if (n == d) {
            return JsValue(JDT_INT32, n);
        }
        return JsValue(JDT_NUMBER, d);
    }

    static JsValue increase(VMContext *ctx, JsProperty *prop, const JsValue &thiz, int n, bool isPost) {
        JsValue newValue;
        if (prop->isGSetter) {
            if (prop->value.type >= JDT_FUNCTION) {
                ctx->vm->callMember(ctx, thiz, prop->value, Arguments());

                auto org = ctx->retValue;
                newValue = increase(ctx, org, n);
                if (prop->setter.isValid()) {
                    // 有 setter，修改.
                    ctx->vm->callMember(ctx, thiz, prop->setter, ArgumentsX(newValue));
                }

                return isPost ? org : newValue;
            } else {
                return jsValueNaN;
            }
        } else {
            auto org = prop->value;
            newValue = increase(ctx, org, n);
            if (prop->isWritable) {
                prop->value = newValue;
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

    // self 是 "this" 放在 runtime->objValues 后对应的 JsValue.
    JsValue                     self;

};

#endif /* IJsObject_hpp */
