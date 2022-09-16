//
//  VirtualMachine.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/24.
//

#ifndef VirtualMachine_hpp
#define VirtualMachine_hpp

#include "VMRuntime.hpp"


class VMScope;
class VMScopeDescriptor;
class VMFunctionFrame;
class VMRuntimeCommon;
class VMRuntime;
class VMContext;
class JsVirtualMachine;
class Arguments;


using VMFunctionFramePtr = std::shared_ptr<VMFunctionFrame>;
using VecVMScopes = std::vector<VMScope *>;
using VecVMStackScopes = std::vector<VMScope *>;
using StackJsValues = std::vector<JsValue>;
using VecVMStackFrames = std::vector<VMFunctionFramePtr>;

void registerGlobalValue(VMContext *ctx, VMScope *globalScope, const char *name, const JsValue &value);
void registerGlobalObject(VMContext *ctx, VMScope *globalScope, const char *name, IJsObject *obj);

JsValue newJsError(VMContext *ctx, JsErrorType errType, const JsValue &message = jsValueUndefined);

enum VMMiscFlags {
    COMMON_STRINGS              = 1,

    VAR_IDX_THIS                = 0,
    VAR_IDX_ARGUMENTS           = 1,

    POOL_STRING_IDX_INVALID     = 0,
};

class Arguments {
public:
    Arguments() { data = nullptr; count = 0; capacity = 0; needFree = false; }
    Arguments(const Arguments &other);
    Arguments(JsValue *args, uint32_t count) : data(args), count(count), capacity(count) { needFree = false; }
    ~Arguments();

    Arguments &operator = (const Arguments &other);
    JsValue &operator[](uint32_t n) const { assert(n < capacity); return data[n]; }

    void copy(const Arguments &other, uint32_t minSize = 0);

    JsValue                     *data;
    uint32_t                    count;
    uint32_t                    capacity;
    bool                        needFree;
};

class ArgumentsX : public Arguments {
public:
    ArgumentsX(const JsValue &one) {
        data = args;
        args[0] = one;
        count = 1;
        needFree = false;
    }

    JsValue                     args[1];

};

/**
 * VMScope 用于存储当前作用域内的标识符信息.
 */
class VMScope {
public:
    VMScope(Scope *scopeDsc) : scopeDsc(scopeDsc) {
        referIdx = 0;
        nextFreeIdx = 0;

        vars.resize(scopeDsc->countLocalVars);
    }

    void dump(BinaryOutputStream &stream);

    int8_t                      referIdx;
    uint32_t                    nextFreeIdx;

    Scope                       *scopeDsc;

    // 作用域内的所有变量
    VecJsValues                 vars; // 当前作用域下的变量

    Arguments                   args;

    // 当使用 with 语句，在执行时会在 'withValue' 的 member 中查找标识符
    JsValue                     withValue;

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

struct TryCatchPoint {
    uint32_t                    flags; // 用于判断是否为当前函数的异常
    uint32_t                    scopeDepth; // 进入 try 时的 scopeDepth，当出现异常后，需要释放 scope.
    uint32_t                    stackSize; // 进入 try 时的 scopeDepth，当出现异常后，需要释放 stack.
    uint32_t                    addrCatch;
    uint32_t                    addrFinally;

    TryCatchPoint(uint32_t flags, uint32_t scopeDepth, uint32_t stackSize, uint32_t addrCatch, uint32_t addrFinally) : flags(flags), scopeDepth(scopeDepth), stackSize(stackSize), addrCatch(addrCatch), addrFinally(addrFinally) {
    }
};

using StackTryCatchPoint = stack<TryCatchPoint>;

/**
 * 运行时的函数调用上下文，包括了当前的调用堆栈等信息
 */
class VMContext {
private:
    VMContext(const VMContext &);
    VMContext &operator=(const VMContext &);

public:
    VMContext(VMRuntime *runtime);

    void throwException(JsErrorType err, cstr_t format, ...);
    void throwException(JsErrorType err, JsValue errorMessage);

    JsVirtualMachine            *vm;
    VMScope                     *curFunctionScope;
    VMRuntime                   *runtime;

    // 调用 native function 时需要传递的参数
    VecVMStackScopes            *stackScopesForNativeFunctionCall;

    StackJsValues               stack;
    VecVMStackFrames            stackFrames; // 当前的函数调用栈，用于记录函数的调用层次

    StackTryCatchPoint          stackTryCatch;

    bool                        isReturnedForTry; // 如果 return 在 try 中，并且有 fainaly 会设置此标志
    JsValue                     retValue; // 函数的返回值

    JsErrorType                 errorInTry;
    JsValue                     errorMessageInTry;

    JsValue                     errorMessage;
    JsErrorType                 error;

};


class JsVirtualMachine {
private:
    JsVirtualMachine(const JsVirtualMachine &);
    JsVirtualMachine &operator=(const JsVirtualMachine &);

public:
    JsVirtualMachine();
    virtual ~JsVirtualMachine();

    void run(cstr_t code, size_t len, VMRuntime *runtime = nullptr);

    void eval(cstr_t code, size_t len, VMContext *ctx, VecVMStackScopes &stackScopes, const Arguments &args);
    void callMember(VMContext *ctx, const JsValue &thiz, const char *memberName, const Arguments &args);
    void callMember(VMContext *ctx, const JsValue &thiz, const JsValue &memberFunc, const Arguments &args);

    JsValue getMemberDot(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &defVal = jsValueUndefined);
    void setMemberDot(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value);

    JsValue getMemberIndex(VMContext *ctx, const JsValue &thiz, const JsValue &prop);
    void setMemberIndex(VMContext *ctx, const JsValue &thiz, const JsValue &prop, const JsValue &value);

    void dump(cstr_t code, size_t len, BinaryOutputStream &stream);
    void dump(BinaryOutputStream &stream);

    VMRuntime *defaultRuntime() { return &_runtime; }

protected:
    void call(Function *function, VMContext *ctx, VecVMStackScopes &stackScopes, const JsValue &thiz, const Arguments &args);

protected:
    VMRuntimeCommon             _runtimeCommon;

    VMRuntime                   _runtime;

};

inline uint8_t readUInt8(uint8_t *&bytecode) { return *bytecode++; }
inline uint16_t readUInt16(uint8_t *&bytecode) { auto r = *(uint16_t *)bytecode; bytecode += 2; return r; }
inline uint32_t readUInt32(uint8_t *&bytecode) { auto r = *(uint32_t *)bytecode; bytecode += 4; return r; }
inline int32_t readInt32(uint8_t *&bytecode) { auto r = *(int32_t *)bytecode; bytecode += 4; return r; }
inline uint64_t readUInt64(uint8_t *&bytecode) { auto r = *(uint64_t *)bytecode; bytecode += 8; return r; }
inline int64_t readInt64(uint8_t *&bytecode) { auto r = *(int64_t *)bytecode; bytecode += 8; return r; }

#endif /* VirtualMachine_hpp */
