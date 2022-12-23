//
//  VMRuntime.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/25.
//

#include <algorithm>

#include "VMRuntime.hpp"
#include "VirtualMachine.hpp"
#include "objects/JsGlobalThis.hpp"
#include "objects/JsDummyObject.hpp"
#include "objects/JsLibObject.hpp"


#define MAX_STACK_SIZE          (1024 * 1024 / 8)
#define JOINED_STRING_MIN_SIZE  256
#define POOL_STRING_SMALL       64
#define POOL_STRING_MID         1024 * 32

#define SMALL_STRING_POOL_COUNT 10
#define MID_STRING_POOL_COUNT   5

#define SMALL_STRING_POOL_SIZE  (1024 * 64)
#define MID_STRING_POOL_SIZE    (1024 * 128)

const uint32_t GC_ALLOCATED_COUNT_THRESHOLD = (uint32_t)1e5;

class StdIOConsole : public IConsole {
public:
    virtual void log(const SizedString &message) override {
        printf("%.*s\n", message.len, message.data);
    }

    virtual void info(const SizedString &message) override {
        printf("[info] %.*s\n", message.len, message.data);
    }

    virtual void warn(const SizedString &message) override {
        printf("[warn] %.*s\n", message.len, message.data);
    }

    virtual void error(const SizedString &message) override {
        printf("[error] %.*s\n", message.len, message.data);
    }

};

VMRuntime::VMRuntime() {
    _vm = nullptr;
    _rtCommon = nullptr;
    _console = nullptr;
    _globalScope = nullptr;

    _objPrototypeString = nullptr;
    _objPrototypeNumber = nullptr;
    _objPrototypeBoolean = nullptr;
    _objPrototypeRegex = nullptr;
    _objPrototypeSymbol = nullptr;
    _objPrototypeObject = nullptr;
    _objPrototypeArray = nullptr;
    _objPrototypeFunction = nullptr;
    _objPrototypeWindow = nullptr;

    _countCommonDobules = 0;
    _countCommonStrings = 0;
    _countCommonObjs = 0;

    _firstFreeDoubleIdx = 0;
    _firstFreeSymbolIdx = 0;
    _firstFreeGetterSetterIdx = 0;
    _firstFreeStringIdx = 0;
    _firstFreeObjIdx = 0;
    _firstFreeVMScopeIdx = 0;
    _firstFreeResourcePoolIdx = 0;

    _nextRefIdx = 1;
    _newAllocatedCount = 0;
    _gcAllocatedCountThreshold = GC_ALLOCATED_COUNT_THRESHOLD;
}

VMRuntime::~VMRuntime() {
    if (_globalScope) {
        delete _globalScope;
    }

    for (auto item : _objValues) {
        delete item;
    }

    for (auto item : _vmScopes) {
        delete item;
    }

    for (auto item : _resourcePools) {
        delete item;
    }

    if (_console) {
        delete _console;
    }

    if (_mainCtx) {
        delete _mainCtx;
    }
}

void VMRuntime::init(JsVirtualMachine *vm) {
    VMRuntimeCommon *rtCommon = VMRuntimeCommon::getInstance();
    this->_vm = vm;
    this->_rtCommon = rtCommon;
    _console = new StdIOConsole();

    _countCommonDobules = (int)rtCommon->_doubleValues.size();
    _countCommonStrings = (int)rtCommon->_stringValues.size();
    _countCommonObjs = (int)rtCommon->_objValues.size();

    // 把 0 占用了，0 为非法的位置
    _symbolValues.push_back(JsSymbol());

    _doubleValues = rtCommon->_doubleValues;
    _stringValues = rtCommon->_stringValues;
    _nativeFunctions = rtCommon->_nativeFunctions;

    _globalScope = new VMGlobalScope(rtCommon->_globalScope);

    // 需要将 rtCommon 中的对象都复制一份.
    for (auto item : rtCommon->_objValues) {
        _objValues.push_back(item->clone());
    }
    delete _objValues[JS_OBJ_GLOBAL_THIS_IDX];
    _objValues[JS_OBJ_GLOBAL_THIS_IDX] = new JsGlobalThis(_globalScope);

    _firstFreeDoubleIdx = 0;
    _firstFreeObjIdx = 0;

    _mainCtx = new VMContext(this, vm);
    _mainCtx->stack.reserve(MAX_STACK_SIZE);

    _objPrototypeString = _objValues[JS_OBJ_PROTOTYPE_IDX_STRING];
    _objPrototypeNumber = _objValues[JS_OBJ_PROTOTYPE_IDX_NUMBER];
    _objPrototypeBoolean = _objValues[JS_OBJ_PROTOTYPE_IDX_BOOL];
    _objPrototypeSymbol = _objValues[JS_OBJ_PROTOTYPE_IDX_SYMBOL];
    _objPrototypeRegex = _objValues[JS_OBJ_PROTOTYPE_IDX_REGEXP];
    _objPrototypeObject = _objValues[JS_OBJ_PROTOTYPE_IDX_OBJECT];
    _objPrototypeArray = _objValues[JS_OBJ_PROTOTYPE_IDX_ARRAY];
    _objPrototypeFunction = _objValues[JS_OBJ_PROTOTYPE_IDX_FUNCTION];
    _objPrototypeWindow = _objValues[JS_OBJ_PROTOTYPE_IDX_WINDOW];
}

