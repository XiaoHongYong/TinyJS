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

class JsObjectFunction : public JsObject {
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


JSVirtualMachine::JSVirtualMachine() {
    _runtimeCommon = new VMRuntimeCommon();
    _runtime = new VMRuntime();

    _runtime->init(this, _runtimeCommon);
}

void JSVirtualMachine::eval(cstr_t code, size_t len, VMContext *vmctx, VecVMStackScopes &stackScopes, const Arguments &args) {
    auto runtime = vmctx->runtime;
    ResourcePool *resPool = new ResourcePool();
    resPool->index = (uint32_t)runtime->resourcePools.size();
    runtime->resourcePools.push_back(resPool);

    auto p = (char *)resPool->pool.allocate(len + 4);
    memcpy(p, code, len);
    memset(p + len, 0, 4);
    code = p;

    JSParser parser(_runtimeCommon, resPool, code, len);

    auto func = parser.parse(stackScopes.back()->scopeDsc, false);
    if (parser.error() != PE_OK) {
        printf("Parse error: %s\n", parser.errorMessage().c_str());
        return;
    }

    {
        BinaryOutputStream stream;
        func->dump(stream);
        auto s = stream.startNew();
        printf("%s\n", code);
        printf("%.*s\n", (int)s.len, s.data);
    }
    
    VecVMStackFrames stackFrames;
    call(func, vmctx, stackScopes, runtime->globalThiz, args);
}

void JSVirtualMachine::dump(cstr_t code, size_t len, BinaryOutputStream &stream) {
    ResourcePool *resPool = new ResourcePool();
    resPool->index = 0;

    JSParser paser(_runtimeCommon, resPool, code, strlen(code));

    Function *rootFunc = new Function(resPool, nullptr, 0);
    auto func = paser.parse(rootFunc->scope, false);

    func->dump(stream);
}

void JSVirtualMachine::dump(BinaryOutputStream &stream) {
    BinaryOutputStream os;
    _runtime->dump(os);
    stream.write("== VMRuntime ==\n");
    writeIndent(stream, os.startNew(), makeSizedString("  "));
}

void JSVirtualMachine::callMember(VMContext *ctx, const JsValue &obj, const char *memberName, const Arguments &args) {

    ctx->stack.push_back(JsValue());
}

void JSVirtualMachine::call(Function *function, VMContext *ctx, VecVMStackScopes &stackScopes, const JsValue &thiz, const Arguments &args) {
    if (function->bytecode == nullptr) {
        function->generateByteCode();
    }

    auto runtime = ctx->runtime;
    auto resourcePool = function->resourcePool;
    auto &stack = ctx->stack;
    auto functionScope = new VMScope(function->scope);
    auto functionScopeDsc = functionScope->scopeDsc;
    auto bytecode = function->bytecode, endBytecode = bytecode + function->lenByteCode;

    ctx->stackFrames.push_back(new VMFunctionFrame(functionScope, function));

    // 将函数根 scope 被引用到的函数添加到变量中
    for (auto f : functionScopeDsc->functionDecls) {
        auto index = runtime->pushObjValue(new JsObjectFunction(stackScopes, f));
        functionScope->vars[f->declare->storageIndex] = JsValue(JDT_FUNCTION, index);
    }

    auto scopeLocal = functionScope;
    stackScopes.push_back(scopeLocal);

    if (function->isArgumentsReferredByChild || functionScopeDsc->isArgumentsUsed) {
        // 参数数组需要被复制
        functionScope->args.copy(args);
    } else {
        functionScope->args = args;
    }

    if (functionScopeDsc->isThisUsed) {
        functionScope->vars[VAR_IDX_THIS] = thiz;
    }
    if (functionScopeDsc->isArgumentsUsed) {
        // TODO new arguments
        // functionScope->vars[VAR_IDX_ARGUMENTS] = thiz;
    }

    while (bytecode < endBytecode) {
        auto code = (OpCode)*bytecode++;
        switch (code) {
            case OP_INVALID: {
                assert(0);
                break;
            }
            case OP_RETURN_VALUE: {
                return;
            }
            case OP_RETURN: {
                assert(0);
                break;
            }
            case OP_DEBUGGER: {
                assert(0);
                break;
            }
            case OP_THROW: {
                assert(0);
                break;
            }
            case OP_JUMP: {
                assert(0);
                break;
            }
            case OP_JUMP_IF_TRUE: {
                assert(0);
                break;
            }
            case OP_JUMP_IF_FALSE: {
                assert(0);
                break;
            }
            case OP_JUMP_IF_UNDEFINED: {
                assert(0);
                break;
            }
            case OP_JUMP_IF_NOT_UNDEFINED: {
                assert(0);
                break;
            }
            case OP_PREPARE_RAW_STRING_TEMPLATE_CALL: {
                assert(0);
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
                    case JDT_FUNCTION: {
                        auto f = (JsObjectFunction *)runtime->getObject(func);
                        call(f->function, ctx, f->stackScopes, thiz, args);
                        break;
                    }
                    case JDT_NATIVE_MEMBER_FUNCTION: {
                        auto f = runtime->getNativeMemberFunction(func.value.index);
                        f(ctx, thiz, args);
                        break;
                    }
                    default:
                        assert(0);
                        break;
                }
                auto ret = stack.back();
                stack.resize(posThiz);
                stack.push_back(ret);
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
                    call(targFunction, ctx, stackScopes, runtime->globalThiz, args);
                } else {
                    // 父函数的子函数
                    auto scope = stackScopes.at(depth);
                    auto targFunction = scope->scopeDsc->function->functions[index];
                    VecVMStackScopes targetStackScopes;
                    targetStackScopes.insert(targetStackScopes.begin(), stackScopes.begin(), stackScopes.begin() + depth + 1);
                    call(targFunction, ctx, targetStackScopes, runtime->globalThiz, args);
                }
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
            case OP_PUSH_IDENTIFIER: {
                assert(0);
                break;
            }
            case OP_PUSH_ID_GLOBAL: {
                auto idx = readUInt16(bytecode);
                stack.push_back(runtime->globalScope->vars[idx]);
                break;
            }
            case OP_PUSH_ID_LOCAL_ARGUMENT: {
                assert(0);
                break;
            }
            case OP_PUSH_ID_LOCAL_SCOPE: {
                auto idx = readUInt16(bytecode);
                stack.push_back(scopeLocal->vars[idx]);
                break;
            }
            case OP_PUSH_ID_PARENT_ARGUMENT: {
                assert(0);
                break;
            }
            case OP_PUSH_ID_PARENT_SCOPE: {
                assert(0);
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
                stack.push_back(makeJsValueOfStringInResourcePool(function->resourcePool->index, idx));
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
                assert(0);
                break;
            }
            case OP_PUSH_MEMBER_INDEX: {
                assert(0);
                break;
            }
            case OP_PUSH_MEMBER_INDEX_INT: {
                assert(0);
                break;
            }
            case OP_PUSH_MEMBER_DOT: {
                auto idx = readUInt32(bytecode);
                auto obj = stack.back();
                auto prop = runtime->getStringByIdx(idx, resourcePool);
                switch (obj.type) {
                    case JDT_NOT_INITIALIZED:
                        ctx->throwException(PE_REFERECNE_ERROR, "Cannot access '?' before initialization");
                        break;
                    case JDT_UNDEFINED:
                        ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of undefined (reading '%.*s')", (int)prop.len, prop.data);
                        break;
                    case JDT_NULL:
                        ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of null (reading '%.*s')", (int)prop.len, prop.data);
                        break;
                    case JDT_BOOL:
                        // TODO...
                        break;
                    case JDT_INT32:
                        // TODO...
                        break;
                    case JDT_NUMBER:
                        // TODO...
                        break;
                    case JDT_STRING: {
                        // TODO...
                        break;
                    }
                    default: {
                        auto jsobj = runtime->getObject(obj);
                        stack.pop_back();
                        stack.push_back(jsobj->get(ctx, prop));
                        break;
                    }

                }
                break;
            }
            case OP_PUSH_MEMBER_DOT_OPTIONAL: {
                assert(0);
                break;
            }
            case OP_PUSH_THIS_MEMBER_DOT: {
                auto idx = readUInt32(bytecode);
                auto prop = runtime->getStringByIdx(idx, resourcePool);
                const JsValue &obj = stack.back();
                switch (obj.type) {
                    case JDT_NOT_INITIALIZED:
                        ctx->throwException(PE_REFERECNE_ERROR, "Cannot access '?' before initialization");
                        break;
                    case JDT_UNDEFINED:
                        ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of undefined (reading %.*s)", prop.len, prop.data);
                        break;
                    case JDT_NULL:
                        ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of null (reading %.*s)", prop.len, prop.data);
                        break;
                    case JDT_BOOL:
                        // TODO...
                        break;
                    case JDT_INT32:
                        // TODO...
                        break;
                    case JDT_NUMBER:
                        // TODO...
                        break;
                    case JDT_STRING: {
                        // TODO...
                        break;
                    }
                    default: {
                        auto jsobj = runtime->getObject(obj);
                        stack.push_back(jsobj->get(ctx, prop));
                        break;
                    }
                }
                break;
            }
            case OP_PUSH_THIS_MEMBER_INDEX: {
                assert(0);
                break;
            }
            case OP_PUSH_THIS_MEMBER_INDEX_INT: {
                assert(0);
                break;
            }
            case OP_PUSH_THIS_MEMBER_DOT_OPTIONAL: {
                assert(0);
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
                        runtime->globalScope->vars[storageIndex] = stack.back();
                        break;
                    default:
                        assert(0);
                        break;
                }
                break;
            }
            case OP_ASSIGN_LOCAL_ARGUMENT: {
                assert(0);
                break;
            }
            case OP_ASSIGN_MEMBER_INDEX: {
                assert(0);
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
                        return;
                    }
                    index = stack.back();
                }

