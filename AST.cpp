//
//  AST.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#include "AST.hpp"


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
void writeIndent(BinaryOutputStream &stream, SizedString str, const SizedString &indent) {
    while (str.len) {
        size_t n;
        const uint8_t *p = str.strlchr('\n');
        if (p) {
            n = size_t(p - str.data) + 1;
        } else {
            n = str.len;
        }
        stream.write(indent);
        stream.write(str.data, n);
        str.data += n;
        str.len -= n;
    }
}

Scope::Scope(Function *function, Scope *parent) : function(function), parent(parent), varDeclares(function->resourcePool->pool) {
    child = sibling = nullptr;
    depth = 0;
    hasWith = false;
    hasEval = false;
    isFunctionScope = false;
    isThisUsed = false;
    isArgumentsUsed = false;

    countLocalVars = 0;

    index = (uint16_t)function->scopes.size();
    assert(function->scopes.size() < 0xFFFF);
    function->scopes.push_back(this);

    if (parent) {
        sibling = parent->child;
        parent->child = this;

        depth = parent->depth + 1;
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
        writeIndent(stream, os.startNew(), makeSizedString("  "));
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
        writeIndent(stream, os.startNew(), makeSizedString("  "));
    }

    if (sibling) {
        sibling->dump(stream);
    }
}

IdentifierDeclare *Scope::addVarDeclaration(const Token &token, bool isConst, bool isScopeVar) {
    auto &pool = varDeclares.get_allocator().getPool();
    auto node = PoolNew(pool, IdentifierDeclare)(token, this);
    node->isConst = isConst;
    node->isScopeVar = isScopeVar;

    if (isScopeVar) {
        node->varStorageType = VST_SCOPE_VAR;
        node->storageIndex = countLocalVars++;
    }

    auto it = varDeclares.find(node->name);
    if (it == varDeclares.end()) {
        varDeclares[node->name] = node;
    } else {
        node = (*it).second;
    }

    return node;
}

void Scope::addArgumentDeclaration(const Token &token, int index) {
    assert(isFunctionScope);

    auto &pool = varDeclares.get_allocator().getPool();
    auto node = PoolNew(pool, IdentifierDeclare)(token, this);
    node->isConst = false;
    node->varStorageType = VST_ARGUMENT;
    node->storageIndex = index;

    auto it = varDeclares.find(node->name);
    if (it == varDeclares.end()) {
        varDeclares[node->name] = node;
    }
}

/**
 * 添加隐含的未声明的全局变量
 */
void Scope::addImplicitVarDeclaration(IdentifierRef *id) {
    assert(parent == nullptr);

    auto &pool = varDeclares.get_allocator().getPool();
    auto node = PoolNew(pool, IdentifierDeclare)(id->name, this);
    node->isConst = false;
    node->isImplicitDeclaration = true;
    node->varStorageType = VST_GLOBAL_VAR;
    node->storageIndex = countLocalVars++;
    varDeclares[id->name] = node;

    id->declare = node;
}

void Scope::addFunctionDeclaration(const Token &name, Function *child) {
    auto declare = function->scope->addVarDeclaration(name);
    declare->isFuncName = true;
    declare->value.function = child;
    child->declare = declare;

    functionDecls.push_back(child);
}

void Scope::addVarReference(IdentifierRef *id) {
    auto it = varDeclares.find(id->name);
    if (it == varDeclares.end()) {
        // 找不到
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
            } else if (declare->isFuncName) {
                // 被修改的函数名，此函数需要在进入 scope 时，创建函数对象.
                
            }
            declare->isModified |= true;
        }

        if (id->isUsedNotAsFunctionCall) {
            declare->isUsedNotAsFunctionCall = true;
        }
        
        id->declare = declare;
    }
}

