//
//  VMRuntime.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/7/25.
//

#include "VMRuntime.hpp"
#include "VirtualMachine.hpp"
#include "WebAPI/WebAPI.hpp"
#include "Built-in/BuiltIn.hpp"
#include <algorithm>


#define MAX_STACK_SIZE          (1024 * 1024 / 8)
#define JOINED_STRING_MIN_SIZE  256
#define POOL_STRING_SMALL       64
#define POOL_STRING_MID         1024 * 32

#define SMALL_STRING_POOL_COUNT 10
#define MID_STRING_POOL_COUNT   5

#define SMALL_STRING_POOL_SIZE  (1024 * 64)
#define MID_STRING_POOL_SIZE    (1024 * 128)


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
    prototypeString = nullptr;
    prototypeNumber = nullptr;
    prototypeBoolean = nullptr;
    prototypeRegex = nullptr;
    prototypeSymbol = nullptr;
    prototypeObject = nullptr;
    prototypeArray = nullptr;
    prototypeFunction = nullptr;

    countImmutableGlobalVars = 0;
    
    addConstStrings(this);

    auto idx = pushDoubleValue(NAN);
    assert(idx.value.index == JsNaNValue.value.index);

    auto resourcePool = new ResourcePool();
    Function *rootFunc = new Function(resourcePool, nullptr, 0);
    globalScope = new VMScope(rootFunc->scope);

    // 添加不能被修改的全局变量
    setGlobalValue("undefined", JsUndefinedValue);
    countImmutableGlobalVars++; assert(countImmutableGlobalVars == globalScope->vars.size());

    setGlobalValue("NaN", JsNaNValue);
    countImmutableGlobalVars++; assert(countImmutableGlobalVars == globalScope->vars.size());

    registerBuiltIns(this);
    registerWebAPIs(this);
}

