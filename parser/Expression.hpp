﻿//
//  Expression.hpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/14.
//

#ifndef Expression_hpp
#define Expression_hpp

#include "parser/ParserTypes.hpp"


class JSParser;
class IJsNode;


class JsCommaExprs : public JsNodes {
public:
    JsCommaExprs(ResourcePool *resourcePool, JsNodeType type = NT_COMMA_EXPRESSION) : JsNodes(resourcePool, type) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!nodes.empty());

        uint16_t countPop = 0;
        uint32_t end = (uint32_t)nodes.size() - 1;

        for (int i = 0; i < end; i++) {
            auto item = nodes[i];
            if (item->type < _NT_CONST_EXPR_END) {
                continue;
            }

            countPop++;
            item->convertToByteCode(stream);

            if (countPop >= 1000) {
                // 及时清理堆栈
                stream.writeOpCode(OP_POP_STACK_TOP_N);
                stream.writeUInt16(countPop);
                countPop = 0;
            }
        }

        if (countPop == 1) {
            stream.writeOpCode(OP_POP_STACK_TOP);
        } else if (countPop > 1) {
            // 及时清理堆栈
            stream.writeOpCode(OP_POP_STACK_TOP_N);
            stream.writeUInt16(countPop);
        }

        nodes.back()->convertToByteCode(stream);
    }

};

class JsParenExpr : public JsCommaExprs {
public:
    JsParenExpr(ResourcePool *resourcePool) : JsCommaExprs(resourcePool, NT_PAREN_EXPRESSION) { }

};

class JsExprBoolTrue : public IJsNode {
public:
    JsExprBoolTrue() : IJsNode(NT_BOOLEAN_TRUE) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_TRUE);
    }

};

class JsExprBoolFalse : public IJsNode {
public:
    JsExprBoolFalse() : IJsNode(NT_BOOLEAN_FALSE) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_FALSE);
    }

};

class JsExprNull : public IJsNode {
public:
    JsExprNull() : IJsNode(NT_NULL) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_NULL);
    }

};

class JsExprString : public IJsNode {
public:
    JsExprString(uint32_t stringIdx) : IJsNode(NT_STRING), stringIdx(stringIdx) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_STRING);
        stream.writeUInt32(stringIdx);
    }

    uint32_t                    stringIdx;

};

class JsExprChar : public IJsNode {
public:
    JsExprChar(uint32_t ch) : IJsNode(NT_CHAR), ch(ch) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_CHAR);
        stream.writeUInt16(ch);
    }

    uint32_t                    ch;

};

class JsExprRegExp : public IJsNode {
public:
    JsExprRegExp(uint32_t regexIdx) : IJsNode(NT_REGEXP), regexIdx(regexIdx) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_REGEXP);
        stream.writeUInt32(regexIdx);
    }

protected:
    uint32_t                    regexIdx;

};

class JsExprInt32 : public IJsNode {
public:
    JsExprInt32(int32_t value) : IJsNode(NT_INT32), value(value) {
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_INT32);
        stream.writeUInt32(value);
    }

    int32_t                     value;

};

class JsExprNumber : public IJsNode {
public:
    JsExprNumber(uint32_t index) : IJsNode(NT_NUMBER), index(index) {
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_DOUBLE);
        stream.writeUInt32(index);
    }

    uint32_t                    index;

};

/**
 * 代表 Identifier
 */
class JsExprIdentifier : public IJsNode {
public:
    JsExprIdentifier(const Token &name, Scope *scope) : IJsNode(NT_IDENTIFIER), name(tokenToStringView(name)), scope(scope) {
        nameStringIdx = -1;
        isModified = false;
        isUsedNotAsFunctionCall = true;
        declare = nullptr;
        next = nullptr;
        noAssignAndRef = false;
    }