void VMRuntime::dump(BinaryOutputStream &stream) {
    BinaryOutputStream os;

    stream.write("== Common VMRuntime ==\n");
    _rtCommon->dump(os);
    writeIndent(stream, os.sizedStringStartNew(), SizedString("  "));

    for (auto rp : _resourcePools) {
        rp->dump(stream);
    }

    int index = 0;
    stream.write("String Values: [\n");
    for (auto &item : _stringValues) {
        if (item.nextFreeIdx != 0) {
            continue;
        }
        if (item.isJoinedString) {
            stream.writeFormat("  %d: ReferIdx: %d, JoinedString: ", index, item.referIdx);
            auto &joinedString = item.value.joinedString;
            if (joinedString.isStringIdxInResourcePool) {
                auto &ss = getStringInResourcePool(joinedString.stringIdx).utf8Str();
                stream.writeFormat("'%.*s' + ", int(ss.len), ss.data);
            } else {
                stream.writeFormat("[%d] + ", joinedString.stringIdx);
            }

            if (joinedString.isNextStringIdxInResourcePool) {
                auto &ss = getStringInResourcePool(joinedString.nextStringIdx).utf8Str();
                stream.writeFormat("'%.*s' + ", int(ss.len), ss.data);
            } else {
                stream.writeFormat("[%d] + ", joinedString.nextStringIdx);
            }
        } else {
            stream.writeFormat("  %d: ReferIdx: %d, ", index, item.referIdx);
            auto &s = item.value.str.utf8Str();
            stream.writeFormat("Value: %.*s,\n", (int)s.len, s.data);
        }
        index++;
    }
    stream.write("  ]\n");

    stream.write("Double Values: [\n");
    for (auto &item : _doubleValues) {
        if (item.nextFreeIdx != 0) {
            continue;
        }
        stream.writeFormat("  ReferIdx: %d, ", item.referIdx);
        stream.writeFormat("Value: %llf,\n", (int)item.value);
    }
    stream.write("]\n");

    stream.write("Object Values: [\n  ");
    for (auto &item : _objValues) {
        if (item->nextFreeIdx != 0) {
            continue;
        }
        stream.writeFormat("  ReferIdx: %d, ", item->referIdx);
        stream.writeFormat("Type: %s,\n", jsDataTypeToString(item->type));
    }
    stream.write("]\n");

    _globalScope->scopeDsc->function->dump(stream);
}

/**
 * 为了提高性能，以及避免堆栈溢出，采用 stack 以及循环的方式来拼接字符串
 */