void VMRuntimeCommon::dump(BinaryOutputStream &stream) {
    stream.writeFormat("Count nativeFunctions: %d\n", (int)nativeFunctions.size());

    stream.write("Strings: [\n");
    for (uint32_t i = 0; i < stringValues.size(); i++) {
        auto item = stringValues[i];
        assert(!item.isJoinedString);
        auto &s = item.value.poolString.value;
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
    auto name = makeSizedString(strName);
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
    auto idx = pushObjValue(obj);
    setGlobalValue(strName, JsValue(obj->type, idx));
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
    auto it = mapStrings.find(value);
    if (it == mapStrings.end()) {
        uint32_t index = (uint32_t)stringValues.size();
        mapStrings[value] = index;
        JsString s;
        s.value.poolString.value = value;
        s.value.poolString.poolIdx = POOL_STRING_IDX_INVALID;
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

    prototypeString = nullptr;
    prototypeNumber = nullptr;
    prototypeBoolean = nullptr;
    prototypeRegex = nullptr;
    prototypeSymbol = nullptr;
    prototypeObject = nullptr;
    prototypeArray = nullptr;
    prototypeFunction = nullptr;

    countCommonDobules = 0;
    countCommonStrings = 0;
    countCommonObjs = 0;

    firstFreeDoubleIdx = 0;
    firstFreeObjIdx = 0;

    // 第一个 string pool 是保留不用的: POOL_STRING_IDX_INVALID
    stringPools.push_back(nullptr);
}

void VMRuntime::init(JsVirtualMachine *vm, VMRuntimeCommon *rtCommon) {
    this->vm = vm;
    this->rtCommon = rtCommon;
    console = new StdIOConsole();

    countCommonDobules = (int)rtCommon->doubleValues.size();
    countCommonStrings = (int)rtCommon->stringValues.size();
    countCommonObjs = (int)rtCommon->objValues.size();

    doubleValues = rtCommon->doubleValues;
    stringValues = rtCommon->stringValues;
    nativeFunctions = rtCommon->nativeFunctions;

    // 需要将 rtCommon 中的对象都复制一份.
    for (auto item : rtCommon->objValues) {
        objValues.push_back(item->clone());
    }
    globalScope = rtCommon->globalScope;
    countImmutableGlobalVars = rtCommon->countImmutableGlobalVars;

    firstFreeDoubleIdx = 0;
    firstFreeObjIdx = 0;

    mainVmCtx = new VMContext(this);
    mainVmCtx->stack.reserve(MAX_STACK_SIZE);

    for (int i = 0; i < SMALL_STRING_POOL_COUNT; i++) {
        auto pool = newStringPool(SMALL_STRING_POOL_SIZE);
        smallStringPools.append(pool);
    }

    for (int i = 0; i < MID_STRING_POOL_COUNT; i++) {
        auto pool = newStringPool(MID_STRING_POOL_SIZE);
        smallStringPools.append(pool);
    }

    objPrototypeString = rtCommon->objPrototypeString->clone();
    objPrototypeNumber = rtCommon->objPrototypeNumber->clone();
    objPrototypeBoolean = rtCommon->objPrototypeBoolean->clone();
    objPrototypeSymbol = rtCommon->objPrototypeSymbol->clone();
    objPrototypeRegex = rtCommon->objPrototypeRegex->clone();
    objPrototypeObject = rtCommon->objPrototypeObject->clone();
    objPrototypeArray = rtCommon->objPrototypeArray->clone();
    objPrototypeFunction = rtCommon->objPrototypeFunction->clone();

    prototypeString = rtCommon->prototypeString;
    prototypeNumber = rtCommon->prototypeNumber;
    prototypeBoolean = rtCommon->prototypeBoolean;
    prototypeSymbol = rtCommon->prototypeSymbol;
    prototypeRegex = rtCommon->prototypeRegex;
    prototypeObject = rtCommon->prototypeObject;
    prototypeArray = rtCommon->prototypeArray;
    prototypeFunction = rtCommon->prototypeFunction;
}

void VMRuntime::dump(BinaryOutputStream &stream) {
    BinaryOutputStream os;

    stream.write("== Common VMRuntime ==\n");
    rtCommon->dump(os);
    writeIndent(stream, os.sizedStringStartNew(), makeSizedString("  "));

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
            auto &poolString = item.value.poolString;
            if (poolString.poolIdx == POOL_STRING_IDX_INVALID) {
                continue;
            }

            stream.writeFormat("  %d: ReferIdx: %d, ", index, item.referIdx);
            stream.writeFormat("PoolIdx: %d, ", poolString.poolIdx);
            stream.writeFormat("Value: %.*s,\n", (int)poolString.value.len, poolString.value.data);
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
        // uint32_t        len;
    };

    assert(js.isJoinedString);

    auto poolStr = allocString(js.value.joinedString.len);

    std::stack<Item> tasks;
    tasks.push({ (uint8_t *)poolStr.value.data, &js.value.joinedString });

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
                            // 是 string pool 类型
                            auto &ss = tail.value.poolString.value;
                            memcpy(tailP, ss.data, ss.len);
                        }
                    }

                    // 继续循环
                    joinedStr = &head.value.joinedString;
                    continue;
                } else {
                    // head 是 poolString 类型
                    auto &ss = head.value.poolString.value;
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
                    // 是 string pool 类型
                    auto &ss = tail.value.poolString.value;
                    memcpy(p, ss.data, ss.len);
                }
            }

            // joinedStr 已经完全复制
            break;
        }
    }
    
    js.value.poolString = poolStr;
}

JsValue VMRuntime::joinSmallString(const SizedString &sz1, const SizedString &sz2) {
    if (sz1.len + sz2.len == 0) {
        return JsStringValueEmpty;
    }

    JsPoolString js = allocString(sz1.len + sz2.len);

    auto p = (uint8_t *)js.value.data;
    memcpy(p, sz1.data, sz1.len); p += sz1.len;
    memcpy(p, sz2.data, sz2.len);

    return pushString(JsString(js));
}

