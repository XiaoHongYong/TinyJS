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
#include "VMRuntime.hpp"
#include "JsArguments.hpp"
#include "JsLibObject.hpp"
#include "JsObjectFunction.hpp"
#include "JsArray.hpp"


void registerGlobalValue(VMContext *ctx, VMScope *globalScope, const char *strName, const JsValue &value) {
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

void registerGlobalObject(VMContext *ctx, VMScope *globalScope, const char *strName, IJsObject *obj) {
    auto idx = ctx->runtime->pushObjValue(obj);

    registerGlobalValue(ctx, globalScope, strName, JsValue(JDT_OBJECT, idx));
}

uint32_t makeTryCatchPointFlags(Function *function, VMScope *scope) {
    uint64_t n = (uint64_t)function ^ (uint64_t)scope;
    return (n & 0xFFFFFFFF) ^ (n >> 32);
}

Arguments::Arguments(const Arguments &other) {
    if (other.needFree) {
        copy(other);
    } else {
        data = other.data;
        capacity = count = other.count;
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
        capacity = count = other.count;
    }

    return *this;
}

void Arguments::copy(const Arguments &other, uint32_t minSize) {
    capacity = std::max(other.count, minSize);
    data = new JsValue[capacity];
    memcpy((void *)data, other.data, sizeof(data[0]) * other.count);
    count = other.count;

    // 将额外的位置赋值为 Undefined.
    while (minSize > count) {
        data[--minSize] = JsUndefinedValue;
    }
}

VMContext::VMContext(VMRuntime *runtime) : runtime(runtime) {
    vm = runtime->vm;;
    stackScopesForNativeFunctionCall = nullptr;
    curFunctionScope = nullptr;
    isReturnedForTry = false;

    errorInTry = PE_OK;
    error = PE_OK;
}

void VMContext::throwException(ParseError err, cstr_t format, ...) {
    if (error != PE_OK) {
        return;
    }

    va_list        args;

    va_start(args, format);
    auto message = stringPrintf(format, args);
    va_end(args);

    message.insert(0, parseErrorToString(err));

    // 将异常值添加到堆栈中
    // TODO: 需要转换为 err 对应的异常类型
    errorMessage = runtime->pushString(SizedString(message.c_str(), message.size()));

    throwException(err, errorMessage);
}

void VMContext::throwException(ParseError err, JsValue errorMessage) {
    this->error = err;
    this->errorMessage = errorMessage;

    if (isReturnedForTry) {
        // 在 finally 中抛出异常，会清除 return 相关内容
        isReturnedForTry = false;
    }

    if (errorInTry != PE_OK) {
        // 在 finally 中抛出异常，会清除上一次的异常内容
        errorInTry = PE_OK;
        errorMessageInTry = JsUndefinedValue;
    }
}


JsVirtualMachine::JsVirtualMachine() {
    _runtimeCommon = new VMRuntimeCommon();
    _runtime = new VMRuntime();

    _runtime->init(this, _runtimeCommon);
}

void JsVirtualMachine::run(cstr_t code, size_t len, VMRuntime *runtime) {
    if (runtime == nullptr) {
        runtime = _runtime;
    }

    VecVMStackScopes stackScopes;
    Arguments args;
    auto ctx = runtime->mainVmCtx;

    stackScopes.push_back(runtime->globalScope);
    ctx->curFunctionScope = runtime->globalScope;

    eval(code, len, ctx, stackScopes, args);
    if (ctx->error) {
        string buf;
        auto message = runtime->toSizedString(ctx, ctx->errorMessage, buf);
        runtime->console->error(CStrPrintf("Uncaught %.*s\n", message.len, message.data).c_str());
    }
}

void JsVirtualMachine::eval(cstr_t code, size_t len, VMContext *vmctx, VecVMStackScopes &stackScopes, const Arguments &args) {
    auto runtime = vmctx->runtime;
    ResourcePool *resPool = new ResourcePool();
    resPool->index = (uint32_t)runtime->resourcePools.size();
    runtime->resourcePools.push_back(resPool);

    auto p = (char *)resPool->pool.allocate(len + 4);
    memcpy(p, code, len);
    memset(p + len, 0, 4);
    code = p;

    JSParser parser(_runtimeCommon, resPool, code, len);

    Function *func = nullptr;
    try {
        func = parser.parse(stackScopes.back()->scopeDsc, false);
    } catch (ParseException &e) {
        vmctx->throwException(e.error, e.message.c_str());
        return;
    }

    {
        BinaryOutputStream stream;
        func->dump(stream);
        auto s = stream.sizedStringStartNew();
        printf("%s\n", code);
        printf("%.*s\n", (int)s.len, s.data);
    }

    // 检查全局变量的空间
    auto globalScope = runtime->globalScope;
    if (globalScope->vars.size() < globalScope->scopeDsc->varDeclares.size()) {
        globalScope->vars.resize(globalScope->scopeDsc->varDeclares.size(), JsNotInitializedValue);
    }
    
    VecVMStackFrames stackFrames;
    call(func, vmctx, stackScopes, runtime->globalThiz, args);
}

void JsVirtualMachine::dump(cstr_t code, size_t len, BinaryOutputStream &stream) {
    ResourcePool *resPool = new ResourcePool();
    resPool->index = 0;

    JSParser paser(_runtimeCommon, resPool, code, strlen(code));

    Function *rootFunc = new Function(resPool, nullptr, 0);
    auto func = paser.parse(rootFunc->scope, false);

    func->dump(stream);
}

void JsVirtualMachine::dump(BinaryOutputStream &stream) {
    BinaryOutputStream os;
    _runtime->dump(os);
    stream.write("== VMRuntime ==\n");
    writeIndent(stream, os.sizedStringStartNew(), makeSizedString("  "));
}

void JsVirtualMachine::callMember(VMContext *ctx, const JsValue &thiz, const char *memberName, const Arguments &args) {
    auto member = getMemberDot(ctx, thiz, memberName);

    callMember(ctx, thiz, member, args);
}

void JsVirtualMachine::callMember(VMContext *ctx, const JsValue &thiz, const JsValue &memberFunc, const Arguments &args) {
    auto runtime = ctx->runtime;
    if (memberFunc.type == JDT_NATIVE_FUNCTION) {
        auto f = runtime->getNativeFunction(memberFunc.value.index);
        ctx->stackScopesForNativeFunctionCall = nullptr;
        f(ctx, thiz, args);
    } else if (memberFunc.type == JDT_LIB_OBJECT) {
        auto obj = (JsLibObject *)runtime->getObject(memberFunc);
        auto f = obj->getFunction();
        if (f) {
            f(ctx, thiz, args);
        } else {
            ctx->throwException(PE_TYPE_ERROR, "value is not a function");
            assert(0);
        }
    } else if (memberFunc.type == JDT_FUNCTION) {
        auto f = (JsObjectFunction *)runtime->getObject(memberFunc);
        call(f->function, ctx, f->stackScopes, thiz, args);
    } else {
        ctx->throwException(PE_TYPE_ERROR, "value is not a function");
        assert(0);
    }
}

JsValue JsVirtualMachine::getMemberDot(VMContext *ctx, const JsValue &thiz, const SizedString &prop) {
    switch (thiz.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_REFERECNE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of undefined (reading '%.*s')", (int)prop.len, prop.data);
            break;
        case JDT_NULL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of null (reading '%.*s')", (int)prop.len, prop.data);
        case JDT_BOOL:
            return _runtimeCommon->objPrototypeBoolean->getByName(ctx, thiz, prop);
        case JDT_INT32:
            return _runtimeCommon->objPrototypeNumber->getByName(ctx, thiz, prop);
        case JDT_NUMBER:
            return _runtimeCommon->objPrototypeNumber->getByName(ctx, thiz, prop);
        case JDT_CHAR:
        case JDT_STRING:
            return _runtimeCommon->objPrototypeString->getByName(ctx, thiz, prop);
        case JDT_SYMBOL:
            return _runtimeCommon->objPrototypeSymbol->getByName(ctx, thiz, prop);
        case JDT_FUNCTION:
            return _runtimeCommon->objPrototypeFunction->getByName(ctx, thiz, prop);
        case JDT_ARRAY:
            return _runtimeCommon->objPrototypeArray->getByName(ctx, thiz, prop);
        default: {
            auto jsthiz = ctx->runtime->getObject(thiz);
            return jsthiz->getByName(ctx, thiz, prop);
        }
    }

    return JsUndefinedValue;
}

void JsVirtualMachine::setMemberDot(VMContext *ctx, const JsValue &thiz, const SizedString &prop, const JsValue &value) {
    switch (thiz.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_REFERECNE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot set properties of undefined (setting '%.*s')", (int)prop.len, prop.data);
            break;
        case JDT_NULL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot set properties of null (setting '%.*s')", (int)prop.len, prop.data);
            break;
        case JDT_BOOL:
        case JDT_INT32:
        case JDT_NUMBER:
        case JDT_CHAR:
        case JDT_STRING:
        case JDT_SYMBOL: {
            // Primitive types' member cannot be set.
            break;
        }
        default: {
            auto jsthiz = ctx->runtime->getObject(thiz);
            jsthiz->setByName(ctx, thiz, prop, value);
            break;
        }
    }
}

JsValue JsVirtualMachine::getMemberIndex(VMContext *ctx, const JsValue &thiz, const JsValue &prop) {
    switch (thiz.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_REFERECNE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of undefined");
            break;
        case JDT_NULL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of null");
        case JDT_BOOL:
            return _runtimeCommon->objPrototypeBoolean->get(ctx, thiz, prop);
        case JDT_INT32:
            return _runtimeCommon->objPrototypeNumber->get(ctx, thiz, prop);
        case JDT_NUMBER:
            return _runtimeCommon->objPrototypeNumber->get(ctx, thiz, prop);
        case JDT_CHAR:
        case JDT_STRING:
            return _runtimeCommon->objPrototypeString->get(ctx, thiz, prop);
        case JDT_SYMBOL:
            return _runtimeCommon->objPrototypeSymbol->get(ctx, thiz, prop);
        default: {
            auto jsthiz = ctx->runtime->getObject(thiz);
            return jsthiz->get(ctx, thiz, prop);
        }
    }

    return JsUndefinedValue;
}

void JsVirtualMachine::setMemberIndex(VMContext *ctx, const JsValue &thiz, const JsValue &prop, const JsValue &value) {
    switch (thiz.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_REFERECNE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot set properties of undefined");
            break;
        case JDT_NULL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot set properties of null");
            break;
        case JDT_BOOL:
        case JDT_INT32:
        case JDT_NUMBER:
        case JDT_CHAR:
        case JDT_STRING:
        case JDT_SYMBOL: {
            // Primitive types' member cannot be set.
            break;
        }
        default: {
            auto jsthiz = ctx->runtime->getObject(thiz);
            jsthiz->set(ctx, thiz, prop, value);
            break;
        }
    }
}

void JsVirtualMachine::call(Function *function, VMContext *ctx, VecVMStackScopes &stackScopes, const JsValue &thiz, const Arguments &args) {
    if (function->bytecode == nullptr) {
        function->generateByteCode();
    }

#ifdef DEBUG
        printf("    > Call function: %d, %.*s\n", function->index, (int)function->name.len, function->name.data);
#endif

    auto prevFunctionScope = ctx->curFunctionScope;
    auto runtime = ctx->runtime;
    auto resourcePool = function->resourcePool;
    auto &stack = ctx->stack;
    auto bytecode = function->bytecode, endBytecode = bytecode + function->lenByteCode;
    auto retValue = JsUndefinedValue;

    VMScope *functionScope, *scopeLocal;
    scopeLocal = new VMScope(function->scope);
    ctx->stackFrames.push_back(new VMFunctionFrame(scopeLocal, function));

    if (function->isCodeBlock) {
        assert(!stackScopes.empty());
        functionScope = prevFunctionScope;
        if (function->scope->countLocalVars > functionScope->vars.size()) {
            functionScope->vars.resize(function->scope->countLocalVars);
        }
    } else {
        functionScope = scopeLocal;
        ctx->curFunctionScope = functionScope;
        auto functionScopeDsc = function->scope;

        scopeLocal = functionScope;

        if (functionScopeDsc->countArguments > args.count) {
            // 传入的参数小于声明的参数数量，需要分配额外的空间给变量
            functionScope->args.copy(args, functionScopeDsc->countArguments);
        } else {
            if (function->isArgumentsReferredByChild || functionScopeDsc->isArgumentsUsed) {
                // 参数数组需要被复制
                functionScope->args.copy(args);
            } else {
                functionScope->args = args;
            }
        }
    }

    stackScopes.push_back(scopeLocal);

    while (bytecode < endBytecode) {
        auto code = (OpCode)*bytecode++;
#ifdef DEBUG
        printf("    %s\n", opCodeToString(code));
#endif
        switch (code) {
            case OP_INVALID: {
                assert(0);
                break;
            }
            case OP_PREPARE_VAR_THIS:
                functionScope->vars[VAR_IDX_THIS] = thiz;
                break;
            case OP_PREPARE_VAR_ARGUMENTS: {
                auto idx = runtime->pushObjValue(new JsArguments(functionScope, &functionScope->args));
                functionScope->vars[VAR_IDX_ARGUMENTS] = JsValue(JDT_OBJECT, idx);
                break;
            }
            case OP_INIT_FUNCTION_TO_VARS: {
                // 将根 scope 被引用到的函数添加到变量中
                for (auto f : function->scope->functionDecls) {
                    auto index = runtime->pushObjValue(new JsObjectFunction(stackScopes, f));
                    functionScope->vars[f->declare->storageIndex] = JsValue(JDT_FUNCTION, index);
                }
                break;
            }
            case OP_INIT_FUNCTION_TO_ARGS: {
                // 将根 scope 被引用到的函数添加到参数中
                for (auto f : *function->scope->functionArgs) {
                    auto index = runtime->pushObjValue(new JsObjectFunction(stackScopes, f));
                    functionScope->args[f->declare->storageIndex] = JsValue(JDT_FUNCTION, index);
                }
                break;
            }
            case OP_RETURN:
            case OP_RETURN_VALUE: {
                if (code == OP_RETURN_VALUE) {
                    retValue = stack.back();
                    stack.pop_back();
                } else {
                    retValue = JsUndefinedValue;
                }
                bytecode = endBytecode;

                // 检查是否在 try finally 中
                auto &stackTryCatch = ctx->stackTryCatch;
                if (!stackTryCatch.empty() && stackTryCatch.top().flags == makeTryCatchPointFlags(function, functionScope)) {
                    do {
                        // 当前函数有 try
                        auto action = stackTryCatch.top();
                        stackTryCatch.pop();
                        if (action.addrFinally) {
                            // 跳转到 finally
                            bytecode = function->bytecode + action.addrFinally;
                            ctx->isReturnedForTry = true;
                            break;
                        }
                    } while (!stackTryCatch.empty() && stackTryCatch.top().flags == makeTryCatchPointFlags(function, functionScope));
                }
                break;
            }
            case OP_DEBUGGER: {
                assert(0);
                break;
            }
            case OP_THROW: {
                ctx->throwException(PE_TYPE_ERROR, stack.back());
                stack.pop_back();
                break;
            }
            case OP_JUMP: {
                auto pos = readUInt32(bytecode);
                bytecode = function->bytecode + pos;
                break;
            }
            case OP_JUMP_IF_TRUE: {
                auto pos = readUInt32(bytecode);
                auto condition = stack.back();
                if (runtime->testTrue(condition)) {
                    bytecode = function->bytecode + pos;
                }
                stack.pop_back();
                break;
            }
            case OP_JUMP_IF_FALSE: {
                auto pos = readUInt32(bytecode);
                auto condition = stack.back();
                if (!runtime->testTrue(condition)) {
                    bytecode = function->bytecode + pos;
                }
                stack.pop_back();
                break;
            }
            case OP_JUMP_IF_NULL_UNDEFINED: {
                auto pos = readUInt32(bytecode);
                auto condition = stack.back();
                if (condition.type <= JDT_NULL) {
                    bytecode = function->bytecode + pos;
                }
                stack.pop_back();
                break;
            }
            case OP_JUMP_IF_NOT_NULL_UNDEFINED: {
                auto pos = readUInt32(bytecode);
                auto condition = stack.back();
                if (condition.type > JDT_NULL) {
                    bytecode = function->bytecode + pos;
                }
                stack.pop_back();
                break;
            }
            case OP_JUMP_IF_NOT_NULL_UNDEFINED_KEEP_VALID: {
                auto pos = readUInt32(bytecode);
                auto condition = stack.back();
                if (condition.type > JDT_NULL) {
                    bytecode = function->bytecode + pos;
                } else {
                    // condition 为 null/undefined，删除栈顶值
                    stack.pop_back();
                }
                break;
            }
            case OP_PREPARE_RAW_STRING_TEMPLATE_CALL: {
                assert(0);
                break;
            }
            case OP_FUNCTION_CALL: {
                uint16_t countArgs = readUInt16(bytecode);
                size_t posFunc = stack.size() - countArgs - 1;
                Arguments args(stack.data() + posFunc + 1, countArgs);
                JsValue func = stack.at(posFunc);
                switch (func.type) {
                    case JDT_FUNCTION: {
                        auto f = (JsObjectFunction *)runtime->getObject(func);
                        call(f->function, ctx, f->stackScopes, runtime->globalThiz, args);
                        break;
                    }
                    case JDT_NATIVE_FUNCTION: {
                        auto f = runtime->getNativeFunction(func.value.index);
                        ctx->stackScopesForNativeFunctionCall = &stackScopes;
                        f(ctx, runtime->globalThiz, args);
                        break;
                    }
                    case JDT_LIB_OBJECT: {
                        auto f = (JsLibObject *)runtime->getObject(func);
                        if (f->getFunction()) {
                            ctx->stackScopesForNativeFunctionCall = &stackScopes;
                            f->getFunction()(ctx, runtime->globalThiz, args);
                        } else {
                            ctx->throwException(PE_TYPE_ERROR, "? is not a function.");
                        }
                        break;
                    }
                    default:
                        assert(0);
                        break;
                }
                stack.resize(posFunc);
                stack.push_back(ctx->retValue);
                break;
            }
            case OP_MEMBER_FUNCTION_CALL: {
                uint16_t countArgs = readUInt16(bytecode);
                size_t posThiz = stack.size() - countArgs - 2;
                Arguments args(stack.data() + posThiz + 2, countArgs);
                JsValue thiz = stack.at(posThiz);
                JsValue func = stack.at(posThiz + 1);
                switch (func.type) {
                    case JDT_FUNCTION: {
                        auto f = (JsObjectFunction *)runtime->getObject(func);
                        call(f->function, ctx, f->stackScopes, thiz, args);
                        break;
                    }
                    case JDT_NATIVE_FUNCTION: {
                        auto f = runtime->getNativeFunction(func.value.index);
                        ctx->stackScopesForNativeFunctionCall = &stackScopes;
                        f(ctx, thiz, args);
                        break;
                    }
                    default:
                        assert(0);
                        break;
                }
                stack.resize(posThiz);
                stack.push_back(ctx->retValue);
                break;
            }
            case OP_DIRECT_FUNCTION_CALL: {
                auto depth = *bytecode++;
                auto index = readUInt16(bytecode);
                auto countArgs = readUInt16(bytecode);
                size_t posArgs = stack.size() - countArgs;
                Arguments args(stack.data() + posArgs, countArgs);

                if (depth + 1 == stackScopes.size()) {
                    // 当前函数的子函数
                    auto targFunction = function->functions[index];
                    call(targFunction, ctx, stackScopes, runtime->globalThiz, args);
                } else {
                    // 父函数的子函数
                    auto scope = stackScopes.at(depth);
                    auto targFunction = scope->scopeDsc->function->functions[index];
                    VecVMStackScopes targetStackScopes;
                    targetStackScopes.insert(targetStackScopes.begin(), stackScopes.begin(), stackScopes.begin() + depth + 1);
                    call(targFunction, ctx, targetStackScopes, runtime->globalThiz, args);
                }
                stack.resize(posArgs);
                stack.push_back(ctx->retValue);
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
            // default:
            //    ERR_LOG1("Unkown opcode: %d", code);
            //    break;
            case OP_POP_STACK_TOP: {
                stack.pop_back();
                break;
            }
            case OP_PUSH_STACK_TOP: {
                assert(0);
                break;
            }
            case OP_PUSH_UNDFINED: {
                stack.push_back(JsUndefinedValue);
                break;
            }
            case OP_PUSH_IDENTIFIER: {
                assert(0);
                break;
            }
            case OP_PUSH_ID_GLOBAL: {
                auto idx = readUInt16(bytecode);
                auto globalScope = runtime->globalScope;
                assert(idx < globalScope->vars.size());
                auto v = globalScope->vars[idx];
                if (v.type == JDT_NOT_INITIALIZED) {
                    auto declare = globalScope->scopeDsc->getVarDeclarationByIndex(idx);
                    assert(declare);
                    ctx->throwException(PE_REFERECNE_ERROR, "%.*s is not defined", (int)declare->name.len, declare->name.data);
                    break;
                }
                stack.push_back(v);
                break;
            }
            case OP_PUSH_ID_LOCAL_ARGUMENT: {
                auto idx = readUInt16(bytecode);
                assert(idx < functionScope->args.capacity);
                stack.push_back(functionScope->args[idx]);
                break;
            }
            case OP_PUSH_ID_LOCAL_SCOPE: {
                auto idx = readUInt16(bytecode);
                stack.push_back(scopeLocal->vars[idx]);
                break;
            }
            case OP_PUSH_ID_PARENT_ARGUMENT: {
                auto scopeDepth = *bytecode++;
                auto idx = readUInt16(bytecode);
                assert(scopeDepth < stackScopes.size());
                auto scope = stackScopes[scopeDepth];
                assert(idx < scope->args.capacity);
                stack.push_back(scope->vars[idx]);
                break;
            }
            case OP_PUSH_ID_PARENT_SCOPE: {
                auto scopeDepth = *bytecode++;
                auto idx = readUInt16(bytecode);
                assert(scopeDepth < stackScopes.size());
                auto scope = stackScopes[scopeDepth];
                assert(idx < scope->vars.size());
                stack.push_back(scope->vars[idx]);
                break;
            }
            case OP_PUSH_ID_GLOBAL_BY_NAME: {
                assert(0);
                break;
            }
            case OP_PUSH_ID_LOCAL_FUNCTION: {
                assert(0);
                break;
            }
            case OP_PUSH_ID_PARENT_FUNCTION: {
                assert(0);
                break;
            }
            case OP_PUSH_STRING: {
                auto idx = readUInt32(bytecode);
                stack.push_back(runtime->stringIdxToJsValue(function->resourcePool->index, idx));
                break;
            }
            case OP_PUSH_COMMON_STRING: {
                assert(0);
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
                v.value.index = makeResourceIndex(function->resourcePool->index, idx);

                stack.push_back(v);
                break;
            }
            case OP_PUSH_FUNCTION_EXPR: {
                auto scopeIdx = readUInt8(bytecode);
                auto funcIdx = readUInt16(bytecode);
                assert(scopeIdx < stackScopes.size());
                auto scope = stackScopes[scopeIdx];
                assert(funcIdx < scope->scopeDsc->functions.size());

                auto index = runtime->pushObjValue(new JsObjectFunction(stackScopes, scope->scopeDsc->functions[funcIdx]));
                stack.push_back(JsValue(JDT_FUNCTION, index));
                break;
            }
            case OP_PUSH_MEMBER_INDEX: {
                auto index = stack.back(); stack.pop_back();
                auto obj = stack.back();
                auto value = getMemberIndex(ctx, obj, index);
                stack.back() = value;
                break;
            }
            case OP_PUSH_MEMBER_INDEX_INT: {
                auto index = JsValue(JDT_INT32, readUInt32(bytecode));
                auto obj = stack.back();
                auto value = getMemberIndex(ctx, obj, index);
                stack.back() = value;
                break;
            }
            case OP_PUSH_MEMBER_DOT: {
                auto idx = readUInt32(bytecode);
                auto obj = stack.back();
                auto prop = runtime->getStringByIdx(idx, resourcePool);
                auto value = getMemberDot(ctx, obj, prop);
                stack.back() = value;
                break;
            }
            case OP_PUSH_MEMBER_DOT_OPTIONAL: {
                auto idx = readUInt32(bytecode);
                auto obj = stack.back();
                if (obj.type <= JDT_NULL) {
                    stack.back() = JsUndefinedValue;
                } else {
                    auto prop = runtime->getStringByIdx(idx, resourcePool);
                    auto value = getMemberDot(ctx, obj, prop);
                    stack.back() = value;
                }
                break;
            }
            case OP_PUSH_THIS_MEMBER_DOT: {
                auto idx = readUInt32(bytecode);
                auto obj = stack.back();
                auto prop = runtime->getStringByIdx(idx, resourcePool);
                auto value = getMemberDot(ctx, obj, prop);
                stack.push_back(value);
                break;
            }
            case OP_PUSH_THIS_MEMBER_INDEX: {
                auto index = stack.back(); stack.pop_back();
                auto obj = stack.back();
                auto value = getMemberIndex(ctx, obj, index);
                stack.push_back(value);
                break;
            }
            case OP_PUSH_THIS_MEMBER_INDEX_INT: {
                auto index = JsValue(JDT_INT32, readUInt32(bytecode));
                auto obj = stack.back();
                auto value = getMemberIndex(ctx, obj, index);
                stack.push_back(value);
                break;
            }
            case OP_PUSH_THIS_MEMBER_DOT_OPTIONAL: {
                auto idx = readUInt32(bytecode);
                auto obj = stack.back();
                if (obj.type > JDT_NULL) {
                    auto prop = runtime->getStringByIdx(idx, resourcePool);
                    auto value = getMemberDot(ctx, obj, prop);
                    stack.push_back(value);
                } else {
                    stack.push_back(JsUndefinedValue);
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
                        assert(scopeDepth < stackScopes.size());
                        scope = stackScopes[scopeDepth];
                        assert(storageIndex < scope->args.capacity);
                        scope->args[storageIndex] = stack.back();
                        break;
                    case VST_SCOPE_VAR:
                    case VST_FUNCTION_VAR:
                        assert(scopeDepth < stackScopes.size());
                        scope = stackScopes[scopeDepth];
                        assert(storageIndex < scope->vars.size());
                        scope->vars[storageIndex] = stack.back();
                        break;
                    case VST_GLOBAL_VAR:
                        if (storageIndex >= runtime->countImmutableGlobalVars) {
                            runtime->globalScope->vars[storageIndex] = stack.back();
                        }
                        break;
                    default:
                        assert(0);
                        break;
                }
                break;
            }
            case OP_ASSIGN_LOCAL_ARGUMENT: {
                auto idx = readUInt16(bytecode);
                assert(idx < functionScope->args.capacity);
                functionScope->args[idx] = stack.back();
                break;
            }
            case OP_ASSIGN_MEMBER_INDEX: {
                auto pos = stack.size() - 3;
                auto value = stack.back();
                auto index = stack[pos + 1];
                auto obj = stack[pos];
                if (obj.type < JDT_OBJECT) {
                    // Primitive 类型都不能设置属性
                    stack.resize(pos);
                    stack.push_back(value);
                    break;
                }

                if (index.type >= JDT_OBJECT) {
                    callMember(ctx, obj, "toString", Arguments());
                    if (ctx->error != PE_OK) {
                        bytecode = endBytecode;
                        break;
                    }
                    index = ctx->retValue;
                }

                auto pobj = runtime->getObject(obj);

                stack.resize(pos);
                stack.push_back(value);

                pobj->set(ctx, obj, index, value);
                break;
            }
            case OP_ASSIGN_MEMBER_DOT: {
                auto idx = readUInt32(bytecode);
                auto value = stack.back();
                auto obj = stack[stack.size() - 2];
                auto prop = runtime->getStringByIdx(idx, resourcePool);
                setMemberDot(ctx, obj, prop, value);
                stack.resize(stack.size() - 2);
                stack.push_back(value);
                break;
            }
            case OP_INCREMENT_ID_PRE: {
                assert(0);
                break;
            }
            case OP_INCREMENT_ID_POST: {
                assert(0);
                break;
            }
            case OP_DECREMENT_ID_PRE: {
                assert(0);
                break;
            }
            case OP_DECREMENT_ID_POST: {
                assert(0);
                break;
            }
            case OP_INCREMENT_MEMBER_DOT_PRE: {
                assert(0);
                break;
            }
            case OP_INCREMENT_MEMBER_DOT_POST: {
                assert(0);
                break;
            }
            case OP_DECREMENT_MEMBER_DOT_PRE: {
                assert(0);
                break;
            }
            case OP_DECREMENT_MEMBER_DOT_POST: {
                assert(0);
                break;
            }
            case OP_INCREMENT_MEMBER_INDEX_PRE: {
                assert(0);
                break;
            }
            case OP_INCREMENT_MEMBER_INDEX_POST: {
                assert(0);
                break;
            }
            case OP_DECREMENT_MEMBER_INDEX_PRE: {
                assert(0);
                break;
            }
            case OP_DECREMENT_MEMBER_INDEX_POST: {
                assert(0);
                break;
            }
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
                                assert(0);
                                break;
                        }
                        break;
                    case JDT_INT32:
                        switch (right.type) {
                            case JDT_INT32:
                                left.value.n32 += right.value.n32;
                                left.type = JDT_INT32;
                                break;
                            default:
                                assert(0);
                                break;
                        }
                        break;
                    case JDT_CHAR:
                    case JDT_STRING:
                        switch (right.type) {
                            case JDT_CHAR:
                            case JDT_STRING:
                                left = runtime->addString(left, right);
                                break;
                            default:
                                assert(0);
                                break;
                        }
                        break;
                    default:
                        assert(0);
                        break;
                }

                stack.back() = left;
                break;
            }
            case OP_SUB: {
                assert(0);
                break;
            }
            case OP_MUL: {
                assert(0);
                break;
            }
            case OP_DIV: {
                assert(0);
                break;
            }
            case OP_MOD: {
                assert(0);
                break;
            }
            case OP_EXP: {
                assert(0);
                break;
            }
            case OP_INEQUAL_STRICT: {
                assert(0);
                break;
            }
            case OP_INEQUAL: {
                assert(0);
                break;
            }
            case OP_EQUAL_STRICT: {
                assert(0);
                break;
            }
            case OP_EQUAL: {
                assert(0);
                break;
            }
            case OP_CONDITIONAL: {
                assert(0);
                break;
            }
            case OP_NULLISH: {
                assert(0);
                break;
            }
            case OP_LOGICAL_OR: {
                assert(0);
                break;
            }
            case OP_LOGICAL_AND: {
                assert(0);
                break;
            }
            case OP_BIT_OR: {
                assert(0);
                break;
            }
            case OP_BIT_XOR: {
                assert(0);
                break;
            }
            case OP_BIT_AND: {
                assert(0);
                break;
            }
            case OP_RATIONAL: {
                assert(0);
                break;
            }
            case OP_SHIFT: {
                assert(0);
                break;
            }
            case OP_UNARY: {
                assert(0);
                break;
            }
            case OP_POST_FIX: {
                assert(0);
                break;
            }
            case OP_PREFIX_NEGATE: {
                assert(0);
                break;
            }
            case OP_LOGICAL_NOT: {
                assert(0);
                break;
            }
            case OP_BIT_NOT: {
                assert(0);
                break;
            }
            case OP_LEFT_SHIFT: {
                assert(0);
                break;
            }
            case OP_RIGHT_SHIFT: {
                assert(0);
                break;
            }
            case OP_UNSIGNED_RIGHT_SHIFT: {
                assert(0);
                break;
            }
            case OP_LESS_THAN: {
                assert(0);
                break;
            }
            case OP_LESS_EQUAL_THAN: {
                assert(0);
                break;
            }
            case OP_GREATER_THAN: {
                assert(0);
                break;
            }
            case OP_GREATER_EQUAL_THAN: {
                assert(0);
                break;
            }
            case OP_IN: {
                assert(0);
                break;
            }
            case OP_INSTANCE_OF: {
                assert(0);
                break;
            }
            case OP_NEW: {
                uint16_t countArgs = readUInt16(bytecode);
                auto posStack = stack.size() - countArgs - 1;
                JsValue func = stack.at(posStack);
                Arguments args(stack.data() + stack.size() - countArgs, countArgs);
                switch (func.type) {
                    case JDT_FUNCTION: {
                        auto obj = (JsObjectFunction *)runtime->getObject(func);
                        assert(obj->type == JDT_FUNCTION);
                        if (obj->function->isMemberFunction) {
                            ctx->throwException(PE_TYPE_ERROR, "? is not a constructor");
                        }

                        auto prototype = obj->getByName(ctx, func, SS_PROTOTYPE);
                        auto thizIdx = runtime->pushObjValue(new JsObject(prototype));
                        auto thizVal = JsValue(JDT_OBJECT, thizIdx);
                        call(obj->function, ctx, obj->stackScopes, thizVal, args);
                        stack.resize(posStack);
                        stack.push_back(thizVal);
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case OP_NEW_TARGET: {
                assert(0);
                break;
            }
            case OP_OBJ_CREATE: {
                auto idx = runtime->pushObjValue(new JsObject());
                stack.push_back(JsValue(JDT_OBJECT, idx));
                break;
            }
            case OP_OBJ_SET_PROPERTY: {
                auto nameIdx = readUInt32(bytecode);
                auto value = stack.back();
                auto &obj = stack[stack.size() - 2];
                assert(obj.type >= JDT_OBJECT);
                if (obj.type < JDT_OBJECT) {
                    // Primitive 类型都不能设置属性
                    stack.pop_back();
                    break;
                }
                auto pobj = runtime->getObject(obj);

                auto name = runtime->getStringByIdx(nameIdx, resourcePool);
                pobj->setByName(ctx, obj, name, value);

                stack.pop_back();
                break;
            }
            case OP_OBJ_SET_COMPUTED_PROPERTY: {
                assert(0);
                break;
            }
            case OP_OBJ_SPREAD_PROPERTY: {
                assert(0);
                break;
            }
            case OP_OBJ_SET_GETTER: {
                assert(0);
                break;
            }
            case OP_OBJ_SET_SETTER: {
                assert(0);
                break;
            }
            case OP_ARRAY_CREATE: {
                auto idx = runtime->pushObjValue(new JsArray());
                stack.push_back(JsValue(JDT_ARRAY, idx));
                break;
            }
            case OP_ARRAY_SPREAD_VALUE: {
                auto item = stack.back(); stack.pop_back();
                auto arr = stack.back();
                runtime->extendObject(arr, item);
                break;
            }
            case OP_ARRAY_PUSH_VALUE: {
                auto item = stack.back(); stack.pop_back();
                auto arr = stack.back();
                assert(arr.type == JDT_ARRAY);
                auto a = (JsArray *)runtime->getObject(arr);
                a->push(ctx, item);
                break;
            }
            case OP_ARRAY_PUSH_UNDEFINED_VALUE: {
                auto arr = stack.back();
                assert(arr.type == JDT_ARRAY);
                auto a = (JsArray *)runtime->getObject(arr);
                a->push(ctx, JsUndefinedValue);
                break;
            }
            case OP_ARRAY_ASSING_CREATE: {
                assert(0);
                break;
            }
            case OP_ARRAY_ASSIGN_REST_VALUE: {
                assert(0);
                break;
            }
            case OP_ARRAY_ASSIGN_PUSH_VALUE: {
                assert(0);
                break;
            }
            case OP_ARRAY_ASSIGN_PUSH_UNDEFINED_VALUE: {
                assert(0);
                break;
            }
            case OP_ITERATOR_CREATE: {
                assert(0);
                break;
            }
            case OP_ITERATOR_NEXT_KEY: {
                assert(0);
                break;
            }
            case OP_ITERATOR_NEXT_VALUE: {
                assert(0);
                break;
            }
            case OP_ITERATOR_NEXT_KEY_VALUE: {
                assert(0);
                break;
            }
            case OP_SPREAD_ARGS: {
                assert(0);
                break;
            }
            case OP_REST_PARAMETER: {
                assert(0);
                break;
            }
            case OP_PUSH_EXCEPTION: {
                stack.push_back(ctx->errorMessage);
                break;
            }
            case OP_TRY_START: {
                auto addrCatch = readUInt32(bytecode);
                auto addrFinally = readUInt32(bytecode);
                ctx->stackTryCatch.push(TryCatchPoint(makeTryCatchPointFlags(function, functionScope),
                        (uint32_t)stackScopes.size(), (uint32_t)stack.size(), addrCatch, addrFinally));
                break;
            }
            case OP_BEGIN_FINALLY_NORMAL: {
                // 顺序执行到此，需要去掉 stackTryCatch 中添加的处理点
                auto &stackTryCatch = ctx->stackTryCatch;
                assert(!stackTryCatch.empty());
                assert(stackTryCatch.top().flags == makeTryCatchPointFlags(function, functionScope));
                assert(stackTryCatch.top().addrCatch == 0);
                assert(stackTryCatch.top().addrFinally != 0);
                stackTryCatch.pop();
                break;
            }
            case OP_FINISH_FINALLY: {
                if (ctx->isReturnedForTry) {
                    // 检查是否执行了 return 指令
                    auto &stackTryCatch = ctx->stackTryCatch;
                    if (!stackTryCatch.empty() && stackTryCatch.top().flags == makeTryCatchPointFlags(function, functionScope)) {
                        // 当前函数还有 try
                        auto action = stackTryCatch.top();
                        stackTryCatch.pop();
                        if (action.addrFinally) {
                            // 跳转到 finally
                            bytecode = function->bytecode + action.addrFinally;
                            break;
                        }
                    }

                    // 没有 try finally 了
                    bytecode = endBytecode;
                    ctx->isReturnedForTry = false;
                } else if (ctx->errorInTry != PE_OK) {
                    // 恢复 try 中的异常标志
                    ctx->error = ctx->errorInTry;
                    ctx->errorMessage = ctx->errorMessageInTry;
                    ctx->errorInTry = PE_OK;
                    ctx->errorMessageInTry = JsUndefinedValue;
                    stack.push_back(ctx->errorMessage);
                }
                break;
            }
        }

        if (ctx->error != PE_OK) {
            // 有异常发生

            // 异常会清除 return 相关内容
            retValue = JsUndefinedValue;

            auto &stackTryCatch = ctx->stackTryCatch;
            if (stackTryCatch.empty() || stackTryCatch.top().flags != makeTryCatchPointFlags(function, functionScope)) {
                // 当前函数没有接收异常处理
                break;
            } else {
                auto &action = stackTryCatch.top();

                // 发生异常，恢复 stackScopes 和 stack
                stackScopes.erase(stackScopes.begin() + action.scopeDepth, stackScopes.end());
                stack.erase(stack.begin() + action.stackSize, stack.end());

                if (action.addrCatch != 0) {
                    // 跳转到 catch 的位置
                    bytecode = function->bytecode + action.addrCatch;

                    // 清除异常标志
                    ctx->error = PE_OK;

                    if (action.addrFinally == 0) {
                        // 没有 finally
                        stackTryCatch.pop();
                    } else {
                        // 表示 Catch 已经处理
                        action.addrCatch = 0;
                    }
                } else {
                    // 跳转到 finally 的位置
                    ctx->errorInTry = ctx->error;
                    ctx->errorMessageInTry = ctx->errorMessage;
                    ctx->error = PE_OK;

                    assert(action.addrFinally != 0);
                    bytecode = function->bytecode + action.addrFinally;
                    stackTryCatch.pop();
                }
            }
        }
    }

    ctx->curFunctionScope = prevFunctionScope;
    ctx->retValue = retValue;

    stackScopes.pop_back();
    ctx->stackFrames.pop_back();
}

/*
VMNativeCodePtr JsVirtualMachine::compile(Scope *scope, Function *function, string &errMessageOut) {
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

JsValue JsVirtualMachine::eval(const VMNativeCodePtr &code, VMContext *vmctx) {
    assert(vmctx);
    assert(code.get());

    return code->entryFunction((uint64_t)vmctx);
}

JsValue JsVirtualMachine::eval(Function *function, VMContext *vmctx) {
    //function->instructions.
}
*/
