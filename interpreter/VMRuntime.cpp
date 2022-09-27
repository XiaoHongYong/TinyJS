//
//  VMRuntime.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/25.
//

#include <algorithm>

#include "VMRuntime.hpp"
#include "VirtualMachine.hpp"
#include "api-web/WebAPI.hpp"
#include "api-built-in/BuiltIn.hpp"
#include "objects/JsGlobalThis.hpp"


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
    doubleValues.push_back(0);
    stringValues.push_back(JsString());
    objValues.push_back(new JsObject());
    nativeFunctions.push_back(JsNativeFunctionInfo(nullptr, SS_EMPTY));

    countImmutableGlobalVars = 0;

    addConstStrings(this);

    auto idx = pushDoubleValue(NAN);
    assert(idx.value.index == jsValueNaN.value.index);

    idx = pushDoubleValue(INFINITY);
    assert(idx.value.index == jsValueInf.value.index);

    Function *rootFunc = PoolNew(_resourcePool.pool, Function)(&_resourcePool, nullptr, 0);
    globalScope = new VMGlobalScope(rootFunc->scope);

    // 添加不能被修改的全局变量
    setGlobalValue("undefined", jsValueUndefined);
    countImmutableGlobalVars++; assert(countImmutableGlobalVars == globalScope->vars.size());

    setGlobalValue("NaN", jsValueNaN);
    countImmutableGlobalVars++; assert(countImmutableGlobalVars == globalScope->vars.size());

    setGlobalValue("Infinity", jsValueInf);
    countImmutableGlobalVars++; assert(countImmutableGlobalVars == globalScope->vars.size());

    setGlobalValue("globalThis", jsValueGlobalThis);
    countImmutableGlobalVars++; assert(countImmutableGlobalVars == globalScope->vars.size());

    setGlobalValue("window", jsValueGlobalThis);
    countImmutableGlobalVars++; assert(countImmutableGlobalVars == globalScope->vars.size());

    setGlobalValue("__proto__", jsValuePrototypeWindow);
    countImmutableGlobalVars++; assert(countImmutableGlobalVars == globalScope->vars.size());

    for (int i = 1; i < JS_OBJ_IDX_RESERVED_MAX; i++) {
        objValues.push_back(nullptr);
    }

    // globalThis 的占位
    objValues[JS_OBJ_GLOBAL_THIS_IDX] = new JsObject();

    registerBuiltIns(this);
    registerWebAPIs(this);
}

VMRuntimeCommon::~VMRuntimeCommon() {
    for (auto obj : objValues) {
        delete obj;
    }

    delete globalScope;
}

void VMRuntimeCommon::dump(BinaryOutputStream &stream) {
    stream.writeFormat("Count nativeFunctions: %d\n", (int)nativeFunctions.size());

    stream.write("Strings: [\n");
    for (uint32_t i = 0; i < stringValues.size(); i++) {
        auto item = stringValues[i];
        assert(!item.isJoinedString);
        auto &s = item.value.str;
        stream.writeFormat("  %d: %.*s,\n", i, (int)s.len, s.data);
    }
    stream.write("]\n");

    stream.write("Doubles: [\n");
    for (uint32_t i = 0; i < doubleValues.size(); i++) {
        auto d = doubleValues[i].value;
        stream.writeFormat("  %d: %llf,\n", i, d);
    }
    stream.write("]\n");

    stream.write("Objects: [\n");
    for (uint32_t i = 0; i < objValues.size(); i++) {
        auto obj = objValues[i];
        stream.writeFormat("  %d: %llf, Type: %s\n", i, obj, jsDataTypeToString(obj->type));
    }
    stream.write("]\n");
}

void VMRuntimeCommon::setGlobalValue(const char *strName, const JsValue &value) {
    auto name = makeStableStr(strName);
    auto scopeDsc = globalScope->scopeDsc;

    auto id = PoolNew(scopeDsc->function->resourcePool->pool, IdentifierDeclare)(name, scopeDsc);
    id->storageIndex = scopeDsc->countLocalVars++;
    id->varStorageType = VST_GLOBAL_VAR;
    id->isReferredByChild = true;

    assert(scopeDsc->varDeclares.find(name) == scopeDsc->varDeclares.end());
    scopeDsc->varDeclares[name] = id;

    assert(id->storageIndex == globalScope->vars.size());
    globalScope->vars.push_back(value);
}

