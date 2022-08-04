//
//  Parser.cpp
//  TinyJS
//
//  Created by henry_xiao on 2022/5/22.
//

#include "Parser.hpp"
#include "VirtualMachine.hpp"


VarInitTree::~VarInitTree() {
    for (auto item : children) {
        delete item;
    }
}

VarInitTree *VarInitTree::newChild() {
    auto child = new VarInitTree();
    children.push_back(child);
    return child;
}

void VarInitTree::generateInitCode(Function *function, bool initExpr) {
    if (callback) {
        initExpr = callback(function, initExpr);
    }

    for (auto child : children) {
        child->callback(function, initExpr);
    }

    if (needPopStackTop) {
        function->instructions.writeOpCode(OP_POP_STACK_TOP);
    }
}

JSParser::JSParser(VMRuntimeCommon *rtc, ResourcePool *resPool, const char *buf, size_t len) : JSLexer(resPool, buf, len) {
    _runtimeCommon = rtc;
    if (rtc) {
        _nextStringIdx = (uint32_t)rtc->stringValues.size();
        _nextDoubleIdx = (uint32_t)rtc->doubleValues.size();
    } else {
        _nextStringIdx = 0;
        _nextDoubleIdx = 0;
    }

    _headIdRefs = nullptr;
    _curFunction = nullptr;
    _curFuncScope = nullptr;
    _curScope = nullptr;
}

Function *JSParser::parse(Scope *parent, bool isExpr) {
    _headIdRefs = nullptr;

    if (parent) {
        _curScope = parent;
        _curFuncScope = parent->function->scope;
        _curFunction = parent->function;
    } else {
        _curFunction = nullptr;
        _curFuncScope = nullptr;
        _curScope = nullptr;
    }

    _readToken();

    auto function = _enterFunction(_curToken);

    if (isExpr) {
        _expectExpression(function->instructions, PRED_NONE);
        function->instructions.writeOpCode(OP_RETURN_VALUE);
    } else {
        while (_curToken.type != TK_EOF && _error == PE_OK) {
            _expectStatment();
        }
    }

    _leaveFunction();

    // 先简单优化一下 scope 的层次, 在后面查找标识符的时候会快点
    _reduceScopeLevels(function);

    // 建立标识符的引用、作用域关系
    _buildIdentifierRefs();

    // 分析标识符地址
    _allocateIdentifierStorage(function->scope, 0);

    return function;
}

void JSParser::_expectStatment() {
    auto &instructions = _curFunction->instructions;
    switch (_curToken.type) {
        case TK_STRING: {
            _peekToken();
            if (_nextToken.type == TK_SEMI_COLON || _nextToken.newLineBefore || _nextToken.type == TK_CLOSE_BRACE) {
                // 是一个字符串 statement, 需判断是不是指令?
                if (tokenToSizedString(_curToken).equal("use strict")) {
                    _curFunction->isStrictMode = true;
                } else {
                    // 忽略其他的字符串语句.
                }
                _readToken();
                _expectSemiColon();
            } else {
                _expectExpressionStmt();
            }
            break;
        }
        case TK_NAME: {
            _peekToken();
            if (_nextToken.type == TK_COLON) {
                _expectLabelStmt();
            } else {
                _expectExpressionStmt();
            }
            break;
        }
        case TK_OPEN_BRACE:
            _expectBlock();
            break;
        case TK_SEMI_COLON:
            // Empty Statement
            _readToken();
            break;

        case TK_BREAK:
            _readToken();
            _expectBreakContinue(TK_BREAK);
            break;

        case TK_CONTINUE:
            _readToken();
            _expectBreakContinue(TK_CONTINUE);
            break;

        case TK_DEBUGGER: {
            _readToken();
            _expectSemiColon();
            instructions.writeOpCode(OP_DEBUGGER);
            break;
        }

        case TK_DO: {
            _readToken();
            auto begin = instructions.tagLabel();
            _expectStatment();

            _expectToken(TK_WHILE);
            _expectParenCondition();
            instructions.writeOpCode(OP_JUMP_IF_TRUE);
            instructions.writeAddress(begin);
            _expectSemiColon();
            break;
        }
        case TK_WHILE: {
            _readToken();
            auto begin = instructions.tagLabel();

            _expectParenCondition();
            instructions.writeOpCode(OP_JUMP_IF_FALSE);
            auto addrFalse = instructions.writeAddressFuture();

            _expectStatment();
            instructions.writeOpCode(OP_JUMP);
            instructions.writeAddress(begin);
            instructions.tagLabel(addrFalse);
            break;
        }
        case TK_FOR:
            _readToken();
            _expectForStatment();
            break;

        case TK_FUNCTION: {
            _expectFunction(instructions, FT_DECLARATION);
            break;
        }

        case TK_IF: {
            _expectParenCondition();
            instructions.writeOpCode(OP_JUMP_IF_FALSE);
            auto endAddr = instructions.writeAddressFuture();
            _expectStatment();
            instructions.tagLabel(endAddr);
            break;
        }

        case TK_RETURN: {
            _readToken();
            if (_curToken.type == TK_SEMI_COLON) {
                _readToken();
                instructions.writeOpCode(OP_RETURN);
            } else if (_canInsertSemicolon()) {
                instructions.writeOpCode(OP_RETURN);
            } else {
                _expectExpression(instructions, PRED_NONE, true);
                instructions.writeOpCode(OP_RETURN_VALUE);
                _expectSemiColon();
            }
            break;
        }

        case TK_SWITCH:
            _readToken();
            _expectSwitchStmt();
            break;

        case TK_THROW: {
            _readToken();
            if (_curToken.newLineBefore) {
                _parseError(PE_ILLEGAL_NEW_LINE_AFTER_THROW, "Illegal newline after throw");
            }
            _expectExpression(instructions, PRED_NONE, true);
            _expectSemiColon();
            instructions.writeOpCode(OP_THROW);
            break;
        }

        case TK_TRY:
            _readToken();
            _expectTryStmt();
            break;

        case TK_VAR:
        case TK_LET:
        case TK_CONST: {
            auto declareType = _curToken.type;
            _readToken();
            _expectVariableDeclarationList(declareType);
            _expectSemiColon();
            break;
        }

        case TK_WITH: {
            _readToken();
            _expectParenCondition();
            _enterScope();
            _curScope->hasWith = true;
            _expectStatment();
            _leaveScope();
            break;
        }

        default:
            _expectExpressionStmt();
            break;
    }
}

void JSParser::_expectExpressionStmt() {
    _expectExpression(_curFunction->instructions, PRED_NONE, true);
    _expectSemiColon();
}

void JSParser::_expectLabelStmt() {

}

