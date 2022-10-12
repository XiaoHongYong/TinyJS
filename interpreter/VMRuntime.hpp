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
class VMGlobalScope;
class VMContext;
class Arguments;
class JsVirtualMachine;
class IJsIterator;


using VecVMScopes = vector<VMScope *>;
using MapIndexToJsObjs = unordered_map<int, IJsObject *>;

typedef void (*JsNativeFunction)(VMContext *ctx, const JsValue &thiz, const Arguments &args);

struct JsNativeFunctionInfo {
    JsNativeFunction            func;
    SizedString                 name;

    JsNativeFunctionInfo(JsNativeFunction f, const SizedString &name) : func(f), name(name) {
    }
};

using VecJsNativeFunction = std::vector<JsNativeFunctionInfo>;

class IConsole {
public:
    virtual ~IConsole() { }

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
    virtual ~VMRuntimeCommon();

    void dump(BinaryOutputStream &stream);

    void setGlobalValue(const char *name, const JsValue &value);
    void setGlobalObject(const char *name, IJsObject *obj);

    void setPrototypeObject(const JsValue &jsVal, IJsObject *obj) {
        assert(jsVal.type == JDT_LIB_OBJECT);
        auto index = jsVal.value.index;
        assert(objValues[index] == nullptr);
        objValues[index] = obj;
    }

    JsValue pushObjectValue(IJsObject *value);
    uint32_t pushNativeFunction(JsNativeFunction f, const SizedString &name) {
        uint32_t n = (uint32_t)nativeFunctions.size();
        nativeFunctions.push_back(JsNativeFunctionInfo(f, name));
        return n;
    }

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
    IJsObject                   *objPrototypeWindow;

    VMGlobalScope               *globalScope;

    // 全局变量的前 countImmutableGlobalVars 是不能被修改的
    uint32_t                    countImmutableGlobalVars;

protected:
    ResourcePool                _resourcePool;

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
    virtual ~VMRuntime();

    void init(JsVirtualMachine *vm, VMRuntimeCommon *rtCommon);

    void setConsole(IConsole *console) { if (this->console) { delete this->console; } this->console = console; }

    void dump(BinaryOutputStream &stream);

    JsValue pushObjectValue(IJsObject *value);
    JsValue pushDoubleValue(double value);
    JsValue pushSymbolValue(JsSymbol &value);
    JsValue pushString(const JsString &str);
    JsValue pushString(const SizedString &str);

    VMScope *newScope(Scope *scope);
    ResourcePool *newResourcePool();

    JsNativeFunction getNativeFunction(uint32_t i) {
        assert(i < nativeFunctions.size());
        return nativeFunctions[i].func;
    }