void VMRuntime::joinString(JsString &js) {
    struct Item {
        uint8_t         *p;
        JsJoinedString  *joinedStr;
    };

    assert(js.isJoinedString);

    auto targStr = allocString(js.value.joinedString.len);

    std::stack<Item> tasks;
    tasks.push({ (uint8_t *)targStr.data, &js.value.joinedString });

    while (!tasks.empty()) {
        auto &task = tasks.top();
        auto joinedStr = task.joinedStr;
        auto p = task.p;
        tasks.pop();

        while (joinedStr) {
            // 复制 head
            if (joinedStr->isStringIdxInResourcePool) {
                // 在 ResourcePool 中
                auto &ss = getStringInResourcePool(joinedStr->stringIdx).utf8Str();
                memcpy(p, ss.data, ss.len);
                p += ss.len;
            } else {
                auto &head = _stringValues[joinedStr->stringIdx];
                if (head.isJoinedString) {
                    // 先复制尾部
                    auto tailP = p + head.value.joinedString.len;
                    if (joinedStr->isNextStringIdxInResourcePool) {
                        // 在 ResourcePool 中
                        auto &ss = getStringInResourcePool(joinedStr->nextStringIdx).utf8Str();
                        memcpy(tailP, ss.data, ss.len);
                    } else {
                        auto &tail = _stringValues[joinedStr->nextStringIdx];
                        if (tail.isJoinedString) {
                            // 添加到 tasks 中
                            tasks.push({tailP, &tail.value.joinedString});
                        } else {
                            auto &ss = tail.value.str.utf8Str();
                            memcpy(tailP, ss.data, ss.len);
                        }
                    }

                    // 继续循环
                    joinedStr = &head.value.joinedString;
                    continue;
                } else {
                    // head 是 SizedString 类型
                    auto &ss = head.value.str.utf8Str();
                    memcpy(p, ss.data, ss.len);
                    p += ss.len;
                }
            }

            // 复制尾部
            if (joinedStr->isNextStringIdxInResourcePool) {
                // 在 ResourcePool 中
                auto &ss = getStringInResourcePool(joinedStr->nextStringIdx).utf8Str();
                memcpy(p, ss.data, ss.len);
            } else {
                auto &tail = _stringValues[joinedStr->nextStringIdx];
                if (tail.isJoinedString) {
                    // 继续循环
                    joinedStr = &tail.value.joinedString;
                    continue;
                } else {
                    // 是 SizedString 类型
                    auto &ss = tail.value.str.utf8Str();
                    memcpy(p, ss.data, ss.len);
                }
            }

            // joinedStr 已经完全复制
            break;
        }
    }

    js.value.str = targStr;
}

JsValue VMRuntime::joinSmallString(const SizedString &sz1, const SizedString &sz2) {
    if (sz1.len + sz2.len == 0) {
        return jsStringValueEmpty;
    }

    auto str = allocString(sz1.len + sz2.len);

    auto p = (uint8_t *)str.data;
    memcpy(p, sz1.data, sz1.len); p += sz1.len;
    memcpy(p, sz2.data, sz2.len);

    return pushString(JsString(str));
}

JsValue VMRuntime::plusString(const SizedString &str1, const JsValue &s2) {
    assert(s2.type == JDT_STRING || s2.type == JDT_CHAR);
    if (s2.type == JDT_CHAR) {
        SizedStringWrapper str2(s2);
        return joinSmallString(str1, str2);
    }

    auto s2Length = getStringLength(s2);
    if (s2Length >= JOINED_STRING_MIN_SIZE) {
        auto s1 = pushString(str1);

        JsJoinedString js(s1, s2, getStringLength(s1) + s2Length);
        return pushString(JsString(js));
    } else {
        auto &str2 = getUtf8String(s2);
        return joinSmallString(str1, str2);
    }
}

JsValue VMRuntime::plusString(const JsValue &s1, const SizedString &str2) {
    assert(s1.type == JDT_STRING || s1.type == JDT_CHAR);
    if (s1.type == JDT_CHAR) {
        SizedStringWrapper str1(s1);
        return joinSmallString(str1, str2);
    }

    auto s1Length = getStringLength(s1);
    if (s1Length >= JOINED_STRING_MIN_SIZE) {
        auto s2 = pushString(str2);

        JsJoinedString js(s1, s2, s1Length + utf8ToUtf16Length(str2));
        return pushString(JsString(js));
    } else {
        auto &str1 = getUtf8String(s1);
        return joinSmallString(str1, str2);
    }
}

