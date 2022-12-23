//
//  VMRuntime.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/25.
//

#ifndef VMRuntime_hpp
#define VMRuntime_hpp

#include "VMRuntimeCommon.hpp"
#include "TimerTasks.hpp"
#include "PromiseTasks.hpp"


using VecVMScopes = vector<VMScope *>;
using MapIndexToJsObjs = unordered_map<int, IJsObject *>;

class IConsole {
public:
    virtual ~IConsole() { }

    virtual void log(const SizedString &message) = 0;
    virtual void info(const SizedString &message) = 0;
    virtual void warn(const SizedString &message) = 0;
    virtual void error(const SizedString &message) = 0;

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

    void init(JsVirtualMachine *vm);

    //
    // 任务相关的函数
    //
    inline void registerToRunPromise(JsPromiseObject *promise) { _promiseTasks.registerToRunPromise(promise); }
    inline int registerTimer(VMContext *ctx, JsValue callback, int32_t duration, bool repeat)
        { return _timerTasks.registerTimer(ctx, callback, duration, repeat); }
    inline void unregisterTimer(int timerId) { _timerTasks.unregisterTimer(timerId); }
    bool onRunTasks();

    void setConsole(IConsole *console) { if (this->_console) { delete this->_console; } this->_console = console; }

    void dump(BinaryOutputStream &stream);

    JsValue pushObject(IJsObject *value);
    JsValue pushDouble(double value);
    JsValue pushSymbol(const JsSymbol &value);
    JsValue pushGetterSetter(const JsValue &getter, const JsValue &setter)
        { return pushGetterSetter(JsGetterSetter(getter, setter)); }
    JsValue pushGetterSetter(const JsGetterSetter &value);
    JsValue pushString(const JsString &str);
    JsValue pushString(const SizedString &str);
    JsValue pushString(const LinkedString *str);

    VMScope *newScope(Scope *scope);
    ResourcePool *newResourcePool();

    JsNativeFunction getNativeFunction(uint32_t i) {
        assert(i < _nativeFunctions.size());
        return _nativeFunctions[i].func;
    }

    const SizedString &getNativeFunctionName(uint32_t i) {
        assert(i < _nativeFunctions.size());
        return _nativeFunctions[i].name;
    }

    double getDouble(const JsValue &val) {
        assert(val.type == JDT_NUMBER);
        if (val.isInResourcePool) {
            uint16_t poolIndex = getPoolIndexOfResource(val.value.index);
            uint16_t strIndex = getIndexOfResource(val.value.index);

            assert(poolIndex < _resourcePools.size());
            auto rp = _resourcePools[poolIndex];
            assert(strIndex < rp->doubles.size());
            return rp->doubles[strIndex];
        } else {
            return _doubleValues[val.value.index].value;
        }
    }

    const JsSymbol &getSymbol(const JsValue &val) {
        assert(val.type == JDT_SYMBOL);
        assert(val.value.index < _symbolValues.size());
        return _symbolValues[val.value.index];
    }

    const SizedStringUtf16 &getStringInResourcePool(uint32_t index, bool needRandAccess = false) {
        uint16_t poolIndex = getPoolIndexOfResource(index);
        uint16_t strIndex = getIndexOfResource(index);
        assert(poolIndex < _resourcePools.size());
        auto rp = _resourcePools[poolIndex];
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
            auto &js = _stringValues[val.value.index];
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
        return _objValues[val.value.index];
    }

    JsGetterSetter &getGetterSetter(const JsValue &val) {
        assert(val.type == JDT_GETTER_SETTER);
        assert(val.value.index <= _getterSetters.size());
        return _getterSetters[val.value.index];
    }

    uint32_t findString(const SizedString &str) {
        return _rtCommon->findStringValue(str);
    }

    const SizedString &getStringByIdx(uint32_t index, const ResourcePool *pool) {
        if (index < _countCommonStrings) {
            return _stringValues[index].value.str.utf8Str();
        }

        index -= _countCommonStrings;
        return pool->strings[index].utf8Str();
    }

    const SwitchJump &getSwitchJumpInResourcePool(uint32_t index, const ResourcePool *pool) {
        return pool->switchCaseJumps[index];
    }

    JsValue stringIdxToJsValue(uint16_t poolIndex, uint32_t index) {
        if (index < _countCommonStrings) {
            return JsValue(JDT_STRING, index);
        }

        index -= _countCommonStrings;
        return makeJsValueOfStringInResourcePool(poolIndex, index);
    }