void JSParser::_expectBlock() {
    _enterScope();

    _expectToken(TK_OPEN_BRACE);

    while (_curToken.type != TK_CLOSE_BRACE && _error == PE_OK) {
        if (_curToken.type == TK_EOF) {
            _parseError(PE_UNEXPECTED_END_OF_INPUT, "Unexpected end of input");
            break;
        }

        _expectStatment();
    }
    _readToken();

    _leaveScope();
}

void JSParser::_expectBreakContinue(TokenType type) {

}

void JSParser::_expectForStatment() {

}

void JSParser::_expectSwitchStmt() {

}

void JSParser::_expectTryStmt() {

}

/**
 * 匹配参数列表，返回参数的个数.
 */
int JSParser::_expectArgumentsList(InstructionOutputStream &instructions) {
    _readToken();

    int count = 0;
    while (_curToken.type != TK_CLOSE_PAREN && _error == PE_OK) {
        if (count != 0) {
            _expectToken(TK_COMMA);
            if (_curToken.type == TK_CLOSE_PAREN)
                break;
        }

        if (_curToken.type == TK_ELLIPSIS) {
            _readToken();
        }

        _expectExpression(instructions);
        count++;
    }

    _expectToken(TK_CLOSE_PAREN);

    return count;
}

void JSParser::_expectVariableDeclarationList(TokenType declareType) {
    VarInitTree varInitTree;

    while (_error == PE_OK) {
        _expectVariableDeclaration(declareType, &varInitTree);
        if (_curToken.type != TK_COMMA || _error != PE_OK) {
            break;
        }
        _readToken();
    }

    varInitTree.generateInitCode(_curFunction, nullptr);
}

/**
 * 解析变量声明
 * @varInitStack VarInitStack 变量的声明初始化指令回调函数。在解析阶段，需要先解析语法树，
 *      但是生成变量初始化代码时，需要从外层（后）往那初始化（前），所以通过 varInitStack
 *      来根据情况生成初始化的代码。
 */
void JSParser::_expectVariableDeclaration(TokenType declareType, VarInitTree *parentVarInitTree) {
    auto declaredToken = _curToken;
    auto child = parentVarInitTree->newChild();

    switch (_curToken.type) {
        case TK_NAME:
            if (declareType == TK_VAR) {
                _curFuncScope->addVarDeclaration(_curToken);
            } else {
                _curScope->addVarDeclaration(_curToken, declareType == TK_CONST);
            }
            _readToken();
            break;
        case TK_OPEN_BRACKET: {
            _readToken();
            _expectArrayAssignable(declareType, child);
            _expectToken(TK_CLOSE_BRACKET);
            break;
        }
        case TK_OPEN_BRACE: {
            _readToken();
            // _expectObjectAssignable(declareType, child);
            _expectToken(TK_CLOSE_BRACE);
            break;
        }
        default:
            assert(0);
            break;
    }

    if (_curToken.type == TK_ASSIGN) {
        _readToken();

        InstructionOutputStream *exprInstructions = _resPool->allocInstructionOutputStream();

        // 将表达式的指令保存起来
        _expectExpression(*exprInstructions, PRED_NONE, false);

        child->setCallback([this, child, declaredToken, exprInstructions](Function *function, bool initExpr) {
            auto &instructions = function->instructions;
            if (initExpr) {
                instructions.writeOpCode(OP_JUMP_IF_UNDEFINED);
                auto labelUseDefault = instructions.writeAddressFuture();
                if (declaredToken.type == TK_NAME) {
                    _assignIdentifier(function, instructions, declaredToken);
                }
                instructions.writeOpCode(OP_JUMP);
                auto end = instructions.writeAddressFuture();

                instructions.tagLabel(labelUseDefault);

                // 插入缺省值表达式的执行代码.
                instructions.writeBranch(exprInstructions);
                if (declaredToken.type == TK_NAME) {
                    _assignIdentifier(function, instructions, declaredToken);
                } else {
                    child->setNeedPopStackTop();
                }

                instructions.tagLabel(end);
            } else {
                // 插入缺省值表达式的执行代码.
                instructions.writeBranch(exprInstructions);
                if (declaredToken.type == TK_NAME) {
                    _assignIdentifier(function, instructions, declaredToken);
                } else {
                    child->setNeedPopStackTop();
                }
            }

            return true;
        });
    } else {
        if (declareType == TK_CONST) {
            _parseError(PE_SYNTAX_ERROR, "Missing initializer in const declaration");
            return;
        }

        child->setCallback([this, child, declaredToken](Function *function, bool initExpr) {
            if (initExpr) {
                if (declaredToken.type == TK_NAME) {
                    _assignIdentifier(function, function->instructions, declaredToken);
                } else {
                    child->setNeedPopStackTop();
                }
            }

            return initExpr;
        });
    }
}

void JSParser::_expectArrayAssignable(TokenType declareType, VarInitTree *parentVarInitTree) {
    int index = 0;
    while (_error == PE_OK) {
        if (_curToken.type == TK_COMMA) {
            // 可以有多个 ',' 连着
            _readToken();
            index++;
        }

        if (_curToken.type == TK_CLOSE_BRACKET) {
            // 提前结束
            _readToken();
        }

        VarInitTree *child = parentVarInitTree->newChild();
        child->setCallback([this, index](Function *function, bool initExpr) {
            if (initExpr) {
                // 从 initExpr 获取第 index 个元素
                function->instructions.writeOpCode(OP_PUSH_STACK_TOP); // Duplicate stack top
                function->instructions.writeOpCode(OP_PUSH_MEMBER_INDEX_INT);
                function->instructions.writeUint32(index);
            }

            return initExpr;
        });

        _expectVariableDeclaration(declareType, child);

        index++;
    }
}

void JSParser::_expectToken(TokenType expected) {
    if (_curToken.type != expected) {
        _parseError(PE_UNEXPECTED_TOKEN, "Unexpected token: %d, expected: %d.", _curToken.type, expected);
    } else {
        _readToken();
    }
}

void JSParser::_expectSemiColon() {
    if (_curToken.type == TK_SEMI_COLON) {
        _readToken();
    } else if (!_canInsertSemicolon()) {
        _parseError(PE_EXPECTED_SEMI_COLON, "SemiColon is expected, unexpected token: %d", _curToken.type);
    }
}

void JSParser::_assignIdentifier(Function *function, InstructionOutputStream &instructions, const Token &name) {
    if (tokenToSizedString(name).equal(SS_THIS)) {
        _parseError(PE_SYNTAX_ERROR, "Unexpected token 'this'");
        return;
    }
    
    auto identifier = _newIdentifierRef(name, function);
    identifier->isModified = true;
    instructions.writeOpCode(OP_ASSIGN_IDENTIFIER);
    instructions.writeIdentifierAddress(identifier);
}