JsValue VMRuntime::plusString(const JsValue &s1, const JsValue &s2) {
    assert(s1.type == JDT_STRING || s1.type == JDT_CHAR);
    assert(s2.type == JDT_STRING || s2.type == JDT_CHAR);

    bool isJoinedString = false;
    SizedStringUtf16 ss1, ss2;
    uint32_t lenUtf16 = 0;
    uint8_t buf1[8], buf2[8];

    if (s1.type == JDT_CHAR) {
        lenUtf16 = utf32CodeToUtf16Length(s1.value.index);
        ss1.set(SizedString(buf1, utf32CodeToUtf8(s1.value.index, buf1)));
    } else {
        if (s1.isInResourcePool) {
            ss1 = getStringInResourcePool(s1.value.index);
            lenUtf16 += ss1.size();
        } else {
            auto &js1 = _stringValues[s1.value.index];
            lenUtf16 += js1.lenUtf16();
            if (js1.isJoinedString) {
                isJoinedString = true;
            } else {
                ss1 = js1.value.str;
            }
        }
    }

    if (s2.type == JDT_CHAR) {
        lenUtf16 += utf32CodeToUtf16Length(s2.value.index);
        ss2.set(SizedString(buf2, utf32CodeToUtf8(s2.value.index, buf2)));
    } else {
        if (s2.isInResourcePool) {
            ss2 = getStringInResourcePool(s2.value.index);
            lenUtf16 += ss2.size();
        } else {
            auto &js2 = _stringValues[s2.value.index];
            lenUtf16 += js2.lenUtf16();
            if (js2.isJoinedString) {
                isJoinedString = true;
            } else {
                ss2 = js2.value.str;
            }
        }
    }

    if (isJoinedString) {
        // 任何一个是 JoinedString
        JsJoinedString js(s1, s2, lenUtf16);
        return pushString(JsString(js));
    }

    if (lenUtf16 >= JOINED_STRING_MIN_SIZE) {
        // 超过 JOINED_STRING_MIN_SIZE，连接两个字符串
        JsJoinedString js(s1, s2, lenUtf16);
        return pushString(JsString(js));
    } else {
        // 直接拼接
        return joinSmallString(ss1.utf8Str(), ss2.utf8Str());
    }
}

JsValue VMRuntime::pushObject(IJsObject *value) {
    _newAllocatedCount++;

    uint32_t n;
    if (_firstFreeObjIdx) {
        n = _firstFreeObjIdx;
        _firstFreeObjIdx = _objValues[_firstFreeObjIdx]->nextFreeIdx;
        delete _objValues[_firstFreeObjIdx];
        _objValues[_firstFreeObjIdx] = value;
    } else {
        n = (uint32_t)_objValues.size();
        _objValues.push_back(value);
    }

    assert(value->type >= JDT_OBJECT);
    auto jsv = JsValue(value->type, n);
    value->self = jsv;
    return jsv;
}

JsValue VMRuntime::pushDouble(double value) {
    _newAllocatedCount++;
    uint32_t n;

    if (_firstFreeDoubleIdx) {
        n = _firstFreeDoubleIdx;
        _firstFreeDoubleIdx = _doubleValues[_firstFreeDoubleIdx].nextFreeIdx;
        _doubleValues[_firstFreeDoubleIdx] = JsDouble(value);
    } else {
        n = (uint32_t)_doubleValues.size();
        _doubleValues.push_back(JsDouble(value));
    }

    return JsValue(JDT_NUMBER, n);
}

JsValue VMRuntime::pushSymbol(const JsSymbol &value) {
    _newAllocatedCount++;
    uint32_t n;

    if (_firstFreeSymbolIdx) {
        n = _firstFreeSymbolIdx;
        _firstFreeSymbolIdx = _symbolValues[_firstFreeSymbolIdx].nextFreeIdx;
        _symbolValues[_firstFreeSymbolIdx] = value;
    } else {
        n = (uint32_t)_symbolValues.size();
        _symbolValues.push_back(value);
    }

    return JsValue(JDT_SYMBOL, n);
}

JsValue VMRuntime::pushGetterSetter(const JsGetterSetter &value) {
    _newAllocatedCount++;
    uint32_t n;

    if (_firstFreeGetterSetterIdx) {
        n = _firstFreeGetterSetterIdx;
        _firstFreeGetterSetterIdx = _getterSetters[_firstFreeGetterSetterIdx].nextFreeIdx;
        _getterSetters[_firstFreeGetterSetterIdx] = value;
    } else {
        n = (uint32_t)_getterSetters.size();
        _getterSetters.push_back(value);
    }

    return JsValue(JDT_GETTER_SETTER, n);
}

JsValue VMRuntime::pushString(const JsString &str) {
    _newAllocatedCount++;
    uint32_t n;

    if (_firstFreeStringIdx) {
        n = _firstFreeStringIdx;
        _firstFreeStringIdx = _stringValues[_firstFreeStringIdx].nextFreeIdx;
        _stringValues[_firstFreeStringIdx] = str;
    } else {
        n = (uint32_t)_stringValues.size();
        _stringValues.push_back(str);
    }

    return JsValue(JDT_STRING, n);
}

JsValue VMRuntime::pushString(const SizedString &str) {
    if (str.len <= 1) {
        if (str.len == 0) {
            return jsStringValueEmpty;
        } else {
            return makeJsValueChar(str.data[0]);
        }
    }

    if (str.isStable()) {
        return pushString(JsString(str));
    } else {
        auto tmp = allocString(str.len);
        memcpy(tmp.data, str.data, str.len);
        return pushString(JsString(tmp));
    }
}