                auto pobj = runtime->getObject(obj);

                stack.resize(pos);
                stack.push_back(value);

                SizedString indexStr;
                indexStr.unused = 0;
                char buf[256];
                switch (index.type) {
                    case JDT_NOT_INITIALIZED:
                        ctx->throwException(PE_TYPE_ERROR, "Cannot access '?' before initialization");
                        return;
                    case JDT_UNDEFINED: indexStr = SS_UNDEFINED; pobj->set(ctx, indexStr, value); break;
                    case JDT_NULL: indexStr = SS_NULL; pobj->set(ctx, indexStr, value); break;
                    case JDT_BOOL: indexStr = index.value.n32 ? SS_TRUE : SS_FALSE; pobj->set(ctx, indexStr, value); break;
                    case JDT_INT32: pobj->set(ctx, index.value.n32, value); break;
                    case JDT_NUMBER:
                        if (index.value.n32 < MAX_INT_TO_CONST_STR) {
                            indexStr = intToSizedString(index.value.n32);
                        } else {
                            indexStr.len = (uint32_t)itoa(index.value.n32, buf);
                            indexStr.data = (uint8_t *)buf;
                            indexStr.unused = 0;
                        }
                        break;
                    case JDT_STRING: {
                        indexStr = runtime->getString(index);
                        if (!index.isInResourcePool && index.value.index < runtime->countCommonStrings) {
                            indexStr.unused = COMMON_STRINGS;
                        }
                        break;
                    }
                    default: {
                        ctx->throwException(PE_TYPE_ERROR, "Cannot convert object to primitive value");
                        return;
                    }
                }
                break;
            }
            case OP_ASSIGN_MEMBER_DOT: {
                auto idx = readUInt32(bytecode);
                auto value = stack.back();
                auto obj = stack[stack.size() - 2];
                auto prop = runtime->getStringByIdx(idx, resourcePool);
                switch (obj.type) {
                    case JDT_NOT_INITIALIZED:
                        ctx->throwException(PE_REFERECNE_ERROR, "Cannot access '?' before initialization");
                        break;
                    case JDT_UNDEFINED:
                        ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of undefined (reading '%.*s')", (int)prop.len, prop.data);
                        break;
                    case JDT_NULL:
                        ctx->throwException(PE_TYPE_ERROR, "Cannot read properties of null (reading '%.*s')", (int)prop.len, prop.data);
                        break;
                    case JDT_BOOL:
                    case JDT_INT32:
                    case JDT_NUMBER:
                    case JDT_STRING: {
                        // Primitive types' member cannot be set.
                        break;
                    }
                    default: {
                        auto jsobj = runtime->getObject(obj);
                        jsobj->set(ctx, prop, value);
                        break;
                    }

                }
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
                    case JDT_STRING:
                        switch (right.type) {
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

                        auto prototype = obj->get(ctx, SS_PROTOTYPE);
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
                pobj->set(ctx, name, value);

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
