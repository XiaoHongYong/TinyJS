//
//  VMRuntime.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/25.
//

#ifndef VMRuntime_hpp
#define VMRuntime_hpp

#include <unordered_map>
#include <stack>
#include "AST.hpp"
#include "ConstStrings.hpp"
#include "StringPool.hpp"


class VMScope;
class VMContext;
class Arguments;
class JSVirtualMachine;


typedef void (*JsNativeMemberFunction)(VMContext *ctx, const JsValue &thiz, const Arguments &args);

using VecJsNativeMemberFunction = std::vector<JsNativeMemberFunction>;

/**
 * 在系统初始化阶段用来存储缺省使用的字符串、double、functions 等资源，这些资源不参与后期的资源释放统计
 */
class VMRuntimeCommon {
private:
    VMRuntimeCommon(const VMRuntimeCommon &);
    VMRuntimeCommon &operator=(const VMRuntimeCommon &);

public:
    VMRuntimeCommon();

    void dump(BinaryOutputStream &stream);

    void setGlobalValue(const char *name, const JsValue &value);
    void setGlobalObject(const char *name, IJsObject *obj);

    uint32_t pushObjValue(IJsObject *value) { uint32_t n = (uint32_t)objValues.size(); objValues.push_back(value); return n; }
    uint32_t pushNativeMemberFunction(JsNativeMemberFunction f) { uint32_t n = (uint32_t)nativeMemberFunctions.size(); nativeMemberFunctions.push_back(f); return n; }

    uint32_t pushDoubleValue(double value);
    uint32_t pushStringValue(const SizedString &value);

    uint32_t findDoubleValue(double value);
    uint32_t findStringValue(const SizedString &value);

public:
    using VecDoubles = std::vector<double>;
    using MapStringToIdx = std::unordered_map<SizedString, uint32_t, SizedStringHash, SizedStrCmpEqual>;
    using MapDoubleToIdx = std::unordered_map<double, uint32_t>;

    MapStringToIdx              mapStrings;
    MapDoubleToIdx              mapDoubles;

    VecJsDoubles                doubleValues;
    VecJsStrings                stringValues;

    VecJsObjects                objValues;
    VecJsNativeMemberFunction   nativeMemberFunctions;

    VMScope                     *globalScope;

};

/**
 * 对象和内存的管理
 */
class VMRuntime {
private:
    VMRuntime(const VMRuntime &);
    VMRuntime &operator=(const VMRuntime &);

public:
    VMRuntime();

    void init(JSVirtualMachine *vm, VMRuntimeCommon *rtCommon);

    void dump(BinaryOutputStream &stream);

    uint32_t pushObjValue(IJsObject *value) { uint32_t n = (uint32_t)objValues.size(); objValues.push_back(value); return n; }
    uint32_t pushDoubleValue(JsDouble &value) { uint32_t n = (uint32_t)doubleValues.size(); doubleValues.push_back(value); return n; }
    uint32_t pushResourcePool(ResourcePool *pool) { uint32_t n = (uint32_t)resourcePools.size(); resourcePools.push_back(pool); return n; }
    uint32_t pushString(const JsString &str) { uint32_t n = (uint32_t)stringValues.size(); stringValues.push_back(str); return n; }

    JsNativeMemberFunction getNativeMemberFunction(uint32_t i) {
        assert(i < rtCommon->nativeMemberFunctions.size());
        return rtCommon->nativeMemberFunctions[i];
    }

    double getDouble(const JsValue &val) {
        assert(val.type == JDT_NUMBER);
        if (val.isInResourcePool) {
            uint16_t poolIndex = getPoolIndexOfResource(val.value.index);
            uint16_t strIndex = getIndexOfResource(val.value.index);

            assert(poolIndex < resourcePools.size());
            auto rp = resourcePools[poolIndex];
            assert(strIndex < rp->doubles.size());
            return rp->doubles[strIndex];
        } else {
            return doubleValues[val.value.index].value;
        }
    }

    SizedString getStringInResourcePool(uint32_t index) {
        uint16_t poolIndex = getPoolIndexOfResource(index);
        uint16_t strIndex = getIndexOfResource(index);
        assert(poolIndex < resourcePools.size());
        auto rp = resourcePools[poolIndex];
        assert(strIndex < rp->strings.size());
        return rp->strings[strIndex];
    }

    SizedString getString(const JsValue &val) {
        assert(val.type == JDT_STRING);
        if (val.isInResourcePool) {
            uint16_t poolIndex = getPoolIndexOfResource(val.value.index);
            uint16_t strIndex = getIndexOfResource(val.value.index);

            assert(poolIndex < resourcePools.size());
            auto rp = resourcePools[poolIndex];
            assert(strIndex < rp->strings.size());
            return rp->strings[strIndex];
        } else {
            auto &js = stringValues[val.value.index];
            if (js.isJoinedString) {
                joinString(js);
            }

            return js.value.poolString.value;
        }
    }

    IJsObject *getObject(const JsValue &val) {
        assert(val.type >= JDT_OBJECT);
        if (val.isInResourcePool) {
            // TODO..
            return nullptr;
            // return rp->strings[val.value.resourcePool.index];
        } else {
            return objValues[val.value.index];
        }
    }

    uint32_t findString(const SizedString &str) {
        return rtCommon->findStringValue(str);
    }

    SizedString getStringByIdx(uint32_t index, const ResourcePool *pool) {
        if (index < countCommonStrings) {
            return stringValues[index].value.poolString.value;
        }

        index -= countCommonStrings;
        auto s = pool->strings[index];
        s.unused = 0;
        return s;
    }

    void joinString(JsString &js);
    JsValue joinSmallString(const SizedString &sz1, const SizedString &sz2);
    JsValue addString(const JsValue &s1, const JsValue &s2);
    JsPoolString allocString(uint32_t size);

protected:
    StringPool *newStringPool(uint32_t size);

public:
    
    JSVirtualMachine            *vm;
    VMRuntimeCommon             *rtCommon;

    VMScope                     *globalScope;
    JsValue                     globalThiz;

    // 主函数的调用上下文
    VMContext                   *mainVmCtx;

    uint32_t                    countCommonDobules;
    uint32_t                    countCommonStrings;
    uint32_t                    countCommonObjs;

    VecJsDoubles                doubleValues;
    VecJsStrings                stringValues;
    VecJsObjects                objValues;
    VecResourcePools            resourcePools;

    uint32_t                    firstFreeDoubleIdx;
    uint32_t                    firstFreeObjIdx;

    VecStringPools              stringPools;
    StringPoolList              smallStringPools;
    StringPoolList              midStringPools;
    StringPoolList              largeStringPools;

};

#endif /* VMRuntime_hpp */