JsValue VMRuntime::pushString(const LinkedString *str) {
    uint32_t len = 0;
    for (auto p = str; p != nullptr; p = p->next) {
        len += p->len;
    }

    if (str->len <= 1) {
        if (str->len == 0) {
            return jsStringValueEmpty;
        } else {
            return makeJsValueChar(str->data[0]);
        }
    }

    auto tmp = allocString(len);
    auto data = tmp.data;
    for (auto p = str; p != nullptr; p = p->next) {
        memcpy(data, p->data, p->len);
        data += p->len;
    }

    return pushString(JsString(tmp));
}

VMScope *VMRuntime::newScope(Scope *scope) {
    _newAllocatedCount++;

    if (_firstFreeVMScopeIdx) {
        auto vs = _vmScopes[_firstFreeVMScopeIdx];
        vs->scopeDsc = scope;
        _firstFreeVMScopeIdx = vs->nextFreeIdx;
        vs->nextFreeIdx = 0;
        return vs;
    } else {
        auto vs = new VMScope(scope);
        _vmScopes.push_back(vs);
        return vs;
    }
}

ResourcePool *VMRuntime::newResourcePool() {
    _newAllocatedCount++;

    if (_firstFreeResourcePoolIdx) {
        auto rp = _resourcePools[_firstFreeResourcePoolIdx];
        _firstFreeResourcePoolIdx = rp->nextFreeIdx;
        rp->nextFreeIdx = 0;
        return rp;
    } else {
        auto rp = new ResourcePool((uint32_t)_resourcePools.size());
        _resourcePools.push_back(rp);
        return rp;
    }
}

double VMRuntime::toNumber(VMContext *ctx, const JsValue &v) {
    double ret = 0;
    toNumber(ctx, v, ret);
    return ret;
}

bool VMRuntime::toNumber(VMContext *ctx, const JsValue &item, double &out) {
    auto v = item;
    if (item.type >= JDT_OBJECT) {
        Arguments noArgs;
        ctx->vm->callMember(ctx, v, SS_TOSTRING, noArgs);
        if (ctx->retValue.type >= JDT_OBJECT) {
            ctx->throwException(JE_TYPE_ERROR, " Cannot convert object to primitive value");
            return false;
        }
        v = ctx->retValue;
    }

    switch (v.type) {
        case JDT_UNDEFINED:
            out = NAN;
            return false;
        case JDT_NULL:
            out = 0;
            return true;
        case JDT_INT32:
        case JDT_BOOL:
            out = v.value.n32;
            return true;
        case JDT_CHAR:
            if (v.value.n32 >= '0' && v.value.n32 <= '9') {
                out = v.value.n32 - '0';
                return true;
            }
            out = NAN;
            return false;
        case JDT_STRING:
            return jsStringToNumber(getUtf8String(v), out);
        case JDT_NUMBER:
            out = getDouble(v);
            return true;
        case JDT_SYMBOL:
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert a Symbol value to a number");
            out = NAN;
            return false;
        default:
            out = NAN;
            return false;
    }
}

JsValue VMRuntime::tryCallJsObjectValueOf(VMContext *ctx, const JsValue &obj) {
    // 先调用 valueOf
    ctx->vm->callMember(ctx, obj, SS_VALUEOF, Arguments());
    if (ctx->retValue.type < JDT_OBJECT) {
        return ctx->retValue;
    }

    return jsObjectToString(ctx, obj);
}

JsValue VMRuntime::jsObjectToString(VMContext *ctx, const JsValue &v) {
    assert(v.type >= JDT_OBJECT);
    ctx->vm->callMember(ctx, v, SS_TOSTRING, Arguments());
    if (ctx->retValue.type >= JDT_OBJECT) {
        ctx->throwException(JE_TYPE_ERROR, " Cannot convert object to primitive value");
        return jsStringValueEmpty;
    }
    return ctx->retValue;
}

