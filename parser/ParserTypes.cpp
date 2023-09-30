//
//  ParserTypes.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#include "ParserTypes.hpp"
#include "generated/ConstStrings.hpp"
#include "Expression.hpp"
#include "Statement.hpp"
#include "interpreter/VirtualMachine.hpp"
#include "objects/IJsObject.hpp"
#include "interpreter/BinaryOperation.hpp"


const char *varStorageTypeToString(VarStorageType type) {
    static const char *VST_NAMES[] = {
        "VST_NOT_SET",
        "VST_GLOBAL_VAR",
        "VST_FUNCTION_VAR",
        "VST_SCOPE_VAR",
        "VST_ARGUMENT",
        "VST_REGISTER",
    };

    assert(type <= CountOf(VST_NAMES));
    return VST_NAMES[type];
}

/**
 * 将 str 中内容的每一行自动插入 indent.
 */
void writeIndent(BinaryOutputStream &stream, StringView str, const StringView &indent) {
    int pos = 0;
    while (str.len) {
        size_t n;
        pos = str.strchr('\n', pos);
        if (pos == -1) {
            n = pos + 1;
        } else {
            n = str.len;
        }
        stream.write(indent);
        stream.write(str.data, n);
        str.data += n;
        str.len -= n;
    }
}

Scope::Scope(ResourcePool *resourcePool, Function *function, Scope *parent) : function(function), parent(parent) {
    child = sibling = nullptr;
    functionArgs = nullptr;
    depth = 0;
    hasWith = false;
    hasEval = false;
    isFunctionScope = false;
    isThisUsed = false;
    isArgumentsUsed = false;

    countLocalVars = 0;
    countArguments = 0;

    index = (uint16_t)function->scopes.size();
    assert(function->scopes.size() < 0xFFFF);
    function->scopes.push_back(this);

    if (parent) {
        sibling = parent->child;
        parent->child = this;

        depth = parent->depth + 1;
    }

    resourcePool->needDestructScope(this);
}

Scope::~Scope() {
    if (functionArgs) {
        functionArgs->~VecFunctions();
    }
}

void Scope::dump(BinaryOutputStream &stream) {
    stream.writeFormat("----- Scope: %d, %llx -----\n", index, this);
    stream.writeFormat("CountLocalVars: %d\n", countLocalVars);
    stream.writeFormat("Depth: %d\n", depth);
    if (hasWith) stream.writeFormat("HasWith: %d\n", hasWith);
    if (hasEval) stream.writeFormat("HasEval:%d\n", hasEval);
    if (isFunctionScope) stream.writeFormat("IsFunctionScope: %d\n", isFunctionScope);

    BinaryOutputStream os;

    stream.write("Variables: \n");
    for (auto &item : varDeclares) {
        item.second->dump(os);
        writeIndent(stream, os.stringViewStartNew(), StringView("  "));
    }
    stream.write("\n");

    stream.write("Functions: ");
    for (auto f : functions) {
        stream.writeFormat("%.*s, ", f->name.len, f->name.data);
    }
    stream.write("\n");

    stream.write("FunctionVariables: ");
    for (auto f : functionDecls) {
        stream.writeFormat("%.*s, ", f->name.len, f->name.data);
    }
    stream.write("\n");

    if (child && function == child->function) {
        // Dump 同一函数的 child
        stream.write("Child scope:\n");
        child->dump(os);
        writeIndent(stream, os.stringViewStartNew(), StringView("  "));
    }

    if (sibling) {
        sibling->dump(stream);
    }
}

IdentifierDeclare *Scope::addVarDeclaration(const StringView &nameStr, bool isConst, bool isScopeVar) {
    auto it = varDeclares.find(nameStr);
    if (it == varDeclares.end()) {
        auto &pool = function->resourcePool->pool;
        auto node = PoolNew(pool, IdentifierDeclare)(nameStr, this);
        node->isConst = isConst;
        node->isScopeVar = isScopeVar;

        if (isScopeVar) {
            node->varStorageType = VST_SCOPE_VAR;
            node->storageIndex = countLocalVars++;
        }

        varDeclares[node->name] = node;
        return node;
    } else {
        return (*it).second;
    }
}