void JSParser::_assignParameter(Function *function, InstructionOutputStream &instructions, int index) {
    instructions.writeOpCode(OP_ASSIGN_LOCAL_ARGUMENT);
    instructions.writeUint16((uint16_t)index);
}

Function *JSParser::_expectFunction(InstructionOutputStream &instructions, FunctionType type, bool ignoreFirstToken) {
    auto parentScope = _curScope;
    auto child = _enterFunction(_curToken);

    if (ignoreFirstToken) {
        _readToken();
    }

    if (type != FT_GETTER && type != FT_SETTER && _curToken.type == TK_MUL) {
        _readToken();
        child->isGenerator = true;
    }

    if (type == FT_DECLARATION || type == FT_EXPRESSION) {
        if (_curToken.type == TK_NAME) {
            child->name = tokenToSizedString(_curToken);
            if (type == FT_DECLARATION) {
                parentScope->addFunctionDeclaration(_curToken, child);
            } else if (type == FT_EXPRESSION) {
                // TODO: 函数表达式的名字在函数的作用域内
                // child->addVarDeclaration(_curToken);
            }
            _readToken();
        } else if (type == FT_DECLARATION) {
            _expectToken(TK_NAME);
            return child;
        }
    }

    if (type == FT_MEMBER_FUNCTION) {
        child->isMemberFunction = true;
    }

    int count = _expectFormalParameters();
    if (type == FT_GETTER && count > 0) {
        _parseError(PE_SYNTAX_ERROR, "Getter must not have any formal parameters.");
        return child;
    } else if (type == FT_GETTER && count != 1) {
        _parseError(PE_SYNTAX_ERROR, "Setter must have exactly one formal parameter.");
        return child;
    }

    // function body
    _expectToken(TK_OPEN_BRACE);
    while (_curToken.type != TK_CLOSE_BRACE && _error == PE_OK) {
        if (_curToken.type == TK_EOF) {
            _parseError(PE_UNEXPECTED_END_OF_INPUT, "Unexpected end of input");
            break;
        }

        _expectStatment();
    }
    _readToken();

    _leaveFunction();

    if (type != FT_DECLARATION) {
        instructions.writeOpCode(OP_PUSH_FUNCTION_EXPR);
        instructions.writeUint8(_curFunction->scope->depth);
        instructions.writeUint16(child->index);
    }

    return child;
}

int JSParser::_expectFormalParameters() {
    _expectToken(TK_OPEN_PAREN);

    VarInitTree varInitTree;

    varInitTree.generateInitCode(_curFunction, nullptr);

    int index = 0;
    while (_curToken.type != TK_CLOSE_PAREN && _error == PE_OK) {
        if (index > 0)
            _expectToken(TK_COMMA);

        if (_curToken.type == TK_ELLIPSIS) {
            // ...id_name
            _readToken();
            _expectToken(TK_NAME);

            _curFuncScope->addArgumentDeclaration(_curToken, index);
            _readToken();
            if (_curToken.type != TK_CLOSE_PAREN) {
                _parseError(PE_REST_PARAM_MUST_BE_LAST_ONE, "SyntaxError: Rest parameter must be last formal parameter");
            }
            break;
        } else {
            _expectParameterDeclaration(index, &varInitTree);
        }
        index++;
    }

    _expectToken(TK_CLOSE_PAREN);
    return index;
}

void JSParser::_expectParameterDeclaration(uint16_t index, VarInitTree *parentVarInitTree) {
    auto declaredToken = _curToken;
    auto child = parentVarInitTree->newChild();

    switch (_curToken.type) {
        case TK_NAME:
            _curFuncScope->addVarDeclaration(_curToken);
            _readToken();
            break;
        case TK_OPEN_BRACKET: {
            _readToken();
            _expectArrayAssignable(TK_VAR, child);
            _expectToken(TK_CLOSE_BRACKET);
            break;
        }
        case TK_OPEN_BRACE: {
            _readToken();
            // _expectObjectAssignable(declareType, child);
            _expectToken(TK_CLOSE_BRACE);
            break;
        }
        default:
            assert(0);
            break;
    }

    if (_curToken.type == TK_ASSIGN) {
        _readToken();

        InstructionOutputStream *exprInstructions = _resPool->allocInstructionOutputStream();

        // 将表达式的指令保存起来
        _expectExpression(*exprInstructions, PRED_NONE, false);

        child->setCallback([this, child, declaredToken, index, exprInstructions](Function *function, bool initExpr) {
            auto &instructions = function->instructions;

            // 参数入栈
            instructions.writeOpCode(OP_PUSH_ID_LOCAL_ARGUMENT);
            instructions.writeUint16(index);

            instructions.writeOpCode(OP_JUMP_IF_UNDEFINED);
            auto labelSpreadParam = instructions.writeAddressFuture();

            // 参数为 undefined，使用缺省的参数
            // 插入缺省值表达式的执行代码.
            instructions.writeBranch(exprInstructions);
            if (declaredToken.type == TK_NAME) {
                _assignParameter(function, instructions, index);
            }

            instructions.writeOpCode(OP_JUMP);
            auto end = instructions.writeAddressFuture();

            instructions.tagLabel(labelSpreadParam);
            if (declaredToken.type != TK_NAME) {
                // 将参数入栈，以便展开
                instructions.writeOpCode(OP_PUSH_ID_LOCAL_ARGUMENT);
                instructions.writeUint16(index);
            } else {
                child->setNeedPopStackTop();
            }

            instructions.tagLabel(end);

            return true;
        });
    } else {
        if (declaredToken.type != TK_NAME) {
            // 参数展开
            child->setCallback([this, child, declaredToken, index](Function *function, bool initExpr) {
                auto &instructions = function->instructions;

                // 参数入栈
                instructions.writeOpCode(OP_PUSH_ID_LOCAL_ARGUMENT);
                instructions.writeUint16(index);

                instructions.writeOpCode(OP_JUMP_IF_NOT_UNDEFINED);
                auto end = instructions.writeAddressFuture();

                // 参数入栈
                instructions.writeOpCode(OP_PUSH_ID_LOCAL_ARGUMENT);
                instructions.writeUint16(index);

                instructions.tagLabel(end);

                child->setNeedPopStackTop();

                return true;
            });
        }
    }
}

enum UnaryPrefixStatus {
    UPS_NONE,
    UPS_INCREAMENT,
    UPS_DECREAMENT,
};