void VMRuntimeCommon::setGlobalObject(const char *strName, IJsObject *obj) {
    setGlobalValue(strName, pushObjectValue(obj));
}

JsValue VMRuntimeCommon::pushObjectValue(IJsObject *value) {
    auto jsv = JsValue(value->type, (uint32_t)objValues.size());
    value->self = jsv;
    objValues.push_back(value);
    return jsv;
}

JsValue VMRuntimeCommon::pushDoubleValue(double value) {
    auto it = mapDoubles.find(value);
    if (it == mapDoubles.end()) {
        uint32_t index = (uint32_t)doubleValues.size();
        mapDoubles[value] = index;
        doubleValues.push_back(JsDouble(value));
        return JsValue(JDT_NUMBER, index);
    } else {
        return JsValue(JDT_NUMBER, (*it).second);
    }
}

JsValue VMRuntimeCommon::pushStringValue(const SizedString &value) {
    assert(value.isStable());

    auto it = mapStrings.find(value);
    if (it == mapStrings.end()) {
        uint32_t index = (uint32_t)stringValues.size();
        mapStrings[value] = index;
        JsString s;
        s.value.str = value;
        stringValues.push_back(s);
        return JsValue(JDT_STRING, index);
    } else {
        return JsValue(JDT_STRING, (*it).second);
    }
}

uint32_t VMRuntimeCommon::findDoubleValue(double value) {
    auto it = mapDoubles.find(value);
    if (it == mapDoubles.end()) {
        return -1;
    } else {
        return (*it).second;
    }
}

uint32_t VMRuntimeCommon::findStringValue(const SizedString &value) {
    auto it = mapStrings.find(value);
    if (it == mapStrings.end()) {
        return -1;
    } else {
        return (*it).second;
    }
}

VMRuntime::VMRuntime() {
    vm = nullptr;
    rtCommon = nullptr;
    console = nullptr;
    globalScope = nullptr;

    objPrototypeString = nullptr;
    objPrototypeNumber = nullptr;
    objPrototypeBoolean = nullptr;
    objPrototypeRegex = nullptr;
    objPrototypeSymbol = nullptr;
    objPrototypeObject = nullptr;
    objPrototypeArray = nullptr;
    objPrototypeFunction = nullptr;
    objPrototypeWindow = nullptr;

    countCommonDobules = 0;
    countCommonStrings = 0;
    countCommonObjs = 0;

    firstFreeDoubleIdx = 0;
    firstFreeSymbolIdx = 0;
    firstFreeStringIdx = 0;
    firstFreeIteratorIdx = 0;
    firstFreeObjIdx = 0;
    firstFreeVMScopeIdx = 0;
    firstFreeResourcePoolIdx = 0;

    _nextRefIdx = 1;
    _newAllocatedCount = 0;
    _gcAllocatedCountThreshold = GC_ALLOCATED_COUNT_THRESHOLD;
}

VMRuntime::~VMRuntime() {
    for (auto item : iteratorValues) {
        delete item;
    }

    for (auto item : objValues) {
        delete item;
    }

    for (auto item : vmScopes) {
        delete item;
    }

    for (auto item : resourcePools) {
        delete item;
    }

    if (console) {
        delete console;
    }

    if (mainVmCtx) {
        delete mainVmCtx;
    }
}