void Scope::addArgumentDeclaration(const StringView &name, int index) {
    assert(isFunctionScope);
    assert(index + 1 > countArguments);

    countArguments = index + 1;

    auto it = varDeclares.find(name);
    if (it == varDeclares.end()) {
        auto &pool = function->resourcePool->pool;
        auto node = PoolNew(pool, IdentifierDeclare)(name, this);
        node->isConst = false;
        node->varStorageType = VST_ARGUMENT;
        node->storageIndex = index;

        varDeclares[node->name] = node;
    } else {
        // 同名的参数会覆盖之前的声明
        auto node = (*it).second;
        assert(node->varStorageType == VST_ARGUMENT);
        node->storageIndex = index;
    }
}

/**
 * 添加隐含的未声明的全局变量
 */
void Scope::addImplicitVarDeclaration(JsExprIdentifier *id) {
    assert(parent == nullptr);

    auto &pool = function->resourcePool->pool;
    auto node = PoolNew(pool, IdentifierDeclare)(id->name, this);
    node->isConst = false;
    node->isImplicitDeclaration = true;
    node->varStorageType = VST_GLOBAL_VAR;
    node->storageIndex = countLocalVars++;
    varDeclares[id->name] = node;

    id->declare = node;
}

void Scope::addFunctionDeclaration(const Token &name, Function *child) {
    IdentifierDeclare *declare = nullptr;
    auto functionScope = function->scope;
    auto it = functionScope->varDeclares.find(tokenToStringView(name));
    if (it != functionScope->varDeclares.end()) {
        declare = (*it).second;
        if (declare->varStorageType == VST_ARGUMENT) {
            // 声明的函数和参数同名
            if (!functionScope->functionArgs) {
                auto &pool = functionScope->function->resourcePool->pool;
                functionScope->functionArgs = PoolNew(pool, VecFunctions);
            }
            functionScope->functionArgs->push_back(child);
        }
    } else {
        auto &pool = functionScope->function->resourcePool->pool;
        declare = PoolNew(pool, IdentifierDeclare)(name, this);
        declare->isConst = false;
        declare->isScopeVar = false;
        varDeclares[declare->name] = declare;
    }

    declare->isFuncName = true;
    declare->value.function = child;
    child->declare = declare;

    functionDecls.push_back(child);
}

IdentifierDeclare *Scope::getVarDeclarationByIndex(int index) {
    for (auto &item : varDeclares) {
        if (item.second->storageIndex == index) {
            return item.second;
        }
    }

    return nullptr;
}

void Scope::addVarReference(JsExprIdentifier *id) {
    auto it = varDeclares.find(id->name);
    if (it == varDeclares.end()) {
        // 找不到
        if (hasWith || hasEval) {
            // 这一层有 eval，with，只能根据名字在运行时查找变量名
            id->nameStringIdx = 0;
            return;
        }

        if (parent) {
            parent->addVarReference(id);
        } else {
            // 未声明的全局变量
            addImplicitVarDeclaration(id);
        }
    } else {
        auto declare = (*it).second;
        declare->isReferred = 1;

        if (id->scope->function != function) {
            // 变量使用时的函数和声明的函数不在一处
            id->scope->function->isReferredParentVars = true;
            if (declare->varStorageType == VST_ARGUMENT) {
                function->isArgumentsReferredByChild = true;
            } else {
                function->isVarsReferredByChild = true;
            }
        }

        if (id->isModified) {
            if (declare->isConst) {
                // TODO: 报告错误
            }
            declare->isModified |= true;
        }

        if (id->isUsedNotAsFunctionCall) {
            declare->isUsedNotAsFunctionCall = true;
        }

        id->declare = declare;
    }
}

IdentifierDeclare::IdentifierDeclare(const StringView &name, Scope *scope) : name(name), scope(scope) {
    isConst = 0;
    isScopeVar = 0;
    isImplicitDeclaration = 0;
    isReferredByChild = 0;
    isReferred = 0;
    isModified = 0;
    isFuncName = 0;
    isUsedNotAsFunctionCall = 0;

    varStorageType = VST_NOT_SET;
    storageIndex = 0;
}