void writePrevInstruction(OpCode &prevOp, uint32_t &prevStringIdx,
        IdentifierRef *identifier, InstructionOutputStream &instructions) {
    if (prevOp == OP_PUSH_IDENTIFIER) {
        instructions.writePushIdentifier(identifier);
    } else if (prevOp == OP_PUSH_MEMBER_INDEX) {
        instructions.writeOpCode(OP_PUSH_MEMBER_INDEX);
    } else if (prevOp == OP_PUSH_MEMBER_DOT || prevOp == OP_PUSH_MEMBER_DOT_OPTIONAL) {
        instructions.writeOpCode(prevOp);
        instructions.writeUint32(prevStringIdx);
    } else {
        assert(prevOp == OP_INVALID);
    }
}

OpCode writePrevInstructionFunctionCall(OpCode &prevOp, uint32_t &prevStringIdx,
        IdentifierRef *identifier, InstructionOutputStream &instructions) {
    if (prevOp == OP_PUSH_MEMBER_INDEX) {
        instructions.writeOpCode(OP_PUSH_THIS_MEMBER_INDEX);
    } else if (prevOp == OP_PUSH_MEMBER_DOT) {
        instructions.writeOpCode(OP_PUSH_THIS_MEMBER_DOT);
        instructions.writeUint32(prevStringIdx);
    } else if (prevOp == OP_PUSH_MEMBER_DOT_OPTIONAL) {
        instructions.writeOpCode(OP_PUSH_THIS_MEMBER_DOT_OPTIONAL);
        instructions.writeUint32(prevStringIdx);
    } else {
        assert(prevOp == OP_PUSH_IDENTIFIER);
        instructions.writePushIdentifier(identifier);
        return OP_FUNCTION_CALL;
    }

    return OP_MEMBER_FUNCTION_CALL;
}

