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
#include "parser/ParserTypes.hpp"
#include "CommonString.hpp"
#include "ConstStrings.hpp"
#include "StringPool.hpp"


class VMScope;
class VMContext;
class Arguments;
class JsVirtualMachine;
class IJsIterator;


using VecJsIterators = vector<IJsIterator *>;

typedef void (*JsNativeFunction)(VMContext *ctx, const JsValue &thiz, const Arguments &args);

struct JsNativeFunctionObject {
    JsNativeFunction            func;
    uint32_t                    objIndx;
    bool                        isAddedToModified;

    JsNativeFunctionObject(JsNativeFunction f) : func(f) {
        objIndx = 0;
        isAddedToModified = false;
    }
};

using VecJsNativeFunction = std::vector<JsNativeFunctionObject>;

class IConsole {
public:
    virtual void log(const SizedString &message) = 0;
    virtual void info(const SizedString &message) = 0;
    virtual void warn(const SizedString &message) = 0;
    virtual void error(const SizedString &message) = 0;

};

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

    JsValue pushObjValue(JsDataType type, IJsObject *value);
    uint32_t pushNativeFunction(JsNativeFunction f) { uint32_t n = (uint32_t)nativeFunctions.size(); nativeFunctions.push_back(JsNativeFunctionObject(f)); return n; }

    JsValue pushDoubleValue(double value);
    JsValue pushStringValue(const SizedString &value);

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
    VecJsNativeFunction         nativeFunctions;

    IJsObject                   *objPrototypeString;
    IJsObject                   *objPrototypeNumber;
    IJsObject                   *objPrototypeBoolean;
    IJsObject                   *objPrototypeSymbol;
    IJsObject                   *objPrototypeRegex;
    IJsObject                   *objPrototypeObject;
    IJsObject                   *objPrototypeArray;
    IJsObject                   *objPrototypeFunction;


    VMScope                     *globalScope;

    // 全局变量的前 countImmutableGlobalVars 是不能被修改的
    uint32_t                    countImmutableGlobalVars;

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

    void init(JsVirtualMachine *vm, VMRuntimeCommon *rtCommon);

    void setConsole(IConsole *console) { this->console = console; }

    void dump(BinaryOutputStream &stream);

    JsValue pushObjValue(JsDataType type, IJsObject *value);
    JsValue pushJsIterator(IJsIterator *it) { uint32_t n = (uint32_t)iteratorValues.size(); iteratorValues.push_back(it); return JsValue(JDT_ITERATOR, n); }
    JsValue pushDoubleValue(double value) { uint32_t n = (uint32_t)doubleValues.size(); doubleValues.push_back(JsDouble(value)); return JsValue(JDT_NUMBER, n); }
    uint32_t pushResourcePool(ResourcePool *pool) { uint32_t n = (uint32_t)resourcePools.size(); resourcePools.push_back(pool); return n; }
    JsValue pushSymbolValue(JsSymbol &value) { uint32_t n = (uint32_t)symbolValues.size(); symbolValues.push_back(value); return JsValue(JDT_SYMBOL, n); }
    JsValue pushString(const JsString &str) { uint32_t n = (uint32_t)stringValues.size(); stringValues.push_back(str); return JsValue(JDT_STRING, n); }
    JsValue pushString(const SizedString &str);

    JsNativeFunction getNativeFunction(uint32_t i) {
        assert(i < nativeFunctions.size());
        return nativeFunctions[i].func;
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

    const JsSymbol &getSymbol(const JsValue &val) {
        assert(val.type == JDT_SYMBOL);
        assert(val.value.index < symbolValues.size());
        return symbolValues[val.value.index];
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
            return getStringInResourcePool(val.value.index);
        } else {
            auto &js = stringValues[val.value.index];
            if (js.isJoinedString) {
                joinString(js);
            }

            return js.value.poolString.value;
        }
    }

    IJsIterator *getJsIterator(const JsValue &val) {
        assert(val.type == JDT_ITERATOR);
        assert(val.value.index < iteratorValues.size());
        return iteratorValues[val.value.index];
    }

    IJsObject *getObject(const JsValue &val) {
        assert(val.type >= JDT_OBJECT);
        if (val.isInResourcePool) {
            // TODO..
            return nullptr;
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
        return pool->strings[index];
    }

    const SwitchJump &getSwitchJumpInResourcePool(uint32_t index, const ResourcePool *pool) {
        return pool->switchCaseJumps[index];
    }

    JsValue stringIdxToJsValue(uint16_t poolIndex, uint32_t index) {
        if (index < countCommonStrings) {
            return JsValue(JDT_STRING, index);
        }

        index -= countCommonStrings;
        return makeJsValueOfStringInResourcePool(poolIndex, index);
    }

    JsValue numberIdxToJsValue(uint16_t poolIndex, uint32_t index) {
        if (index < countCommonDobules) {
            return JsValue(JDT_NUMBER, index);
        }

        index -= countCommonDobules;
        return makeJsValueOfNumberInResourcePool(poolIndex, index);
    }

    void joinString(JsString &js);
    JsValue joinSmallString(const SizedString &sz1, const SizedString &sz2);
    JsValue addString(const SizedString &s1, const JsValue &s2);
    JsValue addString(const JsValue &s1, const SizedString &s2);
    JsValue addString(const JsValue &s1, const JsValue &s2);
    JsPoolString allocString(uint32_t size);

    double toNumber(VMContext *ctx, const JsValue &v);
    bool toNumber(VMContext *ctx, const JsValue &v, double &out);
    JsValue toString(VMContext *ctx, const JsValue &v);
    SizedString toSizedString(VMContext *ctx, const JsValue &v, string &buf);
    JsValue jsObjectToString(VMContext *ctx, const JsValue &v);

    bool isEmptyString(const JsValue &v);
    uint32_t getStringLength(const JsValue &v);

    bool testTrue(const JsValue &v);

    void extendObject(const JsValue &dst, const JsValue &src);

protected:
    StringPool *newStringPool(uint32_t size);

protected:
    VMRuntimeCommon             *rtCommon;

public:
    JsVirtualMachine            *vm;

    IConsole                    *console;

    VMScope                     *globalScope;
    JsValue                     globalThiz;

    // 全局变量的前 countImmutableGlobalVars 是不能被修改的
    uint32_t                    countImmutableGlobalVars;

    // 主函数的调用上下文
    VMContext                   *mainVmCtx;

    uint32_t                    countCommonDobules;
    uint32_t                    countCommonStrings;
    uint32_t                    countCommonObjs;

    VecJsDoubles                doubleValues;
    VecJsSymbols                symbolValues;
    VecJsStrings                stringValues;
    VecJsIterators              iteratorValues;
    VecJsObjects                objValues;
    VecJsNativeFunction         nativeFunctions;
    VecJsNativeFunction         nativeFunctionsModified; // 为了提高 GC 的性能，被修改的函数会被加入到此
    VecResourcePools            resourcePools;

    uint32_t                    firstFreeDoubleIdx;
    uint32_t                    firstFreeObjIdx;

    VecStringPools              stringPools;
    StringPoolList              smallStringPools;
    StringPoolList              midStringPools;
    StringPoolList              largeStringPools;

    IJsObject                   *objPrototypeString;
    IJsObject                   *objPrototypeNumber;
    IJsObject                   *objPrototypeBoolean;
    IJsObject                   *objPrototypeSymbol;
    IJsObject                   *objPrototypeRegex;
    IJsObject                   *objPrototypeObject;
    IJsObject                   *objPrototypeArray;
    IJsObject                   *objPrototypeFunction;

};

#endif /* VMRuntime_hpp */