void IdentifierDeclare::dump(BinaryOutputStream &stream) {
    stream.writeFormat("%.*s ", name.len, name.data);

    if (isConst) stream.writeFormat("isConst:1, ");
    if (isScopeVar) stream.writeFormat("isScopeVar:1, ");
    if (isImplicitDeclaration) stream.writeFormat("isImplicitDeclaration:1, ");
    if (isReferredByChild) stream.writeFormat("isReferredByChild:1, ");
    if (isModified) stream.writeFormat("isModified:1, ");
    if (isFuncName) stream.writeFormat("isFuncName:1, ");
    if (isReferredByChild) stream.writeFormat("isReferredByChild:1, ");

    stream.writeFormat("VarStorageType:%s, ScopeDepth:%d, storageIndex:%d\n",
                       varStorageTypeToString(varStorageType), scope->depth, storageIndex);
    // JsValue                 constValue; // 当 isConst 为 true 时
}

Function::Function(ResourcePool *resourcePool, Scope *parent, uint16_t index, bool isCodeBlock, bool isArrowFunction) : IJsNode(NT_FUNCTION), index(index), resourcePool(resourcePool), isCodeBlock(isCodeBlock), isArrowFunction(isArrowFunction) {
    scope = PoolNew(resourcePool->pool, Scope)(resourcePool, this, parent);
    scope->isFunctionScope = true;

    params = nullptr;
    bytecode = nullptr;
    lenByteCode = 0;
    declare = nullptr;
    isStrictMode = false;
    isVarsReferredByChild = false;
    isReferredParentVars = false;
    isGenerator = false;
    isAsync = false;
    isMemberFunction = false;

    line = 0;
    col = 0;

    resourcePool->needDestructJsNode(this);
}

void Function::generateByteCode() {
    ByteCodeStream stream;

    // 提前将不常用的初始化过程转换为 bytecode，以提高 bytecode 执行的性能
    if (!isCodeBlock) {
        if (!isArrowFunction) {
            if (scope->isThisUsed) {
                stream.writeOpCode(OP_PREPARE_VAR_THIS);
            }

            if (scope->isArgumentsUsed) {
                stream.writeOpCode(OP_PREPARE_VAR_ARGUMENTS);
            }
        }
    }

    if (params) {
        params->convertToByteCode(stream);
    }

    if (!scope->functionDecls.empty()) {
        stream.writeOpCode(OP_INIT_FUNCTION_TO_VARS);
    }

    if (scope->functionArgs) {
        stream.writeOpCode(OP_INIT_FUNCTION_TO_ARGS);
    }

    for (auto item : astNodes) {
        item->convertToByteCode(stream);
    }

    LinkedString *data = stream.startNew();
    if (data) {
        auto bc = resourcePool->pool.duplicate(data);
        bytecode = (uint8_t *)bc.data;
        lenByteCode = (int)bc.len;
    } else {
        bytecode = nullptr;
        lenByteCode = 0;
    }
}

void Function::dump(BinaryOutputStream &stream) {
    if (bytecode == nullptr) {
        generateByteCode();
    }

    if (name.len) {
        stream.writeFormat("Name: %.*s\n", name.len, name.data);
    } else {
        stream.write("Name: (Ananymouse)\n");
    }

    string code((const char *)srcCode.data, srcCode.len > 50 ? 50 : srcCode.len);
    strrep(code, '\n', ' ');
    stream.writeFormat("Source code (line: %d, col: %d, length: %d): %s\n", line, col, srcCode.len, code.c_str());

    stream.writeFormat("Index: %d, %llx\n", index, this);
    if (isStrictMode) stream.writeFormat("IsStrictMode: %d\n", isStrictMode);
    if (isVarsReferredByChild) stream.writeFormat("IsVarsReferredByChild: %d\n", isVarsReferredByChild);
    if (isArgumentsReferredByChild) stream.writeFormat("IsArgumentsReferredByChild: %d\n", isArgumentsReferredByChild);
    if (isReferredParentVars) stream.writeFormat("IsReferredParentVars: %d\n", isReferredParentVars);
    if (isGenerator) stream.writeFormat("IsGenerator: %d\n", isGenerator);
    if (isAsync) stream.writeFormat("IsAsync: %d\n", isAsync);
    if (isMemberFunction) stream.writeFormat("IsMemberFunction: %d\n", isMemberFunction);

    if (declare) {
        stream.writeFormat("ID Declare: %s, %d, %d\n", varStorageTypeToString(declare->varStorageType), declare->scope->depth, declare->storageIndex);
    }

    stream.writeFormat("Scope count: %d\n", scopes.size());
    stream.writeFormat("Children functions count: %d\n", functions.size());

    stream.write("bytecode:\n");
    decodeBytecode(bytecode, lenByteCode, stream);

    stream.write("Children scopes:\n");
    BinaryOutputStream os;
    scope->dump(os);
    writeIndent(stream, os.stringViewStartNew(), StringView("  "));

    stream.write("\nChildren functions:\n");
    for (auto f : functions) {
        f->dump(os);

        stream.writeFormat("  ------- Child function: %d, %llx -------\n", f->index, f);
        writeIndent(stream, os.stringViewStartNew(), StringView("  "));
    }
}

