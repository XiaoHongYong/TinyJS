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

inline JsValue getPropertyValue(VMContext *ctx, const JsProperty *prop, const JsValue &thiz) {
    if (prop->isGSetter && prop->value.type >= JDT_FUNCTION) {
        ctx->vm->callMember(ctx, thiz, prop->value, Arguments());
        return ctx->retValue;
    } else {
        return prop->value;
    }
}

inline JsValue getPropertyValue(VMContext *ctx, const JsProperty &prop, const JsValue &thiz) {
    if (prop.isGSetter && prop.value.type >= JDT_FUNCTION) {
        ctx->vm->callMember(ctx, thiz, prop.value, Arguments());
        return ctx->retValue;
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
        referIdx = 0;
        nextFreeIdx = 0;
    }
    virtual ~IJsObject() {}

    bool getBool(VMContext *ctx, const JsValue &thiz, const SizedString &name);
    bool getBool(VMContext *ctx, const JsValue &thiz, const JsValue &name);

    void defineProperty(VMContext *ctx, const JsValue &name, const JsProperty &descriptor);
    bool getOwnPropertyDescriptor(VMContext *ctx, const JsValue &name, JsProperty &descriptorOut);

    JsValue get(VMContext *ctx, const JsValue &thiz, const JsValue &name, const JsValue &defVal = jsValueUndefined);
    void set(VMContext *ctx, const JsValue &thiz, const JsValue &name, const JsValue &value);
    JsValue increase(VMContext *ctx, const JsValue &thiz, const JsValue &name, int n, bool isPost);
    bool remove(VMContext *ctx, const JsValue &name);

    virtual void definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) = 0;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) = 0;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) = 0;

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) = 0;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) = 0;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) = 0;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) = 0;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) = 0;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) = 0;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp = true) = 0;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) = 0;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) = 0;

    virtual bool removeByName(VMContext *ctx, const SizedString &name) = 0;
    virtual bool removeByIndex(VMContext *ctx, uint32_t index) = 0;
    virtual bool removeBySymbol(VMContext *ctx, uint32_t index) = 0;

    virtual IJsObject *clone() = 0;

    virtual IJsIterator *getIteratorObject(VMContext *ctx) = 0;
    virtual JsValue getIterator(VMContext *ctx);

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
            return getPropertyValue(ctx, prop, thiz);
        }
        return defVal;
    }

    JsValue getBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &defVal = jsValueUndefined) {
        auto prop = getRawBySymbol(ctx, index, true);
        if (prop) {
            return getPropertyValue(ctx, prop, thiz);
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
            string buf;
            auto name = ctx->runtime->toSizedString(ctx, JsValue(JDT_SYMBOL, index), buf);
            ctx->throwException(PE_TYPE_ERROR, "Cannot redefine property: %.*s", (int)name.len, name.data);
        }
    }

    inline void set(VMContext *ctx, JsProperty *prop, const JsValue &thiz, const JsValue &value) {
        assert(prop);
        // 自身的属性，可以直接修改
        if (prop->isGSetter) {
            if (prop->setter.type > JDT_OBJECT) {
                // 调用 setter 函数
                ArgumentsX args(value);
                ctx->vm->callMember(ctx, thiz, prop->setter, args);
            }
        } else if (prop->isWritable) {
            prop->value = value;
        }
    }

    inline JsValue increase(VMContext *ctx, JsValue &v, int x) {
        if (v.type == JDT_INT32) {
            int64_t n = v.value.n32 + x;
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
            return JsValue(JDT_INT32, d);
        }
        return JsValue(JDT_NUMBER, d);
    }

    JsValue increase(VMContext *ctx, JsProperty *prop, const JsValue &thiz, int n, bool isPost) {
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

    virtual void definePropertyByName(VMContext *ctx, const SizedString &name, const JsProperty &descriptor) override;
    virtual void definePropertyByIndex(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;
    virtual void definePropertyBySymbol(VMContext *ctx, uint32_t index, const JsProperty &descriptor) override;

    virtual void setByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) override;
    virtual void setByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;
    virtual void setBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, const JsValue &value) override;

    virtual JsValue increaseByName(VMContext *ctx, const JsValue &thiz, const SizedString &name, int n, bool isPost) override;
    virtual JsValue increaseByIndex(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;
    virtual JsValue increaseBySymbol(VMContext *ctx, const JsValue &thiz, uint32_t index, int n, bool isPost) override;

    virtual JsProperty *getRawByName(VMContext *ctx, const SizedString &name, JsNativeFunction &funcGetterOut, bool includeProtoProp = true) override;
    virtual JsProperty *getRawByIndex(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;
    virtual JsProperty *getRawBySymbol(VMContext *ctx, uint32_t index, bool includeProtoProp = true) override;

    virtual bool removeByName(VMContext *ctx, const SizedString &name) override;
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

#endif /* IJsObject_hpp */
