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
#include "strings/CommonString.hpp"
#include "generated/ConstStrings.hpp"


class VMScope;
class VMGlobalScope;
class VMContext;
class Arguments;
class JsVirtualMachine;
class IJsIterator;

struct JsNativeFunctionInfo {
    JsNativeFunction            func;
    StringView                 name;

    JsNativeFunctionInfo(JsNativeFunction f, const StringView &name) : func(f), name(name) {
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
    VMRuntimeCommon();

public:
    virtual ~VMRuntimeCommon();

    static VMRuntimeCommon *getInstance();

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
    JsValue pushNativeFunction(JsNativeFunction f, const StringView &name) {
        uint32_t n = (uint32_t)_nativeFunctions.size();
        _nativeFunctions.push_back(JsNativeFunctionInfo(f, name));
        return JsValue(JDT_NATIVE_FUNCTION, n);
    }

    JsValue pushDouble(double value);
    JsValue pushStringValue(const StringView &value);

    uint32_t findDoubleValue(double value);
    uint32_t findStringValue(const StringView &value);

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

protected:
    friend class VMRuntime;

    using VecDoubles = std::vector<double>;
    using MapStringToIdx = std::unordered_map<StringView, uint32_t, StringViewHash, SizedStrCmpEqual>;
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