void JSParser::_expectExpression(InstructionOutputStream &instructions, Precedence pred, bool enableComma, bool enableIn) {
    OpCode prevOp = OP_INVALID;
    uint32_t prevStringIdx = 0;
    UnaryPrefixStatus upsStatus = UPS_NONE;
    IdentifierRef *identifier = nullptr;

    if (_curToken.type == TK_UNARY_PREFIX) {
        // !, ~
        if (_curToken.buf[0] == '+') {
            upsStatus = UPS_INCREAMENT;
        } else {
            upsStatus = UPS_DECREAMENT;
        }
        _readToken();
    }

    switch (_curToken.type) {
        case TK_NAME: {
            auto name = _curToken;
            _readToken();
            if (_curToken.type == TK_ARROW) {
                // Arrow function
                _enterFunction(name);

                _curFuncScope->addArgumentDeclaration(name, 0);

                if (_curToken.type == TK_OPEN_BRACE) {
                    _expectBlock();
                } else {
                    _expectExpression(instructions, PRED_NONE, false);
                    instructions.writeOpCode(OP_RETURN_VALUE);
                }

                instructions.writeOpCode(OP_PUSH_FUNCTION_EXPR);
                instructions.writeUint8(_curFunction->scope->parent->function->scope->depth);
                instructions.writeUint16((uint16_t)_curFunction->index);

                _leaveFunction();
            } else {
                identifier = _newIdentifierRef(name, _curFunction);
                prevOp = OP_PUSH_IDENTIFIER;
            }
            break;
        }
        case TK_TEMPLATE_NO_SUBSTITUTION:
        case TK_STRING:
            instructions.writeOpCode(OP_PUSH_STRING);
            instructions.writeUint32(_getStringIndex(_curToken));
            _readToken();
            break;
        case TK_NUMBER: {
            int32_t n = (int32_t)_curToken.number;
            if (_curToken.number == n) {
                instructions.writeOpCode(OP_PUSH_INT32);
                instructions.writeUint32(_curToken.number);
            } else {
                instructions.writeOpCode(OP_PUSH_DOUBLE);
                instructions.writeUint32(_getDoubleIndex(_curToken.number));
            }
            _readToken();
            break;
        }
        case TK_OPEN_BRACE:
            _expectObjectLiteralExpression(instructions);
            break;
        case TK_OPEN_BRACKET:
            // arrayLiteralExpression(stream);
            break;
        case TK_OPEN_PAREN:
            if (_isArrowFunction()) {
                _expectArrowFunction(instructions);
            } else {
                _expectParenExpression(instructions);
            }
            break;
        case TK_FUNCTION: {
            _expectFunction(instructions, FT_EXPRESSION);
            break;
        }
        case TK_CLASS:
            // classDeclaration(stream, true);
            break;
        case TK_NEW: {
            _readToken();
            if (_curToken.type == TK_DOT) {
                _readToken();
                if (_curToken.type == TK_NAME && tokenToSizedString(_curToken).equal("target")) {
                    instructions.writeOpCode(OP_NEW_TARGET);
                } else {
                    _parseError(PE_UNEXPECTED_TOKEN, "Unexpected token.");
                }
                break;
            }

            _expectExpression(instructions, PRED_LEFT_HAND_EXPR, false);

            int countArgs = 0;
            if (_curToken.type == TK_OPEN_PAREN) {
                countArgs = _expectArgumentsList(instructions);
            }

            instructions.writeOpCode(OP_NEW);
            instructions.writeUint16((uint16_t)countArgs);
            break;
        }
        case TK_UNARY_PREFIX: {
            // !, ~
            auto opcode = _curToken.opr;
            _readToken();
            _expectExpression(instructions, PRED_UNARY_PREFIX, true);

            instructions.writeUint8(opcode);
            break;
        }
        case TK_POSTFIX:
        case TK_ADD: {
            _readToken();
            _expectExpression(instructions, PRED_UNARY_PREFIX, true);
            instructions.writeOpCode(OP_INCREMENT_ID_POST);
            break;
        }
        case TK_TEMPLATE_HEAD:
            // templateLiteral(stream);
            break;
        default:
            _parseError(PE_SYNTAX_ERROR, "Unexpected token: %.*s", _curToken.len, _curToken.buf);
            break;
    }

    bool tryNext = true;
    while (tryNext && _error == PE_OK) {
        switch (_curToken.type) {
            case TK_OPEN_BRACKET:
                // Member index expression
                _readToken();
                _expectExpression(instructions, PRED_NONE, true);
                _expectToken(TK_CLOSE_BRACKET);

                writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);
                prevOp = OP_PUSH_MEMBER_INDEX;
                break;

            case TK_OPTIONAL_DOT: {
               // Optional member dot expression
               _readToken();
               if (_curToken.type == TK_NAME || isKeyword(_curToken.type)) {
                   int stringIdx = _getStringIndex(_curToken);

                   writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);
                   prevOp = OP_PUSH_MEMBER_DOT_OPTIONAL;
                   prevStringIdx = stringIdx;
               } else {
                   _expectToken(TK_NAME);
               }
               break;
           }
            case TK_DOT: {
                // Member dot expression
                _readToken();
                if (_curToken.type == TK_NAME || isKeyword(_curToken.type)) {
                    int stringIdx = _getStringIndex(_curToken);
                    _readToken();

                    writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);
                    prevOp = OP_PUSH_MEMBER_DOT;
                    prevStringIdx = stringIdx;
                } else {
                    _expectToken(TK_NAME);
                }
                break;
            }
            case TK_OPEN_PAREN: {
                // Arguments expression
                if (pred >= PRED_POSTFIX) {
                    writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);
                    return;
                }

                if (prevOp == OP_PUSH_IDENTIFIER) {
                    // 函数调用
                    identifier->isUsedNotAsFunctionCall = false;
                    auto push = instructions.writeFunctionCallPush(identifier);
                    int countArgs = _expectArgumentsList(instructions);
                    instructions.writeFunctionCall(push);
                    instructions.writeUint16((uint16_t)countArgs);
                } else {
                    auto op = writePrevInstructionFunctionCall(prevOp, prevStringIdx, identifier, instructions);

                    int countArgs = _expectArgumentsList(instructions);

                    instructions.writeOpCode(op);
                    instructions.writeUint16((uint16_t)countArgs);
                }

                prevOp = OP_INVALID;
                break;
            }
            case TK_TEMPLATE_NO_SUBSTITUTION: {
                if (pred >= PRED_POSTFIX) {
                    writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);
                    return;
                }

                auto op = writePrevInstructionFunctionCall(prevOp, prevStringIdx, identifier, instructions);

                instructions.writeOpCode(OP_PREPARE_RAW_STRING_TEMPLATE_CALL);
                VecInts indices;
                indices.push_back(_getStringIndex(_curToken));
                indices.push_back(_getStringIndex(_escapeString(tokenToSizedString(_curToken))));
                instructions.writeUint16(_getRawStringsIndex(indices));
                instructions.writeUint16(0); // Count of expr values.

                instructions.writeOpCode(op);

                prevOp = OP_INVALID;
                break;
            }
            case TK_TEMPLATE_HEAD: {
                if (pred >= PRED_POSTFIX) {
                    writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);
                    return;
                }

                auto op = writePrevInstructionFunctionCall(prevOp, prevStringIdx, identifier, instructions);

                VecInts indices;
                int countValues = _expectRawTemplateCall(instructions, indices);

                instructions.writeOpCode(OP_PREPARE_RAW_STRING_TEMPLATE_CALL);
                instructions.writeUint16(_getRawStringsIndex(indices));
                instructions.writeUint16(countValues); // Count of expr values.

                instructions.writeOpCode(op);

                prevOp = OP_INVALID;
                break;
            }
            default:
                tryNext = false;
                break;
        }
    }

    if (_curToken.type == TK_POSTFIX) {
        // Postfix expression
        if (pred >= PRED_POSTFIX) {
            writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);
            return;
        }

        if (prevOp == OP_PUSH_IDENTIFIER) {
            if (_curToken.buf[0] == '+') {
                instructions.writeOpCode(OP_INCREMENT_ID_POST);
            } else {
                instructions.writeOpCode(OP_DECREMENT_ID_POST);
            }
            assert(identifier);
            instructions.writeIdentifierAddress(identifier);
        } else if (prevOp == OP_PUSH_MEMBER_INDEX) {
            if (_curToken.buf[0] == '+') {
                instructions.writeOpCode(OP_INCREMENT_MEMBER_INDEX_POST);
            } else {
                instructions.writeOpCode(OP_DECREMENT_MEMBER_INDEX_POST);
            }
        } else if (prevOp == OP_PUSH_MEMBER_DOT || prevOp == OP_PUSH_MEMBER_DOT_OPTIONAL) {
            if (_curToken.buf[0] == '+') {
                instructions.writeOpCode(OP_INCREMENT_MEMBER_DOT_POST);
            } else {
                instructions.writeOpCode(OP_DECREMENT_MEMBER_DOT_POST);
            }
            instructions.writeUint32(prevStringIdx);
        } else {
            assert(prevOp == OP_INVALID);
        }

        _readToken();
    }

    if (upsStatus != UPS_NONE) {
        // Now handle prefix expression
        if (prevOp == OP_INVALID) {
            _parseError(PE_INVALID_LEFT_HAND_EXPR, "Invalid left-hand side expression in prefix operation");
            return;
        }

        if (prevOp == OP_PUSH_IDENTIFIER) {
            if (upsStatus == UPS_INCREAMENT) {
                instructions.writeOpCode(OP_INCREMENT_ID_PRE);
            } else {
                instructions.writeOpCode(OP_DECREMENT_ID_PRE);
            }
            assert(identifier);
            instructions.writeIdentifierAddress(identifier);
        } else if (prevOp == OP_PUSH_MEMBER_INDEX) {
            if (upsStatus == UPS_INCREAMENT) {
                instructions.writeOpCode(OP_INCREMENT_MEMBER_INDEX_PRE);
            } else {
                instructions.writeOpCode(OP_DECREMENT_MEMBER_INDEX_PRE);
            }
        } else if (prevOp == OP_PUSH_MEMBER_DOT || prevOp == OP_PUSH_MEMBER_DOT_OPTIONAL) {
            if (upsStatus == UPS_INCREAMENT) {
                instructions.writeOpCode(OP_INCREMENT_MEMBER_DOT_PRE);
            } else {
                instructions.writeOpCode(OP_DECREMENT_MEMBER_DOT_PRE);
            }
            instructions.writeUint32(prevStringIdx);
        } else {
            assert(prevOp == OP_INVALID);
        }
    }

    switch (_curToken.type) {
        case TK_ASSIGN: {
            if (pred > PRED_ASSIGNMENT) {
                writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);
                return;
            }

            if (prevOp == OP_INVALID || upsStatus != UPS_NONE) {
                _parseError(PE_INVALID_LEFT_HAND_EXPR, "Invalid left-hand side in assignment");
                return;
            }

            _readToken();
            _expectExpression(instructions, PRED_ASSIGNMENT, false);

            if (prevOp == OP_PUSH_IDENTIFIER) {
                instructions.writeOpCode(OP_ASSIGN_IDENTIFIER);
                assert(identifier);
                instructions.writeIdentifierAddress(identifier);
            } else if (prevOp == OP_PUSH_MEMBER_INDEX) {
                instructions.writeOpCode(OP_ASSIGN_MEMBER_INDEX);
            } else if (prevOp == OP_PUSH_MEMBER_DOT || prevOp == OP_PUSH_MEMBER_DOT_OPTIONAL) {
                instructions.writeOpCode(OP_ASSIGN_MEMBER_DOT);
                instructions.writeUint32(prevStringIdx);
            } else {
                assert(prevOp == OP_INVALID);
            }
            break;
        }
        case TK_ASSIGN_X: {
            if (pred > PRED_ASSIGNMENT) {
                writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);
                return;
            }

            if (prevOp == OP_INVALID || upsStatus != UPS_NONE) {
                _parseError(PE_INVALID_LEFT_HAND_EXPR, "Invalid left-hand side in assignment");
                return;
            }

            auto opr = _curToken.opr;

            // 将 left += right 转换为 left = left + right
            // push left
            writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);

            _readToken();
            _expectExpression(instructions, PRED_ASSIGNMENT, false);

            // binary operator
            instructions.writeUint8(opr);

            // assign
            if (prevOp == OP_PUSH_IDENTIFIER) {
                instructions.writeOpCode(OP_ASSIGN_IDENTIFIER);
                assert(identifier);
                instructions.writeIdentifierAddress(identifier);
            } else if (prevOp == OP_PUSH_MEMBER_INDEX) {
                instructions.writeOpCode(OP_ASSIGN_MEMBER_INDEX);
            } else if (prevOp == OP_PUSH_MEMBER_DOT || prevOp == OP_PUSH_MEMBER_DOT_OPTIONAL) {
                instructions.writeOpCode(OP_ASSIGN_MEMBER_DOT);
            } else {
                assert(prevOp == OP_INVALID);
            }
            break;
        }
        default:
            writePrevInstruction(prevOp, prevStringIdx, identifier, instructions);
            break;
    }

    tryNext = true;
    while (tryNext && _error == PE_OK) {
        switch (_curToken.type) {
            case TK_CONDITIONAL: {
                if (pred >= PRED_ADD)
                    return;
                _expectExpression(instructions, PRED_NONE, false);
                _expectToken(TK_COLON);
                _expectExpression(instructions, PRED_NONE, false);
                instructions.writeOpCode(OP_CONDITIONAL);
                break;
            }
            case TK_NULLISH: {
                if (pred >= PRED_NULLISH)
                    return;
                _expectExpression(instructions, PRED_NULLISH, false);
                instructions.writeOpCode(OP_NULLISH);
                break;
            }
            case TK_LOGICAL_OR: {
                if (pred >= PRED_LOGICAL_OR)
                    return;
                _expectExpression(instructions, PRED_LOGICAL_OR, false);
                instructions.writeOpCode(OP_LOGICAL_OR);
                break;
            }
            case TK_LOGICAL_AND: {
                if (pred >= PRED_LOGICAL_AND)
                    return;
                _expectExpression(instructions, PRED_LOGICAL_AND, false);
                instructions.writeOpCode(OP_LOGICAL_AND);
                break;
            }
            case TK_BIT_OR: {
                if (pred >= PRED_BIT_OR)
                    return;
                _expectExpression(instructions, PRED_BIT_OR, false);
                instructions.writeOpCode(OP_BIT_OR);
                break;
            }
            case TK_BIT_XOR: {
                if (pred >= PRED_BIT_XOR)
                    return;
                _expectExpression(instructions, PRED_BIT_XOR, false);
                instructions.writeOpCode(OP_BIT_XOR);
                break;
            }
            case TK_BIT_AND: {
                if (pred >= PRED_BIT_AND)
                    return;
                _expectExpression(instructions, PRED_BIT_AND, false);
                instructions.writeOpCode(OP_BIT_AND);
                break;
            }
            case TK_EQUALITY: {
                if (pred >= PRED_EQUALITY)
                    return;
                auto opcode = _curToken.opr;
                _expectExpression(instructions, PRED_EQUALITY, false);
                instructions.writeUint8(opcode);
                break;
            }
            case TK_RATIONAL: {
                if (pred >= PRED_RATIONAL)
                    return;
                auto opcode = _curToken.opr;
                _expectExpression(instructions, PRED_RATIONAL, false);
                instructions.writeUint8(opcode);
                break;
            }
            case TK_IN:
                if (pred >= PRED_RATIONAL || !enableIn)
                    return;
                _expectExpression(instructions, PRED_RATIONAL, false);
                instructions.writeOpCode(OP_IN);
                break;

            case TK_INSTANCEOF:
                if (pred >= PRED_RATIONAL || !enableIn)
                    return;
                _expectExpression(instructions, PRED_RATIONAL, false);
                instructions.writeOpCode(OP_INSTANCE_OF);
                break;

            case TK_SHIFT: {
                if (pred >= PRED_SHIFT)
                    return;
                auto opcode = _curToken.opr;
                _expectExpression(instructions, PRED_SHIFT, false);
                instructions.writeUint8(opcode);
                break;
            }
            case TK_ADD: {
                if (pred >= PRED_ADD)
                    return;
                auto opcode = _curToken.opr;
                _readToken();
                _expectExpression(instructions, PRED_ADD, false);
                instructions.writeUint8(opcode);
                break;
            }
            case TK_MUL: {
                if (pred >= PRED_MUL)
                    return;
                auto opcode = _curToken.opr;
                _expectExpression(instructions, PRED_MUL, false);
                instructions.writeUint8(opcode);
                break;
            }
            case TK_EXP: {
                if (pred >= PRED_EXP)
                    return;
                _expectExpression(instructions, PRED_EXP, false);
                instructions.writeOpCode(OP_EXP);
                break;
            }
                //                    TK_UNARY,


            default:
                tryNext = false;
                break;
        }
    }

    while (enableComma && _curToken.type == TK_COMMA && _error == PE_OK) {
        _readToken();
        _expectExpression(instructions, PRED_NONE, false);
    }
}

