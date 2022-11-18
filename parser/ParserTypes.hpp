//
//  ParserTypes.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#ifndef ParserTypes_hpp
#define ParserTypes_hpp

#include <deque>
#include <unordered_map>
#include <regex>
#include "Lexer.hpp"
#include "ByteCodeStream.hpp"


class JsExprIdentifier;
class IdentifierDeclare;
class Function;
class Scope;
class ResourcePool;
class IJsNode;
class JsNodeParameters;
class JsStmtSwitch;
class VMRuntime;

using MapNameToIdentifiers = unordered_map<SizedString, IdentifierDeclare *, SizedStringHash, SizedStrCmpEqual>;
using VecScopes = vector<Scope *>;
using VecFunctions = vector<Function *>;
using VecResourcePools = vector<ResourcePool *>;
using VecJsNodes = vector<IJsNode *>;
using DequeJsNodes = deque<IJsNode *>;
using DequeScopes = deque<Scope *>;

/**
 * 语法树结点的类型定义
 */
enum JsNodeType : uint8_t {
    NT_NOT_SET,

    // Expression 的类型
    NT_BOOLEAN_TRUE,
    NT_BOOLEAN_FALSE,
    NT_NULL,
    NT_STRING,
    NT_CHAR,
    NT_INT32,
    NT_NUMBER,
    NT_REGEXP,

    // 常量类型的 expression 结束位置
    _NT_CONST_EXPR_END,

    NT_IDENTIFIER,
    NT_MEMBER_INDEX,
    NT_MEMBER_DOT,
    NT_FUNCTION_CALL,
    NT_NEW,
    NT_PREPARE_TEMPLATE,
    NT_TEMPLATE_FUNCTION_CALL,

    NT_NEW_TARGET,
    NT_COMMA_EXPRESSION,
    NT_PAREN_EXPRESSION,

    NT_ARRAY,
    NT_ARRAY_ITEM,
    NT_ARRAY_ITEM_SPREAD,
    NT_ARRAY_ITEM_EMPTY,

    NT_OBJECT,
    NT_OBJECT_PROPERTY,
    NT_OBJECT_PROPERTY_GETTER,
    NT_OBJECT_PROPERTY_SETTER,
    NT_OBJECT_PROPERTY_COMPUTED,
    NT_OBJECT_PROPERTY_SPREAD,

    NT_FUNCTION_EXPR,

    NT_UNARY_PREFIX,
    NT_DELETE,
    NT_PREFIX_XCREASE,
    NT_POSTFIX,

    NT_CONDITIONAL,
    NT_NULLISH,
    NT_LOGICAL_OR,
    NT_LOGICAL_AND,
    NT_BINARAY_OP,

    NT_ASSIGN,
    NT_ASSIGN_X,
    NT_ASSIGN_WITH_STACK_TOP,
    NT_ASSIGN_WITH_PARAMETER,
    NT_USE_DEFAULT_PARAMETER,

    // Statement 或者相关依赖的 NodeType
    NT_VAR_DECLARTION_LIST,
    NT_PARAMETERS,
    NT_SPREAD_ARGUMENT,
    NT_REST_PARAMETER,
    NT_FUNCTION,

    NT_IF,
    NT_FOR,
    NT_FOR_IN,
    NT_DO_WHILE,
    NT_WHILE,
    NT_SWITCH,
    NT_SWITCH_BRANCH, // case or default
    NT_DEFAULT,
    NT_BREAK,
    NT_CONTINUE,
    NT_TRY,
    NT_RETURN,
    NT_RETURN_VALUE,
    NT_BLOCK,
    NT_EMPTY_STMT,
    NT_THROW,
    NT_WITH,
    NT_EXPR_STMT,
    NT_DEBUGGER,
};

/**
 * 语法树结点的基本接口定义
 */
class IJsNode {
public:
    IJsNode(JsNodeType type) : type(type) {
        isBeingAssigned = false;
    }
    virtual ~IJsNode() {}

    // 用于判断 Array/Object 是否可以作为一个表达式.
    // 带 shorthand property initializer 的是不能作为表达式的，比如: { x=y }
    virtual bool canBeExpression() { return true; }
    virtual void setBeingAssigned() { throw ParseException(JE_SYNTAX_ERROR, "Invalid destructuring assignment target"); }

