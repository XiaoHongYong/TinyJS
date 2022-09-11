//
//  Statements.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/14.
//

#ifndef Statements_hpp
#define Statements_hpp

#include "parser/ParserTypes.hpp"


inline void writeEnterScope(Scope *scope, ByteCodeStream &stream) {
    if (scope->countLocalVars > 0) {
        stream.writeOpCode(OP_ENTER_SCOPE);
        stream.writeUInt16(scope->index);
    }
}

inline void writeLeaveScope(Scope *scope, ByteCodeStream &stream) {
    if (scope->countLocalVars > 0) {
        stream.writeOpCode(OP_LEAVE_SCOPE);
    }
}

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
        assert(!isBeingAssigned);
        stream.writeOpCode(OP_REST_PARAMETER);
        stream.writeUInt16(index);
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);
        assert(0 && "TBD");
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
        assert(!isBeingAssigned);
        stream.writeOpCode(OP_PUSH_ID_LOCAL_ARGUMENT);
        stream.writeUInt16(index);

        target->convertAssignableToByteCode(nullptr, stream);

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
        stream.writeUInt16(index);

        stream.writeOpCode(OP_JUMP_IF_NOT_NULL_UNDEFINED_KEEP_VALID);
        auto addrEnd = stream.writeReservedAddress();
        // 插入缺省值表达式的执行代码.
        defVal->convertToByteCode(stream);
        *addrEnd = stream.address();

        left->convertAssignableToByteCode(nullptr, stream);
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

    void convertToByteCode(ByteCodeStream &stream) override;

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
        if (cond) {
            cond->convertToByteCode(stream);
            stream.writeOpCode(OP_JUMP_IF_FALSE);
            addrLoopEnd = stream.writeReservedAddress();
        }

        stream.enterBreakContinueArea();
        stmt->convertToByteCode(stream);
        stream.leaveBreakContinueArea();

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
        stream.enterBreakContinueArea();
        stmt->convertToByteCode(stream);
        stream.leaveBreakContinueArea();

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

        stream.enterBreakContinueArea();
        stmt->convertToByteCode(stream);
        stream.leaveBreakContinueArea();

        stream.writeOpCode(OP_JUMP);
        stream.writeAddress(begin);

        *addrEnd = stream.address();
    }

    IJsNode                     *cond, *stmt;

};

class JsStmtBreak : public IJsNode {
public:
    JsStmtBreak() : IJsNode(NT_BREAK) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_JUMP);
        stream.writeBreakAddress();
    }

};

class JsStmtContinue : public IJsNode {
public:
    JsStmtContinue() : IJsNode(NT_CONTINUE) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_JUMP);
        stream.writeContinueAddress();
    }

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

class JsSwitchBranch : public JsNodes {
public:
    JsSwitchBranch(IJsNode *exprCase) : JsNodes(NT_SWITCH_BRANCH), exprCase(exprCase), addrBranch(nullptr) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        // expr 已经在 JsStmtSwitch 中调用，不在此调用

        JsNodes::convertToByteCode(stream);
    }

    // 如果为 null，则表示此分支为 default
    IJsNode                     *exprCase;

    // 此分支的 statement 的执行跳转地址
    VMAddress                   *addrBranch;

};

class JsStmtSwitch : public IJsNode {
public:
    JsStmtSwitch(ResourcePool *resPool, IJsNode *cond) : IJsNode(NT_SWITCH), resPool(resPool), cond(cond), defBranch(nullptr) { }

    virtual void convertToByteCode(ByteCodeStream &stream);

    void buildCaseJumps(VMRuntime *runtime, uint16_t poolIndex, SwitchJump *switchJump);

    void push(JsSwitchBranch *node) { branches.push_back(node); }

    vector<JsSwitchBranch *>    branches;

    ResourcePool                *resPool;
    IJsNode                     *cond;
    JsSwitchBranch              *defBranch;

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

#endif /* Statements_hpp */