    // 提供给 JsExprAssignX 使用，临时修改此标志
    void setNotBeingAssigned() {
        isBeingAssigned = false;
    }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
    }

    void writeAddress(ByteCodeStream &stream) {
        stream.writeUInt8(declare->varStorageType);
        stream.writeUInt8(declare->scope->depth);
        stream.writeUInt16(declare->storageIndex);
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        if (noAssignAndRef) {
            // 未被赋值或者引用
            return;
        }

        assert(!isBeingAssigned);

        if (nameStringIdx != -1) {
            stream.writeOpCode(OP_PUSH_ID_BY_NAME);
            stream.writeUInt32(nameStringIdx);
            return;
        }

        // 将 identifier 压栈，根据不同的类型，优化使用不同的指令
        if (declare->varStorageType == VST_ARGUMENT) {
            if (declare->scope->function == scope->function) {
                stream.writeOpCode(OP_PUSH_ID_LOCAL_ARGUMENT);
            } else {
                stream.writeOpCode(OP_PUSH_ID_PARENT_ARGUMENT);
                stream.writeUInt8(declare->scope->depth);
                assert(declare->scope->isFunctionScope);
            }
            stream.writeUInt16(declare->storageIndex);
        } else if (declare->varStorageType == VST_SCOPE_VAR || declare->varStorageType == VST_FUNCTION_VAR) {
            if (scope == declare->scope) {
                stream.writeOpCode(OP_PUSH_ID_LOCAL_SCOPE);
            } else {
                if (declare->scope->depth == 0) {
                    stream.writeOpCode(OP_PUSH_ID_GLOBAL);
                } else {
                    stream.writeOpCode(OP_PUSH_ID_PARENT_SCOPE);
                    stream.writeUInt8(declare->scope->depth);
                }
            }
            stream.writeUInt16(declare->storageIndex);
        } else if (declare->varStorageType == VST_GLOBAL_VAR) {
            stream.writeOpCode(OP_PUSH_ID_GLOBAL);
            stream.writeUInt16(declare->storageIndex);
        } else {
            assert(declare->varStorageType == VST_NOT_SET);
            assert(declare->isFuncName);
            if (declare->isFuncName) {
                if (scope == declare->scope) {
                    stream.writeOpCode(OP_PUSH_ID_LOCAL_FUNCTION);
                } else {
                    stream.writeOpCode(OP_PUSH_ID_PARENT_FUNCTION);
                    stream.writeUInt8(declare->scope->depth);
                }
            }
            stream.writeUInt16(declare->storageIndex);
        }
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);
        if (valueOpt) {
            valueOpt->convertToByteCode(stream);
        }

        stream.writeOpCode(OP_ASSIGN_IDENTIFIER);
        writeAddress(stream);
    }

    StringView             name;

    // 当在有 eval/with 的 scope 时才有效
    uint32_t                nameStringIdx;

    // 是否被修改了
    bool                    isModified;
    bool                    isUsedNotAsFunctionCall;

    bool                    noAssignAndRef;

    IdentifierDeclare       *declare;

    // 此变量所在的 scope
    Scope                   *scope;

    // 所有被引用到的 Identifier 会通过 next 连起来，方便后期分析遍历
    JsExprIdentifier        *next;

};

class JsExprMemberIndex : public IJsNode {
public:
    JsExprMemberIndex(IJsNode *obj, IJsNode *index) : IJsNode(NT_MEMBER_INDEX), obj(obj), index(index) { }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!isBeingAssigned);
        obj->convertToByteCode(stream);
        index->convertToByteCode(stream);
        stream.writeOpCode(OP_PUSH_MEMBER_INDEX);
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);

        obj->convertToByteCode(stream);
        index->convertToByteCode(stream);

        if (valueOpt) {
            valueOpt->convertToByteCode(stream);
            stream.writeUInt8( OP_ASSIGN_MEMBER_INDEX);
        } else {
            stream.writeUInt8( OP_ASSIGN_VALUE_AHEAD_MEMBER_INDEX);
        }
    }

    IJsNode                     *obj;
    IJsNode                     *index;

};

class JsExprMemberDot : public IJsNode {
public:
    JsExprMemberDot(IJsNode *obj, uint32_t stringIdx, bool isOptional = false) : IJsNode(NT_MEMBER_DOT), obj(obj), stringIdx(stringIdx), isOptional(isOptional) { }

    virtual void setBeingAssigned() {
        if (isOptional) {
            throw ParseException(JE_SYNTAX_ERROR, "Invalid left-hand side in assignment");
        }
        isBeingAssigned = true;
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!isBeingAssigned);
        obj->convertToByteCode(stream);
        stream.writeOpCode(isOptional ? OP_PUSH_MEMBER_DOT_OPTIONAL : OP_PUSH_MEMBER_DOT);
        stream.writeUInt32(stringIdx);
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(!isOptional);
        assert(isBeingAssigned);

        obj->convertToByteCode(stream);

        if (valueOpt) {
            valueOpt->convertToByteCode(stream);
            stream.writeUInt8( OP_ASSIGN_MEMBER_DOT);
        } else {
            stream.writeUInt8( OP_ASSIGN_VALUE_AHEAD_MEMBER_DOT);
        }

