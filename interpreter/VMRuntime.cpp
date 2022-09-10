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
    objPrototypeString = nullptr;
    objPrototypeNumber = nullptr;
    objPrototypeBoolean = nullptr;
    objPrototypeSymbol = nullptr;
    objPrototypeRegex = nullptr;
    objPrototypeObject = nullptr;
    objPrototypeArray = nullptr;
    objPrototypeFunction = nullptr;

    // 把 0 占用了，0 为非法的位置
    doubleValues.push_back(0);
    stringValues.push_back(JsString());
    objValues.push_back(new JsObject());
    nativeFunctions.push_back(nullptr);

    countImmutableGlobalVars = 0;

    addConstStrings(this);

    auto idx = pushDoubleValue(NAN);
    assert(idx.value.index == jsValueNaN.value.index);

    idx = pushDoubleValue(INFINITY);
    assert(idx.value.index == jsValueInf.value.index);

    auto resourcePool = new ResourcePool();
    Function *rootFunc = new Function(resourcePool, nullptr, 0);
    globalScope = new VMScope(rootFunc->scope);

    // 添加不能被修改的全局变量
    setGlobalValue("undefined", jsValueUndefined);
    countImmutableGlobalVars++; assert(countImmutableGlobalVars == globalScope->vars.size());

    setGlobalValue("NaN", jsValueNaN);
    countImmutableGlobalVars++; assert(countImmutableGlobalVars == globalScope->vars.size());

    setGlobalValue("Infinity", jsValueInf);
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
    auto name = SizedString(strName);
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
    setGlobalValue(strName, pushObjValue(obj->type, obj));
}

JsValue VMRuntimeCommon::pushObjValue(JsDataType type, IJsObject *value) {
    auto jsv = JsValue(type, (uint32_t)objValues.size());
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

    objPrototypeString = nullptr;
    objPrototypeNumber = nullptr;
    objPrototypeBoolean = nullptr;
    objPrototypeRegex = nullptr;
    objPrototypeSymbol = nullptr;
    objPrototypeObject = nullptr;
    objPrototypeArray = nullptr;
    objPrototypeFunction = nullptr;

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

    // 把 0 占用了，0 为非法的位置
    symbolValues.push_back(JsSymbol());

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

    objPrototypeString = objValues[JS_OBJ_PROTOTYPE_IDX_STRING];
    objPrototypeNumber = objValues[JS_OBJ_PROTOTYPE_IDX_NUMBER];
    objPrototypeBoolean = objValues[JS_OBJ_PROTOTYPE_IDX_BOOL];
    objPrototypeSymbol = objValues[JS_OBJ_PROTOTYPE_IDX_SYMBOL];
    objPrototypeRegex = objValues[JS_OBJ_PROTOTYPE_IDX_REGEXP];
    objPrototypeObject = objValues[JS_OBJ_PROTOTYPE_IDX_OBJECT];
    objPrototypeArray = objValues[JS_OBJ_PROTOTYPE_IDX_ARRAY];
    objPrototypeFunction = objValues[JS_OBJ_PROTOTYPE_IDX_FUNCTION];
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
        return jsStringValueEmpty;
    }

    JsPoolString js = allocString(sz1.len + sz2.len);

    auto p = (uint8_t *)js.value.data;
    memcpy(p, sz1.data, sz1.len); p += sz1.len;
    memcpy(p, sz2.data, sz2.len);

    return pushString(JsString(js));
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
        JsJoinedString js(s1, s2);
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
        JsJoinedString js(s1, s2);
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
        pool = midStringPools.front();
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

JsValue VMRuntime::pushObjValue(JsDataType type, IJsObject *value) {
    auto jsv = JsValue(type, (uint32_t)objValues.size());
    value->self = jsv;
    objValues.push_back(value);
    return jsv;
}

JsValue VMRuntime::pushString(const SizedString &str) {
    if (str.len == 0) {
        return jsStringValueEmpty;
    }

    JsPoolString js = allocString(str.len);
    memcpy(js.value.data, str.data, str.len);

    return pushString(JsString(js));
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
            buf.push_back(val.value.n32);
            return SizedString(buf.c_str(), 1);
        }
        case JDT_STRING: {
            return getString(val);
        }
        case JDT_SYMBOL: {
            auto index = val.value.n32;
            assert(index < symbolValues.size());
            buf = symbolValues[index].toString();
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
                len = js.value.poolString.value.len;
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
