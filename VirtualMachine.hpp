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


using VecVMScopes = std::vector<VMScope *>;
using VecVMStackScopes = std::vector<VMScope *>;
using VecVMStackFrames = std::vector<VMFunctionFrame *>;
using StackJsValues = std::vector<JsValue>;

void registerGlobalValue(VMContext *ctx, VMScope *globalScope, const char *name, const JsValue &value);
void registerGlobalObject(VMContext *ctx, VMScope *globalScope, const char *name, IJsObject *obj);

enum VMMiscFlags {
    COMMON_STRINGS              = 1,

    VAR_IDX_THIS                = 0,
    VAR_IDX_ARGUMENTS           = 1,

    POOL_STRING_IDX_INVALID     = 0,
};

class Arguments {
public:
    Arguments() { data = nullptr; count = 0; needFree = false; }
    Arguments(const Arguments &other);
    Arguments(JsValue *args, uint32_t count) : data(args), count(count) { needFree = false; }
    ~Arguments();

    Arguments &operator = (const Arguments &other);
    JsValue &operator[](uint32_t n) const { assert(n < count); return data[n]; }

    void copy(const Arguments &other);

    JsValue                     *data;
    uint32_t                    count;
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
    VMScope(Scope *scopeDsc) : scopeDsc(scopeDsc) { vars.resize(scopeDsc->countLocalVars); }

    void dump(BinaryOutputStream &stream);

    Scope                       *scopeDsc;

    // 作用域内的所有变量
    VecJsValues                 vars; // 当前作用域下的变量

    Arguments                   args;

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
 * 运行时的函数调用上下文，包括了当前的调用堆栈等信息
 */
class VMContext {
private:
    VMContext(const VMContext &);
    VMContext &operator=(const VMContext &);

public:
    VMContext(VMRuntime *runtime) : runtime(runtime) {
        // stackFrames.push_back(&runtime->global);
        vm = runtime->vm;
    }

    void throwException(ParseError err, cstr_t format, ...);

    JsVirtualMachine            *vm;
    VMScope                     *curFunctionScope;
    VMRuntime                   *runtime;
    StackJsValues               stack;
    VecVMStackFrames            stackFrames; // 当前的函数调用栈，用于记录函数的调用层次
    ParseError                  error;
    string                      errorMessage;

};


class JsVirtualMachine {
private:
    JsVirtualMachine(const JsVirtualMachine &);
    JsVirtualMachine &operator=(const JsVirtualMachine &);

public:
    JsVirtualMachine();

    void eval(cstr_t code, size_t len, VMContext *ctx, VecVMStackScopes &stackScopes, const Arguments &args);
    void callMember(VMContext *ctx, const JsValue &thiz, const char *memberName, const Arguments &args);
    void callMember(VMContext *ctx, const JsValue &thiz, const JsValue &memberFunc, const Arguments &args);

    JsValue getMemberDot(VMContext *ctx, const JsValue &thiz, const SizedString &prop);
    void setMemberDot(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value);

    void dump(cstr_t code, size_t len, BinaryOutputStream &stream);
    void dump(BinaryOutputStream &stream);

    VMRuntime *defaultRuntime() { return _runtime; }
    
protected:
    void call(Function *function, VMContext *ctx, VecVMStackScopes &stackScopes, const JsValue &thiz, const Arguments &args);

protected:
    VMRuntimeCommon             *_runtimeCommon;

    VMRuntime                   *_runtime;

    JitRuntime                  _jit;

};

inline uint16_t readUInt16(uint8_t *&bytecode) { auto r = *(uint16_t *)bytecode; bytecode += 2; return r; }
inline uint32_t readUInt32(uint8_t *&bytecode) { auto r = *(uint32_t *)bytecode; bytecode += 4; return r; }
inline int32_t readInt32(uint8_t *&bytecode) { auto r = *(int32_t *)bytecode; bytecode += 4; return r; }
inline uint64_t readUInt64(uint8_t *&bytecode) { auto r = *(uint64_t *)bytecode; bytecode += 8; return r; }
inline int64_t readInt64(uint8_t *&bytecode) { auto r = *(int64_t *)bytecode; bytecode += 8; return r; }

#endif /* VirtualMachine_hpp */
