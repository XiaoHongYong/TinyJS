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
#include "ByteCodeStream.hpp"


class JsExprIdentifier;
class IdentifierDeclare;
class Function;
class Scope;
class ResourcePool;
class IJsNode;
class JsNodeParameters;

using MapNameToIdentifiers = map<SizedString, IdentifierDeclare *, SizedStrCmpLess, Allocator<pair<const SizedString, IdentifierDeclare *> > >;
using VecScopes = vector<Scope *>;
using VecFunctions = vector<Function *>;
using VecResourcePools = vector<ResourcePool *>;
using VecJsNodes = vector<IJsNode *>;

/**
 * 语法树结点的类型定义
 */
enum JsNodeType : uint8_t {
    NT_NOT_SET,
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
    NT_TRY,
    NT_RETURN,
    NT_RETURN_VALUE,
    NT_BLOCK,
    NT_EMPTY_STMT,
    NT_INSTRUCTION,
    NT_THROW,
    NT_WITH,
    NT_EXPR_STMT,
    NT_DEBUGGER,
    NT_NEW_TARGET,

    // Expression 的类型
    NT_COMMA_EXPRESSION,
    NT_STRING,
    NT_INT32,
    NT_NUMBER,
    NT_REGEXP,

    NT_IDENTIFIER,
    NT_MEMBER_INDEX,
    NT_MEMBER_DOT,
    NT_FUNCTION_CALL,
    NT_NEW,
    NT_PREPARE_TEMPLATE,
    NT_TEMPLATE_FUNCTION_CALL,

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
    virtual void setBeingAssigned() { throw ParseException(PE_SYNTAX_ERROR, "Invalid destructuring assignment target"); }

    virtual void convertToByteCode(ByteCodeStream &stream) { }

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

    Scope(Function *function, Scope *parent);

    void dump(BinaryOutputStream &stream);

    IdentifierDeclare *addVarDeclaration(const Token &token, bool isConst = false, bool isScopeVar = false);
    void addArgumentDeclaration(const Token &token, int index);
    void addImplicitVarDeclaration(JsExprIdentifier *id);
    void addFunctionDeclaration(const Token &name, Function *child);
    IdentifierDeclare *getVarDeclarationByIndex(int index);

    void addVarReference(JsExprIdentifier *id);

    bool isAllocateFunctionVar() { return !isFunctionScope || hasEval || hasWith; }

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

inline void writeEnterScope(Scope *scope, ByteCodeStream &stream) {
    if (scope->countLocalVars > 0) {
        stream.writeUint8(OP_ENTER_SCOPE);
        stream.writeUint16(scope->index);
    }
}

inline void writeLeaveScope(Scope *scope, ByteCodeStream &stream) {
    if (scope->countLocalVars > 0) {
        stream.writeUint8(OP_LEAVE_SCOPE);
    }
}

class JsNodes : public IJsNode {
public:
    JsNodes(JsNodeType type) : IJsNode(type) { }

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

    VecJsNodes                  nodes;

};

class JsNodeVarDeclarationList : public JsNodes {
public:
    JsNodeVarDeclarationList() : JsNodes(NT_VAR_DECLARTION_LIST) { }

};

class JsStmtBlock : public JsNodes {
public:
    JsStmtBlock(Scope *scope) : JsNodes(NT_BLOCK), scope(scope) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        writeEnterScope(scope, stream);

        for (auto item : nodes) {
            item->convertToByteCode(stream);
        }

        writeLeaveScope(scope, stream);
    }

protected:
    Scope                       *scope;

};

class JsNodeSpreadArgument : public IJsNode {
public:
    JsNodeSpreadArgument(IJsNode *expr) : IJsNode(NT_SPREAD_ARGUMENT), expr(expr) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_SPREAD_ARGS);
    }

protected:
    IJsNode                     *expr;

};

class JsNodeRestParameter : public IJsNode {
public:
    JsNodeRestParameter(IJsNode *id, uint16_t index) : IJsNode(NT_REST_PARAMETER), id(id), index(index) { }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
        id->setBeingAssigned();
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_REST_PARAMETER);
        stream.writeUint16(index);
    }

protected:
    IJsNode                     *id;
    uint16_t                    index;

};

/**
 * 给声明的参数赋值为栈顶的值
 */
class JsNodeAssignWithParameter : public IJsNode {
public:
    JsNodeAssignWithParameter(IJsNode *target, uint16_t index) : IJsNode(NT_ASSIGN_WITH_PARAMETER), target(target), index(index) { }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
        target->setBeingAssigned();
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_ID_LOCAL_ARGUMENT);
        stream.writeUint16(index);

        target->convertToByteCode(stream);

        stream.writeOpCode(OP_POP_STACK_TOP);
    }

protected:
    IJsNode                     *target;
    uint16_t                    index;

};

/**
 * 如果第 index 参数为 null/undefined, 则使用缺省的参数值
 */