    virtual void convertToByteCode(ByteCodeStream &stream) { }

    // 正常情况下，各个 Assignable 需要实现自己的此函数
    // 目前只有实现了 setBeingAssigned 的才应该实现此函数，但是并非所有的都需要实现.
    // valueOpt 可以为 null，表示需要从前一个指令返回的栈顶获取值.
    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) { assert(0 && "Should NOT enter here!"); }

    JsNodeType                  type;
    JsDataType                  dataType;

protected:
    bool                        isBeingAssigned; // 此 Expression 是否为赋值的左边

};

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

    void dump(BinaryOutputStream &stream);

    SizedString             name;

    // 此变量所在的 scope
    Scope                   *scope;

    // const 声明的，不能被修改
    uint8_t                 isConst : 1;

    uint8_t                 isScopeVar : 1;

    // 是否为隐式声明的变量
    uint8_t                 isImplicitDeclaration : 1;

    // 此变量是否被子函数引用到了
    uint8_t                 isReferredByChild : 1;

    // 此变量是否被引用到了
    uint8_t                 isReferred : 1;

    // 是否被修改了
    uint8_t                 isModified : 1;

    // 是否为函数名
    uint8_t                 isFuncName : 1;

    // 标识符是否被除函数调用外的方式引用到了(比如赋值，参数等)
    // 此标志的目的是为了避免一进入函数就必须得把所有的子函数都初始化一遍.
    uint8_t                 isUsedNotAsFunctionCall : 1;

    // 变量存储的类型
    VarStorageType          varStorageType;
    uint16_t                storageIndex; // 存储的索引，当 varStorageType 为 VST_GLOBAL, VST_STACK, VST_ARGS 等有效

    JsValue                 constValue; // 当 isConst 为 true 时
    union {
        Function            *function; // 当 isFuncName 为 true 时，对应声明的函数
    } value;

};

class Scope {
public:
    Function                *function;
    Scope                   *parent, *child, *sibling;
    VecFunctions            functions; // 此 scope 内的所有函数，包括函数表达式.
    VecFunctions            functionDecls;  // 仅仅包括声明的函数
    VecFunctions            *functionArgs;  // 和参数同名的函数
    MapNameToIdentifiers    varDeclares;
    uint16_t                countLocalVars;
    uint16_t                countArguments;
    uint16_t                index; // Scope 的索引编号
    int8_t                  depth;
    uint8_t                 hasWith : 1, hasEval : 1;
    uint8_t                 isFunctionScope : 1; // 是否为函数的 scope
    uint8_t                 isThisUsed : 1; // 是否 'this' 被使用了
    uint8_t                 isArgumentsUsed : 1; // 是否 'arguments' 被使用了

    Scope(ResourcePool *resourcePool, Function *function, Scope *parent);
    ~Scope();

    void dump(BinaryOutputStream &stream);

    IdentifierDeclare *addVarDeclaration(const SizedString &name, bool isConst = false, bool isScopeVar = false);
    IdentifierDeclare *addVarDeclaration(const Token &token, bool isConst = false, bool isScopeVar = false) {
        return addVarDeclaration(tokenToSizedString(token), isConst, isScopeVar);
    }
    void addArgumentDeclaration(const SizedString &name, int index);
    void addArgumentDeclaration(const Token &token, int index) { addArgumentDeclaration(tokenToSizedString(token), index); }
    void addImplicitVarDeclaration(JsExprIdentifier *id);
    void addFunctionDeclaration(const Token &name, Function *child);
    IdentifierDeclare *getVarDeclarationByIndex(int index);
    IdentifierDeclare *getVarDeclarationByName(const SizedString &name) {
        auto it = varDeclares.find(name);
        if (it == varDeclares.end()) {
            return nullptr;
        }
        return (*it).second;
    }

    void addVarReference(JsExprIdentifier *id);

    bool isAllocateFunctionVar() { return !isFunctionScope || hasEval || hasWith; }
    bool isNeeded() const { return countLocalVars > 0 || hasEval || hasWith; }