    const SizedString &getNativeFunctionName(uint32_t i) {
        assert(i < nativeFunctions.size());
        return nativeFunctions[i].name;
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

    const SizedStringUtf16 &getStringInResourcePool(uint32_t index, bool needRandAccess = false) {
        uint16_t poolIndex = getPoolIndexOfResource(index);
        uint16_t strIndex = getIndexOfResource(index);
        assert(poolIndex < resourcePools.size());
        auto rp = resourcePools[poolIndex];
        assert(strIndex < rp->strings.size());
        auto &str = rp->strings[strIndex];

        if (needRandAccess && !str.canRandomAccess()) {
            rp->convertUtf8ToUtf16(str);
        }

        return str;
    }

    const SizedStringUtf16 &getString(const JsValue &val, bool needRandAccess = false) {
        assert(val.type == JDT_STRING);
        if (val.isInResourcePool) {
            return getStringInResourcePool(val.value.index, needRandAccess);
        } else {
            auto &js = stringValues[val.value.index];
            if (js.isJoinedString) {
                joinString(js);
            }

            if (needRandAccess && !js.value.str.canRandomAccess()) {
                convertUtf8ToUtf16(js.value.str);
            }
            return js.value.str;
        }
    }

    inline const SizedString &getUtf8String(const JsValue &val) { return getString(val).utf8Str(); }
    inline const SizedStringUtf16 &getStringWithRandAccess(const JsValue &val) { return getString(val, true); }

    IJsObject *getObject(const JsValue &val) {
        assert(val.type >= JDT_OBJECT);
        assert(!val.isInResourcePool);
        return objValues[val.value.index];
    }

    uint32_t findString(const SizedString &str) {
        return rtCommon->findStringValue(str);
    }

    const SizedString &getStringByIdx(uint32_t index, const ResourcePool *pool) {
        if (index < countCommonStrings) {
            return stringValues[index].value.str.utf8Str();
        }

        index -= countCommonStrings;
        return pool->strings[index].utf8Str();
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
    JsValue plusString(const SizedString &s1, const JsValue &s2);
    JsValue plusString(const JsValue &s1, const SizedString &s2);
    JsValue plusString(const JsValue &s1, const JsValue &s2);

    inline SizedString allocString(uint32_t size) { return SizedString(new uint8_t[size], size); }
    inline void freeString(const SizedString &s) { assert(s.data); delete [] s.data; }
    inline void freeUtf16String(const SizedStringUtf16 &s) { assert(s.isUtf16Valid()); delete [] s.utf16Data(); }

    double toNumber(VMContext *ctx, const JsValue &v);
    bool toNumber(VMContext *ctx, const JsValue &v, double &out);
    JsValue toString(VMContext *ctx, const JsValue &v);
    LockedSizedStringWrapper toSizedString(VMContext *ctx, const JsValue &v);
    JsValue jsObjectToString(VMContext *ctx, const JsValue &obj);
    JsValue tryCallJsObjectValueOf(VMContext *ctx, const JsValue &v);
    SizedString toTypeName(const JsValue &v);

    bool isEmptyString(const JsValue &v);
    uint32_t getStringLength(const JsValue &v);

    bool testTrue(const JsValue &v);

    void extendObject(VMContext *ctx, const JsValue &dst, const JsValue &src, bool includePrototypeProps = true);

public:
    //
    // 和 Garbage Collect 有关的函数
    //

    uint32_t countAllocated() const ;

    uint32_t garbageCollect();
    bool shouldGarbageCollect() { return _newAllocatedCount >= _gcAllocatedCountThreshold; }
    void setGarbageCollectThreshold(uint32_t count) { _gcAllocatedCountThreshold = count; }

    uint8_t nextReferIdx() const { return _nextRefIdx; }

    void markReferIdx(const JsValue &val);
    void markReferIdx(VMScope *scope);

    inline void markReferIdx(const JsProperty &prop) {
        if (prop.setter.type >= JDT_NUMBER) {
            markReferIdx(prop.setter);
        }

        if (prop.value.type >= JDT_NUMBER) {
            markReferIdx(prop.value);
        }
    }

    inline void markReferIdx(ResourcePool *pool) {
        pool->referIdx = _nextRefIdx;
    }

    inline void markResourcePoolReferIdx(uint32_t index) {
        uint16_t poolIndex = getPoolIndexOfResource(index);
        assert(poolIndex < resourcePools.size());
        auto rp = resourcePools[poolIndex];
        rp->referIdx = _nextRefIdx;
    }

    inline void markSymbolUsed(uint32_t index) {
        assert(index < symbolValues.size());
        symbolValues[index].referIdx = _nextRefIdx;
    }

    void markJoinedStringReferIdx(const JsJoinedString &joinedString);

    void convertUtf8ToUtf16(SizedStringUtf16 &str);

protected:
    VMRuntimeCommon             *rtCommon;

public:
    JsVirtualMachine            *vm;

    IConsole                    *console;

    VMGlobalScope               *globalScope;
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
    VecJsObjects                objValues;
    VecVMScopes                 vmScopes;
    VecJsNativeFunction         nativeFunctions;
    MapIndexToJsObjs            nativeFunctionObjs; // 为了提高 GC 的性能，被修改的函数单独存放.
    VecResourcePools            resourcePools;

    uint32_t                    firstFreeDoubleIdx;
    uint32_t                    firstFreeSymbolIdx;
    uint32_t                    firstFreeStringIdx;
    uint32_t                    firstFreeObjIdx;
    uint32_t                    firstFreeVMScopeIdx;
    uint32_t                    firstFreeResourcePoolIdx;

    IJsObject                   *objPrototypeString;
    IJsObject                   *objPrototypeNumber;
    IJsObject                   *objPrototypeBoolean;
    IJsObject                   *objPrototypeSymbol;
    IJsObject                   *objPrototypeRegex;
    IJsObject                   *objPrototypeObject;
    IJsObject                   *objPrototypeArray;
    IJsObject                   *objPrototypeFunction;
    IJsObject                   *objPrototypeWindow;

    JsNativeFunction            funcObjectPrototypeValueOf;

    uint8_t                     _nextRefIdx;
    uint32_t                    _newAllocatedCount;
    uint32_t                    _gcAllocatedCountThreshold;

};

#endif /* VMRuntime_hpp */