void VMRuntime::init(JsVirtualMachine *vm, VMRuntimeCommon *rtCommon) {
    this->vm = vm;
    this->rtCommon = rtCommon;
    console = new StdIOConsole();

    countCommonDobules = (int)rtCommon->doubleValues.size();
    countCommonStrings = (int)rtCommon->stringValues.size();
    countCommonObjs = (int)rtCommon->objValues.size();

    // 把 0 占用了，0 为非法的位置
    symbolValues.push_back(JsSymbol());

    doubleValues = rtCommon->doubleValues;
    stringValues = rtCommon->stringValues;
    nativeFunctions = rtCommon->nativeFunctions;

    globalScope = rtCommon->globalScope;
    countImmutableGlobalVars = rtCommon->countImmutableGlobalVars;

    // 需要将 rtCommon 中的对象都复制一份.
    for (auto item : rtCommon->objValues) {
        objValues.push_back(item->clone());
    }
    delete objValues[JS_OBJ_GLOBAL_THIS_IDX];
    objValues[JS_OBJ_GLOBAL_THIS_IDX] = new JsGlobalThis(globalScope);

    firstFreeDoubleIdx = 0;
    firstFreeObjIdx = 0;

    mainVmCtx = new VMContext(this);
    mainVmCtx->stack.reserve(MAX_STACK_SIZE);

    objPrototypeString = objValues[JS_OBJ_PROTOTYPE_IDX_STRING];
    objPrototypeNumber = objValues[JS_OBJ_PROTOTYPE_IDX_NUMBER];
    objPrototypeBoolean = objValues[JS_OBJ_PROTOTYPE_IDX_BOOL];
    objPrototypeSymbol = objValues[JS_OBJ_PROTOTYPE_IDX_SYMBOL];
    objPrototypeRegex = objValues[JS_OBJ_PROTOTYPE_IDX_REGEXP];
    objPrototypeObject = objValues[JS_OBJ_PROTOTYPE_IDX_OBJECT];
    objPrototypeArray = objValues[JS_OBJ_PROTOTYPE_IDX_ARRAY];
    objPrototypeFunction = objValues[JS_OBJ_PROTOTYPE_IDX_FUNCTION];
    objPrototypeWindow = objValues[JS_OBJ_PROTOTYPE_IDX_WINDOW];

    auto valueOf = objPrototypeObject->getByName(nullptr, objPrototypeObject->self, SS_VALUEOF);
    assert(valueOf.type == JDT_NATIVE_FUNCTION);
    funcObjectPrototypeValueOf = getNativeFunction(valueOf.value.index);
}