JsValue VMRuntime::addString(const JsValue &s1, const JsValue &s2) {
    JsString js1, js2;
    SizedString ss1, ss2;

    if (s1.isInResourcePool) {
        ss1 = getStringInResourcePool(s1.value.index);
    } else {
        js1 = stringValues[s1.value.index];
        if (!js1.isJoinedString) {
            ss1 = js1.value.poolString.value;
        }
    }

    if (s2.isInResourcePool) {
        ss2 = getStringInResourcePool(s2.value.index);
    } else {
        js2 = stringValues[s2.value.index];
        if (!js2.isJoinedString) {
            ss2 = js2.value.poolString.value;
        }
    }

    if ((!s1.isInResourcePool && js1.isJoinedString) || (!s2.isInResourcePool && js2.isJoinedString)) {
        // 任何一个是 JoinedString
        JsJoinedString js;

        js.stringIdx = s1.value.index;
        js.isStringIdxInResourcePool = s1.isInResourcePool;

        js.nextStringIdx = s2.value.index;
        js.isNextStringIdxInResourcePool = s2.isInResourcePool;
        return pushString(JsString(js));
    }

    if (!s1.isInResourcePool && js1.value.poolString.poolIdx != POOL_STRING_IDX_INVALID) {
        // 看看能否直接扩展 string
        auto &poolString = js1.value.poolString;

        assert(poolString.poolIdx < stringPools.size());
        auto pool = stringPools[poolString.poolIdx];

        if (pool->ptr() == poolString.value.data + poolString.value.len && pool->capacity() >= ss2.len) {
            // poolString 正好是 pool 的最后一个，刚好可以扩展
            memcpy(pool->ptr(), ss2.data, ss2.len);
            pool->alloc(ss2.len);
            JsPoolString poolStr2 = poolString;
            poolStr2.value.len += ss2.len;
            return pushString(JsString(poolStr2));
        }
    }

    if (ss1.len + ss2.len >= JOINED_STRING_MIN_SIZE) {
        // 超过 JOINED_STRING_MIN_SIZE，连接两个字符串
        JsJoinedString js;

        js.stringIdx = s1.value.index;
        js.isStringIdxInResourcePool = s1.isInResourcePool;

        js.nextStringIdx = s2.value.index;
        js.isNextStringIdxInResourcePool = s2.isInResourcePool;
        return pushString(JsString(js));
    } else {
        // 直接拼接
        return joinSmallString(ss1, ss2);
    }
}

JsPoolString VMRuntime::allocString(uint32_t size) {
    StringPool *pool = nullptr;
    if (size <= POOL_STRING_SMALL) {
        while (smallStringPools.count() >= SMALL_STRING_POOL_COUNT) {
            pool = smallStringPools.front();
            if (pool != nullptr && pool->capacity() >= size) {
                break;
            } else {
                smallStringPools.remove(pool);
                pool = nullptr;
            }
        }

        if (!pool) {
            pool = newStringPool(SMALL_STRING_POOL_SIZE);
            smallStringPools.append(pool);
        }
    } else if (size <= POOL_STRING_MID) {
        StringPool *pool = midStringPools.front();
        if (pool != nullptr && pool->capacity() < size) {
            midStringPools.remove(pool);
            smallStringPools.append(pool);
            pool = nullptr;
        }

        if (!pool) {
            pool = newStringPool(MID_STRING_POOL_SIZE);
            midStringPools.append(pool);
        }
    } else {
        // 较大的字符串，分配独立的 pool.
        pool = newStringPool(size + 1024 * 16);
        largeStringPools.append(pool);
    }

    auto p = pool->alloc(size);
    JsPoolString poolStr;
    poolStr.value.data = p;
    poolStr.value.len = size;
    poolStr.poolIdx = pool->id();
    return poolStr;
}