JsNodes::JsNodes(ResourcePool *resourcePool, JsNodeType type) : IJsNode(type) {
    resourcePool->needDestructJsNode(this);
}

ResourcePool::ResourcePool(uint32_t index) : index(index) {
    referIdx = 0;
    nextFreeIdx = 0;
}

ResourcePool::~ResourcePool() {
    free();
}

void ResourcePool::addRegexp(Token &token, const StringView &str, uint32_t flags) {
    token.param.index = (uint32_t)regexps.size();

    regexps.push_back({ StringView(token.buf, token.len),
        std::regex((cstr_t)str.data, (cstr_t)str.data + str.len, (std::regex::flag_type)flags), flags });
}

void ResourcePool::convertUtf8ToUtf16(StringViewUtf16 &str) {
    auto &utf8Str = str.utf8Str();
    auto dataUtf16 = (utf16_t *)pool.allocate(str.size() * sizeof(utf16_t));
    utf8ToUtf16(utf8Str.data, utf8Str.len, dataUtf16, str.size());
    str.setUtf16(dataUtf16, str.size());
}

void ResourcePool::dump(BinaryOutputStream &stream) {
    stream.writeFormat("------ ResourcePool(%d, %llx) ------\n", index, (uint64_t)this);
    stream.writeFormat("  ReferIdx: %d\n", referIdx);
    stream.writeFormat("  Memory Size: %lld\n", pool.totalSize());

    stream.write("  Strings: [\n");
    for (auto &s : strings) {
        stream.write("    ");
        stream.write(s.utf8Str());
        stream.write(",\n");
    }
    stream.write("  ]\n");

    stream.write("  Doubles: [\n");
    for (auto d : doubles) {
        stream.writeFormat("    %llf,\n", d);
    }
    stream.write("  ]\n");
}

void ResourcePool::free() {
    for (auto item : toDestructNodes) {
        item->~IJsNode();
    }
    toDestructNodes.clear();
    toDestructNodes.shrink_to_fit();

    for (auto item : toDestructScopes) {
        item->~Scope();
    }
    toDestructScopes.clear();
    toDestructScopes.shrink_to_fit();

    pool.reset();

    strings.clear();
    strings.shrink_to_fit();

    doubles.clear();
    doubles.shrink_to_fit();

    switchCaseJumps.clear();
    switchCaseJumps.shrink_to_fit();
}

bool jsValueStrictLessThan(VMRuntime *runtime, const JsValue &left, const JsValue &right);

struct CaseJumpLessCmp {
    CaseJumpLessCmp(VMRuntime *runtime) : runtime(runtime) {}

    bool operator()(const CaseJump &a, const CaseJump &b) const {
        return jsValueStrictLessThan(runtime, a.caseConds, b.caseConds);
    }

    bool operator()(const CaseJump &a, const JsValue &b) const {
        return jsValueStrictLessThan(runtime, a.caseConds, b);
    }

    VMRuntime               *runtime;

};

struct CaseJumpEqualCmp {
    CaseJumpEqualCmp(VMRuntime *runtime) : runtime(runtime) {}

    bool operator()(const CaseJump &a, const JsValue &b) const {
        return relationalStrictEqual(runtime, a.caseConds, b);
    }

    VMRuntime               *runtime;

};

VMAddress SwitchJump::findAddress(VMRuntime *runtime, uint16_t poolIndex, const JsValue &cond) {
    if (stmtSwitch) {
        // 需要转换地址
        stmtSwitch->buildCaseJumps(runtime, poolIndex, this);
        stmtSwitch = nullptr;

        sort(caseJumps, caseJumpsEnd, CaseJumpLessCmp(runtime));
    }

    auto caseJump = lower_bound(caseJumps, caseJumpsEnd, cond, CaseJumpLessCmp(runtime));
    if (caseJump != caseJumpsEnd) {
        if (relationalStrictEqual(runtime, cond, caseJump->caseConds)) {
            return caseJump->addr;
        }
    }

    return defaultAddr;
}