void JSParser::_expectParenCondition() {
}

int JSParser::_expectRawTemplateCall(InstructionOutputStream &instructions, VecInts &indices) {
    indices.push_back(_getStringIndex(_curToken));
    indices.push_back(_getStringIndex(_escapeString(tokenToSizedString(_curToken))));

    int count = 0;
    while (_error == PE_OK) {
        _expectExpression(instructions, PRED_NONE, true);
        count++;

        if (_curToken.type != TK_CLOSE_BRACE) {
            _expectToken(TK_CLOSE_BRACE);
        }

        _readInTemplateMid();

        indices.push_back(_getStringIndex(_curToken));
        indices.push_back(_getStringIndex(_escapeString(tokenToSizedString(_curToken))));

        if (_curToken.type == TK_TEMPLATE_TAIL) {
            _readToken();
            break;
        }

        _expectToken(TK_TEMPLATE_MIDDLE);
    }

    return count;
}

void JSParser::_expectArrowFunction(InstructionOutputStream &instructions) {

}

void JSParser::_expectParenExpression(InstructionOutputStream &instructions) {

}

void JSParser::_expectObjectLiteralExpression(InstructionOutputStream &instructions) {
    _expectToken(TK_OPEN_BRACE);
    bool first = true;

    instructions.writeOpCode(OP_OBJ_CREATE);
    while (_error == PE_OK && _curToken.type != TK_CLOSE_BRACE) {
        if (first) {
            first = false;
        } else {
            _expectToken(TK_COMMA);
        }
        if (_curToken.type == TK_CLOSE_BRACE) {
            break;
        }

        auto type = _curToken.type;
        auto name = _curToken;
        _peekToken();

        if (_curToken == "async" && _nextToken.type == TK_NAME) {
            // async function
            _readToken();
            auto nameIdx = _getStringIndex(_curToken);
            auto child = _expectFunction(instructions, FT_MEMBER_FUNCTION);
            child->isAsync = true;
            instructions.writeOpCode(OP_OBJ_SET_PROPERTY);
            instructions.writeUint32(nameIdx);
        } else if (type == TK_NAME && (_nextToken.type != TK_COLON && _nextToken.type != TK_OPEN_PAREN) && (name == "get" || name == "set")) {
            // getter, setter
            if (!canTokenBeMemberName(_nextToken.type)) {
                _parseError(PE_SYNTAX_ERROR, "Unexpected token: %.*s", _curToken.len, _curToken.buf);
                return;
            }
            auto nameIdx = _getStringIndex(_nextToken);
            _readToken();
            if (name == "get") {
                _expectFunction(instructions, FT_GETTER);
                instructions.writeOpCode(OP_OBJ_SET_GETTER);
            } else {
                _expectFunction(instructions, FT_SETTER);
                instructions.writeOpCode(OP_OBJ_SET_SETTER);
            }
            instructions.writeUint32(nameIdx);
        } else if (type == TK_NAME && (_nextToken.type == TK_COMMA || _nextToken.type == TK_CLOSE_BRACE)) {
            // name,
            auto id = _newIdentifierRef(_curToken, _curFunction);
            instructions.writePushIdentifier(id);
            instructions.writeOpCode(OP_OBJ_SET_PROPERTY);
            instructions.writeUint32(_getStringIndex(id->name));
        } else if (type == TK_ELLIPSIS) {
            // ...expr
            _readToken();
            _expectExpression(instructions, PRED_NONE);
            instructions.writeOpCode(OP_OBJ_SPREAD_PROPERTY);
        } else if (type == TK_MUL) {
            // * generator function
            Function *child = nullptr;
            _readToken();
            if (_curToken.type == TK_OPEN_BRACKET) {
                // [ expr ]
                _readToken();
                _expectExpression(instructions, PRED_NONE, true);
                _expectToken(TK_CLOSE_BRACKET);
                child = _expectFunction(instructions, FT_MEMBER_FUNCTION);
                instructions.writeOpCode(OP_OBJ_SET_COMPUTED_PROPERTY);
            } else {
                auto nameIdx = _getStringIndex(_curToken);
                child = _expectFunction(instructions, FT_MEMBER_FUNCTION);
                instructions.writeOpCode(OP_OBJ_SET_PROPERTY);
                instructions.writeUint32(nameIdx);
            }
            child->isGenerator = true;
        } else {
            int nameIdx = -1;
            if (type == TK_OPEN_BRACKET) {
                // [ expr ]
                _readToken();
                _expectExpression(instructions, PRED_NONE, true);
                _expectToken(TK_CLOSE_BRACKET);
            } else {
                if (!canTokenBeMemberName(type)) {
                    _parseError(PE_SYNTAX_ERROR, "Unexpected token: %.*s", _curToken.len, _curToken.buf);
                    return;
                }
                nameIdx = _getStringIndex(_curToken);
                _readToken();
            }

            if (_curToken.type == TK_OPEN_PAREN) {
                // 函数
                _expectFunction(instructions, FT_MEMBER_FUNCTION, false);
            } else {
                _expectToken(TK_COLON);
                _expectExpression(instructions, PRED_NONE);
            }

            if (nameIdx == -1) {
                instructions.writeOpCode(OP_OBJ_SET_COMPUTED_PROPERTY);
            } else {
                instructions.writeOpCode(OP_OBJ_SET_PROPERTY);
                instructions.writeUint32(nameIdx);
            }
        }
    }

    _expectToken(TK_CLOSE_BRACE);
}

