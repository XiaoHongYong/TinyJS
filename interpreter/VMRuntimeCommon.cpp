//
//  VMRuntimeCommon.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/12/17.
//

#include "VMRuntimeCommon.hpp"
#include "VirtualMachine.hpp"
#include "api-web/WebAPI.hpp"
#include "api-built-in/BuiltIn.hpp"


VMRuntimeCommon::VMRuntimeCommon() {
    objPrototypeString = nullptr;
    objPrototypeNumber = nullptr;
    objPrototypeBoolean = nullptr;
    objPrototypeSymbol = nullptr;
    objPrototypeRegex = nullptr;
    objPrototypeObject = nullptr;
    objPrototypeArray = nullptr;
    objPrototypeFunction = nullptr;
    objPrototypeWindow = nullptr;

    // 把 0 占用了，0 为非法的位置
    _doubleValues.push_back(0);
    _stringValues.push_back(JsString());
    _objValues.push_back(new JsObject());
    _nativeFunctions.push_back(JsNativeFunctionInfo(nullptr, SS_EMPTY));

    addConstStrings(this);

    auto idx = pushDouble(NAN);
    assert(idx.value.index == jsValueNaN.value.index);

    idx = pushDouble(INFINITY);
    assert(idx.value.index == jsValueInf.value.index);

    idx = pushDouble(2.220446049250313e-16);
    assert(idx.value.index == jsValueEpsilonNumber.value.index);

    idx = pushDouble(9007199254740991);
    assert(idx.value.index == jsValueMaxSafeInt.value.index);

    idx = pushDouble(1.7976931348623157e+308);
    assert(idx.value.index == jsValueMaxNumber.value.index);

    idx = pushDouble(-9007199254740991);
    assert(idx.value.index == jsValueMinSafeInt.value.index);

    idx = pushDouble(5e-324);
    assert(idx.value.index == jsValueMinNumber.value.index);

    idx = pushDouble(-INFINITY);
    assert(idx.value.index == jsValueNegInf.value.index);

    _globalScope = new VMGlobalScope();

    // 添加不能被修改的全局变量
    setGlobalValue("undefined", jsValueUndefined.asProperty(0));
    setGlobalValue("NaN", jsValueNaN.asProperty(0));
    setGlobalValue("Infinity", jsValueInf.asProperty(0));
    setGlobalValue("globalThis", jsValueGlobalThis.asProperty(0));
    setGlobalValue("window", jsValueGlobalThis.asProperty(0));
    setGlobalValue("__proto__", jsValuePrototypeWindow.asProperty(0));

    for (int i = 1; i < JS_OBJ_IDX_RESERVED_MAX; i++) {
        _objValues.push_back(nullptr);
    }

    // globalThis 的占位
    _objValues[JS_OBJ_GLOBAL_THIS_IDX] = new JsObject();

    registerBuiltIns(this);
    registerWebAPIs(this);
}

VMRuntimeCommon::~VMRuntimeCommon() {
    for (auto obj : _objValues) {
        delete obj;
    }

    delete _globalScope;
}

static VMRuntimeCommon *_instance = nullptr;

VMRuntimeCommon *VMRuntimeCommon::getInstance() {
    // static VMRuntimeCommon *_instance = new VMRuntimeCommon();
    if (_instance == nullptr) {
        _instance = new VMRuntimeCommon();
    }
    return _instance;
}

void VMRuntimeCommon::dump(BinaryOutputStream &stream) {
    stream.writeFormat("Count _nativeFunctions: %d\n", (int)_nativeFunctions.size());

    stream.write("Strings: [\n");
    for (uint32_t i = 0; i < _stringValues.size(); i++) {
        auto &item = _stringValues[i];
        assert(!item.isJoinedString);
        auto &s = item.value.str.utf8Str();
        stream.writeFormat("  %d: %.*s,\n", i, (int)s.len, s.data);
    }
    stream.write("]\n");

    stream.write("Doubles: [\n");
    for (uint32_t i = 0; i < _doubleValues.size(); i++) {
        auto d = _doubleValues[i].value;
        stream.writeFormat("  %d: %llf,\n", i, d);
    }
    stream.write("]\n");

    stream.write("Objects: [\n");
    for (uint32_t i = 0; i < _objValues.size(); i++) {
        auto obj = _objValues[i];
        stream.writeFormat("  %d: %llf, Type: %s\n", i, obj, jsDataTypeToString(obj->type));
    }
    stream.write("]\n");
}

void VMRuntimeCommon::setGlobalValue(const char *strName, const JsValue &value) {
    _globalScope->set(makeStableStr(strName), value);
}

void VMRuntimeCommon::setGlobalObject(const char *strName, IJsObject *obj) {
    _globalScope->set(makeStableStr(strName), pushObject(obj).asProperty(JP_WRITABLE | JP_CONFIGURABLE));
}

JsValue VMRuntimeCommon::pushObject(IJsObject *value) {
    auto jsv = JsValue(value->type, (uint32_t)_objValues.size());
    value->self = jsv;
    _objValues.push_back(value);
    return jsv;
}

JsValue VMRuntimeCommon::pushDouble(double value) {
    auto it = _mapDoubles.find(value);
    if (it == _mapDoubles.end()) {
        uint32_t index = (uint32_t)_doubleValues.size();
        _mapDoubles[value] = index;
        _doubleValues.push_back(JsDouble(value));
        return JsValue(JDT_NUMBER, index);
    } else {
        return JsValue(JDT_NUMBER, (*it).second);
    }
}

JsValue VMRuntimeCommon::pushStringValue(const StringView &value) {
    assert(value.isStable());

    auto it = _mapStrings.find(value);
    if (it == _mapStrings.end()) {
        uint32_t index = (uint32_t)_stringValues.size();
        _mapStrings[value] = index;
        JsString s;
        s.value.str = value;
        _stringValues.push_back(s);
        return JsValue(JDT_STRING, index);
    } else {
        return JsValue(JDT_STRING, (*it).second);
    }
}

uint32_t VMRuntimeCommon::findDoubleValue(double value) {
    auto it = _mapDoubles.find(value);
    if (it == _mapDoubles.end()) {
        return -1;
    } else {
        return (*it).second;
    }
}

uint32_t VMRuntimeCommon::findStringValue(const StringView &value) {
    auto it = _mapStrings.find(value);
    if (it == _mapStrings.end()) {
        return -1;
    } else {
        return (*it).second;
    }
}
