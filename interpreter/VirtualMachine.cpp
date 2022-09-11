//
//  VirtualMachine.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/24.
//

#include "VirtualMachine.hpp"
#include "Parser.hpp"
#include "IJsObject.hpp"
#include "VMRuntime.hpp"
#include "JsArguments.hpp"
#include "JsLibObject.hpp"
#include "JsObjectFunction.hpp"
#include "JsArray.hpp"
#include "JsRegExp.hpp"
#include "BinaryOperation.hpp"
#include "UnaryOperation.hpp"
#include "JsString.hpp"


void registerGlobalValue(VMContext *ctx, VMScope *globalScope, const char *strName, const JsValue &value) {
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

void registerGlobalObject(VMContext *ctx, VMScope *globalScope, const char *strName, IJsObject *obj) {
    registerGlobalValue(ctx, globalScope, strName, ctx->runtime->pushObjValue(JDT_OBJECT, obj));
}

uint32_t makeTryCatchPointFlags(Function *function, VMScope *scope) {
    uint64_t n = (uint64_t)function ^ (uint64_t)scope;
    return (n & 0xFFFFFFFF) ^ (n >> 32);
}

bool jsValueStrictLessThan(VMRuntime *runtime, const JsValue &left, const JsValue &right) {
    switch (left.type) {
        case JDT_NOT_INITIALIZED:
        case JDT_UNDEFINED:
        case JDT_NULL:
            return left.type < right.type;
        case JDT_BOOL:
            if (left.type < right.type) return true;
            else if (left.type == right.type) return left.value.n32 < right.value.n32;
            return false;
        case JDT_INT32:
            if (right.type == JDT_INT32) return left.value.n32 < right.value.n32;
            else if (right.type == JDT_NUMBER) {
                auto d = runtime->getDouble(right);
                return left.value.n32 < d;
            } else return left.type < right.type;
        case JDT_NUMBER:
            if (right.type == JDT_INT32) {
                auto d = runtime->getDouble(right);
                return left.value.n32 < d;
            } else if (right.type == JDT_NUMBER) {
                auto d1 = runtime->getDouble(left);
                auto d2 = runtime->getDouble(right);
                return d1 < d2;
            } else {
                return left.type < right.type;
            }
        case JDT_CHAR: {
            if (right.type == JDT_CHAR) {
                return left.value.n32 < right.value.n32;
            } else if (right.type == JDT_STRING) {
                SizedStringWrapper s1(left);
                auto s2 = runtime->getString(right);
                return s1.cmp(s2) < 0;
            } else {
                return left.type < right.type;
            }
        }
        case JDT_STRING: {
            if (right.type == JDT_CHAR) {
                auto s1 = runtime->getString(left);
                SizedStringWrapper s2(right);
                return s1.cmp(s2) < 0;
            } else if (right.type == JDT_STRING) {
                auto s1 = runtime->getString(left);
                auto s2 = runtime->getString(right);
                return s1.cmp(s2) < 0;
            } else {
                return left.type < right.type;
            }
        }
        default:
            // 只接受以上的几种基本类型的比较
            return false;
    }
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
        data[--minSize] = jsValueUndefined;
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

void VMContext::throwException(JsErrorType err, cstr_t format, ...) {
    if (error != PE_OK) {
        return;
    }

    va_list        args;

    va_start(args, format);
    auto message = stringPrintf(format, args);
    va_end(args);

    auto errValue = runtime->pushString(SizedString(message.c_str(), message.size()));

    throwException(err, newJsError(this, err, errValue));
}

void VMContext::throwException(JsErrorType err, JsValue errorMessage) {
    this->error = err;
    this->errorMessage = errorMessage;

    if (isReturnedForTry) {
        // 在 finally 中抛出异常，会清除 return 相关内容
        isReturnedForTry = false;
    }

    if (errorInTry != PE_OK) {
        // 在 finally 中抛出异常，会清除上一次的异常内容
        errorInTry = PE_OK;
        errorMessageInTry = jsValueUndefined;
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
        runtime->console->error(stringPrintf("Uncaught %.*s\n", message.len, message.data).c_str());
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
        globalScope->vars.resize(globalScope->scopeDsc->varDeclares.size(), jsValueNotInitialized);
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
    writeIndent(stream, os.sizedStringStartNew(), SizedString("  "));
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

JsValue JsVirtualMachine::getMemberDot(VMContext *ctx, const JsValue &thiz, const SizedString &name) {
    auto runtime = ctx->runtime;
    switch (thiz.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_REFERECNE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of undefined (reading '%.*s')", (int)name.len, name.data);
            break;
        case JDT_NULL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of null (reading '%.*s')", (int)name.len, name.data);
        case JDT_BOOL:
            return runtime->objPrototypeBoolean->getByName(ctx, thiz, name);
        case JDT_INT32:
            return runtime->objPrototypeNumber->getByName(ctx, thiz, name);
        case JDT_NUMBER:
            return runtime->objPrototypeNumber->getByName(ctx, thiz, name);
        case JDT_CHAR:
        case JDT_STRING:
            return runtime->objPrototypeString->getByName(ctx, thiz, name);
        case JDT_SYMBOL:
            return runtime->objPrototypeSymbol->getByName(ctx, thiz, name);
        default: {
            auto jsthiz = runtime->getObject(thiz);
            return jsthiz->getByName(ctx, thiz, name);
        }
    }

    return jsValueUndefined;
}

void JsVirtualMachine::setMemberDot(VMContext *ctx, const JsValue &thiz, const SizedString &name, const JsValue &value) {
    auto runtime = ctx->runtime;
    switch (thiz.type) {
        case JDT_NOT_INITIALIZED:
            ctx->throwException(PE_REFERECNE_ERROR, "Cannot access '?' before initialization");
            break;
        case JDT_UNDEFINED:
            ctx->throwException(PE_TYPE_ERROR, "Cannot set properties of undefined (setting '%.*s')", (int)name.len, name.data);
            break;
        case JDT_NULL:
            ctx->throwException(PE_TYPE_ERROR, "Cannot set properties of null (setting '%.*s')", (int)name.len, name.data);
            break;
        case JDT_BOOL:
            return runtime->objPrototypeBoolean->setByName(ctx, thiz, name, value);
        case JDT_INT32:
            return runtime->objPrototypeNumber->setByName(ctx, thiz, name, value);
        case JDT_NUMBER:
            return runtime->objPrototypeNumber->setByName(ctx, thiz, name, value);
        case JDT_CHAR:
        case JDT_STRING:
            return runtime->objPrototypeString->setByName(ctx, thiz, name, value);
        case JDT_SYMBOL:
            return runtime->objPrototypeSymbol->setByName(ctx, thiz, name, value);
        default: {
            auto jsthiz = ctx->runtime->getObject(thiz);
            jsthiz->setByName(ctx, thiz, name, value);
            break;
        }
    }
}

JsValue JsVirtualMachine::getMemberIndex(VMContext *ctx, const JsValue &thiz, const JsValue &name) {
    auto runtime = ctx->runtime;
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
            return runtime->objPrototypeBoolean->get(ctx, thiz, name);
        case JDT_INT32:
            return runtime->objPrototypeNumber->get(ctx, thiz, name);
        case JDT_NUMBER:
            return runtime->objPrototypeNumber->get(ctx, thiz, name);
        case JDT_CHAR:
        case JDT_STRING:
            return getStringMemberIndex(ctx, thiz, name);
        case JDT_SYMBOL:
            return runtime->objPrototypeSymbol->get(ctx, thiz, name);
        default: {
            auto jsthiz = runtime->getObject(thiz);
            return jsthiz->get(ctx, thiz, name);
        }
    }

    return jsValueUndefined;
}

void JsVirtualMachine::setMemberIndex(VMContext *ctx, const JsValue &thiz, const JsValue &name, const JsValue &value) {
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
            jsthiz->set(ctx, thiz, name, value);
            break;
        }
    }
}

inline void opIncreaseIdentifier(VMContext *ctx, VMRuntime *runtime, uint8_t *&bytecode, VecVMStackScopes &stackScopes, int inc, bool isPost) {
    auto varStorageType = *bytecode++;
    auto scopeDepth = *bytecode++;
    auto storageIndex = readUInt16(bytecode);

    JsValue value = jsValueUndefined;
    switch (varStorageType) {
        case VST_ARGUMENT: {
            assert(scopeDepth < stackScopes.size());
            auto scope = stackScopes[scopeDepth];
            assert(storageIndex < scope->args.capacity);
            value = increaseJsValue(ctx, scope->args[storageIndex], inc, isPost);
            break;
        }
        case VST_SCOPE_VAR:
        case VST_FUNCTION_VAR: {
            assert(scopeDepth < stackScopes.size());
            auto scope = stackScopes[scopeDepth];
            assert(storageIndex < scope->vars.size());
            value = increaseJsValue(ctx, scope->vars[storageIndex], inc, isPost);
            break;
        }
        case VST_GLOBAL_VAR:
            if (storageIndex >= runtime->countImmutableGlobalVars) {
                value = increaseJsValue(ctx, runtime->globalScope->vars[storageIndex], inc, isPost);
            } else {
                auto temp = runtime->globalScope->vars[storageIndex];
                value = increaseJsValue(ctx, temp, inc, isPost);
            }
            break;
        default:
            assert(0);
            break;
    }

    ctx->stack.push_back(value);
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
    auto retValue = jsValueUndefined;

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
                functionScope->vars[VAR_IDX_ARGUMENTS] = runtime->pushObjValue(JDT_OBJECT, new JsArguments(functionScope, &functionScope->args));
                break;
            }
            case OP_INIT_FUNCTION_TO_VARS: {
                // 将根 scope 被引用到的函数添加到变量中
                for (auto f : function->scope->functionDecls) {
                    functionScope->vars[f->declare->storageIndex] = runtime->pushObjValue(JDT_FUNCTION, new JsObjectFunction(stackScopes, f));
                }
                break;
            }
            case OP_INIT_FUNCTION_TO_ARGS: {
                // 将根 scope 被引用到的函数添加到参数中
                for (auto f : *function->scope->functionArgs) {
                    functionScope->args[f->declare->storageIndex] = runtime->pushObjValue(JDT_FUNCTION, new JsObjectFunction(stackScopes, f));
                }
                break;
            }
            case OP_RETURN:
            case OP_RETURN_VALUE: {
                if (code == OP_RETURN_VALUE) {
                    retValue = stack.back();
                    stack.pop_back();
                } else {
                    retValue = jsValueUndefined;
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
                ctx->throwException(PE_ERROR, stack.back());
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
            case OP_SWITCH_CASE_CMP_JUMP: {
                auto pos = readUInt32(bytecode);
                auto caseCond = stack.back(); stack.pop_back();
                auto switchCond = stack.back();
                if (relationalStrictEqual(runtime, caseCond, switchCond)) {
                    // 找到了相同的跳转条件
                    stack.pop_back(); // 不再比较 switchCond 了
                    bytecode = function->bytecode + pos;
                }
                break;
            }
            case OP_SWITCH_CASE_FAST_CMP_JUMP: {
                auto idx = readUInt16(bytecode);
                auto switchJump = runtime->getSwitchJumpInResourcePool(idx, resourcePool);
                auto switchCond = stack.back(); stack.pop_back();

                VMAddress addr = switchJump.findAddress(runtime, resourcePool->index, switchCond);
                bytecode = function->bytecode + addr;
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
                    case JDT_LIB_OBJECT: {
                        auto libobj = (JsLibObject *)runtime->getObject(func);
                        auto f = libobj->getFunction();
                        if (f) {
                            f(ctx, thiz, args);
                        } else {
                            ctx->throwException(PE_TYPE_ERROR, "value is not a function");
                            assert(0);
                        }
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
                    functionScope->vars[f->declare->storageIndex] = runtime->pushObjValue(JDT_FUNCTION, new JsObjectFunction(stackScopes, f));
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
            case OP_POP_STACK_TOP_N: {
                auto count = readUInt16(bytecode);
                auto start = stack.end() - count;
                stack.erase(start, stack.end());
                stack.pop_back();
                break;
            }
            case OP_PUSH_STACK_TOP: {
                assert(0);
                break;
            }
            case OP_PUSH_UNDFINED: {
                stack.push_back(jsValueUndefined);
                break;
            }
            case OP_PUSH_TRUE: {
                stack.push_back(jsValueTrue);
                break;
            }
            case OP_PUSH_FALSE: {
                stack.push_back(jsValueFalse);
                break;
            }
            case OP_PUSH_NULL: {
                stack.push_back(jsValueNull);
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
            case OP_PUSH_REGEXP: {
                auto idx = readUInt32(bytecode);
                auto str = runtime->getStringByIdx(idx, resourcePool);
                auto re = new JsRegExp(str);
                stack.push_back(runtime->pushObjValue(JDT_REGEX, re));
                break;
            }
            case OP_PUSH_INT32: {
                auto value = readInt32(bytecode);
                stack.push_back(JsValue(JDT_INT32, value));
                break;
            }
            case OP_PUSH_DOUBLE: {
                auto idx = readUInt32(bytecode);
                stack.push_back(runtime->numberIdxToJsValue(function->resourcePool->index, idx));
                break;
            }
            case OP_PUSH_FUNCTION_EXPR: {
                auto scopeIdx = readUInt8(bytecode);
                auto funcIdx = readUInt16(bytecode);
                assert(scopeIdx < stackScopes.size());
                auto scope = stackScopes[scopeIdx];
                assert(funcIdx < scope->scopeDsc->function->functions.size());
                auto v = runtime->pushObjValue(JDT_FUNCTION, new JsObjectFunction(stackScopes, scope->scopeDsc->function->functions[funcIdx]));
                stack.push_back(v);
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
                auto name = runtime->getStringByIdx(idx, resourcePool);
                auto value = getMemberDot(ctx, obj, name);
                stack.back() = value;
                break;
            }
            case OP_PUSH_MEMBER_DOT_OPTIONAL: {
                auto idx = readUInt32(bytecode);
                auto obj = stack.back();
                if (obj.type <= JDT_NULL) {
                    stack.back() = jsValueUndefined;
                } else {
                    auto name = runtime->getStringByIdx(idx, resourcePool);
                    auto value = getMemberDot(ctx, obj, name);
                    stack.back() = value;
                }
                break;
            }
            case OP_PUSH_THIS_MEMBER_DOT: {
                auto idx = readUInt32(bytecode);
                auto obj = stack.back();
                auto name = runtime->getStringByIdx(idx, resourcePool);
                auto value = getMemberDot(ctx, obj, name);
                stack.push_back(value);
                break;
            }
            case OP_PUSH_MEMBER_INDEX_NO_POP: {
                auto index = stack.back();
                auto obj = stack[stack.size() - 2];
                auto value = getMemberIndex(ctx, obj, index);
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
                    auto name = runtime->getStringByIdx(idx, resourcePool);
                    auto value = getMemberDot(ctx, obj, name);
                    stack.push_back(value);
                } else {
                    stack.push_back(jsValueUndefined);
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
                auto value = stack.back(); stack.pop_back();
                auto index = stack.back(); stack.pop_back();
                auto obj = stack.back();

                assignMemberIndexOperation(ctx, runtime, obj, index, value);

                stack.back() = value;
                break;
            }
            case OP_ASSIGN_MEMBER_DOT: {
                auto idx = readUInt32(bytecode);
                auto value = stack.back(); stack.pop_back();
                auto obj = stack.back(); stack.pop_back();
                auto name = runtime->getStringByIdx(idx, resourcePool);
                setMemberDot(ctx, obj, name, value);
                stack.push_back(value);
                break;
            }
            case OP_ASSIGN_VALUE_AHEAD_MEMBER_INDEX: {
                // 和 OP_ASSIGN_MEMBER_INDEX 不同的是，先 push value，再 push member index
                auto index = stack.back(); stack.pop_back();
                auto obj = stack.back(); stack.pop_back();
                auto value = stack.back();

                assignMemberIndexOperation(ctx, runtime, obj, index, value);
                break;
            }
            case OP_ASSIGN_VALUE_AHEAD_MEMBER_DOT: {
                // 和 OP_ASSIGN_MEMBER_DOT 不同的是，先 push value，再 push member dot
                auto idx = readUInt32(bytecode);
                auto obj = stack.back(); stack.pop_back();
                auto value = stack.back();
                auto name = runtime->getStringByIdx(idx, resourcePool);

                setMemberDot(ctx, obj, name, value);
                break;
            }
            case OP_INCREMENT_ID_PRE: {
                opIncreaseIdentifier(ctx, runtime, bytecode, stackScopes, 1, false);
                break;
            }
            case OP_INCREMENT_ID_POST: {
                opIncreaseIdentifier(ctx, runtime, bytecode, stackScopes, 1, true);
                break;
            }
            case OP_DECREMENT_ID_PRE: {
                opIncreaseIdentifier(ctx, runtime, bytecode, stackScopes, -1, false);
                break;
            }
            case OP_DECREMENT_ID_POST: {
                opIncreaseIdentifier(ctx, runtime, bytecode, stackScopes, -1, true);
                break;
            }
            case OP_INCREMENT_MEMBER_DOT_PRE: {
                auto idx = readUInt32(bytecode);
                auto name = runtime->getStringByIdx(idx, resourcePool);
                JsValue obj = stack.back(); stack.pop_back();
                auto value = increaseMemberDot(ctx, obj, name, 1, false);
                stack.push_back(value);
                break;
            }
            case OP_INCREMENT_MEMBER_DOT_POST: {
                auto idx = readUInt32(bytecode);
                auto name = runtime->getStringByIdx(idx, resourcePool);
                JsValue obj = stack.back(); stack.pop_back();
                auto value = increaseMemberDot(ctx, obj, name, 1, true);
                stack.push_back(value);
                break;
            }
            case OP_DECREMENT_MEMBER_DOT_PRE: {
                auto idx = readUInt32(bytecode);
                auto name = runtime->getStringByIdx(idx, resourcePool);
                JsValue obj = stack.back(); stack.pop_back();
                auto value = increaseMemberDot(ctx, obj, name, -1, false);
                stack.push_back(value);
                break;
            }
            case OP_DECREMENT_MEMBER_DOT_POST: {
                auto idx = readUInt32(bytecode);
                auto name = runtime->getStringByIdx(idx, resourcePool);
                JsValue obj = stack.back(); stack.pop_back();
                auto value = increaseMemberDot(ctx, obj, name, -1, true);
                stack.push_back(value);
                break;
            }
            case OP_INCREMENT_MEMBER_INDEX_PRE: {
                JsValue index = stack.back(); stack.pop_back();
                JsValue obj = stack.back();
                auto value = increaseMemberIndex(ctx, obj, index, 1, false);
                stack.back() = value;
                break;
            }
            case OP_INCREMENT_MEMBER_INDEX_POST: {
                JsValue index = stack.back(); stack.pop_back();
                JsValue obj = stack.back();
                auto value = increaseMemberIndex(ctx, obj, index, 1, true);
                stack.back() = value;
                break;
            }
            case OP_DECREMENT_MEMBER_INDEX_PRE: {
                JsValue index = stack.back(); stack.pop_back();
                JsValue obj = stack.back();
                auto value = increaseMemberIndex(ctx, obj, index, -1, false);
                stack.back() = value;
                break;
            }
            case OP_DECREMENT_MEMBER_INDEX_POST: {
                JsValue index = stack.back(); stack.pop_back();
                JsValue obj = stack.back();
                auto value = increaseMemberIndex(ctx, obj, index, -1, true);
                stack.back() = value;
                break;
            }
            case OP_ADD: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                stack.back() = plusOperate(ctx, runtime, left, right);
                break;
            }
            case OP_SUB: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                stack.back() = arithmeticBinaryOperation(ctx, runtime, left, right, BinaryOpSub());
                break;
            }
            case OP_MUL: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                stack.back() = arithmeticBinaryOperation(ctx, runtime, left, right, BinaryOpMul());
                break;
            }
            case OP_DIV: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                stack.back() = arithmeticBinaryOperation(ctx, runtime, left, right, BinaryOpDiv());
                break;
            }
            case OP_MOD: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                stack.back() = arithmeticBinaryOperation(ctx, runtime, left, right, BinaryOpMod());
                break;
            }
            case OP_EXP: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                stack.back() = arithmeticBinaryOperation(ctx, runtime, left, right, BinaryOpExp());
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
                JsValue &v = stack.back();
                if (v.type == JDT_INT32) {
                    v.value.n32 = -v.value.n32;
                } else {
                    auto d = runtime->toNumber(ctx, v);
                    int32_t n = (int32_t)d;
                    if (n == d) {
                        v = JsValue(JDT_INT32, -n);
                    } else if (isnan(d)) {
                        v = jsValueNaN;
                    } else {
                        v = runtime->pushDoubleValue(-d);
                    }
                }
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
            case OP_INEQUAL_STRICT: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back(); stack.pop_back();
                stack.push_back(JsValue(JDT_BOOL, !relationalStrictEqual(runtime, left, right)));
                break;
            }
            case OP_INEQUAL: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                bool ret = relationalEqual(ctx, runtime, left, right);
                stack.back() = JsValue(JDT_BOOL, !ret);
                break;
            }
            case OP_EQUAL_STRICT: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back(); stack.pop_back();
                stack.push_back(JsValue(JDT_BOOL, relationalStrictEqual(runtime, left, right)));
                break;
            }
            case OP_EQUAL: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                bool ret = relationalEqual(ctx, runtime, left, right);
                stack.back() = JsValue(JDT_BOOL, ret);
                break;
            }
            case OP_LESS_THAN: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                bool ret = relationalOperate(ctx, runtime, left, right, RelationalOpLessThan());
                stack.back() = JsValue(JDT_BOOL, ret);
                break;
            }
            case OP_LESS_EQUAL_THAN: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                bool ret = relationalOperate(ctx, runtime, left, right, RelationalOpLessEqThan());
                stack.back() = JsValue(JDT_BOOL, ret);
                break;
            }
            case OP_GREATER_THAN: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                bool ret = relationalOperate(ctx, runtime, left, right, RelationalOpGreaterThan());
                stack.back() = JsValue(JDT_BOOL, ret);
                break;
            }
            case OP_GREATER_EQUAL_THAN: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back();
                bool ret = relationalOperate(ctx, runtime, left, right, RelationalOpGreaterEqThan());
                stack.back() = JsValue(JDT_BOOL, ret);
                break;
            }
            case OP_IN: {
                JsValue right = stack.back(); stack.pop_back();
                JsValue left = stack.back(); stack.pop_back();
                if (right.type < JDT_OBJECT) {
                    string buf1, buf2;
                    auto s1 = runtime->toSizedString(ctx, left, buf1);
                    auto s2 = runtime->toSizedString(ctx, right, buf2);
                    ctx->throwException(PE_TYPE_ERROR, "Cannot use 'in' operator to search for '%.*s' in %.*s",
                        (int)s1.len, s1.data, (int)s2.len, s2.data);
                    break;
                }

                auto pobj = runtime->getObject(right);
                JsNativeFunction funcGetterTmp = nullptr;
                auto prop = pobj->getRaw(ctx, left, funcGetterTmp, true);
                stack.push_back(JsValue(JDT_BOOL, prop != nullptr));
                break;
            }
            case OP_INSTANCE_OF: {
                assert(0);
                break;
            }
            case OP_DELETE_MEMBER_DOT: {
                auto obj = stack.back(); stack.pop_back();
                auto stringIdx = readUInt32(bytecode);
                if (obj.type >= JDT_OBJECT) {
                    auto *pobj = runtime->getObject(obj);
                    auto name = runtime->getStringByIdx(stringIdx, resourcePool);
                    if (!pobj->removeByName(ctx, name)) {
                        stack.push_back(jsValueFalse);
                        break;
                    }
                }
                stack.push_back(jsValueTrue);
                break;
            }
            case OP_DELETE_MEMBER_INDEX: {
                auto index = stack.back(); stack.pop_back();
                auto obj = stack.back(); stack.pop_back();
                if (obj.type >= JDT_OBJECT) {
                    auto *pobj = runtime->getObject(obj);
                    pobj->remove(ctx, index);
                }
                stack.push_back(jsValueTrue);
                break;
            }
            case OP_DELETE: {
                stack.pop_back();
                stack.push_back(jsValueTrue);
                break;
            }
            case OP_TYPEOF: {
                auto value = stack.back(); stack.pop_back();
                switch (value.type) {
                    case JDT_UNDEFINED: stack.push_back(jsStringValueUndefined); break;
                    case JDT_NULL: stack.push_back(jsStringValueObject); break;
                    case JDT_BOOL: stack.push_back(jsStringValueBoolean); break;
                    case JDT_INT32: stack.push_back(jsStringValueNumber); break;
                    case JDT_NUMBER: stack.push_back(jsStringValueNumber); break;
                    case JDT_SYMBOL: stack.push_back(jsStringValueSymbol); break;
                    case JDT_CHAR: stack.push_back(jsStringValueString); break;
                    case JDT_STRING: stack.push_back(jsStringValueString); break;
                    default: stack.push_back(jsStringValueObject); break;
                }
                break;
            }
            case OP_VOID: {
                stack.push_back(jsValueUndefined);
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
                        if (prototype.type < JDT_OBJECT) {
                            prototype = jsValuePrototypeObject;
                        }
                        auto thizVal = runtime->pushObjValue(JDT_OBJECT, new JsObject(prototype));
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
                stack.push_back(runtime->pushObjValue(JDT_OBJECT, new JsObject()));
                break;
            }
            case OP_OBJ_SET_PROPERTY: {
                auto nameIdx = readUInt32(bytecode);
                auto value = stack.back(); stack.pop_back();
                auto obj = stack.back();
                assert(obj.type == JDT_OBJECT);
                auto pobj = runtime->getObject(obj);
                auto name = runtime->getStringByIdx(nameIdx, resourcePool);
                pobj->setByName(ctx, obj, name, value);
                break;
            }
            case OP_OBJ_SET_COMPUTED_PROPERTY: {
                auto value = stack.back(); stack.pop_back();
                auto name = stack.back(); stack.pop_back();
                auto obj = stack.back();
                assert(obj.type == JDT_OBJECT);
                auto pobj = (JsObject *)runtime->getObject(obj);
                pobj->set(ctx, obj, name, value);
                break;
            }
            case OP_OBJ_SPREAD_PROPERTY: {
                auto src = stack.back(); stack.pop_back();
                auto obj = stack.back();
                assert(obj.type == JDT_OBJECT);
                runtime->extendObject(obj, src);
                break;
            }
            case OP_OBJ_SET_GETTER: {
                auto idx = readUInt32(bytecode);
                auto value = stack.back(); stack.pop_back();
                auto obj = stack.back();
                assert(obj.type == JDT_OBJECT);
                auto pobj = (JsObject *)runtime->getObject(obj);
                auto name = runtime->getStringByIdx(idx, resourcePool);
                JsProperty descriptor;
                descriptor.isGSetter = true;
                descriptor.value = value;
                descriptor.setter = jsValueNotInitialized;
                pobj->definePropertyByName(ctx, name, descriptor);
                break;
            }
            case OP_OBJ_SET_SETTER: {
                auto idx = readUInt32(bytecode);
                auto value = stack.back(); stack.pop_back();
                auto obj = stack.back();
                assert(obj.type == JDT_OBJECT);
                auto pobj = (JsObject *)runtime->getObject(obj);
                auto name = runtime->getStringByIdx(idx, resourcePool);
                JsProperty descriptor(jsValueNotInitialized);
                descriptor.isGSetter = true;
                descriptor.setter = value;
                pobj->definePropertyByName(ctx, name, descriptor);
                break;
            }
            case OP_ARRAY_CREATE: {
                stack.push_back(runtime->pushObjValue(JDT_ARRAY, new JsArray()));
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
                a->push(ctx, jsValueUndefined);
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
            case OP_ITERATOR_IN_CREATE:
            case OP_ITERATOR_OF_CREATE: {
                auto obj = stack.back(); stack.pop_back();
                IJsIterator *it = nullptr;
                if (obj.type <= JDT_NUMBER) {
                    string buf;
                    auto s = runtime->toSizedString(ctx, obj, buf);
                    ctx->throwException(PE_TYPE_ERROR, "%.*s is not iterable", s.len, s.data);
                    break;
                } else if (obj.type == JDT_SYMBOL) {
                    if (code == OP_ITERATOR_OF_CREATE) {
                        string buf;
                        auto s = runtime->toSizedString(ctx, obj, buf);
                        ctx->throwException(PE_TYPE_ERROR, "%.*s is not iterable", s.len, s.data);
                        break;
                    }
                    it = new EmptyJsIterator();
                } else if (obj.type == JDT_CHAR || obj.type == JDT_STRING) {
                    it = newJsStringIterator(runtime, obj);
                } else {
                    auto pobj = runtime->getObject(obj);
                    if (code == OP_ITERATOR_OF_CREATE && !pobj->isOfIterable()) {
                        string buf;
                        auto s = runtime->toSizedString(ctx, obj, buf);
                        ctx->throwException(PE_TYPE_ERROR, "%.*s is not iterable", s.len, s.data);
                        break;
                    } else {
                        assert(pobj);
                        it = pobj->getIteratorObject(ctx);
                    }
                }
                assert(it);
                stack.push_back(runtime->pushJsIterator(it));
                break;
            }
            case OP_ITERATOR_NEXT_KEY: {
                auto addrEnd = readUInt32(bytecode);
                auto it = stack.back();
                auto pit = runtime->getJsIterator(it);
                assert(pit);
                JsValue key;
                if (pit->nextKey(key)) {
                    stack.push_back(key);
                } else {
                    // 遍历完成
                    bytecode = function->bytecode + addrEnd;

                    // 弹出 it
                    stack.pop_back();
                }
                break;
            }
            case OP_ITERATOR_NEXT_VALUE: {
                auto addrEnd = readUInt32(bytecode);
                auto it = stack.back();
                auto pit = runtime->getJsIterator(it);
                assert(pit);
                JsValue key;
                if (pit->nextValue(key)) {
                    stack.push_back(key);
                } else {
                    // 遍历完成
                    bytecode = function->bytecode + addrEnd;

                    // 弹出 it
                    stack.pop_back();
                }
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
                    ctx->errorMessageInTry = jsValueUndefined;
                    stack.push_back(ctx->errorMessage);
                }
                break;
            }
        }

        if (ctx->error != PE_OK) {
            // 有异常发生

            // 异常会清除 return 相关内容
            retValue = jsValueUndefined;

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