        stream.writeUInt32(stringIdx);
    }

    IJsNode                     *obj;
    uint32_t                    stringIdx;
    bool                        isOptional;

};

class JsExprDelete : public IJsNode {
public:
    JsExprDelete(IJsNode *expr) : IJsNode(NT_DELETE), expr(expr) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        if (expr->type == NT_MEMBER_DOT) {
            auto target = (JsExprMemberDot *)expr;
            target->obj->convertToByteCode(stream);
            stream.writeOpCode(OP_DELETE_MEMBER_DOT);
            stream.writeUInt32(target->stringIdx);
        } else if (expr->type == NT_MEMBER_INDEX) {
            auto target = (JsExprMemberIndex *)expr;
            target->obj->convertToByteCode(stream);
            target->index->convertToByteCode(stream);
            stream.writeOpCode(OP_DELETE_MEMBER_INDEX);
        } else if (expr->type == NT_IDENTIFIER) {
            auto id = (JsExprIdentifier *)expr;
            if (id->nameStringIdx != -1) {
                stream.writeOpCode(OP_DELETE_ID_BY_NAME);
                stream.writeUInt32(id->nameStringIdx);
            } else {
                auto declare = id->declare;
                if (declare->varStorageType == VST_GLOBAL_VAR) {
                    stream.writeOpCode(OP_DELETE_ID_GLOBAL);
                    stream.writeUInt16(declare->storageIndex);
                } else {
                    stream.writeOpCode(OP_PUSH_TRUE);
                }
            }
        } else {
            expr->convertToByteCode(stream);
            stream.writeOpCode(OP_DELETE);
        }
    }

protected:
    IJsNode                     *expr;

};

class JsExprUnaryPrefix : public IJsNode {
public:
    JsExprUnaryPrefix(IJsNode *expr, OpCode code) : IJsNode(NT_UNARY_PREFIX), expr(expr), code(code) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        expr->convertToByteCode(stream);
        stream.writeOpCode(code);
    }

protected:
    IJsNode                     *expr;
    OpCode                      code;

};

class JsExprPrefixXCrease : public IJsNode {
public:
    JsExprPrefixXCrease(IJsNode *expr, bool increase) : IJsNode(NT_PREFIX_XCREASE), expr(expr), increase(increase) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        if (expr->type == NT_MEMBER_DOT) {
            auto dst = (JsExprMemberDot *)expr;
            dst->obj->convertToByteCode(stream);
            stream.writeOpCode(increase ? OP_INCREMENT_MEMBER_DOT_PRE : OP_DECREMENT_MEMBER_DOT_PRE);
            stream.writeUInt32(dst->stringIdx);
        } else if (expr->type == NT_MEMBER_INDEX) {
            auto dst = (JsExprMemberIndex *)expr;
            dst->obj->convertToByteCode(stream);
            dst->index->convertToByteCode(stream);

            stream.writeOpCode(increase ? OP_INCREMENT_MEMBER_INDEX_PRE : OP_DECREMENT_MEMBER_INDEX_PRE);
        } else {
            assert(expr->type == NT_IDENTIFIER);
            stream.writeOpCode(increase ? OP_INCREMENT_ID_PRE : OP_DECREMENT_ID_PRE);
            auto id = (JsExprIdentifier *)expr;
            id->writeAddress(stream);
        }
    }

protected:
    IJsNode                     *expr;
    bool                        increase;

};

class JsExprPostfix : public IJsNode {
public:
    JsExprPostfix(IJsNode *expr, bool increase) : IJsNode(NT_POSTFIX), expr(expr), increase(increase) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        if (expr->type == NT_MEMBER_DOT) {
            auto dst = (JsExprMemberDot *)expr;
            dst->obj->convertToByteCode(stream);
            stream.writeOpCode(increase ? OP_INCREMENT_MEMBER_DOT_POST : OP_DECREMENT_MEMBER_DOT_POST);
            stream.writeUInt32(dst->stringIdx);
        } else if (expr->type == NT_MEMBER_INDEX) {
            auto dst = (JsExprMemberIndex *)expr;
            dst->obj->convertToByteCode(stream);
            dst->index->convertToByteCode(stream);

            stream.writeOpCode(increase ? OP_INCREMENT_MEMBER_INDEX_POST : OP_DECREMENT_MEMBER_INDEX_POST);
        } else {
            assert(expr->type == NT_IDENTIFIER);
            stream.writeOpCode(increase ? OP_INCREMENT_ID_POST : OP_DECREMENT_ID_POST);
            auto id = (JsExprIdentifier *)expr;
            id->writeAddress(stream);
        }
    }