    JsValue numberIdxToJsValue(uint16_t poolIndex, uint32_t index) {
        if (index < _countCommonDobules) {
            return JsValue(JDT_NUMBER, index);
        }

        index -= _countCommonDobules;
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
    LockedSizedStringWrapper toSizedString(VMContext *ctx, const JsValue &v, bool isStrict = false);
    inline LockedSizedStringWrapper toSizedStringStrictly(VMContext *ctx, const JsValue &v) {
        return toSizedString(ctx, v, true); }
    JsValue jsObjectToString(VMContext *ctx, const JsValue &obj);
    JsValue tryCallJsObjectValueOf(VMContext *ctx, const JsValue &v);
    SizedString toTypeName(const JsValue &v);

    bool isEmptyString(const JsValue &v);
    uint32_t getStringLength(const JsValue &v);
    bool isNan(const JsValue &v);

    bool testTrue(const JsValue &v);

    void extendObject(VMContext *ctx, const JsValue &dst, const JsValue &src, bool includePrototypeProps = true);

    IConsole *console() { return _console; };
    VMContext *mainCtx() { return _mainCtx; }
    VMGlobalScope *globalScope() { return _globalScope; }
    JsVirtualMachine *vm() { return _vm; }

    IJsObject *objPrototypeString() { return _objPrototypeString; }
    IJsObject *objPrototypeNumber() { return _objPrototypeNumber; }
    IJsObject *objPrototypeBoolean() { return _objPrototypeBoolean; }
    IJsObject *objPrototypeSymbol() { return _objPrototypeSymbol; }
    IJsObject *objPrototypeRegex() { return _objPrototypeRegex; }
    IJsObject *objPrototypeObject() { return _objPrototypeObject; }
    IJsObject *objPrototypeArray() { return _objPrototypeArray; }
    IJsObject *objPrototypeFunction() { return _objPrototypeFunction; }
    IJsObject *objPrototypeWindow() { return _objPrototypeWindow; }

public:
    //
    // 和 Garbage Collect 有关的函数
    //

    uint32_t countAllocated() const ;

    uint32_t garbageCollect();
    bool shouldGarbageCollect() { return _newAllocatedCount >= _gcAllocatedCountThreshold; }
    void setGarbageCollectThreshold(uint32_t count) { _gcAllocatedCountThreshold = count; }

    inline uint8_t nextReferIdx() const { return _nextRefIdx; }

    void markReferIdx(const JsValue &val);
    void markReferIdx(VMScope *scope);

    inline void markReferIdx(ResourcePool *pool) {
        pool->referIdx = _nextRefIdx;
    }

    inline void markResourcePoolReferIdx(uint32_t index) {
        uint16_t poolIndex = getPoolIndexOfResource(index);
        assert(poolIndex < _resourcePools.size());
        auto rp = _resourcePools[poolIndex];
        rp->referIdx = _nextRefIdx;
    }

    inline void markSymbolUsed(uint32_t index) {
        assert(index < _symbolValues.size());
        _symbolValues[index].referIdx = _nextRefIdx;
    }

    void markJoinedStringReferIdx(const JsJoinedString &joinedString);

    void convertUtf8ToUtf16(SizedStringUtf16 &str);

protected:
    VMRuntimeCommon             *_rtCommon;

protected:
    JsVirtualMachine            *_vm;

    IConsole                    *_console;

    VMGlobalScope               *_globalScope;

    // 主函数的调用上下文
    VMContext                   *_mainCtx;

    uint32_t                    _countCommonDobules;
    uint32_t                    _countCommonStrings;
    uint32_t                    _countCommonObjs;

    VecJsDoubles                _doubleValues;
    VecJsSymbols                _symbolValues;
    VecJsGetterSetters          _getterSetters;
    VecJsStrings                _stringValues;
    VecJsObjects                _objValues;
    VecVMScopes                 _vmScopes;
    VecJsNativeFunction         _nativeFunctions;
    VecResourcePools            _resourcePools;

    uint32_t                    _firstFreeDoubleIdx;
    uint32_t                    _firstFreeSymbolIdx;
    uint32_t                    _firstFreeGetterSetterIdx;
    uint32_t                    _firstFreeStringIdx;
    uint32_t                    _firstFreeObjIdx;
    uint32_t                    _firstFreeVMScopeIdx;
    uint32_t                    _firstFreeResourcePoolIdx;

    IJsObject                   *_objPrototypeString;
    IJsObject                   *_objPrototypeNumber;
    IJsObject                   *_objPrototypeBoolean;
    IJsObject                   *_objPrototypeSymbol;
    IJsObject                   *_objPrototypeRegex;
    IJsObject                   *_objPrototypeObject;
    IJsObject                   *_objPrototypeArray;
    IJsObject                   *_objPrototypeFunction;
    IJsObject                   *_objPrototypeWindow;

    PromiseTasks                _promiseTasks;
    TimerTasks                  _timerTasks;

    uint8_t                     _nextRefIdx;
    uint32_t                    _newAllocatedCount;
    uint32_t                    _gcAllocatedCountThreshold;

};

#endif /* VMRuntime_hpp */