IdentifierDeclare::IdentifierDeclare(const SizedString &name, Scope *scope) : name(name), scope(scope) {
    isConst = 0;
    isScopeVar = 0;
    isImplicitDeclaration = 0;
    isReferredByChild = 0;
    isReferred = 0;
    isModified = 0;
    isFuncName = 0;
    isUsedNotAsFunctionCall = 0;

    varStorageType = VST_NOT_SET;
    scopeDepth = scope->depth;
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
                       varStorageTypeToString(varStorageType), scopeDepth, storageIndex);
    // JsValue                 constValue; // 当 isConst 为 true 时
}

IdentifierRef::IdentifierRef(const Token &token, Scope *scope) : scope(scope) {
    name = tokenToSizedString(token);

    isModified = false;
    isUsedNotAsFunctionCall = false;
    declare = nullptr;
    next = nullptr;
}

Function::Function(ResourcePool *resourcePool, Scope *parent, uint16_t index, bool isCodeBlock, bool isArrowFunction) : instructions(resourcePool), index(index), resourcePool(resourcePool), isCodeBlock(isCodeBlock), isArrowFunction(isArrowFunction) {
    scope = PoolNew(resourcePool->pool, Scope)(this, parent);
    scope->isFunctionScope = true;

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
}

void Function::generateByteCode() {
    BinaryOutputStream stream;
    instructions.convertToByteCode(stream);
    auto data = stream.startNew();
    bytecode = resourcePool->pool.duplicate(data.data, data.len);
    lenByteCode = (int)data.len;
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
        stream.writeFormat("ID Declare: %s, %d, %d\n", varStorageTypeToString(declare->varStorageType), declare->scopeDepth, declare->storageIndex);
    }
    
    // stream.writeFormat("Instructions count: %d\n", instructions._instructions.size());
    stream.writeFormat("Scope count: %d\n", scopes.size());
    stream.writeFormat("Children functions count: %d\n", functions.size());

    stream.write("bytecode:\n");
    decodeBytecode(bytecode, lenByteCode, stream);

    stream.write("Children scopes:\n");
    BinaryOutputStream os;
    scope->dump(os);
    writeIndent(stream, os.startNew(), makeSizedString("  "));

    stream.write("\nChildren functions:\n");
    for (auto f : functions) {
        f->dump(os);

        stream.writeFormat("  ------- Child function: %d, %llx -------\n", f->index, f);
        writeIndent(stream, os.startNew(), makeSizedString("  "));
    }
}

ResourcePool::ResourcePool() {
    _streamBuf = nullptr;
    index = 0;
    referIdx = 0;
}

void ResourcePool::dump(BinaryOutputStream &stream) {
    stream.writeFormat("------ ResourcePool(%d, %llx) ------\n", index, (uint64_t)this);
    stream.writeFormat("  ReferIdx: %d\n", referIdx);
    stream.writeFormat("  Memory Size: %lld\n", pool.totalSize());

    stream.write("  Strings: [\n");
    for (auto &s : strings) {
        stream.write("    ");
        stream.write(s);
        stream.write(",\n");
    }
    stream.write("  ]\n");

    stream.write("  Doubles: [\n");
    for (auto d : doubles) {
        stream.writeFormat("    %llf,\n", d);
    }
    stream.write("  ]\n");
}

StreamBuffer *ResourcePool::allocStreamBuffer() {
    if (_streamBuf) {
        auto p = _streamBuf;
        _streamBuf = _streamBuf->next;
        p->next = nullptr;
        return p;
    } else {
        return newStreamBuffer();
    }
}

void ResourcePool::freeStreamBuffer(StreamBuffer *buf) {
    assert(buf->next == nullptr);
    buf->next = _streamBuf;
    _streamBuf = buf;
}

InstructionOutputStream *ResourcePool::allocInstructionOutputStream() {
    auto stream = PoolNew(pool, InstructionOutputStream)(this);
    return stream;
}

StreamBuffer *ResourcePool::newStreamBuffer() {
    const int BUFF_SIZE = 1024 * 4;

    auto buf = (StreamBuffer *)pool.allocate(BUFF_SIZE);
    buf->next = nullptr;
    buf->end = (uint8_t *)buf + BUFF_SIZE;
    buf->pos = buf->last = (uint8_t *)buf + sizeof(StreamBuffer);

    return buf;
}