class JsNodeUseDefaultParameter : public IJsNode {
public:
    JsNodeUseDefaultParameter(IJsNode *left, IJsNode *defVal, uint16_t index) : IJsNode(NT_USE_DEFAULT_PARAMETER), left(left), defVal(defVal), index(index) { }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
        left->setBeingAssigned();
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_ID_LOCAL_ARGUMENT);
        stream.writeUint16(index);

        stream.writeUint8(OP_JUMP_IF_NOT_NULL_UNDEFINED_KEEP_VALID);
        auto addrEnd = stream.writeReservedAddress();

        // 插入缺省值表达式的执行代码.
        defVal->convertToByteCode(stream);
        left->convertToByteCode(stream);

        *addrEnd = stream.address();
    }

protected:
    IJsNode                     *left, *defVal;
    uint16_t                    index;

};

/**
 * 函数声明的参数定义
 */
class JsNodeParameters : public JsNodes {
public:
    JsNodeParameters() : JsNodes(NT_PARAMETERS) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        for (auto item : nodes) {
            if (item->type != NT_IDENTIFIER) {
                item->convertToByteCode(stream);
            }
        }
    }

};

class JsStmtEmpty : public IJsNode {
public:
    JsStmtEmpty() : IJsNode(NT_EMPTY_STMT) { }

};

class JsStmtForIn : public IJsNode {
public:
    JsStmtForIn(IJsNode *var, IJsNode *obj, IJsNode *stmt, bool isIn, Scope *scope) : IJsNode(NT_FOR_IN), var(var), obj(obj), stmt(stmt), isIn(isIn), scope(scope) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        writeEnterScope(scope, stream);

        obj->convertToByteCode(stream);
        stream.writeOpCode(OP_ITERATOR_CREATE);

        auto addrLoopStart = stream.address();
        if (isIn) {
            stream.writeOpCode(OP_ITERATOR_NEXT_KEY);
        } else {
            stream.writeOpCode(OP_ITERATOR_NEXT_VALUE);
        }
        auto addrLoopEnd = stream.writeReservedAddress();

        var->convertToByteCode(stream);

        if (stmt)
            stmt->convertToByteCode(stream);

        stream.writeOpCode(OP_JUMP);
        stream.writeAddress(addrLoopStart);
        *addrLoopEnd = stream.address();

        writeLeaveScope(scope, stream);
    }

    IJsNode                     *var, *obj, *stmt;
    bool                        isIn;
    Scope                       *scope;

};

class JsStmtFor : public IJsNode {
public:
    JsStmtFor(IJsNode *init, IJsNode *cond, IJsNode *finalExpr, IJsNode *stmt, Scope *scope) : IJsNode(NT_FOR), init(init), cond(cond), finalExpr(finalExpr), stmt(stmt), scope(scope) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        writeEnterScope(scope, stream);

        if (init) {
            init->convertToByteCode(stream);
            if (init->type != NT_VAR_DECLARTION_LIST) {
                // 表达式：需要弹出栈顶值
                stream.writeOpCode(OP_POP_STACK_TOP);
            }
        }

        uint32_t *addrLoopEnd = nullptr;
        auto addrLoopStart = stream.address();
        if (init) {
            init->convertToByteCode(stream);
            stream.writeOpCode(OP_JUMP_IF_FALSE);
            addrLoopEnd = stream.writeReservedAddress();
        }

        stmt->convertToByteCode(stream);

        if (finalExpr)
            finalExpr->convertToByteCode(stream);

        stream.writeOpCode(OP_JUMP);
        stream.writeAddress(addrLoopStart);
        if (addrLoopEnd) {
            *addrLoopEnd = stream.address();
        }

        writeLeaveScope(scope, stream);
    }

    IJsNode                     *init, *cond, *finalExpr, *stmt;
    Scope                       *scope;

};

class JsStmtDoWhile : public IJsNode {
public:
    JsStmtDoWhile(IJsNode *cond, IJsNode *stmt) : IJsNode(NT_DO_WHILE), cond(cond), stmt(stmt) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        auto begin = stream.address();
        stmt->convertToByteCode(stream);

        cond->convertToByteCode(stream);
        stream.writeOpCode(OP_JUMP_IF_TRUE);
        stream.writeAddress(begin);
    }

    IJsNode                     *cond, *stmt;

};

class JsStmtWhile : public IJsNode {
public:
    JsStmtWhile(IJsNode *cond, IJsNode *stmt) : IJsNode(NT_WHILE), cond(cond), stmt(stmt) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        auto begin = stream.address();
        cond->convertToByteCode(stream);
        stream.writeOpCode(OP_JUMP_IF_FALSE);
        auto addrEnd = stream.writeReservedAddress();

        stmt->convertToByteCode(stream);

        stream.writeOpCode(OP_JUMP);
        stream.writeAddress(begin);

        *addrEnd = stream.address();
    }

    IJsNode                     *cond, *stmt;

};

