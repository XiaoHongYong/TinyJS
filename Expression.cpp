//
//  Expression.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/8/14.
//

#include "Expression.hpp"
#include "Parser.hpp"



void JsExprArrayItem::convertAssignableToByteCode(IJsNode *valueOpt, ByteCodeStream &stream) {
    assert(isBeingAssigned);
    assert(valueOpt == nullptr);

    switch (expr->type) {
        case NT_IDENTIFIER:
        case NT_MEMBER_DOT:
        case NT_MEMBER_INDEX:
        case NT_ARRAY:
        case NT_OBJECT:
            // 都从堆栈顶上赋值
            expr->convertAssignableToByteCode(nullptr, stream);
            break;
        case NT_ASSIGN: {
            auto assign = (JsExprAssign *)expr;

            // 如果栈顶的值为 undefined，则使用缺省值，否则使用栈顶值
            stream.writeUint8(OP_JUMP_IF_NOT_NULL_UNDEFINED_KEEP_VALID);
            auto addrEnd = stream.writeReservedAddress();

            // 插入缺省值表达式的执行代码.
            assign->right->convertToByteCode(stream);

            *addrEnd = stream.address();

            assign->left->convertAssignableToByteCode(nullptr, stream);

            break;
        }
        default:
            assert(0 && "NOT supported for array assignable.");
            break;
    }
}
