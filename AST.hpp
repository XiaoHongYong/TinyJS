//
//  AST.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#ifndef AST_hpp
#define AST_hpp

#include "Lexer.hpp"
#include "JitCodeCompiler.hpp"
#include "InstructionOutputStream.hpp"


class IdentifierRef;
class IdentifierDeclare;
class Function;
class Scope;
class ResourcePool;

using MapNameToIdentifiers = map<SizedString, IdentifierDeclare *, SizedStrCmpLess, Allocator<pair<const SizedString, IdentifierDeclare *> > >;
using VecScopes = vector<Scope *>;
using VecFunctions = vector<Function *>;
using VecResourcePools = vector<ResourcePool *>;

enum VarStorageType : int8_t {
    VST_NOT_SET,
    VST_GLOBAL_VAR,
    VST_FUNCTION_VAR,
    VST_SCOPE_VAR,
    VST_ARGUMENT,
    VST_REGISTER,
//    VST_STACK,
//    VST_FUNCTION,
//    VST_MEM,
//    VST_IM,
};

const char *varStorageTypeToString(VarStorageType type);

class IdentifierDeclare {
public:
    IdentifierDeclare(const SizedString &name, Scope *scope);
    IdentifierDeclare(const Token &token, Scope *scope) : IdentifierDeclare(tokenToSizedString(token), scope) { }

    SizedString             name;

    // 此变量所在的 scope
    Scope                   *scope;

    // const 声明的，不能被修改
    uint8_t                 isConst : 1;

    // 是否为隐式声明的变量
    uint8_t                 isImplicitDeclaration : 1;

    // 此变量是否被子函数引用到了
    uint8_t                 isReferredByChild : 1;

    // 是否被修改了
    uint8_t                 isModified : 1;

    // 是否为函数名
    uint8_t                 isFuncName : 1;

    // 变量存储的类型
    VarStorageType          varStorageType;
    int8_t                  scopeDepth; // 所在 Scope 的层级
    int16_t                 storageIndex; // 存储的索引，当 varStorageType 为 VST_GLOBAL, VST_STACK, VST_ARGS 等有效

    JsValue                 constValue; // 当 isConst 为 true 时
    union {
        Function            *function; // 当 isFuncName 为 true 时，对应声明的函数
    } value;

};

class IdentifierRef {
public:
    IdentifierRef(const Token &token, Scope *scope);

    SizedString             name;

    // 是否被修改了
    bool                    isModified;

    IdentifierDeclare       *declare;

    // 此变量所在的 scope
    Scope                   *scope;

    // 所有被引用到的 Identifier 会通过 next 连起来，方便后期分析遍历
    IdentifierRef           *next;

};


class Scope {
public:
    Function                *function;
    Scope                   *parent, *child, *sibling;
    VecFunctions            functions; // 此 scope 内的所有函数，包括函数表达式.
    VecFunctions            functionDecls;  // 仅仅包括声明的函数
    MapNameToIdentifiers    varDeclares;
    uint16_t                countLocalVars;
    uint16_t                index; // Scope 的索引编号
    int8_t                  depth;
    bool                    hasWith, hasEval;
    bool                    isFunctionScope; // 是否为函数的 scope

    Scope(Function *function, Scope *parent);

    IdentifierDeclare *addVarDeclaration(const Token &token, bool isConst = false);
    void addArgumentDeclaration(const Token &token, int index);
    void addImplicitVarDeclaration(IdentifierRef *id);
    void addFunctionDeclaration(const Token &name, Function *child);

    void addVarReference(IdentifierRef *id);

    bool isAllocateFunctionVar() { return !isFunctionScope || hasEval || hasWith; }
    
};

class Function {
public:
    Function(ResourcePool *resourcePool, Scope *parent, uint16_t index);

    void generateByteCode();
    void addFunction(Function *f) {
        functions.push_back(f);
    }

    void dump(BinaryOutputStream &stream);
    
public:
    ResourcePool            *resourcePool;

    SizedString             name;
    SizedString             srcCode;
    int                     line, col;

    uint8_t                 *bytecode;
    int                     lenByteCode;

    InstructionOutputStream instructions;
    VecFunctions            functions; // 所有的子函数列表(包括了子 scope 中的)，用于根据 index 快速找到子 function.
    VecScopes               scopes; // 所有的子 scopes 列表

    Scope                   *scope;

    IdentifierDeclare       *declare; // 变量声明

    uint16_t                index;

    bool                    isStrictMode;
    
    // 是否有本地的变量被子函数引用
    bool                    isVarsReferredByChild;

    // 参数是否被子函数引用到了
    bool                    isArgumentsReferredByChild;

    // 是否引用到了父函数的变量
    bool                    isReferredParentVars;
    
    bool                    isGenerator;

};

/**
 * 负责在解析阶段的内存分配
 */
class ResourcePool {
public:
    uint32_t                index; // 在 VMRuntime 中的索引
    int8_t                  referIdx; // 用于资源回收时所用
    AllocatorPool           pool;
    VecSizedStrings         strings;
    vector<double>          doubles;

public:
    ResourcePool();

    StreamBuffer *allocStreamBuffer();
    void freeStreamBuffer(StreamBuffer *buf);

    InstructionOutputStream *allocInstructionOutputStream();

protected:
    StreamBuffer *newStreamBuffer();

    StreamBuffer            *_streamBuf;

};

#endif /* AST_hpp */