StringPool *VMRuntime::newStringPool(uint32_t size) {
    if (stringPools.size() + 1 >= 0xFFFF) {
        throw std::bad_alloc();
    }

    auto pool = new StringPool((uint16_t)stringPools.size(), size);
    stringPools.push_back(pool);
    return pool;
}

JsValue VMRuntime::pushString(const SizedString &str) {
    if (str.len == 0) {
        return JsStringValueEmpty;
    }

    JsPoolString js = allocString(str.len);
    memcpy(js.value.data, str.data, str.len);

    return pushString(JsString(js));
}

double VMRuntime::toNumber(VMContext *ctx, const JsValue &item) {
    auto v = item;
    if (item.type >= JDT_OBJECT) {
        Arguments noArgs;
        vm->callMember(ctx, v, "toString", noArgs);
        v = ctx->retValue;
    }

    switch (v.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_REFERECNE_ERROR, "Cannot access variable before initialization");
            return NAN;
        case JDT_UNDEFINED:
        case JDT_NULL:
            return 0;
        case JDT_INT32:
        case JDT_BOOL:
            return v.value.n32;
        case JDT_CHAR:
            if (v.value.n32 >= '0' && v.value.n32 <= '9') {
                return v.value.n32 - '0';
            }
            return NAN;
        case JDT_STRING: {
            auto s = getString(v);
            s.trim();
            double v;
            auto p = parseNumber(s, v);
            if (p != s.data + s.len) {
                v = NAN;
            }
            return v;
        }
        case JDT_NUMBER:
            return getDouble(v);
        case JDT_SYMBOL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert a Symbol value to a number");
            return NAN;
        default:
            return NAN;
    }
}

JsValue VMRuntime::toString(VMContext *ctx, const JsValue &v) {
    JsValue val = v;
    if (val.type >= JDT_OBJECT) {
        vm->callMember(ctx, v, "toString", Arguments());
        val = ctx->retValue;
    }

    switch (val.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED: return JsStringValueUndefined;
        case JDT_NULL: return JsStringValueNull;
        case JDT_BOOL: return val.value.n32 ? JsStringValueTrue : JsStringValueFalse;
        case JDT_INT32: {
            NumberToSizedString str(val.value.n32);
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
            CStrPrintf str("Symbol(%s)", symbolValues[index].name);
            return pushString(SizedString(str.c_str(), str.size()));
        }
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return JsUndefinedValue;
}

SizedString VMRuntime::toSizedString(VMContext *ctx, const JsValue &v, string &buf) {
    JsValue val = v;
    if (val.type >= JDT_OBJECT) {
        vm->callMember(ctx, v, "toString", Arguments());
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
            buf.push_back(val.value.n32);
            return SizedString(buf.c_str(), 1);
        }
        case JDT_STRING: {
            return getString(val);
        }
        case JDT_SYMBOL: {
            auto index = val.value.n32;
            assert(index < symbolValues.size());
            CStrPrintf str("Symbol(%s)", symbolValues[index].name);
            buf = str.c_str();
            return SizedString(buf);
        }
        default: {
            ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
        }
    }

    return SizedString();
}

bool VMRuntime::isEmptyString(const JsValue &v) {
    assert(v.type == JDT_STRING);
    if (v.value.index == JS_STRING_IDX_EMPTY) {
        return true;
    }

    if (v.isInResourcePool) {
        return getStringInResourcePool(v.value.index).len == 0;
    }

    auto js = stringValues[v.value.index];
    if (js.isJoinedString) {
        return js.value.joinedString.len > 0;
    } else {
        return js.value.poolString.value.len > 0;
    }
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
            return f != 0;
        }
        case JDT_CHAR:
            return true;
        case JDT_STRING:
            return !isEmptyString(value);
        default:
            return true;
    }
}

void VMRuntime::extendObject(const JsValue &dst, const JsValue &src) {
    auto objDst = getObject(dst);
    auto objSrc = getObject(src);
}