protected:
    IJsNode                     *expr;
    bool                        increase;

};

class JsExprFunctionCall : public IJsNode {
public:
    JsExprFunctionCall(ResourcePool *resourcePool, IJsNode *func, JsNodeType type = NT_FUNCTION_CALL) : IJsNode(type), func(func) {
        resourcePool->needDestructJsNode(this);

        if (func->type == NT_IDENTIFIER) {
            auto id = (JsExprIdentifier *)func;
            id->isUsedNotAsFunctionCall = false;
        }
    }

    virtual void pushArgs(ByteCodeStream &stream) {
        for (auto arg : args) {
            arg->convertToByteCode(stream);
        }
    }

    void functionCall(JsExprIdentifier *functionName, ByteCodeStream &stream) {
        if (functionName->nameStringIdx != -1) {
            stream.writeOpCode(OP_PUSH_ID_BY_NAME);
            stream.writeUInt32(functionName->nameStringIdx);

            pushArgs(stream);

            stream.writeOpCode(OP_FUNCTION_CALL);
            stream.writeUInt16((uint16_t)args.size());
            return;
        }

        bool isDirectFunctionCall = false;
        auto declare = functionName->declare;

        if (declare->varStorageType == VST_ARGUMENT) {
            if (declare->scope->function == functionName->scope->function) {
                stream.writeOpCode(OP_PUSH_ID_LOCAL_ARGUMENT);
            } else {
                stream.writeOpCode(OP_PUSH_ID_PARENT_ARGUMENT);
                stream.writeUInt8(declare->scope->depth);
                assert(declare->scope->isFunctionScope);
            }
            stream.writeUInt16(declare->storageIndex);
        } else if (!declare->isModified && declare->isFuncName) {
            // 调用的函数未被修改，并且是函数名，减少创建函数对象和压栈的操作.
            isDirectFunctionCall = true;
        } else {
            // stream.writeOpCode(OP_PUSH_IDENTIFIER);
            if (declare->scope->parent == nullptr || declare->varStorageType == VST_GLOBAL_VAR) {
                stream.writeOpCode(OP_PUSH_ID_GLOBAL);
            } else if (functionName->scope == declare->scope) {
                stream.writeOpCode(OP_PUSH_ID_LOCAL_SCOPE);
            } else {
                stream.writeOpCode(OP_PUSH_ID_PARENT_SCOPE);
                stream.writeUInt8(declare->scope->depth);
            }
            stream.writeUInt16(declare->storageIndex);
        }

        pushArgs(stream);

        if (isDirectFunctionCall) {
            auto function = functionName->declare->value.function;
            auto parent = function->scope->parent->function;

            stream.writeOpCode(OP_DIRECT_FUNCTION_CALL);
            stream.writeUInt8(parent->scope->depth);
            stream.writeUInt16(function->index);
        } else {
            stream.writeOpCode(OP_FUNCTION_CALL);
        }

        stream.writeUInt16((uint16_t)args.size());
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        switch (func->type) {
            case NT_IDENTIFIER: {
                // 函数调用
                functionCall((JsExprIdentifier *)func, stream);
                break;
            }
            case NT_MEMBER_INDEX: {
                auto e = (JsExprMemberIndex *)func;
                e->obj->convertToByteCode(stream);
                e->index->convertToByteCode(stream);

                stream.writeOpCode(OP_PUSH_THIS_MEMBER_INDEX);

                pushArgs(stream);

                stream.writeOpCode(OP_MEMBER_FUNCTION_CALL);
                stream.writeUInt16((uint16_t)args.size());
                break;
            }
            case NT_MEMBER_DOT: {
                auto e = (JsExprMemberDot *)func;
                e->obj->convertToByteCode(stream);

                if (e->isOptional) {
                    stream.writeOpCode(OP_PUSH_THIS_MEMBER_DOT_OPTIONAL);
                } else {
                    stream.writeOpCode(OP_PUSH_THIS_MEMBER_DOT);
                }
                stream.writeUInt32(e->stringIdx);

                pushArgs(stream);

                stream.writeOpCode(OP_MEMBER_FUNCTION_CALL);
                stream.writeUInt16((uint16_t)args.size());
                break;
            }
            default:
                func->convertToByteCode(stream);
                pushArgs(stream);

                stream.writeOpCode(OP_FUNCTION_CALL);
                stream.writeUInt16((uint16_t)args.size());
                break;
        }
    }