JsValue VMRuntime::toString(VMContext *ctx, const JsValue &v) {
    JsValue val = v;
    if (val.type >= JDT_OBJECT) {
        val = jsObjectToString(ctx, v);
    }

    switch (val.type) {
        case JDT_UNDEFINED: return jsStringValueUndefined;
        case JDT_NULL: return jsStringValueNull;
        case JDT_BOOL: return val.value.n32 ? jsStringValueTrue : jsStringValueFalse;
        case JDT_INT32: {
            SizedStringWrapper str(val.value.n32);
            return pushString(str);
        }
        case JDT_NUMBER: {
            char buf[64];
            auto len = floatToString(getDouble(val), buf);
            return pushString(SizedString(buf, len));
        }
        case JDT_CHAR: {
            SizedStringWrapper str(val);
            return pushString(str.str());
        }
        case JDT_STRING: {
            return val;
        }
        case JDT_SYMBOL: {
            auto index = val.value.n32;
            assert(index < _symbolValues.size());
            return pushString(SizedString(_symbolValues[index].toString()));
        }
        default: {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return jsValueUndefined;
}

LockedSizedStringWrapper VMRuntime::toSizedString(VMContext *ctx, const JsValue &v, bool isStrict) {
    JsValue val = v;
    if (val.type >= JDT_OBJECT) {
        ctx->vm->callMember(ctx, v, SS_TOSTRING, Arguments());
        if (ctx->retValue.type >= JDT_OBJECT) {
            ctx->throwException(JE_TYPE_ERROR, " Cannot convert object to primitive value");
            return SizedString();
        }
        val = ctx->retValue;
    }

    switch (val.type) {
        case JDT_UNDEFINED: return SS_UNDEFINED;
        case JDT_NULL: return SS_NULL;
        case JDT_BOOL: return LockedSizedStringWrapper(val.value.n32 ? SS_TRUE : SS_FALSE);
        case JDT_INT32: return LockedSizedStringWrapper(val.value.n32);
        case JDT_NUMBER: return LockedSizedStringWrapper(getDouble(val));
        case JDT_CHAR: return LockedSizedStringWrapper(val);
        case JDT_STRING: return getUtf8String(val);
        case JDT_SYMBOL: {
            if (isStrict) {
                ctx->throwException(JE_TYPE_ERROR, "Cannot convert a Symbol value to a string");
                return LockedSizedStringWrapper();
            }
            auto index = val.value.n32;
            assert(index < _symbolValues.size());
            return LockedSizedStringWrapper(_symbolValues[index].toString());
        }
        default: {
            ctx->throwException(JE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return SS_EMPTY;
}

SizedString VMRuntime::toTypeName(const JsValue &value) {
    switch (value.type) {
        case JDT_UNDEFINED: return SS_UNDEFINED;
        case JDT_NULL: return SS_NULL;
        case JDT_BOOL: return SS_BOOLEAN;
        case JDT_INT32:
        case JDT_NUMBER: return SS_NUMBER;
        case JDT_SYMBOL: return SS_SYMBOL;
        case JDT_CHAR:
        case JDT_STRING: return SS_STRING;
        case JDT_FUNCTION:
        case JDT_BOUND_FUNCTION:
        case JDT_NATIVE_FUNCTION:
            return SS_FUNCTION;
        case JDT_LIB_OBJECT: {
            auto obj = (JsLibObject *)getObject(value);
            if (obj->getFunction()) {
                return SS_FUNCTION;
            }
            return SS_OBJECT;
        }
        default: return SS_OBJECT;
    }
}

bool VMRuntime::isEmptyString(const JsValue &v) {
    assert(v.type == JDT_STRING);
    if (v.isInResourcePool) {
        return getStringInResourcePool(v.value.index).size() == 0;
    } else if (v.value.index == JS_STRING_IDX_EMPTY) {
        return true;
    }

    auto &js = _stringValues[v.value.index];
    if (js.isJoinedString) {
        return js.value.joinedString.len == 0;
    } else {
        return js.value.str.size() == 0;
    }
}

uint32_t VMRuntime::getStringLength(const JsValue &value) {
    assert(value.type == JDT_STRING || value.type == JDT_CHAR);

    uint32_t len = 0;
    if (value.type == JDT_CHAR) {
        len = 1;
    } else {
        if (value.isInResourcePool) {
            len = getStringInResourcePool(value.value.index).size();
        } else {
            auto &js = _stringValues[value.value.index];
            if (js.isJoinedString) {
                len = js.value.joinedString.lenUtf16;
            } else {
                len = js.value.str.size();
            }
        }
    }

    return len;
}

bool VMRuntime::isNan(const JsValue &v) {
    return v.type == JDT_NUMBER && isnan(getDouble(v));
}

bool VMRuntime::testTrue(const JsValue &value) {
    switch (value.type) {
        case JDT_UNDEFINED:
        case JDT_NULL:
            return false;
        case JDT_BOOL:
        case JDT_INT32:
            return value.value.n32 != 0;
        case JDT_NUMBER: {
            auto f = getDouble(value);
            return f != 0 && !isnan(f);
        }
        case JDT_CHAR:
            return true;
        case JDT_STRING:
            return !isEmptyString(value);
        default:
            return true;
    }
}

void VMRuntime::extendObject(VMContext *ctx, const JsValue &dst, const JsValue &src, bool includePrototypeProps) {
    assert(dst.type >= JDT_OBJECT);

    auto objDst = getObject(dst);

    if (src.type == JDT_STRING) {
        auto &s = getStringWithRandAccess(src);
        auto size = s.size();
        for (uint32_t i = 0; i < size; i++) {
            objDst->setByIndex(ctx, dst, i, makeJsValueChar(s.chartAt(i)));
        }
    } else if (src.type == JDT_CHAR) {
        objDst->setByIndex(ctx, dst, 0, src);
    } else if (src.type < JDT_OBJECT) {
        return;
    }

    auto objSrc = getObject(src);
    auto it = objSrc->getIteratorObject(ctx, includePrototypeProps);
    JsValue key, value;
    while (it->next(nullptr, &key, &value)) {
        objDst->set(ctx, dst, key, value);
    }
}

template<typename ARRAY>
uint32_t freeValues(ARRAY &arr, uint32_t startIdx, uint32_t _firstFreeIdx, uint8_t nextRefIdx, uint32_t &countFreedOut) {

    // 将未标记的对象释放了
    uint32_t size = (uint32_t)arr.size();
    for (uint32_t i = startIdx; i < size; i++) {
        auto &item = arr[i];
        if (item.referIdx != nextRefIdx) {
            item.nextFreeIdx = _firstFreeIdx;
            _firstFreeIdx = i;
            countFreedOut++;
        }
    }

    return _firstFreeIdx;
}

/**
 * 统计分配的各类存储对象的数量
 */
uint32_t VMRuntime::countAllocated() const {
    return uint32_t(_doubleValues.size() - _countCommonDobules
        + _symbolValues.size() + _stringValues.size() - _countCommonStrings
        + _objValues.size() - _countCommonObjs
        + _vmScopes.size() + _resourcePools.size());
}

/**
 * 返回释放的对象数量
 */
uint32_t VMRuntime::garbageCollect() {
    // 先标记所有的对象
    for (uint32_t i = 1; i < _countCommonObjs; i++) {
        auto item = _objValues[i];
        item->referIdx = _nextRefIdx;
        item->markReferIdx(this);
    }

    _timerTasks.markReferIdx(this);
    _promiseTasks.markReferIdx(this);

    //
    // 释放未标记的对象
    //
    uint32_t countFreed = 0;

    _firstFreeDoubleIdx = freeValues(_doubleValues, _countCommonDobules, _firstFreeDoubleIdx, _nextRefIdx, countFreed);
    _firstFreeSymbolIdx = freeValues(_symbolValues, 0, _firstFreeSymbolIdx, _nextRefIdx, countFreed);
    _firstFreeGetterSetterIdx = freeValues(_getterSetters, 0, _firstFreeGetterSetterIdx, _nextRefIdx, countFreed);

    auto size = (uint32_t)_stringValues.size();
    for (uint32_t i = _countCommonStrings; i < size; i++) {
        auto &item = _stringValues[i];
        if (item.referIdx != _nextRefIdx) {
            item.nextFreeIdx = _firstFreeStringIdx;
            _firstFreeStringIdx = i;
            if (!item.value.str.utf8Str().isStable()) {
                freeString(item.value.str.utf8Str());
            }
            if (item.value.str.utf16Data()) {
                freeUtf16String(item.value.str);
            }
            countFreed++;
        }
    }

    size = (uint32_t)_objValues.size();
    for (uint32_t i = _countCommonObjs; i < size; i++) {
        auto item = _objValues[i];
        if (item->referIdx != _nextRefIdx) {
            delete item;
            _objValues[i] = item = new JsDummyObject();
            item->nextFreeIdx = _firstFreeObjIdx;
            _firstFreeObjIdx = i;
            countFreed++;
        }
    }

    size = (uint32_t)_vmScopes.size();
    for (uint32_t i = 0; i < size; i++) {
        auto item = _vmScopes[i];
        if (item->referIdx != _nextRefIdx) {
            item->free();
            item->nextFreeIdx = _firstFreeVMScopeIdx;
            _firstFreeVMScopeIdx = i;
            countFreed++;
        }
    }

    size = (uint32_t)_resourcePools.size();
    for (uint32_t i = 0; i < size; i++) {
        auto item = _resourcePools[i];
        if (item->referIdx != _nextRefIdx) {
            item->free();
            item->nextFreeIdx = _firstFreeResourcePoolIdx;
            _firstFreeResourcePoolIdx = i;
            countFreed++;
        }
    }

    _nextRefIdx++;
    if (_nextRefIdx == 0) {
        _nextRefIdx = 1;
    }
    return countFreed;
}

void VMRuntime::markReferIdx(const JsValue &val) {
    if (val.type < JDT_NUMBER) {
        return;
    }

    switch (val.type) {
        case JDT_NUMBER: {
            if (val.value.index < _countCommonDobules) {
                if (val.isInResourcePool) {
                    markResourcePoolReferIdx(val.value.index);
                } else {
                    _doubleValues[val.value.index].referIdx = _nextRefIdx;
                }
            }
            break;
        }
        case JDT_SYMBOL: {
            assert(val.value.index < _symbolValues.size());
            _symbolValues[val.value.index].referIdx = _nextRefIdx;
            break;
        }
        case JDT_GETTER_SETTER: {
            assert(val.value.index < _getterSetters.size());
            _getterSetters[val.value.index].referIdx = _nextRefIdx;
            break;
        }
        case JDT_STRING: {
            if (val.value.index < _countCommonStrings) {
                break;
            }

            if (val.isInResourcePool) {
                markResourcePoolReferIdx(val.value.index);
            } else {
                auto &item = _stringValues[val.value.index];
                if (item.referIdx != _nextRefIdx) {
                    item.referIdx = _nextRefIdx;
                    if (item.isJoinedString) {
                        markJoinedStringReferIdx(item.value.joinedString);
                    }
                }
            }
            break;
        }
        default: {
            if (val.value.index < _countCommonObjs) {
                auto obj = getObject(val);
                if (obj->referIdx != _nextRefIdx) {
                    obj->referIdx = _nextRefIdx;
                    obj->markReferIdx(this);
                }
            }
            break;
        }
    }
}

void VMRuntime::markReferIdx(VMScope *scope) {
    if (scope->referIdx == _nextRefIdx) {
        return;
    }

    scope->referIdx = _nextRefIdx;

}

void VMRuntime::markJoinedStringReferIdx(const JsJoinedString &joinedString) {
    // 使用 stackStrings 避免函数嵌套调用堆栈溢出
    vector<int> stackStrings;

    if (joinedString.isStringIdxInResourcePool) {
        markResourcePoolReferIdx(joinedString.stringIdx);
    } else {
        stackStrings.push_back(joinedString.stringIdx);
    }

    if (joinedString.isNextStringIdxInResourcePool) {
        markResourcePoolReferIdx(joinedString.nextStringIdx);
    } else {
        stackStrings.push_back(joinedString.nextStringIdx);
    }

    while (!stackStrings.empty()) {
        int idx = stackStrings.back();
        stackStrings.pop_back();

        auto &js = _stringValues[idx];
        if (js.referIdx != _nextRefIdx) {
            js.referIdx = _nextRefIdx;
            if (js.isJoinedString) {
                auto &joinedString = js.value.joinedString;
                if (joinedString.isStringIdxInResourcePool) {
                    markResourcePoolReferIdx(joinedString.stringIdx);
                } else {
                    stackStrings.push_back(joinedString.stringIdx);
                }

                if (joinedString.isNextStringIdxInResourcePool) {
                    markResourcePoolReferIdx(joinedString.nextStringIdx);
                } else {
                    stackStrings.push_back(joinedString.nextStringIdx);
                }
            }
        }
    }
}

void VMRuntime::convertUtf8ToUtf16(SizedStringUtf16 &str) {
    assert(str.utf16Data() == nullptr);

    auto &utf8Str = str.utf8Str();
    auto dataUtf16 = new utf16_t[str.size()];
    utf8ToUtf16(utf8Str.data, utf8Str.len, dataUtf16, str.size());
    str.setUtf16(dataUtf16, str.size());
}

bool VMRuntime::onRunTasks() {
    bool hasTasks = _promiseTasks.run();

    if (_timerTasks.run()) {
        hasTasks = true;
    }

    return hasTasks;
}
