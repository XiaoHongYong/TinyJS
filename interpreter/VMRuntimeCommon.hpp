//
//  VMRuntimeCommon.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/17.
//

#ifndef VMRuntimeCommon_hpp
#define VMRuntimeCommon_hpp

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

struct JsNativeFunctionInfo {
    JsNativeFunction            func;
    SizedString                 name;

    JsNativeFunctionInfo(JsNativeFunction f, const SizedString &name) : func(f), name(name) {
    }
};

using VecJsNativeFunction = std::vector<JsNativeFunctionInfo>;

/**
 * 在系统初始化阶段用来存储缺省使用的字符串、double、functions 等资源，这些资源不参与后期的资源释放统计
 */
class VMRuntimeCommon {
private:
    VMRuntimeCommon(const VMRuntimeCommon &);
    VMRuntimeCommon &operator=(const VMRuntimeCommon &);

public:
    VMRuntimeCommon(JsVirtualMachine *vm);
    virtual ~VMRuntimeCommon();

    void dump(BinaryOutputStream &stream);

    void setGlobalValue(const char *name, const JsValue &value);
    void setGlobalObject(const char *name, IJsObject *obj);

    void setPrototypeObject(const JsValue &jsVal, IJsObject *obj) {
        assert(jsVal.type == JDT_LIB_OBJECT);
        auto index = jsVal.value.index;
        assert(_objValues[index] == nullptr);
        _objValues[index] = obj;
    }

    JsValue pushObject(IJsObject *value);
    JsValue pushNativeFunction(JsNativeFunction f, const SizedString &name) {
        uint32_t n = (uint32_t)_nativeFunctions.size();
        _nativeFunctions.push_back(JsNativeFunctionInfo(f, name));
        return JsValue(JDT_NATIVE_FUNCTION, n);
    }

    JsValue pushDouble(double value);
    JsValue pushStringValue(const SizedString &value);

    uint32_t findDoubleValue(double value);
    uint32_t findStringValue(const SizedString &value);

    uint32_t countStringValues() const { return (uint32_t)_stringValues.size(); }
    uint32_t countDoubleValues() const { return (uint32_t)_doubleValues.size(); }

public:
    IJsObject                   *objPrototypeString;
    IJsObject                   *objPrototypeNumber;
    IJsObject                   *objPrototypeBoolean;
    IJsObject                   *objPrototypeSymbol;
    IJsObject                   *objPrototypeRegex;
    IJsObject                   *objPrototypeObject;
    IJsObject                   *objPrototypeArray;
    IJsObject                   *objPrototypeFunction;
    IJsObject                   *objPrototypeWindow;

    JsVirtualMachine            *vm;

protected:
    friend class VMRuntime;

    using VecDoubles = std::vector<double>;
    using MapStringToIdx = std::unordered_map<SizedString, uint32_t, SizedStringHash, SizedStrCmpEqual>;
    using MapDoubleToIdx = std::unordered_map<double, uint32_t>;

    MapStringToIdx              _mapStrings;
    MapDoubleToIdx              _mapDoubles;

    VecJsDoubles                _doubleValues;
    VecJsStrings                _stringValues;

    VecJsObjects                _objValues;
    VecJsNativeFunction         _nativeFunctions;

    VMGlobalScope               *_globalScope;

};

#endif /* VMRuntimeCommon_hpp */