    IJsNode                     *func;
    VecJsNodes                  args;

};

/**
 * Template 函数的调用和普通函数的区别在于第一个参数是字符串模板变量.
 */
class JsExprPrepareTemplate : public IJsNode {
public:
    JsExprPrepareTemplate(uint32_t rawStringIdx) : IJsNode(NT_PREPARE_TEMPLATE) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PREPARE_RAW_STRING_TEMPLATE_CALL);
        stream.writeUInt16(rawStringIdx);
    }

protected:

    uint32_t                    rawStringIdx;

};

class JsExprTemplateFunctionCall : public JsExprFunctionCall {
public:
    JsExprTemplateFunctionCall(ResourcePool *resourcePool, IJsNode *func, uint32_t rawStringIdx) : JsExprFunctionCall(resourcePool, func, NT_TEMPLATE_FUNCTION_CALL), parepareTemplate(rawStringIdx) {

        args.push_back(&parepareTemplate);
    }

protected:
    JsExprPrepareTemplate       parepareTemplate;

};

class JsExprNew : public IJsNode {
public:
    JsExprNew(ResourcePool *resourcePool, IJsNode *obj) : IJsNode(NT_NEW), obj(obj) {
        resourcePool->needDestructJsNode(this);
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        obj->convertToByteCode(stream);

        for (auto item : args) {
            item->convertToByteCode(stream);
        }

        stream.writeOpCode(OP_NEW);
        stream.writeUInt16((uint16_t)args.size());
        assert(args.size() < 0xffff);
    }

    VecJsNodes                  args;

protected:
    IJsNode                     *obj;

};

/**
 * Function 解析
 */
class JsFunctionExpr : public IJsNode {
public:
    JsFunctionExpr(Function *func) : IJsNode(NT_FUNCTION_EXPR), function(func) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_PUSH_FUNCTION_EXPR);
        stream.writeUInt8(function->scope->parent->function->scope->depth);
        stream.writeUInt16((uint16_t)function->index);
    }

    Function *getFunction() { return function; }

protected:
    Function                    *function;

};

class JsExprArrayItem : public IJsNode {
public:
    JsExprArrayItem(IJsNode *expr) : IJsNode(NT_ARRAY_ITEM), expr(expr) {
    }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
        expr->setBeingAssigned();
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!isBeingAssigned);

        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_ARRAY_PUSH_VALUE);
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream);

protected:
    IJsNode                         *expr;

};

class JsExprArrayItemSpread : public IJsNode {
public:
    JsExprArrayItemSpread(IJsNode *expr) : IJsNode(NT_ARRAY_ITEM_SPREAD), expr(expr) {
    }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
        expr->setBeingAssigned();
        if (expr->type == NT_ASSIGN) {
            throw ParseException(JE_SYNTAX_ERROR, "Invalid destructuring assignment target");
        }
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!isBeingAssigned);
        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_ARRAY_SPREAD_VALUE);
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);
        stream.writeOpCode(OP_ARRAY_ASSIGN_REST_VALUE);
    }

protected:
    IJsNode                         *expr;

};

class JsExprArrayItemEmpty : public IJsNode {
public:
    JsExprArrayItemEmpty() : IJsNode(NT_ARRAY_ITEM_EMPTY) {
    }

    virtual void setBeingAssigned() {
        // Array 定义中两个 “,,” 的项，掠过
        isBeingAssigned = true;
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!isBeingAssigned);
        stream.writeOpCode(OP_ARRAY_PUSH_EMPTY_VALUE);
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);
        // 仅仅占位
    }

};

/**
 * 代表 Array 类型的表达式，综合了 ArrayExpression 和 ArrayAssinable
 */
class JsExprArray : public JsNodes {
public:
    JsExprArray(ResourcePool *resourcePool) : JsNodes(resourcePool, NT_ARRAY) {
    }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;