class JsStmtIf : public IJsNode {
public:
    JsStmtIf(IJsNode *cond, IJsNode *stmtTrue, IJsNode *stmtFalse) : IJsNode(NT_IF), cond(cond), stmtTrue(stmtTrue), stmtFalse(stmtFalse) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        cond->convertToByteCode(stream);

        if (stmtTrue) {
            stream.writeOpCode(OP_JUMP_IF_FALSE);
            auto addrFalse = stream.writeReservedAddress();

            stmtTrue->convertToByteCode(stream);
            if (stmtFalse) {
                stream.writeOpCode(OP_JUMP);
                auto addrEnd = stream.writeReservedAddress();

                *addrFalse = stream.address();
                stmtFalse->convertToByteCode(stream);

                *addrEnd = stream.address();
            } else {
                *addrFalse = stream.address();
            }
        } else {
            if (stmtFalse) {
                stream.writeOpCode(OP_JUMP_IF_TRUE);
                auto addrEnd = stream.writeReservedAddress();
                stmtFalse->convertToByteCode(stream);
                *addrEnd = stream.address();
            } else {
                // true/false 都没有
                stream.writeOpCode(OP_POP_STACK_TOP);
            }
        }
    }

    IJsNode                     *cond, *stmtTrue, *stmtFalse;

};

class JsStmtReturnValue : public IJsNode {
public:
    JsStmtReturnValue(IJsNode *expr) : IJsNode(NT_RETURN_VALUE), expr(expr) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(expr);
        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_RETURN_VALUE);
    }

    IJsNode                     *expr;

};

class JsStmtReturn : public IJsNode {
public:
    JsStmtReturn() : IJsNode(NT_RETURN) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_RETURN);
    }

};

class JsStmtThrow : public IJsNode {
public:
    JsStmtThrow(IJsNode *expr) : IJsNode(NT_THROW), expr(expr) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(expr);
        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_THROW);
    }

    IJsNode                     *expr;

};

class JsStmtWith : public IJsNode {
public:
    JsStmtWith(IJsNode *expr, IJsNode *stmt, Scope *scope) : IJsNode(NT_WITH), expr(expr), stmt(stmt), scope(scope) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        writeEnterScope(scope, stream);

        expr->convertToByteCode(stream);

        // TODO: ...

        writeLeaveScope(scope, stream);
    }

    IJsNode                     *expr, *stmt;
    Scope                       *scope;

};

class JsStmtTry : public IJsNode {
public:
    JsStmtTry(IJsNode *stmtTry, IJsNode *exprCatch, IJsNode *stmtCatch, Scope *scopeCatch, IJsNode *stmtFinal) : IJsNode(NT_TRY), stmtTry(stmtTry), exprCatch(exprCatch), stmtCatch(stmtCatch), scopeCatch(scopeCatch), stmtFinal(stmtFinal) {
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_TRY_START);
        auto addrCatch = stream.writeReservedAddress();
        auto addrFinally = stream.writeReservedAddress();

        stmtTry->convertToByteCode(stream);

        if (stmtCatch) {
            // 顺序从 try 执行的指令应该跳转到 catch 结束
            stream.writeOpCode(OP_JUMP);
            auto addrCatchEnd = stream.writeReservedAddress();

            // 有异常发生时跳转到的地址
            *addrCatch = stream.address();

            if (exprCatch) {
                writeEnterScope(scopeCatch, stream);
                stream.writeOpCode(OP_PUSH_EXCEPTION);

                // 将异常赋值到此变量
                exprCatch->convertToByteCode(stream);

                // 将栈顶的值删除
                stream.writeOpCode(OP_POP_STACK_TOP);
            }

            stmtCatch->convertToByteCode(stream);

            if (exprCatch) {
                writeLeaveScope(scopeCatch, stream);
            }

            *addrCatchEnd = stream.address();
        }

        if (stmtFinal) {
            // 顺序执行会有 OP_FINALLY_JUMP
            stream.writeOpCode(OP_BEGIN_FINALLY_NORMAL);

            // 有异常发生会从此开始执行
            *addrFinally = stream.address();

            stmtFinal->convertToByteCode(stream);

            stream.writeOpCode(OP_FINISH_FINALLY);
        }
    }

    IJsNode                     *stmtTry, *exprCatch, *stmtCatch, *stmtFinal;
    Scope                       *scopeCatch;

};

class JsStmtExpr : public IJsNode {
public:
    JsStmtExpr(IJsNode *expr) : IJsNode(NT_EXPR_STMT), expr(expr) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(expr);
        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_POP_STACK_TOP);
    }

    IJsNode                     *expr;

};

class JsStmtDebugger : public IJsNode {
public:
    JsStmtDebugger() : IJsNode(NT_DEBUGGER) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_DEBUGGER);
    }

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

    void dump(BinaryOutputStream &stream);

};

void writeIndent(BinaryOutputStream &stream, SizedString str, const SizedString &indent);

#endif /* AST_hpp */
