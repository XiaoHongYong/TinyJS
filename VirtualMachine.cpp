//
//  VirtualMachine.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/24.
//

#include "VirtualMachine.hpp"
#include "JitCodeCompiler.hpp"
#include "Parser.hpp"
#include "IJsObject.hpp"
#include "WebAPI/WebAPI.hpp"


#define MAX_STACK_SIZE 1024 * 1024

void registerGlobalValue(VMContext *ctx, VMScope *globalScope, const char *strName, const JsValue &value) {
    auto name = makeSizedString(strName);
    auto scopeDsc = globalScope->scopeDsc;

    auto id = PoolNew(scopeDsc->function->resourcePool->pool, IdentifierDeclare)(name, scopeDsc);
    id->storageIndex = scopeDsc->countLocalVars++;
    id->scopeDepth = scopeDsc->depth;
    id->varStorageType = VST_GLOBAL_VAR;
    id->isReferredByChild = true;

    assert(scopeDsc->varDeclares.find(name) == scopeDsc->varDeclares.end());
    scopeDsc->varDeclares[name] = id;

    assert(id->storageIndex == globalScope->vars.size());
    globalScope->vars.push_back(value);
}

void registerGlobalObject(VMContext *ctx, VMScope *globalScope, const char *strName, IJsObject *obj) {
    auto idx = ctx->runtime->pushObjValue(obj);

    registerGlobalValue(ctx, globalScope, strName, JsValue(JDT_OBJECT, idx));
}

class JsObjectFunction : public IJsObject {
public:
    JsObjectFunction(const VecVMStackScopes &stackScopes, Function *function) : stackScopes(stackScopes), function(function) {
        type = JDT_FUNCTION;
    }

    VecVMStackScopes            stackScopes;
    
    // 当前函数的代码
    Function                    *function;

};

Arguments::Arguments(const Arguments &other) {
    if (other.needFree) {
        copy(other);
    } else {
        data = other.data;
        count = other.count;
    }
}

Arguments::~Arguments() {
    if (needFree) {
        delete [] data;
    }
}

Arguments & Arguments::operator = (const Arguments &other) {
    if (other.needFree) {
        copy(other);
    } else {
        data = other.data;
        count = other.count;
    }

    return *this;
}

void Arguments::copy(const Arguments &other) {
    data = new JsValue[other.count];
    memcpy((void *)data, other.data, sizeof(data[0]) * other.count);
    count = other.count;
}


void VMContext::throwException(ParseError err, cstr_t format, ...) {
    if (error != PE_OK) {
        return;
    }

    error = err;
    CStrPrintf strf;

    va_list        args;

    va_start(args, format);
    strf.vprintf(format, args);
    va_end(args);

    errorMessage = strf.c_str();
}

JSVirtualMachine::JSVirtualMachine() : _runtime(this) {
    _mainVmCtx = new VMContext(&_runtime);
    _mainVmCtx->stack.reserve(MAX_STACK_SIZE);

    auto resourcePool = new ResourcePool();
    Function *rootFunc = new Function(resourcePool, nullptr, 0);
    _globalScope = new VMScope(rootFunc->scope);

    registerWebAPIs(_mainVmCtx, _globalScope);
}

void JSVirtualMachine::eval(cstr_t code, size_t len, VMContext *vmctx, VecVMStackScopes &stackScopes, const Arguments &args) {
    ResourcePool *resPool = new ResourcePool();
    resPool->index = (uint32_t)vmctx->runtime->resourcePools.size();
    vmctx->runtime->resourcePools.push_back(resPool);

    code = resPool->pool.duplicate(code, len);

    JSParser paser(resPool, code, strlen(code));

    auto func = paser.parse(stackScopes.back()->scopeDsc, false);

    VecVMStackFrames stackFrames;
    call(func, vmctx, stackScopes, args);
}

void JSVirtualMachine::dump(cstr_t code, size_t len, BinaryOutputStream &stream) {
    ResourcePool *resPool = new ResourcePool();
    resPool->index = 0;

    JSParser paser(resPool, code, strlen(code));

    Function *rootFunc = new Function(resPool, nullptr, 0);
    auto func = paser.parse(rootFunc->scope, false);

    func->dump(stream);
}

void JSVirtualMachine::callMember(VMContext *ctx, const JsValue &obj, const char *memberName, Arguments &args) {

    ctx->stack.push_back(JsValue());
}