        for (auto it = nodes.begin(); it != nodes.end(); ++it) {
            auto item = *it;
            if (item->type == NT_ARRAY_ITEM_SPREAD) {
                if (it + 1 != nodes.end()) {
                    throw ParseException(JE_SYNTAX_ERROR, "Rest element must be last element");
                }
            }
            item->setBeingAssigned();
        }
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!isBeingAssigned);
        // Array 表达式
        stream.writeOpCode(OP_ARRAY_CREATE);

        for (auto item : nodes) {
            item->convertToByteCode(stream);
        }
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);

        // Array Assignable 赋值
        auto size = nodes.size();
        for (int i = 0; i < size; i++) {
            auto item = nodes[i];
            // item->initFromStackTop = true;

            // 从 initExpr 获取第 index 个元素，压入堆栈，用于赋值
            stream.writeOpCode(OP_PUSH_THIS_MEMBER_INDEX_INT);
            stream.writeUInt32(i);

            item->convertAssignableToByteCode(nullptr, stream);

            // 弹出压入的 OP_PUSH_THIS_MEMBER_INDEX_INT
            stream.writeOpCode(OP_POP_STACK_TOP);
        }
    }

};

class JsObjectProperty : public IJsNode {
public:
    JsObjectProperty(uint32_t nameIdx, IJsNode *expr) : IJsNode(NT_OBJECT_PROPERTY), nameIdx(nameIdx), expr(expr) { }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
        expr->setBeingAssigned();
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!isBeingAssigned);
        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_OBJ_SET_PROPERTY);
        stream.writeUInt32(nameIdx);
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);
        assert(0 && "TBD");
    }

protected:
    IJsNode                     *expr;
    uint32_t                    nameIdx;

};


/**
 * JsObjectPropertyShortInit 表示 { x=y } 中的 x=y 部分.
 */
class JsObjectPropertyShortInit : public IJsNode {
public:
    JsObjectPropertyShortInit(uint32_t nameIdx, IJsNode *id, IJsNode *defVal) : IJsNode(NT_OBJECT_PROPERTY), nameIdx(nameIdx), id(id), defVal(defVal) { }

    virtual bool canBeExpression() {
        // 不能作为表达式的
        return false;
    }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(0 && "NOT a valid js grammer.");
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);

        stream.writeOpCode(OP_PUSH_THIS_MEMBER_DOT);
        stream.writeUInt32(nameIdx);

        stream.writeOpCode(OP_JUMP_IF_NOT_NULL_UNDEFINED_KEEP_VALID);
        auto addrUseStack = stream.writeReservedAddress();

        // 使用缺省值
        defVal->convertToByteCode(stream);

        *addrUseStack = stream.address();
        assert(id->type == NT_IDENTIFIER);
        id->convertToByteCode(stream);
    }

protected:
    IJsNode                     *id, *defVal;
    uint32_t                    nameIdx;

};

class JsObjectPropertyGetter : public IJsNode {
public:
    JsObjectPropertyGetter(uint32_t nameIdx, IJsNode *expr) : IJsNode(NT_OBJECT_PROPERTY), nameIdx(nameIdx), expr(expr) { }

    virtual void setBeingAssigned() {
        throw ParseException(JE_SYNTAX_ERROR, "Invalid destructuring assignment target");
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_OBJ_SET_GETTER);
        stream.writeUInt32(nameIdx);
    }

protected:
    IJsNode                     *expr;
    uint32_t                    nameIdx;

};

class JsObjectPropertySetter : public IJsNode {
public:
    JsObjectPropertySetter(uint32_t nameIdx, IJsNode *expr) : IJsNode(NT_OBJECT_PROPERTY_SETTER), nameIdx(nameIdx), expr(expr) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_OBJ_SET_SETTER);
        stream.writeUInt32(nameIdx);
    }

    virtual void setBeingAssigned() {
        throw ParseException(JE_SYNTAX_ERROR, "Invalid destructuring assignment target");
    }

protected:
    IJsNode                     *expr;
    uint32_t                    nameIdx;

};

class JsObjectPropertyComputed : public IJsNode {
public:
    JsObjectPropertyComputed(IJsNode *name, IJsNode *value) : IJsNode(NT_OBJECT_PROPERTY_COMPUTED), name(name), value(value) { }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
        value->setBeingAssigned();
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!isBeingAssigned);
        name->convertToByteCode(stream);
        value->convertToByteCode(stream);
        stream.writeOpCode(OP_OBJ_SET_COMPUTED_PROPERTY);
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);

        name->convertToByteCode(stream);
        assert(0);
        stream.writeOpCode(OP_PUSH_THIS_MEMBER_INDEX);
        value->convertToByteCode(stream);
    }

protected:
    IJsNode                     *name, *value;

};