IdentifierRef *JSParser::_newIdentifierRef(const Token &token, Function *function) {
    auto ret = PoolNew(_resPool->pool, IdentifierRef)(token, _curScope);
    ret->next = _headIdRefs;
    ret->isUsedNotAsFunctionCall = true;
    _headIdRefs = ret;
    return ret;
}

/**
 * 预测接下来括号中的内容，以及之后的是否为 Arrow function
 */
bool JSParser::_isArrowFunction() {
    auto bk_bufPos = _bufPos;
    auto bk_line = _line, bk_col = _col;
    auto bk_newLineBefore = _newLineBefore;
    auto bk_prevTokenType = _prevTokenType;
    auto bk_curToken = _curToken;

    bool isArrowFunction = true;

    _readToken();
    bool first = true;
    while (_curToken.type != TK_CLOSE_PAREN && _error == PE_OK) {
        if (first)
            first = false;
        else
            _expectToken(TK_COMMA);

        if (_curToken.type == TK_NAME || canKeywordBeVarName(_curToken.type)) {
            _readToken();

            if (_curToken.type == TK_ASSIGN) {
                // 忽略 '=' 后的内容.
                _readToken();
                _ignoreExpression();
            }
        } else {
            isArrowFunction = false;
            break;
        }
    }

    if (isArrowFunction) {
        _readToken();
        isArrowFunction = _curToken.type == TK_ARROW;
    }

    _bufPos = bk_bufPos;
    _line = bk_line;
    _col = bk_col;
    _newLineBefore = bk_newLineBefore;
    _prevTokenType = bk_prevTokenType;
    _curToken = bk_curToken;

    return isArrowFunction;
}

void JSParser::_ignoreExpression() {
    int matches = 0;
    while (_curToken.type != TK_COMMA && matches == 0 && _error == PE_OK) {
        _readToken();
        if (_curToken.type == TK_OPEN_PAREN || _curToken.type == TK_OPEN_BRACKET ||
            _curToken.type == TK_OPEN_BRACE || _curToken.type == TK_TEMPLATE_HEAD || _curToken.type == TK_TEMPLATE_MIDDLE) {
            matches++;
        } else if (_curToken.type == TK_CLOSE_PAREN || _curToken.type == TK_CLOSE_BRACE || _curToken.type == TK_CLOSE_BRACKET) {
            matches--;
        }
    }
}