void JSVirtualMachine::call(Function *function, VMContext *ctx, VecVMStackScopes &stackScopes, const Arguments &args) {
    if (function->bytecode == nullptr) {
        function->generateByteCode();
    }

    auto runtime = ctx->runtime;
    auto resourcePool = function->resourcePool;
    auto &stack = ctx->stack;
    auto functionScope = new VMScope(function->scope);
    auto bytecode = function->bytecode, endBytecode = bytecode + function->lenByteCode;

    ctx->stackFrames.push_back(new VMFunctionFrame(functionScope, function));

    auto scopeLocal = functionScope;
    stackScopes.push_back(scopeLocal);

    if (function->isArgumentsReferredByChild) {
        // 参数数组需要被复制
        functionScope->args.copy(args);
    } else {
        functionScope->args = args;
    }

    while (bytecode < endBytecode) {
        auto code = *bytecode++;
        switch (code) {
            case OP_ADD: {
                JsValue right = stack.back();
                stack.pop_back();
                JsValue left = stack.back();
                switch (left.type) {
                    case JDT_BOOL:
                        switch (right.type) {
                            case JDT_BOOL:
                                left.value.n32 += right.value.n32;
                                left.type = JDT_INT32;
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }

                stack.back() = left;
                break;
            }
            case OP_DIRECT_FUNCTION_CALL: {
                auto depth = *bytecode++;
                auto index = readUInt16(bytecode);
                auto countArgs = readUInt16(bytecode);
                Arguments args(stack.data() + stack.size() - countArgs, countArgs);

                if (depth + 1 == stackScopes.size()) {
                    // 当前函数的子函数
                    auto targFunction = function->functions[index];
                    call(targFunction, ctx, stackScopes, args);
                } else {
                    // 父函数的子函数
                    auto scope = stackScopes.at(depth);
                    auto targFunction = scope->scopeDsc->function->functions[index];
                    VecVMStackScopes targetStackScopes;
                    targetStackScopes.insert(targetStackScopes.begin(), stackScopes.begin(), stackScopes.begin() + depth + 1);
                    call(targFunction, ctx, targetStackScopes, args);
                }
                break;
            }
            case OP_FUNCTION_CALL: {
                uint16_t countArgs = readUInt16(bytecode);
                JsValue func = stack.at(stack.size() - countArgs - 1);
                switch (func.type) {
                    case JDT_FUNCTION:

                        break;

                    default:
                        break;
                }
                break;
            }
            case OP_MEMBER_FUNCTION_CALL: {
                uint16_t countArgs = readUInt16(bytecode);
                size_t posThiz = stack.size() - countArgs - 2;
                Arguments args(stack.data() + posThiz + 2, countArgs);
                JsValue thiz = stack.at(posThiz);
                JsValue func = stack.at(posThiz + 1);
                switch (func.type) {
                    case JDT_NATIVE_MEMBER_FUNCTION: {
                        auto f = runtime->nativeMemberFunctions[func.value.objIndex];
                        f(ctx, thiz, args);
                        break;
                    }
                    default:
                        break;
                }
                auto ret = stack.back();
                stack.resize(posThiz);
                stack.push_back(ret);
                break;
            }
            case OP_ENTER_SCOPE: {
                auto scopeIdx = readUInt16(bytecode);
                auto childScope = function->scopes[scopeIdx];
                scopeLocal = new VMScope(childScope);
                stackScopes.push_back(scopeLocal);

                // 将当前 scope 的 functionDecls 添加到 functionScope 中
                auto &childFuncs = childScope->functionDecls;
                for (auto f : childFuncs) {
                    auto index = runtime->pushObjValue(new JsObjectFunction(stackScopes, f));
                    functionScope->vars[f->declare->storageIndex] = JsValue(JDT_FUNCTION, index);
                }

                if (childScope->hasWith || childScope->hasEval) {
                    
                }
                break;
            }
            case OP_LEAVE_SCOPE: {
                stackScopes.pop_back();
                scopeLocal = stackScopes.back();
                break;
            }
            case OP_PUSH_STRING: {
                auto idx = readUInt32(bytecode);
                stack.push_back(makeJsValueOfStringInResourcePool(function->resourcePool->index, idx));
                break;
            }
            case OP_PUSH_INT32: {
                auto value = readInt32(bytecode);
                stack.push_back(JsValue(value));
                break;
            }
            case OP_PUSH_DOUBLE: {
                auto idx = readUInt32(bytecode);

                JsValue v;
                v.type = JDT_NUMBER;
                v.isInResourcePool = true;
                v.value.resourcePool.poolIndex = function->resourcePool->index;
                v.value.resourcePool.index = idx;

                stack.push_back(v);
                break;
            }
            case OP_PUSH_ID_LOCAL_SCOPE: {
                auto idx = readUInt16(bytecode);
                stack.push_back(scopeLocal->vars[idx]);
                break;
            }
            case OP_PUSH_ID_GLOBAL: {
                auto idx = readUInt16(bytecode);
                stack.push_back(_globalScope->vars[idx]);
                break;
            }
            case OP_PUSH_THIS_MEMBER_DOT: {
                auto idx = readUInt32(bytecode);
                auto prop = resourcePool->strings[idx];
                const JsValue &obj = stack.back();
                switch (obj.type) {
                    case JDT_NOT_INITIALIZED:
                        ctx->throwException(PE_REFERECNE_ERROR, "Cannot access variable before initialization");
                        break;
                    case JDT_UNDEFINED:
                        ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of undefined (reading %.*s)", prop.len, prop.data);
                        break;
                    case JDT_NULL:
                        ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of null (reading %.*s)", prop.len, prop.data);
                        break;
                    case JDT_INT32:
                        // TODO...
                        break;
                    case JDT_BOOL:
                        // TODO...
                        break;
                    case JDT_NUMBER:
                        // TODO...
                        break;
                    case JDT_STRING: {
                        // TODO...
                        break;
                    }
                    case JDT_OBJECT: {
                        auto jsobj = runtime->getObject(obj);
                        stack.push_back(jsobj->get(ctx, prop));
                        break;
                    }
                    case JDT_REGEX:
                    case JDT_ARRAY:
                    case JDT_FUNCTION:
                    case JDT_NATIVE_FUNCTION:
                    {
                        break;
                    }
                    default:
                        assert(0);
                        break;
                }
                break;
            }
            case OP_ASSIGN_IDENTIFIER: {
                auto varStorageType = *bytecode++;
                auto scopeDepth = *bytecode++;
                auto storageIndex = readUInt16(bytecode);
                VMScope *scope;
                switch (varStorageType) {
                    case VST_ARGUMENT:
                        scope = stackScopes[scopeDepth];
                        scope->args[storageIndex] = stack.back();
                        break;
                    case VST_SCOPE_VAR:
                    case VST_FUNCTION_VAR:
                        scope = stackScopes[scopeDepth];
                        scope->vars[storageIndex] = stack.back();
                        break;
                    case VST_GLOBAL_VAR:
                        _globalScope->vars[storageIndex] = stack.back();
                        break;
                    default:
                        assert(0);
                        break;
                }
                break;
            }
            default:
                ERR_LOG1("Unkown opcode: %d", code);
                break;
        }
    }

    stackScopes.pop_back();
    ctx->stackFrames.pop_back();
}

/*
VMNativeCodePtr JSVirtualMachine::compile(Scope *scope, Function *function, string &errMessageOut) {
    CodeHolder code;
    SimpleAsmJitErrorHandler errHandler;

    code.init(_jit.codeInfo());
    code.setErrorHandler(&errHandler);

    JitCodeCompiler compiler(code);

    // Refer JitEval::FuncEval
    compiler.addFunc(FuncSignatureT<uint64_t, uint64_t>());

    compiler.saveArg1();

//    Label nullLable = compiler.newLabel();

    // SimpleOperandPtr ret = function->genJitCode(compiler);
    // SimpleOperandPtr ret = genJitOfExpression(compiler, expr, nullLable);

    // compiler.ret_so(ret);

    // Return null
//    compiler.bind(nullLable);
//    compiler.mov(x86::byte_ptr(compiler.getSavedArg1()->gp, offsetof(ExeExprBlockCtx, isResultNull)), Imm(1));
//    ret = compiler.makeSimpleOperandPtr(0);
//    compiler.ret_so(ret);

    compiler.endFunc();
    compiler.finalize();

    // printf("%s\n", expr->toString().c_str());
    // compiler.dumpToConsole();

    VMNativeCodePtr vmcode = std::make_shared<VMNativeCode>();

    auto err = _jit.add(&vmcode->entryFunction, &code);
    if (err != 0) {
        errMessageOut = errHandler.getErrorMessage();
        return nullptr;
    }

    if (errHandler.failed()) {
        errMessageOut = errHandler.getErrorMessage();
        return nullptr;
    }

    return nullptr;
}

JsValue JSVirtualMachine::eval(const VMNativeCodePtr &code, VMContext *vmctx) {
    assert(vmctx);
    assert(code.get());

    return code->entryFunction((uint64_t)vmctx);
}

JsValue JSVirtualMachine::eval(Function *function, VMContext *vmctx) {
    //function->instructions.
}
*/