class JsObjectPropertySpread : public IJsNode {
public:
    JsObjectPropertySpread(IJsNode *expr) : IJsNode(NT_OBJECT_PROPERTY_SPREAD), expr(expr) { }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
        expr->setBeingAssigned();
        if (expr->type == NT_ASSIGN) {
            throw ParseException(JE_SYNTAX_ERROR, "`...` must be followed by an assignable reference in assignment contexts");
        }
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!isBeingAssigned);
        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_OBJ_SPREAD_PROPERTY);
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);
        assert(0 && "TBD");
    }

protected:
    IJsNode                     *expr;

};

/**
 * 代表 Object 类型的表达式，综合了 ObjectExpression 和 ObjectAssinable
 */
class JsExprObject : public JsNodes {
public:
    JsExprObject(ResourcePool *resourcePool) : JsNodes(resourcePool, NT_OBJECT) { }

    void checkCanBeExpression() {
        if (!isBeingAssigned && !canBeExpression()) {
            throw ParseException(JE_SYNTAX_ERROR, "Invalid shorthand property initializer");
        }
    }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;

        for (auto it = nodes.begin(); it != nodes.end(); ++it) {
            auto item = *it;
            if (item->type == NT_OBJECT_PROPERTY_SPREAD) {
                if (it + 1 != nodes.end()) {
                    throw ParseException(JE_SYNTAX_ERROR, "Rest element must be last element");
                }
            }
            item->setBeingAssigned();
        }
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        assert(!isBeingAssigned);
        // Object 表达式
        stream.writeOpCode(OP_OBJ_CREATE);

        for (auto item : nodes) {
            item->convertToByteCode(stream);
        }
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        // Object Assignable 赋值
        assert(isBeingAssigned);
        assert(0 && "TBD");
    }

};

/**
 * JsExprAssignWithStackTop 是表达式中的 left=defVal 部分。 [a, left=defVal] = value
 * 如果有从 stack 赋值，则会使用 stack 的值，否则使用 defVal 的值.
 */
class JsExprAssignWithStackTop : public IJsNode {
public:
    JsExprAssignWithStackTop(IJsNode *left, IJsNode *defVal = nullptr) : IJsNode(NT_ASSIGN_WITH_STACK_TOP), left(left), defVal(defVal) {
    }

    virtual void setBeingAssigned() {
        isBeingAssigned = true;
        left->setBeingAssigned();
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        // 两种方式都有可能会调用到.
        convertAssignableToByteCode(nullptr, stream);
    }

    virtual void convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
        assert(isBeingAssigned);
        if (defVal) {
            // 如果栈顶的值为 undefined，则使用缺省值，否则使用栈顶值
            stream.writeOpCode(OP_JUMP_IF_NOT_NULL_UNDEFINED_KEEP_VALID);
            auto addrEnd = stream.writeReservedAddress();

            // 插入缺省值表达式的执行代码.
            defVal->convertToByteCode(stream);

            *addrEnd = stream.address();
        }

        left->convertAssignableToByteCode(nullptr, stream);
    }

    IJsNode *defaultValue() { return defVal; }

public:
    IJsNode                         *left, *defVal;
    bool                            isAssignFromStack;

};

class JsExprAssign : public IJsNode {
public:
    JsExprAssign(IJsNode *left, IJsNode *right, bool isVarDeclaration = false) : IJsNode(NT_ASSIGN), left(left), right(right) {
        if (!isVarDeclaration && left->type == NT_IDENTIFIER) {
            ((JsExprIdentifier *)left)->isModified = true;
        }

        if (right->type == NT_FUNCTION_EXPR && left->type == NT_IDENTIFIER) {
            auto fe = (JsFunctionExpr *)right;
            if (fe->getFunction()->name.empty()) {
                // 给函数表达式赋值名字
                auto id = (JsExprIdentifier *)left;
                fe->getFunction()->name = id->name;
            }
        }

        left->setBeingAssigned();
    }

    virtual void setBeingAssigned() {
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        left->convertAssignableToByteCode(right, stream);
    }

public:
    IJsNode                         *left, *right;

};