    void setHasEval() {
        if (!hasEval) {
            if (parent) parent->setHasEval();
            hasEval = 1;
            isArgumentsUsed = 1;
            isThisUsed = 1;
        }
    }

};

class Function : public IJsNode {
public:
    Function(ResourcePool *resourcePool, Scope *parent, uint16_t index, bool isCodeBlock = false, bool isArrowFunction = false);

    void generateByteCode();
    void addFunction(Function *f) {
        functions.push_back(f);
    }

    void dump(BinaryOutputStream &stream);

public:
    ResourcePool            *resourcePool;

    SizedString             name;
    SizedString             srcCode;
    JsValue                 srcCodeValue;
    int                     line, col;

    uint8_t                 *bytecode;
    int                     lenByteCode;

    JsNodeParameters        *params;
    VecJsNodes              astNodes;
    VecFunctions            functions; // 所有的子函数列表(包括了子 scope 中的)，用于根据 index 快速找到子 function.
    VecScopes               scopes; // 所有的子 scopes 列表

    Scope                   *scope;

    IdentifierDeclare       *declare; // 变量声明

    uint16_t                index;

    // 是 eval 的代码片段，不是 function
    bool                    isCodeBlock;

    bool                    isStrictMode;

    // 是否有本地的变量被子函数引用
    bool                    isVarsReferredByChild;

    // 参数是否被子函数引用到了
    bool                    isArgumentsReferredByChild;

    // 是否引用到了父函数的变量
    bool                    isReferredParentVars;

    bool                    isGenerator;

    bool                    isAsync;

    bool                    isMemberFunction;

    bool                    isArrowFunction;

};

class JsNodes : public IJsNode {
public:
    JsNodes(ResourcePool *resourcePool, JsNodeType type);

    virtual bool canBeExpression() {
        for (auto item : nodes) {
            if (!item->canBeExpression()) return false;
        }
        return true;
    }

    virtual void setBeingAssigned() {
        for (auto item : nodes) {
            item->setBeingAssigned();
        }
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        for (auto item : nodes) {
            item->convertToByteCode(stream);
        }
    }

    void push(IJsNode *node) { nodes.push_back(node); }
    size_t count() { return nodes.size(); }

    VecJsNodes              nodes;

};


struct CaseJump {
    JsValue                 caseConds;
    uint32_t                addr;
};

struct SwitchJump {
    JsStmtSwitch            *stmtSwitch;
    CaseJump                *caseJumps, *caseJumpsEnd;
    uint32_t                defaultAddr;

    VMAddress findAddress(VMRuntime *runtime, uint16_t poolIndex, const JsValue &cond);
};

struct RegexpInfo {
    SizedString             str;
    std::regex              re;
    uint32_t                flags;
};

using VecSwitchJumps = vector<SwitchJump>;

/**
 * 负责在解析阶段的内存分配
 */
class ResourcePool {
public:
    uint32_t                index; // 在 VMRuntime 中的索引
    int8_t                  referIdx; // 用于资源回收时所用
    uint32_t                nextFreeIdx; // 下一个空闲的索引位置

    AllocatorPool           pool;
    VecSizedStringUtf16s    strings;
    vector<double>          doubles;
    VecSwitchJumps          switchCaseJumps;
    vector<RegexpInfo>      regexps;

    // 当 ResourcePool 被释放时，需要调用 toDestructNodes, toDestructScopes 的析构函数
    DequeJsNodes            toDestructNodes;
    DequeScopes             toDestructScopes;

public:
    ResourcePool(uint32_t index = 0);
    ~ResourcePool();

    void addRegexp(Token &token, const SizedString &str, uint32_t flags);

    inline void needDestructJsNode(IJsNode *node) { toDestructNodes.push_back(node); }
    inline void needDestructScope(Scope *scope) { toDestructScopes.push_back(scope); }

    void convertUtf8ToUtf16(SizedStringUtf16 &str);

    void dump(BinaryOutputStream &stream);

    void free();

};

void writeIndent(BinaryOutputStream &stream, SizedString str, const SizedString &indent);

#endif /* ParserTypes_hpp */