void _reduceScopeLevels(Scope *scope, Scope *validParent) {
    scope->parent = validParent;

    if (scope->sibling) {
        _reduceScopeLevels(scope->sibling, validParent);
    }

    if (!scope->varDeclares.empty()) {
        // 重新添加到 parent
        scope->sibling = validParent->child;
        validParent->child = scope;
    }

    if (scope->child) {
        if (!scope->varDeclares.empty()) {
            validParent = scope;
        }

        auto child = scope->child;
        scope->child = nullptr;
        _reduceScopeLevels(child, validParent);
    }
}

/**
 * 精简 scope 的层次(去掉没有变量声明的 scope)，加快访问速度
 */
void JSParser::_reduceScopeLevels(Function *function) {
    if (function->scope->child) {
        auto child = function->scope->child;
        function->scope->child = nullptr;
        ::_reduceScopeLevels(child, function->scope);
    }

    for (auto f : function->functions) {
        _reduceScopeLevels(f);
    }
}

void JSParser::_buildIdentifierRefs() {
    for (auto p = _headIdRefs; p; p = p->next) {
        p->scope->addVarReference(p);
    }
}

void JSParser::_allocateIdentifierStorage(Scope *scope, int registerIndex) {
    if (scope->sibling) {
        _allocateIdentifierStorage(scope->sibling, registerIndex);
    }

    auto functionScope = scope->function->scope;

    for (auto &item : scope->varDeclares) {
        auto declare = item.second;
        if (declare->varStorageType != VST_NOT_SET) {
            if (declare->isReferred) {
                if (declare->name.equal(SS_THIS)) {
                    declare->scope->isThisUsed = true;
                } else if (declare->name.equal(SS_ARGUMENTS)) {
                    declare->scope->isArgumentsUsed = true;
                }
            }
            continue;
        }

        if (declare->isFuncName) {
            if (declare->isUsedNotAsFunctionCall && declare->varStorageType != VST_FUNCTION_VAR) {
                // 函数被引用到了（如果仅仅是函数调用，isUsedNotAsFunctionCall 不会为 true），需要分配变量地址
                declare->varStorageType = VST_FUNCTION_VAR;
                declare->storageIndex = functionScope->countLocalVars++;
            }
        } else {
            // 变量
            if (true || declare->isReferredByChild || scope->hasEval || scope->hasWith) {
                // 被子函数引用 或者 有 eval, with，则必须将变量保存到 vars 中
                if (scope->parent == nullptr) {
                    declare->varStorageType = VST_GLOBAL_VAR;
                } else {
                    declare->varStorageType = VST_SCOPE_VAR;
                    declare->storageIndex = scope->countLocalVars++;
                }
            } else {
                // 可直接存储在寄存器中
                declare->varStorageType = VST_REGISTER;
                declare->storageIndex = registerIndex++;
            }
        }
    }

    if (scope->isAllocateFunctionVar()) {
        // 需要给函数分配变量地址
        for (auto f : scope->functionDecls) {
            if (f->declare->varStorageType != VST_FUNCTION_VAR) {
                f->declare->varStorageType = VST_FUNCTION_VAR;
                f->declare->storageIndex = functionScope->countLocalVars++;
            }
        }
    } else {
        // 仅仅保留有地址的函数，用于执行时的初始化
        VecFunctions funcVars;
        for (auto f : scope->functionDecls) {
            if (f->declare->varStorageType == VST_FUNCTION_VAR) {
                funcVars.push_back(f);
            }
        }
        scope->functionDecls = funcVars;
    }

    // 为子 scope 分配
    if (scope->child) {
        _allocateIdentifierStorage(scope->child, registerIndex);
    }
}

int JSParser::_getDoubleIndex(double value) {
    if (_runtimeCommon) {
        auto idx = _runtimeCommon->findDoubleValue(value);
        if (idx != -1) {
            return idx;
        }
    }

    int idx;
    auto it = _doubles.find(value);
    if (it == _doubles.end()) {
        idx = _nextDoubleIdx++;
        _doubles[value] = idx;

        _resPool->doubles.push_back(value);
    } else {
        idx = (*it).second;
    }

    return idx;
}

int JSParser::_getStringIndex(const SizedString &str) {
    if (_runtimeCommon) {
        auto idx = _runtimeCommon->findStringValue(str);
        if (idx != -1) {
            return idx;
        }
    }

    int idx;
    auto it = _strings.find(str);
    if (it == _strings.end()) {
        idx = _nextStringIdx++;
        _strings[str] = idx;

        SizedString value = str;
        value.unused = 0;
        _resPool->strings.push_back(value);
    } else {
        idx = (*it).second;
    }

    return idx;
}

int JSParser::_getRawStringsIndex(const VecInts &indices) {
    _v2Ints.push_back(indices);
    return (int)_v2Ints.size() - 1;
}

Function *JSParser::_enterFunction(const Token &tokenStart, bool isArrowFunction) {
    auto child = PoolNew(_resPool->pool, Function)(_resPool, _curScope, (int16_t)_curFunction->functions.size());

    child->line = tokenStart.line;
    child->col = tokenStart.col;
    child->srcCode.data = tokenStart.buf;

    if (!isArrowFunction) {
        // Arrow Function 的 this, arguments 使用的是父函数的.
        // 添加 this, arguments 的声明
        Token name = tokenStart;
        name.buf = (uint8_t *)SS_THIS.data; name.len = SS_THIS.len;
        child->scope->addVarDeclaration(name, false, true);

        name.buf = (uint8_t *)SS_ARGUMENTS.data; name.len = SS_ARGUMENTS.len;
        child->scope->addVarDeclaration(name, false, true);
    }

    _curScope->functions.push_back(child);
    _curFunction->functions.push_back(child);
    assert(_curFunction->functions.size() <= 0xFFFF);

    _stackFunctions.push_back(_curFunction);
    _curFunction = child;

    _stackScopes.push_back(_curScope);
    _stackScopes.push_back(_curFuncScope);
    _curScope = _curFuncScope = _curFunction->scope;

    return child;
}

void JSParser::_leaveFunction() {
    _curFunction->srcCode.len = uint32_t(_curToken.buf - _curFunction->srcCode.data);

    _curFunction->instructions.finish();

    _curFunction = _stackFunctions.back(); _stackFunctions.pop_back();

    _curFuncScope = _stackScopes.back(); _stackScopes.pop_back();
    _curScope = _stackScopes.back(); _stackScopes.pop_back();
}

void JSParser::_enterScope() {
    _stackScopes.push_back(_curScope);
    _curScope = PoolNew(_resPool->pool, Scope)(_curFunction, _curScope);
}

void JSParser::_leaveScope() {
    if (_curScope->varDeclares.size() > 0) {
        // 释放局部变量
        // instructions.writeOpCode(OP_FREE_STACK_VARS);
        // instructions.writeUint16((uint16_t)scope->varDeclares.size());
    }

    _curScope = _stackScopes.back(); _stackScopes.pop_back();
}