class JsExprAssignX : public IJsNode {
public:
    JsExprAssignX(IJsNode *left, IJsNode *right, OpCode xopr) : IJsNode(NT_ASSIGN_X), left(left), right(right), xopr(xopr) {
        left->setBeingAssigned();
    }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        if (left->type == NT_MEMBER_DOT) {
            auto dst = (JsExprMemberDot *)left;
            dst->obj->convertToByteCode(stream);
            stream.writeOpCode(OP_PUSH_THIS_MEMBER_DOT);
            stream.writeUInt32(dst->stringIdx);

            right->convertToByteCode(stream);
            stream.writeOpCode(xopr);

            stream.writeOpCode(OP_ASSIGN_MEMBER_DOT);
            stream.writeUInt32(dst->stringIdx);
        } else if (left->type == NT_MEMBER_INDEX) {
            auto dst = (JsExprMemberIndex *)left;
            dst->obj->convertToByteCode(stream);
            dst->index->convertToByteCode(stream);

            // 不 pop object 和 index
            stream.writeOpCode(OP_PUSH_MEMBER_INDEX_NO_POP);

            right->convertToByteCode(stream);
            stream.writeOpCode(xopr);

            stream.writeOpCode(OP_ASSIGN_MEMBER_INDEX);
        } else {
            assert(left->type == NT_IDENTIFIER);
            auto id = (JsExprIdentifier *)left;
            id->setNotBeingAssigned();
            id->convertToByteCode(stream);
            id->setBeingAssigned();

            right->convertToByteCode(stream);
            stream.writeOpCode(xopr);

            left->convertAssignableToByteCode(nullptr, stream);
        }
    }

protected:
    IJsNode                         *left, *right;
    OpCode                          xopr;

};

class JsExprConditional : public IJsNode {
public:
    JsExprConditional(IJsNode *cond, IJsNode *exprTrue, IJsNode *exprFalse) : IJsNode(NT_CONDITIONAL), cond(cond), exprTrue(exprTrue), exprFalse(exprFalse) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        cond->convertToByteCode(stream);

        stream.writeOpCode(OP_JUMP_IF_FALSE);
        auto addrFalse = stream.writeReservedAddress();

        exprTrue->convertToByteCode(stream);
        stream.writeOpCode(OP_JUMP);
        auto addrEnd = stream.writeReservedAddress();

        *addrFalse = stream.address();
        exprFalse->convertToByteCode(stream);

        *addrEnd = stream.address();
    }

protected:
    IJsNode                         *cond, *exprTrue, *exprFalse;

};

class JsExprNullish : public IJsNode {
public:
    JsExprNullish(IJsNode *expr, IJsNode *exprIfNull) : IJsNode(NT_NULLISH), expr(expr), exprIfNull(exprIfNull) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        expr->convertToByteCode(stream);
        stream.writeOpCode(OP_JUMP_IF_NOT_NULL_UNDEFINED_KEEP_VALID);
        auto addrEnd = stream.writeReservedAddress();
        exprIfNull->convertToByteCode(stream);
        *addrEnd = stream.address();
    }

protected:
    IJsNode                         *expr, *exprIfNull;

};

class JsExprLogicalOr : public IJsNode {
public:
    JsExprLogicalOr(IJsNode *left, IJsNode *right) : IJsNode(NT_LOGICAL_OR), left(left), right(right) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        left->convertToByteCode(stream);
        stream.writeOpCode(OP_JUMP_IF_TRUE_KEEP_VALID);
        auto addrEnd = stream.writeReservedAddress();
        right->convertToByteCode(stream);
        *addrEnd = stream.address();
    }

protected:
    IJsNode                         *left, *right;

};

class JsExprLogicalAnd : public IJsNode {
public:
    JsExprLogicalAnd(IJsNode *left, IJsNode *right) : IJsNode(NT_LOGICAL_AND), left(left), right(right) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        left->convertToByteCode(stream);
        stream.writeOpCode(OP_JUMP_IF_FALSE_KEEP_COND);
        auto addrEnd = stream.writeReservedAddress();
        right->convertToByteCode(stream);
        *addrEnd = stream.address();
    }

protected:
    IJsNode                         *left, *right;

};

class JsExprBinaryOp : public IJsNode {
public:
    JsExprBinaryOp(IJsNode *left, IJsNode *right, OpCode code) : IJsNode(NT_BINARAY_OP), left(left), right(right), code(code) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        left->convertToByteCode(stream);
        right->convertToByteCode(stream);
        stream.writeOpCode(code);
    }

protected:
    IJsNode                         *left, *right;
    OpCode                          code;

};

class JsExprNewTarget : public IJsNode {
public:
    JsExprNewTarget() : IJsNode(NT_NEW_TARGET) { }

    virtual void convertToByteCode(ByteCodeStream &stream) {
        stream.writeOpCode(OP_NEW_TARGET);
    }

};

JsValue convertConstExprToJsValue(VMRuntime *rt, uint16_t poolIndex, IJsNode *node);

#endif /* Expression_hpp */
