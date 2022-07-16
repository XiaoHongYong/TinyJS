//
//  VirtualMachine.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/24.
//

#ifndef VirtualMachine_hpp
#define VirtualMachine_hpp

#include <unordered_map>
#include <stack>
#include "AST.hpp"


class VMScope;
class VMScopeDescriptor;
class VMFunctionFrame;
class VMRuntime;
class VMContext;
class JSVirtualMachine;
class Arguments;

typedef void (*JsNativeMemberFunction)(VMContext *ctx, const JsValue &thiz, const Arguments &args);

using VecVMScopes = std::vector<VMScope *>;
using VecVMStackScopes = std::vector<VMScope *>;
using VecVMStackFrames = std::vector<VMFunctionFrame *>;
using VecJsNativeMemberFunction = std::vector<JsNativeMemberFunction>;
using StackJsValues = std::vector<JsValue>;
using MapNameToJsValue = std::unordered_map<SizedString, JsValue, SizedStringHash, SizedStrCmpEqual>;

void registerGlobalValue(VMContext *ctx, VMScope *globalScope, const char *name, const JsValue &value);
void registerGlobalObject(VMContext *ctx, VMScope *globalScope, const char *name, IJsObject *obj);

class Arguments {
public:
    Arguments() { data = nullptr; count = 0; needFree = false; }
    Arguments(const Arguments &other);
    Arguments(JsValue *args, uint32_t count) : data(args), count(count) { needFree = false; }
    ~Arguments();

    Arguments &operator = (const Arguments &other);
    JsValue &operator[](uint32_t n) { assert(n < count); return data[n]; }

    void copy(const Arguments &other);
    
    JsValue                     *data;
    uint32_t                    count;
    bool                        needFree;
};

class JsObjectArray {
public:

};

/**
 * VMScope 用于存储当前作用域内的标识符信息.
 */
class VMScope {
public:
    VMScope(Scope *scopeDsc) : scopeDsc(scopeDsc) { vars.resize(scopeDsc->countLocalVars); }

    void dump(BinaryOutputStream &stream);

    Scope                       *scopeDsc;

    // 作用域内的所有变量
    VecJsValues                 vars; // 当前作用域下的变量

    Arguments                   args;

    // 父作用域
    VecVMStackScopes            *parentScopes;

};

/**
 * VMFunctionFrame 即是函数调用的 frame，也是当前函数的 root scope.
 */
class VMFunctionFrame {
public:
    VMFunctionFrame(VMScope *scope, Function *function) : scope(scope), function(function) { }

    VMScope                     *scope;
    Function                    *function;

};


/**
 * 对象和内存的管理
 */
class VMRuntime {
public:
    VMRuntime(JSVirtualMachine *vm) : vm(vm) { globalScope = nullptr; firstFreeDoubleIdx = 0; firstFreeObjIdx = 0; }

    void dump(BinaryOutputStream &stream);

    uint32_t pushObjValue(IJsObject *value) { uint32_t n = (uint32_t)objValues.size(); objValues.push_back(value); return n; }
    uint32_t pushDoubleValue(JsDouble &value) { uint32_t n = (uint32_t)doubleValues.size(); doubleValues.push_back(value); return n; }
    uint32_t pushResourcePool(ResourcePool *pool) { uint32_t n = (uint32_t)resourcePools.size(); resourcePools.push_back(pool); return n; }
    uint32_t pushNativeMemberFunction(JsNativeMemberFunction f) { uint32_t n = (uint32_t)nativeMemberFunctions.size(); nativeMemberFunctions.push_back(f); return n; }

    double getDouble(const JsValue &val) {
        if (val.isInResourcePool) {
            assert(val.value.resourcePool.poolIndex < resourcePools.size());
            auto rp = resourcePools[val.value.resourcePool.poolIndex];
            assert(val.value.resourcePool.index < rp->doubles.size());
            return rp->doubles[val.value.resourcePool.index];
        } else {
            return doubleValues[val.value.objIndex].value;
        }
    }

    SizedString getString(const JsValue &val) {
        if (val.isInResourcePool) {
            assert(val.value.resourcePool.poolIndex < resourcePools.size());
            auto rp = resourcePools[val.value.resourcePool.poolIndex];
            assert(val.value.resourcePool.index < rp->strings.size());
            return rp->strings[val.value.resourcePool.index];
        } else {
            return stringValues[val.value.objIndex].value;
        }
    }

    IJsObject *getObject(const JsValue &val) {
        if (val.isInResourcePool) {
            assert(val.value.resourcePool.poolIndex < resourcePools.size());
            auto rp = resourcePools[val.value.resourcePool.poolIndex];
            assert(val.value.resourcePool.index < rp->strings.size());
            // TODO..
            return nullptr;
            // return rp->strings[val.value.resourcePool.index];
        } else {
            return objValues[val.value.objIndex];
        }
    }
    
    JSVirtualMachine            *vm;
    VMScope                     *globalScope;

    VecJsDoubles                doubleValues;
    VecJsStrings                stringValues;
    VecJsObjects                objValues;
    VecJsNativeMemberFunction   nativeMemberFunctions;
    VecResourcePools            resourcePools;

    uint32_t                    firstFreeDoubleIdx;
    uint32_t                    firstFreeObjIdx;

};

/**
 * 运行时的函数调用上下文，包括了当前的调用堆栈等信息
 */
class VMContext {
public:
    VMContext(VMRuntime *runtime) : runtime(runtime) {
        // stackFrames.push_back(&runtime->global);
    }

    void throwException(ParseError err, cstr_t format, ...);

    VMRuntime                   *runtime;
    StackJsValues               stack;
    VecVMStackFrames            stackFrames; // 当前的函数调用栈，用于记录函数的调用层次
    ParseError                  error;
    string                      errorMessage;

};


class JSVirtualMachine {
public:
    JSVirtualMachine();

    void eval(cstr_t code, size_t len, VMContext *vmctx, VecVMStackScopes &stackScopes, const Arguments &args);
    void callMember(VMContext *vmctx, const JsValue &obj, const char *memberName, Arguments &args);

    void dump(cstr_t code, size_t len, BinaryOutputStream &stream);
    void dump(BinaryOutputStream &stream);

    VMContext *mainVmContext() { return _mainVmCtx; }
    VMScope *globalScope() { return _globalScope; }

protected:
    void call(Function *function, VMContext *ctx, VecVMStackScopes &stackScopes, const Arguments &args);

protected:
    VMRuntime                   _runtime;

    // 主函数的调用上下文
    VMContext                   *_mainVmCtx;
    VMScope                     *_globalScope;

    JitRuntime                  _jit;

};

inline uint16_t readUInt16(uint8_t *&bytecode) { auto r = *(uint16_t *)bytecode; bytecode += 2; return r; }
inline uint32_t readUInt32(uint8_t *&bytecode) { auto r = *(uint32_t *)bytecode; bytecode += 4; return r; }
inline int32_t readInt32(uint8_t *&bytecode) { auto r = *(int32_t *)bytecode; bytecode += 4; return r; }
inline uint64_t readUInt64(uint8_t *&bytecode) { auto r = *(uint64_t *)bytecode; bytecode += 8; return r; }
inline int64_t readInt64(uint8_t *&bytecode) { auto r = *(int64_t *)bytecode; bytecode += 8; return r; }

#endif /* VirtualMachine_hpp */