void VMRuntime::dump(BinaryOutputStream &stream) {
    BinaryOutputStream os;

    stream.write("== Common VMRuntime ==\n");
    rtCommon->dump(os);
    writeIndent(stream, os.sizedStringStartNew(), SizedString("  "));

    for (auto rp : resourcePools) {
        rp->dump(stream);
    }

    int index = 0;
    stream.write("String Values: [\n");
    for (auto &item : stringValues) {
        if (item.nextFreeIdx != 0) {
            continue;
        }
        if (item.isJoinedString) {
            stream.writeFormat("  %d: ReferIdx: %d, JoinedString: ", index, item.referIdx);
            auto &joinedString = item.value.joinedString;
            if (joinedString.isStringIdxInResourcePool) {
                auto ss = getStringInResourcePool(joinedString.stringIdx);
                stream.writeFormat("'%.*s' + ", int(ss.len), ss.data);
            } else {
                stream.writeFormat("[%d] + ", joinedString.stringIdx);
            }

            if (joinedString.isNextStringIdxInResourcePool) {
                auto ss = getStringInResourcePool(joinedString.nextStringIdx);
                stream.writeFormat("'%.*s' + ", int(ss.len), ss.data);
            } else {
                stream.writeFormat("[%d] + ", joinedString.nextStringIdx);
            }
        } else {
            stream.writeFormat("  %d: ReferIdx: %d, ", index, item.referIdx);
            stream.writeFormat("Value: %.*s,\n", (int)item.value.str.len, item.value.str.data);
        }
        index++;
    }
    stream.write("  ]\n");

    stream.write("Double Values: [\n");
    for (auto &item : doubleValues) {
        if (item.nextFreeIdx != 0) {
            continue;
        }
        stream.writeFormat("  ReferIdx: %d, ", item.referIdx);
        stream.writeFormat("Value: %llf,\n", (int)item.value);
    }
    stream.write("]\n");

    stream.write("Object Values: [\n  ");
    for (auto &item : objValues) {
        if (item->nextFreeIdx != 0) {
            continue;
        }
        stream.writeFormat("  ReferIdx: %d, ", item->referIdx);
        stream.writeFormat("Type: %s,\n", jsDataTypeToString(item->type));
    }
    stream.write("]\n");

    globalScope->scopeDsc->function->dump(stream);
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
                auto ss = getStringInResourcePool(joinedStr->stringIdx);
                memcpy(p, ss.data, ss.len);
                p += ss.len;
            } else {
                auto &head = stringValues[joinedStr->stringIdx];
                if (head.isJoinedString) {
                    // 先复制尾部
                    auto tailP = p + head.value.joinedString.len;
                    if (joinedStr->isNextStringIdxInResourcePool) {
                        // 在 ResourcePool 中
                        auto ss = getStringInResourcePool(joinedStr->nextStringIdx);
                        memcpy(tailP, ss.data, ss.len);
                    } else {
                        auto &tail = stringValues[joinedStr->nextStringIdx];
                        if (tail.isJoinedString) {
                            // 添加到 tasks 中
                            tasks.push({tailP, &tail.value.joinedString});
                        } else {
                            auto &ss = tail.value.str;
                            memcpy(tailP, ss.data, ss.len);
                        }
                    }

                    // 继续循环
                    joinedStr = &head.value.joinedString;
                    continue;
                } else {
                    // head 是 SizedString 类型
                    auto &ss = head.value.str;
                    memcpy(p, ss.data, ss.len);
                    p += ss.len;
                }
            }

            // 复制尾部
            if (joinedStr->isNextStringIdxInResourcePool) {
                // 在 ResourcePool 中
                auto ss = getStringInResourcePool(joinedStr->nextStringIdx);
                memcpy(p, ss.data, ss.len);
            } else {
                auto &tail = stringValues[joinedStr->nextStringIdx];
                if (tail.isJoinedString) {
                    // 继续循环
                    joinedStr = &tail.value.joinedString;
                    continue;
                } else {
                    // 是 SizedString 类型
                    auto &ss = tail.value.str;
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

JsValue VMRuntime::addString(const SizedString &str1, const JsValue &s2) {
    assert(s2.type == JDT_STRING);

    if (getStringLength(s2) >= JOINED_STRING_MIN_SIZE) {
        auto s1 = pushString(str1);

        JsJoinedString js(s1, s2);
        return pushString(JsString(js));
    } else {
        auto str2 = getString(s2);
        return joinSmallString(str1, str2);
    }
}

JsValue VMRuntime::addString(const JsValue &s1, const SizedString &str2) {
    assert(s1.type == JDT_STRING);

    if (getStringLength(s1) >= JOINED_STRING_MIN_SIZE) {
        auto s2 = pushString(str2);

        JsJoinedString js(s1, s2);
        return pushString(JsString(js));
    } else {
        auto str1 = getString(s1);
        return joinSmallString(str1, str2);
    }
}

JsValue VMRuntime::addString(const JsValue &s1, const JsValue &s2) {
    assert(s1.type == JDT_STRING);
    assert(s2.type == JDT_STRING);

    JsString js1, js2;
    SizedString ss1, ss2;

    if (s1.isInResourcePool) {
        ss1 = getStringInResourcePool(s1.value.index);
    } else {
        js1 = stringValues[s1.value.index];
        if (!js1.isJoinedString) {
            ss1 = js1.value.str;
        }
    }

    if (s2.isInResourcePool) {
        ss2 = getStringInResourcePool(s2.value.index);
    } else {
        js2 = stringValues[s2.value.index];
        if (!js2.isJoinedString) {
            ss2 = js2.value.str;
        }
    }

    if (js1.isJoinedString || js2.isJoinedString) {
        // 任何一个是 JoinedString
        JsJoinedString js(s1, s2);
        return pushString(JsString(js));
    }

    if (ss1.len + ss2.len >= JOINED_STRING_MIN_SIZE) {
        // 超过 JOINED_STRING_MIN_SIZE，连接两个字符串
        JsJoinedString js(s1, s2);
        return pushString(JsString(js));
    } else {
        // 直接拼接
        return joinSmallString(ss1, ss2);
    }
}

JsValue VMRuntime::pushObjectValue(IJsObject *value) {
    _newAllocatedCount++;

    uint32_t n;
    if (firstFreeObjIdx) {
        n = firstFreeObjIdx;
        firstFreeObjIdx = objValues[firstFreeObjIdx]->nextFreeIdx;
        delete objValues[firstFreeObjIdx];
        objValues[firstFreeObjIdx] = value;
    } else {
        n = (uint32_t)objValues.size();
        objValues.push_back(value);
    }

    auto jsv = JsValue(value->type, n);
    value->self = jsv;
    return jsv;
}

JsValue VMRuntime::pushJsIterator(IJsIterator *it) {
    _newAllocatedCount++;
    uint32_t n;

    if (firstFreeIteratorIdx) {
        n = firstFreeIteratorIdx;
        firstFreeIteratorIdx = iteratorValues[firstFreeIteratorIdx]->nextFreeIdx;
        delete iteratorValues[firstFreeIteratorIdx];
        iteratorValues[firstFreeIteratorIdx] = it;
    } else {
        n = (uint32_t)iteratorValues.size();
        iteratorValues.push_back(it);
    }

    return JsValue(JDT_ITERATOR, n);
}

JsValue VMRuntime::pushDoubleValue(double value) {
    _newAllocatedCount++;
    uint32_t n;

    if (firstFreeDoubleIdx) {
        n = firstFreeDoubleIdx;
        firstFreeDoubleIdx = doubleValues[firstFreeDoubleIdx].nextFreeIdx;
        doubleValues[firstFreeDoubleIdx] = JsDouble(value);
    } else {
        n = (uint32_t)doubleValues.size();
        doubleValues.push_back(JsDouble(value));
    }

    return JsValue(JDT_NUMBER, n);
}

JsValue VMRuntime::pushSymbolValue(JsSymbol &value) {
    _newAllocatedCount++;
    uint32_t n;

    if (firstFreeSymbolIdx) {
        n = firstFreeSymbolIdx;
        firstFreeSymbolIdx = symbolValues[firstFreeSymbolIdx].nextFreeIdx;
        symbolValues[firstFreeSymbolIdx] = value;
    } else {
        n = (uint32_t)symbolValues.size();
        symbolValues.push_back(value);
    }

    return JsValue(JDT_SYMBOL, n);
}

JsValue VMRuntime::pushString(const JsString &str) {
    _newAllocatedCount++;
    uint32_t n;

    if (firstFreeStringIdx) {
        n = firstFreeStringIdx;
        firstFreeStringIdx = stringValues[firstFreeStringIdx].nextFreeIdx;
        stringValues[firstFreeStringIdx] = str;
    } else {
        n = (uint32_t)stringValues.size();
        stringValues.push_back(str);
    }

    return JsValue(JDT_STRING, n);
}

JsValue VMRuntime::pushString(const SizedString &str) {
    if (str.len == 0) {
        return jsStringValueEmpty;
    }

    if (str.isStable()) {
        return pushString(JsString(str));
    } else {
        auto tmp = allocString(str.len);
        memcpy(tmp.data, str.data, str.len);
        return pushString(JsString(tmp));
    }
}

VMScope *VMRuntime::newScope(Scope *scope) {
    _newAllocatedCount++;

    if (firstFreeVMScopeIdx) {
        auto vs = vmScopes[firstFreeVMScopeIdx];
        vs->scopeDsc = scope;
        firstFreeVMScopeIdx = vs->nextFreeIdx;
        vs->nextFreeIdx = 0;
        return vs;
    } else {
        auto vs = new VMScope(scope);
        vmScopes.push_back(vs);
        return vs;
    }
}

ResourcePool *VMRuntime::newResourcePool() {
    _newAllocatedCount++;

    if (firstFreeResourcePoolIdx) {
        auto rp = resourcePools[firstFreeResourcePoolIdx];
        firstFreeResourcePoolIdx = rp->nextFreeIdx;
        rp->nextFreeIdx = 0;
        return rp;
    } else {
        auto rp = new ResourcePool((uint32_t)resourcePools.size());
        resourcePools.push_back(rp);
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
        vm->callMember(ctx, v, "toString", noArgs);
        if (ctx->retValue.type >= JDT_OBJECT) {
            ctx->throwException(PE_TYPE_ERROR, " Cannot convert object to primitive value");
            return false;
        }
        v = ctx->retValue;
    }

    switch (v.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_REFERECNE_ERROR, "Cannot access variable before initialization");
            out = NAN;
            return false;
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
            return jsStringToNumber(getString(v), out);
        case JDT_NUMBER:
            out = getDouble(v);
            return true;
        case JDT_SYMBOL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a number");
            out = NAN;
            return false;
        default:
            out = NAN;
            return false;
    }
}

JsValue VMRuntime::tryCallJsObjectValueOf(VMContext *ctx, const JsValue &obj) {
    // 先调用 valueOf
    ctx->vm->callMember(ctx, obj, "valueOf", Arguments());
    if (ctx->retValue.type < JDT_OBJECT) {
        return ctx->retValue;
    }

    return jsObjectToString(ctx, obj);
}

JsValue VMRuntime::jsObjectToString(VMContext *ctx, const JsValue &v) {
    assert(v.type >= JDT_OBJECT);
    vm->callMember(ctx, v, "toString", Arguments());
    if (ctx->retValue.type >= JDT_OBJECT) {
        ctx->throwException(PE_TYPE_ERROR, " Cannot convert object to primitive value");
        return jsStringValueEmpty;
    }
    return ctx->retValue;
}

JsValue VMRuntime::toString(VMContext *ctx, const JsValue &v) {
    JsValue val = v;
    if (val.type >= JDT_OBJECT) {
        vm->callMember(ctx, v, "toString", Arguments());
        if (ctx->retValue.type >= JDT_OBJECT) {
            ctx->throwException(PE_TYPE_ERROR, " Cannot convert object to primitive value");
            return jsStringValueEmpty;
        }
        val = ctx->retValue;
    }

    switch (val.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
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
            char buf[32];
            buf[0] = val.value.n32;
            return pushString(SizedString(buf, 1));
        }
        case JDT_STRING: {
            return val;
        }
        case JDT_SYMBOL: {
            auto index = val.value.n32;
            assert(index < symbolValues.size());
            return pushString(SizedString(symbolValues[index].toString()));
        }
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return jsValueUndefined;
}

SizedString VMRuntime::toSizedString(VMContext *ctx, const JsValue &v, string &buf) {
    JsValue val = v;
    if (val.type >= JDT_OBJECT) {
        vm->callMember(ctx, v, "toString", Arguments());
        if (ctx->retValue.type >= JDT_OBJECT) {
            ctx->throwException(PE_TYPE_ERROR, " Cannot convert object to primitive value");
            return SizedString();
        }
        val = ctx->retValue;
    }

    switch (val.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: return SS_UNDEFINED;
        case JDT_NULL: return SS_NULL;
        case JDT_BOOL: return val.value.n32 ? SS_TRUE : SS_FALSE;
        case JDT_INT32: {
            auto n = val.value.n32;
            auto ss = intToSizedString(n);
            if (ss.len == 0) {
                buf.resize(32);
                ss.len = (uint32_t)::itoa(n, (char *)buf.data());
                ss.data = (uint8_t *)buf.data();
            }

            return ss;
        }
        case JDT_NUMBER: {
            buf.resize(64);
            auto len = floatToString(getDouble(val), buf.data());
            return SizedString(buf.data(), len);
        }
        case JDT_CHAR: {
            buf.assign(1, val.value.n32);
            return SizedString(buf.c_str(), 1);
        }
        case JDT_STRING: {
            return getString(val);
        }
        case JDT_SYMBOL: {
            auto index = val.value.n32;
            assert(index < symbolValues.size());
            buf.assign(symbolValues[index].toString());
            return SizedString(buf);
        }
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return SizedString();
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
        return getStringInResourcePool(v.value.index).len == 0;
    } else if (v.value.index == JS_STRING_IDX_EMPTY) {
        return true;
    }

    auto js = stringValues[v.value.index];
    if (js.isJoinedString) {
        return js.value.joinedString.len > 0;
    } else {
        return js.value.str.len > 0;
    }
}

uint32_t VMRuntime::getStringLength(const JsValue &value) {
    assert(value.type == JDT_STRING || value.type == JDT_CHAR);

    uint32_t len = 0;
    if (value.type == JDT_CHAR) {
        len = 1;
    } else {
        if (value.isInResourcePool) {
            auto ss = getStringInResourcePool(value.value.index);
            len = ss.len;
        } else {
            auto &js = stringValues[value.value.index];
            if (js.isJoinedString) {
                len = js.value.joinedString.len;
            } else {
                len = js.value.str.len;
            }
        }
    }

    return len;
}

bool VMRuntime::testTrue(const JsValue &value) {
    switch (value.type) {
        case JDT_NOT_INITIALIZED:
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
        auto s = getString(src);
        for (uint32_t i = 0; i < s.len; i++) {
            objDst->setByIndex(ctx, dst, i, JsValue(JDT_CHAR, s.data[i]));
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
uint32_t freeValues(ARRAY &arr, uint32_t startIdx, uint32_t firstFreeIdx, uint8_t nextRefIdx, uint32_t &countFreedOut) {

    // 将未标记的对象释放了
    uint32_t size = (uint32_t)arr.size();
    for (uint32_t i = startIdx; i < size; i++) {
        auto &item = arr[i];
        if (item.referIdx != nextRefIdx) {
            item.nextFreeIdx = firstFreeIdx;
            firstFreeIdx = i;
            countFreedOut++;
        }
    }

    return firstFreeIdx;
}

/**
 * 统计分配的各类存储对象的数量
 */
uint32_t VMRuntime::countAllocated() const {
    return uint32_t(doubleValues.size() - countCommonDobules
        + symbolValues.size() + stringValues.size() - countCommonStrings
        + objValues.size() - countCommonObjs + iteratorValues.size()
        + vmScopes.size() + resourcePools.size());
}

/**
 * 返回释放的对象数量
 */
uint32_t VMRuntime::garbageCollect() {
    // 先标记所有的对象
    for (uint32_t i = 1; i < countCommonObjs; i++) {
        auto item = objValues[i];
        item->referIdx = _nextRefIdx;
        item->markReferIdx(this);
    }

    //
    // 释放未标记的对象
    //
    uint32_t countFreed = 0;

    firstFreeDoubleIdx = freeValues(doubleValues, countCommonDobules, firstFreeDoubleIdx, _nextRefIdx, countFreed);
    firstFreeSymbolIdx = freeValues(symbolValues, 0, firstFreeSymbolIdx, _nextRefIdx, countFreed);

    auto size = (uint32_t)stringValues.size();
    for (uint32_t i = countCommonStrings; i < size; i++) {
        auto &item = stringValues[i];
        if (item.referIdx != _nextRefIdx) {
            item.nextFreeIdx = firstFreeStringIdx;
            firstFreeStringIdx = i;
            if (!item.value.str.isStable()) {
                freeString(item.value.str);
            }
            countFreed++;
        }
    }

    size = (uint32_t)objValues.size();
    for (uint32_t i = countCommonObjs; i < size; i++) {
        auto item = objValues[i];
        if (item->referIdx != _nextRefIdx) {
            delete item;
            objValues[i] = item = new JsDummyObject();
            item->nextFreeIdx = firstFreeObjIdx;
            firstFreeObjIdx = i;
            countFreed++;
        }
    }

    size = (uint32_t)iteratorValues.size();
    for (uint32_t i = 0; i < size; i++) {
        auto item = iteratorValues[i];
        if (item->referIdx != _nextRefIdx) {
            delete item;
            iteratorValues[i] = item = new EmptyJsIterator();
            item->nextFreeIdx = firstFreeIteratorIdx;
            firstFreeIteratorIdx = i;
            countFreed++;
        }
    }

    size = (uint32_t)vmScopes.size();
    for (uint32_t i = 0; i < size; i++) {
        auto item = vmScopes[i];
        if (item->referIdx != _nextRefIdx) {
            item->free();
            item->nextFreeIdx = firstFreeVMScopeIdx;
            firstFreeVMScopeIdx = i;
            countFreed++;
        }
    }

    size = (uint32_t)resourcePools.size();
    for (uint32_t i = 0; i < size; i++) {
        auto item = resourcePools[i];
        if (item->referIdx != _nextRefIdx) {
            item->free();
            item->nextFreeIdx = firstFreeResourcePoolIdx;
            firstFreeResourcePoolIdx = i;
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
            if (val.value.index < countCommonDobules) {
                if (val.isInResourcePool) {
                    markResourcePoolReferIdx(val.value.index);
                } else {
                    doubleValues[val.value.index].referIdx = _nextRefIdx;
                }
            }
            break;
        }
        case JDT_SYMBOL: {
            assert(val.value.index < symbolValues.size());
            symbolValues[val.value.index].referIdx = _nextRefIdx;
            break;
        }
        case JDT_STRING: {
            if (val.value.index < countCommonStrings) {
                break;
            }

            if (val.isInResourcePool) {
                markResourcePoolReferIdx(val.value.index);
            } else {
                auto &item = stringValues[val.value.index];
                if (item.referIdx != _nextRefIdx) {
                    item.referIdx = _nextRefIdx;
                    if (item.isJoinedString) {
                        markJoinedStringReferIdx(item.value.joinedString);
                    }
                }
            }
            break;
        }
        case JDT_ITERATOR: {
            assert(val.value.index < iteratorValues.size());
            iteratorValues[val.value.index]->referIdx = _nextRefIdx;
            break;
        }
        default: {
            if (val.value.index < countCommonObjs) {
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

        auto &js = stringValues[idx];
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
